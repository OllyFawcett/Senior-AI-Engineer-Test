#pragma once
#include "DetectionTypes.h"
#include "YOLOv11ONNX.h"

class ObjectTracker
{
public:
	ObjectTracker(const std::map<DetectionTypes::DetectionType, bool>& objectsToTrack, const float maxDistanceForMatch);
	
	bool AddNewDetections(std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections);
	uint32_t GetUniqueDetectionCount(const DetectionTypes::DetectionType detectorType);
	uint32_t GetUnMatchedDetectionCount(const DetectionTypes::DetectionType detectorType);

private:
	cv::Point2f GenerateCentroid(const YOLOv11ONNX::Detection& detection);

	float m_maxDistanceForMatch;
	std::map<DetectionTypes::DetectionType, bool> m_objectsToTrack;
	std::map<DetectionTypes::DetectionType, std::vector<cv::Point2f>> m_uniqueDetectionCentroids;
	std::map<DetectionTypes::DetectionType, std::vector<cv::Point2f>> m_unMatchedDetectionCentroids;
};