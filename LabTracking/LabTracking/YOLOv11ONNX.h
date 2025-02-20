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
    YOLOv11ONNX(std::filesystem::path& modelPath);

    std::vector<float> Run(const cv::Mat& inputImage);

private:
    Ort::Env m_env;
    Ort::SessionOptions m_sessionOptions;
    Ort::Session m_session;

    std::vector<const char*> m_inputNodeNames;
    std::vector<const char*> m_outputNodeNames;
    std::vector<int64_t> m_inputNodeDims;
};