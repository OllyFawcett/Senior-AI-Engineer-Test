#pragma once
#include "YOLOv11ONNX.h"
#include <opencv2/opencv.hpp>
#include <mutex>
#include "DetectionTypes.h"

class DetectorsHandler
{
public:
	DetectorsHandler();
	bool AddDetector(const DetectionTypes::DetectorType& type, const std::string& modelPath);
	bool DetectAndCreateDisplayImage(cv::Mat& image, std::map<DetectionTypes::DetectorType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections);

private:
	bool DrawDetectionsOnImage(cv::Mat& image, std::map<DetectionTypes::DetectorType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections);
	std::map<DetectionTypes::DetectorType, std::shared_ptr<YOLOv11ONNX>> m_detectors;
	std::map<DetectionTypes::DetectorType, cv::Scalar> m_boxColours;
	std::map<DetectionTypes::DetectorType, std::pair<float, float>> m_detectorThresholds;
	std::map<DetectionTypes::DetectorType, std::map<uint32_t, std::string>> m_detectorLabels;
	std::recursive_mutex m_mutex;

};