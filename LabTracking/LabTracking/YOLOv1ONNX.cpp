#include "YOLOv11ONNX.h"

YOLOv11ONNX::YOLOv11ONNX(std::filesystem::path& modelPath)
    : m_env(ORT_LOGGING_LEVEL_WARNING, "YOLOv11ONNX")
    , m_sessionOptions()
    , m_session(m_env, modelPath.wstring().c_str(), m_sessionOptions)
{
    m_sessionOptions.SetIntraOpNumThreads(1);
    m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    Ort::AllocatorWithDefaultOptions allocator;

    size_t numInputNodes = m_session.GetInputCount();
    for (size_t i = 0; i < numInputNodes; i++)
    {
        auto inputNameAllocated = m_session.GetInputNameAllocated(i, allocator);
        std::string inputName(inputNameAllocated.get());
        m_inputNodeNames.push_back(inputName.c_str());

        Ort::TypeInfo typeInfo = m_session.GetInputTypeInfo(i);
        auto tensor_info = typeInfo.GetTensorTypeAndShapeInfo();
        m_inputNodeDims = tensor_info.GetShape();
    }

    size_t numOutputNodes = m_session.GetOutputCount();
    for (size_t i = 0; i < numOutputNodes; i++)
    {
        auto outputNameAllocated = m_session.GetOutputNameAllocated(i, allocator);
        std::string outputName(outputNameAllocated.get());
        m_outputNodeNames.push_back(outputName.c_str());
    }
}

std::vector<float> YOLOv11ONNX::Run(const cv::Mat& inputImage)
{
    cv::Mat image;
    inputImage.convertTo(image, CV_32F, 1.0 / 255.0);
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

    int height = static_cast<int>(m_inputNodeDims[2]);
    int width = static_cast<int>(m_inputNodeDims[3]);
    cv::resize(image, image, cv::Size(width, height));

    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    std::vector<float> inputTensorValues;
    for (const auto& channel : channels) {
        size_t numElements = channel.total();
        inputTensorValues.insert(inputTensorValues.end(),
            channel.ptr<float>(0),
            channel.ptr<float>(0) + numElements);
    }


    std::vector<int64_t> inputShape = { 1, image.channels(), height, width };

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        inputTensorValues.data(),
        inputTensorValues.size(),
        inputShape.data(),
        inputShape.size());

    std::vector<const char*> inputNames;
    for (const auto& name : m_inputNodeNames)
        inputNames.push_back(name);

    std::vector<const char*> outputNames;
    for (const auto& name : m_outputNodeNames)
        outputNames.push_back(name);

    auto outputTensors = m_session.Run(Ort::RunOptions{ nullptr },
        inputNames.data(),
        &inputTensor, 1,
        outputNames.data(),
        outputNames.size());

    float* output_data = outputTensors.front().GetTensorMutableData<float>();
    size_t output_size = outputTensors.front().GetTensorTypeAndShapeInfo().GetElementCount();
    return std::vector<float>(output_data, output_data + output_size);
}
