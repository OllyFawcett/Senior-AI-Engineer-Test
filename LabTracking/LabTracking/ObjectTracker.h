#pragma once
#include "DetectionTypes.h"
#include "YOLOv11ONNX.h"

class ObjectTracker
{
public:
	//Only relevent to petri dishes but other state could be added for different objects
	enum class ObjectState
	{
		FILLED,
		UN_FILLED,
		UNKNOWN
	};
	struct Object
	{
		cv::Point2f centroid;
		cv::Scalar colour;
		ObjectState state;
	};
	ObjectTracker(const std::map<DetectionTypes::DetectionType, bool>& objectsToTrack, const float maxDistanceForMatch);
	
	bool AddNewDetections(const cv::Mat& image, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections);
	uint32_t GetUniqueDetectionCount(const DetectionTypes::DetectionType detectorType);
	uint32_t GetUnMatchedDetectionCount(const DetectionTypes::DetectionType detectorType);

private:
	Object GenerateObject(const cv::Mat& image, const DetectionTypes::DetectionType& detectionType, const YOLOv11ONNX::Detection& detection);

	float m_maxDistanceForMatch;
	std::map<DetectionTypes::DetectionType, bool> m_objectsToTrack;
	std::map<DetectionTypes::DetectionType, std::vector<Object>> m_uniqueDetectionCentroidsAndColour;
	std::map<DetectionTypes::DetectionType, std::vector<Object>> m_unmatchedDetectionCentroidsAndColour;
};