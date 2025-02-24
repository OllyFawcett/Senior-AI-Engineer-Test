#pragma once
#include "DetectionTypes.h"
#include "YOLOv11ONNX.h"

class ObjectTracker
{
public:
	ObjectTracker(const std::map<DetectionTypes::DetectorType, bool>& objectsToTrack, const float maxDistanceForMatch);
	
	bool AddNewDetections(std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections);
	uint32_t GetUniqueDetectionCount(const DetectionTypes::DetectorType detectorType);
	uint32_t GetUnMatchedDetectionCount(const DetectionTypes::DetectorType detectorType);

private:
	cv::Point2f GenerateCentroid(const YOLOv11ONNX::Detection& detection);

	float m_maxDistanceForMatch;
	std::map<DetectionTypes::DetectorType, bool> m_objectsToTrack;
	std::map<DetectionTypes::DetectorType, std::vector<cv::Point2f>> m_uniqueDetectionCentroids;
	std::map<DetectionTypes::DetectorType, std::vector<cv::Point2f>> m_unMatchedDetectionCentroids;
};