#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <filesystem>

class YOLOv11ONNX 
{
public:
    struct BoundingBox 
    {
        int x;
        int y;
        int width;
        int height;
    };
    struct Detection 
    {
        BoundingBox box;
        float conf{};
        int classId{};
    };

    YOLOv11ONNX(const std::string& modelPath, const bool useGPU);

    std::vector<Detection> Detect(const cv::Mat& image, float confThreshold, float iouThreshold);

private:
    bool Preprocess(const cv::Mat& image, float*& blob, std::vector<int64_t>& inputTensorShape, cv::Mat& resizedImage);
    bool Postprocess(const cv::Size& originalImageSize, const cv::Size& resizedImageShape,
                    const std::vector<Ort::Value>& outputTensors, float confThreshold, float iouThreshold, 
                    std::vector<Detection>& detections);
    bool FitWithinSize(const cv::Mat& image, cv::Mat& outImage, const cv::Size& newShape);
    BoundingBox ScaleCoords(const cv::Size& imageShape, BoundingBox coords,
        const cv::Size& imageOriginalShape);
    void NMSBoxes(const std::vector<BoundingBox>& boundingBoxes, const std::vector<float>& scores,
                float scoreThreshold,float nmsThreshold, std::vector<uint32_t>& indices);
    Ort::Env m_env;                    
    Ort::SessionOptions m_sessionOptions;  
    Ort::Session m_session;               
    bool m_isDynamicInputShape;                 
    cv::Size m_inputImageShape;                  

    std::vector<Ort::AllocatedStringPtr> m_inputNodeNameAllocatedStrings;
    std::vector<const char*> m_inputNames;
    std::vector<Ort::AllocatedStringPtr> m_outputNodeNameAllocatedStrings;
    std::vector<const char*> m_outputNames;

    size_t m_numInputNodes;
    size_t m_numOutputNodes;       
};