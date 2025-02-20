#pragma once
#include "YOLOv11ONNX.h"
#include <opencv2/opencv.hpp>

class DetectorsHandler
{
public:
	enum class DetectorType
	{
		HANDS,
		PETRI_DISHES,
		BOTTLES
	};
	DetectorsHandler();
	bool AddDetector(const DetectorType& type, const std::string& modelPath);
	bool DetectAndCreateDisplayImage(cv::Mat& image, std::map<DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections);

private:
	bool DrawDetectionsOnImage(cv::Mat& image, std::map<DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections);
	std::map<DetectorType, std::shared_ptr<YOLOv11ONNX>> m_detectors;
	std::map<DetectorType, cv::Scalar> m_boxColours;
};