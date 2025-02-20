#include "YOLOv11ONNX.h"
#include <numeric>

YOLOv11ONNX::YOLOv11ONNX(const std::string& modelPath, const bool useGPU)
    : m_env(nullptr)
    , m_session(nullptr)
{
    m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "ONNX_DETECTION");
    m_sessionOptions = Ort::SessionOptions();

    m_sessionOptions.SetIntraOpNumThreads(std::min(6, static_cast<int>(std::thread::hardware_concurrency())));
    m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    std::vector<std::string> availableProviders = Ort::GetAvailableProviders();
    auto cudaAvailable = std::find(availableProviders.begin(), availableProviders.end(), "CUDAExecutionProvider");
    OrtCUDAProviderOptions cudaOption;

    if (useGPU && cudaAvailable != availableProviders.end()) 
    {
        m_sessionOptions.AppendExecutionProvider_CUDA(cudaOption); 
    }

    std::wstring w_modelPath(modelPath.begin(), modelPath.end());
    m_session = Ort::Session(m_env, w_modelPath.c_str(), m_sessionOptions);

    Ort::AllocatorWithDefaultOptions allocator;

    Ort::TypeInfo inputTypeInfo = m_session.GetInputTypeInfo(0);
    std::vector<int64_t> inputTensorShapeVec = inputTypeInfo.GetTensorTypeAndShapeInfo().GetShape();
    m_isDynamicInputShape = (inputTensorShapeVec.size() >= 4) && (inputTensorShapeVec[2] == -1 && inputTensorShapeVec[3] == -1); 

    auto input_name = m_session.GetInputNameAllocated(0, allocator);
    m_inputNodeNameAllocatedStrings.push_back(std::move(input_name));
    m_inputNames.push_back(m_inputNodeNameAllocatedStrings.back().get());

    auto output_name = m_session.GetOutputNameAllocated(0, allocator);
    m_outputNodeNameAllocatedStrings.push_back(std::move(output_name));
    m_outputNames.push_back(m_outputNodeNameAllocatedStrings.back().get());

    if (inputTensorShapeVec.size() >= 4) 
    {
        m_inputImageShape = cv::Size(static_cast<uint32_t>(inputTensorShapeVec[3]), static_cast<uint32_t>(inputTensorShapeVec[2]));
    }
    else 
    {
        throw std::runtime_error("Invalid input tensor shape.");
    }

    m_numInputNodes = m_session.GetInputCount();
    m_numOutputNodes = m_session.GetOutputCount();
}

std::vector<YOLOv11ONNX::Detection> YOLOv11ONNX::Detect(const cv::Mat& image, float confThreshold, float iouThreshold) 
{
    std::vector<YOLOv11ONNX::Detection> detections;
    if (!image.empty())
    {
        float* blobPtr = nullptr;
        std::vector<int64_t> inputTensorShape = { 1, 3, m_inputImageShape.height, m_inputImageShape.width };

        cv::Mat preprocessedImage;
        if (Preprocess(image, blobPtr, inputTensorShape, preprocessedImage))
        {
            size_t inputTensorSize = std::accumulate(inputTensorShape.begin(), inputTensorShape.end(), 1ull, std::multiplies<size_t>());
            std::vector<float> inputTensorValues(blobPtr, blobPtr + inputTensorSize);

            delete[] blobPtr;
            static Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
                memoryInfo,
                inputTensorValues.data(),
                inputTensorSize,
                inputTensorShape.data(),
                inputTensorShape.size()
            );

            std::vector<Ort::Value> outputTensors = m_session.Run(
                Ort::RunOptions{ nullptr },
                m_inputNames.data(),
                &inputTensor,
                m_numInputNodes,
                m_outputNames.data(),
                m_numOutputNodes
            );

            cv::Size resizedImageShape(static_cast<int>(inputTensorShape[3]), static_cast<int>(inputTensorShape[2]));

            Postprocess(image.size(), resizedImageShape, outputTensors, confThreshold, iouThreshold, detections);
        }
    }
    return detections; 
}

bool YOLOv11ONNX::Preprocess(const cv::Mat& image, float*& blob, std::vector<int64_t>& inputTensorShape, cv::Mat& resizedImage)
{
    bool success = false;
    if (!image.empty())
    {
        if (FitWithinSize(image, resizedImage, m_inputImageShape))
        {
            inputTensorShape[2] = resizedImage.rows;
            inputTensorShape[3] = resizedImage.cols;

            resizedImage.convertTo(resizedImage, CV_32FC3, 1 / 255.0f);

            blob = new float[resizedImage.cols * resizedImage.rows * resizedImage.channels()];

            std::vector<cv::Mat> chw(resizedImage.channels());
            for (uint32_t i = 0; i < resizedImage.channels(); ++i)
            {
                chw[i] = cv::Mat(resizedImage.rows, resizedImage.cols, CV_32FC1, blob + i * resizedImage.cols * resizedImage.rows);
            }
            cv::split(resizedImage, chw);
            success = true;
        }
    }
    return success;
}

bool YOLOv11ONNX::FitWithinSize(const cv::Mat& image, cv::Mat& outImage, const cv::Size& newShape)
{
    bool success = false;
    if (!image.empty() and (image.rows != 0) and (image.cols != 0))
    {
        float ratio = std::min(static_cast<float>(newShape.height) / image.rows,
            static_cast<float>(newShape.width) / image.cols);

        uint32_t newUnpadW = static_cast<uint32_t>(std::round(image.cols * ratio));
        uint32_t newUnpadH = static_cast<uint32_t>(std::round(image.rows * ratio));

        uint32_t dw = newShape.width - newUnpadW;
        uint32_t dh = newShape.height - newUnpadH;

        uint32_t padLeft = dw / 2;
        uint32_t padRight = dw - padLeft;
        uint32_t padTop = dh / 2;
        uint32_t padBottom = dh - padTop;

        cv::resize(image, outImage, cv::Size(newUnpadW, newUnpadH), 0, 0, cv::INTER_LINEAR);
        cv::copyMakeBorder(outImage, outImage, padTop, padBottom, padLeft, padRight, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
        success = true;
    }
    return success;
}

bool YOLOv11ONNX::Postprocess(const cv::Size& originalImageSize, const cv::Size& resizedImageShape,
                              const std::vector<Ort::Value>& outputTensors, float confThreshold, 
                              float iouThreshold, std::vector<Detection>& detections) 
{
    bool success = false;
    if ((originalImageSize.height > 0) and (originalImageSize.width > 0) and (resizedImageShape.width > 0) and (resizedImageShape.height > 0))
    {
        const float* rawOutput = outputTensors[0].GetTensorData<float>();
        const std::vector<int64_t> outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();

        const size_t num_features = outputShape[1];
        const size_t num_detections = outputShape[2];

        if (num_detections > 0)
        {
            const uint32_t numClasses = static_cast<uint32_t>(num_features) - 4;
            if (numClasses > 0)
            {
                std::vector<BoundingBox> boxes;
                boxes.reserve(num_detections);
                std::vector<float> confs;
                confs.reserve(num_detections);
                std::vector<int> classIds;
                classIds.reserve(num_detections);
                std::vector<BoundingBox> nms_boxes;
                nms_boxes.reserve(num_detections);

                const float* ptr = rawOutput;

                for (size_t d = 0; d < num_detections; ++d)
                {
                    float centerX = ptr[0 * num_detections + d];
                    float centerY = ptr[1 * num_detections + d];
                    float width = ptr[2 * num_detections + d];
                    float height = ptr[3 * num_detections + d];

                    uint32_t classId = -1;
                    float maxScore = -FLT_MAX;
                    for (uint32_t c = 0; c < numClasses; ++c)
                    {
                        const float score = ptr[d + (4 + c) * num_detections];
                        if (score > maxScore)
                        {
                            maxScore = score;
                            classId = c;
                        }
                    }

                    if (maxScore > confThreshold)
                    {
                        float left = centerX - width / 2.0f;
                        float top = centerY - height / 2.0f;
                        BoundingBox unscaledBox = { left, top, width, height };
                        BoundingBox scaledBox = ScaleCoords(resizedImageShape, unscaledBox, originalImageSize);
                        BoundingBox roundedBox;
                        roundedBox.x = std::round(scaledBox.x);
                        roundedBox.y = std::round(scaledBox.y);
                        roundedBox.width = std::round(scaledBox.width);
                        roundedBox.height = std::round(scaledBox.height);

                        BoundingBox nmsBox = roundedBox;
                        nmsBox.x += classId * 7680;
                        nmsBox.y += classId * 7680;

                        nms_boxes.push_back(nmsBox);
                        boxes.push_back(roundedBox);
                        confs.push_back(maxScore);
                        classIds.push_back(classId);
                    }
                }

                std::vector<uint32_t> indices;
                NMSBoxes(nms_boxes, confs, confThreshold, iouThreshold, indices);

                detections.reserve(indices.size());
                for (const uint32_t idx : indices)
                {
                    detections.push_back(Detection{boxes[idx],confs[idx],classIds[idx]});
                }
                success = true;
            }
        }
    }
    return success;
}

YOLOv11ONNX::BoundingBox YOLOv11ONNX::ScaleCoords(const cv::Size& imageShape, BoundingBox coords, const cv::Size& imageOriginalShape) 
{
    BoundingBox result;
    float gain = std::min(static_cast<float>(imageShape.height) / static_cast<float>(imageOriginalShape.height),
        static_cast<float>(imageShape.width) / static_cast<float>(imageOriginalShape.width));

    int padX = static_cast<uint32_t>(std::round((imageShape.width - imageOriginalShape.width * gain) / 2.0f));
    int padY = static_cast<uint32_t>(std::round((imageShape.height - imageOriginalShape.height * gain) / 2.0f));

    result.x = static_cast<uint32_t>(std::round((coords.x - padX) / gain));
    result.y = static_cast<uint32_t>(std::round((coords.y - padY) / gain));
    result.width = static_cast<uint32_t>(std::round(coords.width / gain));
    result.height = static_cast<uint32_t>(std::round(coords.height / gain));

    return result;
}

void YOLOv11ONNX::NMSBoxes(const std::vector<BoundingBox>& boundingBoxes,
    const std::vector<float>& scores,
    float scoreThreshold,
    float nmsThreshold,
    std::vector<uint32_t>& indices)
{
    indices.clear();

    const size_t numBoxes = boundingBoxes.size();
    if (numBoxes == 0) {
        return;
    }
    std::vector<int> sortedIndices;
    sortedIndices.reserve(numBoxes);
    for (size_t i = 0; i < numBoxes; ++i) {
        if (scores[i] >= scoreThreshold) {
            sortedIndices.push_back(static_cast<int>(i));
        }
    }

    if (sortedIndices.empty()) {
        return;
    }

    std::sort(sortedIndices.begin(), sortedIndices.end(),
        [&scores](int idx1, int idx2) {
            return scores[idx1] > scores[idx2];
        });

    std::vector<float> areas(numBoxes, 0.0f);
    for (size_t i = 0; i < numBoxes; ++i) {
        areas[i] = boundingBoxes[i].width * boundingBoxes[i].height;
    }

    std::vector<bool> suppressed(numBoxes, false);

    for (size_t i = 0; i < sortedIndices.size(); ++i) {
        int currentIdx = sortedIndices[i];
        if (suppressed[currentIdx]) {
            continue;
        }

        indices.push_back(currentIdx);

        const BoundingBox& currentBox = boundingBoxes[currentIdx];
        const float x1_max = currentBox.x;
        const float y1_max = currentBox.y;
        const float x2_max = currentBox.x + currentBox.width;
        const float y2_max = currentBox.y + currentBox.height;
        const float area_current = areas[currentIdx];

        for (size_t j = i + 1; j < sortedIndices.size(); ++j) {
            int compareIdx = sortedIndices[j];
            if (suppressed[compareIdx]) {
                continue;
            }

            const BoundingBox& compareBox = boundingBoxes[compareIdx];
            const float x1 = std::max(x1_max, static_cast<float>(compareBox.x));
            const float y1 = std::max(y1_max, static_cast<float>(compareBox.y));
            const float x2 = std::min(x2_max, static_cast<float>(compareBox.x + compareBox.width));
            const float y2 = std::min(y2_max, static_cast<float>(compareBox.y + compareBox.height));

            const float interWidth = x2 - x1;
            const float interHeight = y2 - y1;

            if (interWidth <= 0 || interHeight <= 0) {
                continue;
            }

            const float intersection = interWidth * interHeight;
            const float unionArea = area_current + areas[compareIdx] - intersection;
            const float iou = (unionArea > 0.0f) ? (intersection / unionArea) : 0.0f;

            if (iou > nmsThreshold) {
                suppressed[compareIdx] = true;
            }
        }
    }

}