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
		DetectionTypes::DetectionType type;
		cv::Point2f centroid;
		cv::Scalar colour;
		ObjectState state;
	};
	struct HandInteractions
	{
		bool touchingPetriDish;
		bool touchingBottle;
	};
	ObjectTracker(const std::map<DetectionTypes::DetectionType, bool>& objectsToTrack, const float maxDistanceForMatch);
	
	std::vector<std::pair<YOLOv11ONNX::Detection, Object>> AddNewDetections(const cv::Mat& image, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections);
	uint32_t GetUniqueDetectionCount(const DetectionTypes::DetectionType detectorType);
	uint32_t GetUnMatchedDetectionCount(const DetectionTypes::DetectionType detectorType);
	HandInteractions CheckHandTouches(const std::vector<std::pair<YOLOv11ONNX::Detection, Object>>& items);

private:
	Object GenerateObject(const cv::Mat& image, const DetectionTypes::DetectionType& detectionType, const YOLOv11ONNX::Detection& detection);
	bool CheckForStateChange(const Object& previousObject, const Object& newObject, const float tolerance, ObjectState& state);
	bool BoxesOverlap(const YOLOv11ONNX::Detection& detection1, const YOLOv11ONNX::Detection& detection2);

	float m_maxDistanceForMatch;
	std::map<DetectionTypes::DetectionType, bool> m_objectsToTrack;
	std::map<DetectionTypes::DetectionType, std::vector<Object>> m_uniqueDetectionCentroidsAndColour;
	std::map<DetectionTypes::DetectionType, std::vector<Object>> m_unmatchedDetectionCentroidsAndColour;
};