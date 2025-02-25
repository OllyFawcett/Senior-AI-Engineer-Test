#include "ObjectTracker.h"

ObjectTracker::ObjectTracker(const std::map<DetectionTypes::DetectionType, bool>& objectsToTrack, const float maxDistanceForMatch)
	: m_objectsToTrack(objectsToTrack)
	, m_maxDistanceForMatch(maxDistanceForMatch)
{
}

std::vector<std::pair<YOLOv11ONNX::Detection, ObjectTracker::Object>> ObjectTracker::AddNewDetections(const cv::Mat& image, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = false;
	std::vector<std::pair<YOLOv11ONNX::Detection, Object>> generatedObjects;
	if (!detections.empty() and !image.empty())
	{
		for (std::pair<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>> detectorAndDetection : detections)
		{
			for (YOLOv11ONNX::Detection& detection : detectorAndDetection.second)
			{
				bool addedDetection = false;
				Object object = GenerateObject(image, detectorAndDetection.first, detection);
				if (!m_objectsToTrack[detectorAndDetection.first])
				{
					generatedObjects.push_back({ detection, object });
				}
				else
				{
					if (m_uniqueDetectionCentroidsAndColour[detectorAndDetection.first].empty() and
						m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].empty())
					{
						m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].push_back(object);
						generatedObjects.push_back({ detection, object });
						addedDetection = true;
					}
					else if (!m_uniqueDetectionCentroidsAndColour[detectorAndDetection.first].empty())
					{
						for (auto it = m_uniqueDetectionCentroidsAndColour[detectorAndDetection.first].begin();
							it != m_uniqueDetectionCentroidsAndColour[detectorAndDetection.first].end(); ++it)
						{
							float dx = it->centroid.x - object.centroid.x;
							float dy = it->centroid.y - object.centroid.y;
							float dist = std::sqrt(dx * dx + dy * dy);

							if (dist < m_maxDistanceForMatch)
							{
								//State is currently only relevent to petri dishes 
								if (detectorAndDetection.first == DetectionTypes::DetectionType::PETRI_DISHES)
								{
									ObjectState newState = object.state;
									CheckForStateChange(*it, object, 15, newState);
									object.state = newState;
								}
								it->centroid.x = object.centroid.x;
								it->centroid.y = object.centroid.y;
								it->state = object.state;
								addedDetection = true;
								generatedObjects.push_back({ detection, object });
								break;
							}
						}
					}
					if (!addedDetection)
					{
						bool foundMatch = false;
						for (auto it = m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].begin();
							it != m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].end();)
						{
							float dx = it->centroid.x - object.centroid.x;
							float dy = it->centroid.y - object.centroid.y;
							float dist = std::sqrt(dx * dx + dy * dy);

							if (dist < m_maxDistanceForMatch)
							{
								m_uniqueDetectionCentroidsAndColour[detectorAndDetection.first].push_back(object);
								it = m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].erase(it);
								foundMatch = true;
								generatedObjects.push_back({ detection, object });
								break;
							}
							else
							{
								++it;
							}
						}
						if (!foundMatch)
						{
							m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].push_back(object);
							generatedObjects.push_back({ detection, object });
						}
					}
				}
			}
		}

	}
	return generatedObjects;
}

uint32_t ObjectTracker::GetUniqueDetectionCount(const DetectionTypes::DetectionType DetectionType)
{
	return m_uniqueDetectionCentroidsAndColour[DetectionType].size();
}

uint32_t ObjectTracker::GetUnMatchedDetectionCount(const DetectionTypes::DetectionType DetectionType)
{
	return m_unmatchedDetectionCentroidsAndColour[DetectionType].size();
}

ObjectTracker::Object ObjectTracker::GenerateObject(const cv::Mat& image, const DetectionTypes::DetectionType& detectionType, const YOLOv11ONNX::Detection& detection)
{
	Object object;
	object.centroid.x = detection.box.x + (detection.box.width / 2);
	object.centroid.y = detection.box.y + (detection.box.height / 2);
	object.type = detectionType;

	cv::Rect roi(detection.box.x, detection.box.y, detection.box.width, detection.box.height);

	roi.x = std::max(0, roi.x);
	roi.y = std::max(0, roi.y);
	roi.width = std::min(roi.width, image.cols - roi.x);
	roi.height = std::min(roi.height, image.rows - roi.y);

	cv::Mat roiImage = image(roi);
	object.colour = cv::mean(roiImage);
	if (detectionType == DetectionTypes::DetectionType::PETRI_DISHES)
	{
		object.state = ObjectState::UN_FILLED;
	}
	else
	{
		object.state = ObjectState::UNKNOWN;
	}
	return object;
}

bool ObjectTracker::CheckForStateChange(const Object& previousObject, const Object& newObject, const float tolerance, ObjectState& state)
{
	bool changed = false;
	if (previousObject.state == ObjectState::FILLED)
	{
		double diffB = std::abs(previousObject.colour[0] - newObject.colour[0]);
		double diffG = std::abs(previousObject.colour[1] - newObject.colour[1]);
		double diffR = std::abs(previousObject.colour[2] - newObject.colour[2]);

		if ((diffB > tolerance) || (diffG > tolerance) || (diffR > tolerance))
		{
			changed = true;
			state = ObjectState::FILLED;
		}
	}
	return changed;
}

bool ObjectTracker::BoxesOverlap(const YOLOv11ONNX::Detection& detection1, const YOLOv11ONNX::Detection& detection2)
{
	const YOLOv11ONNX::BoundingBox& a = detection1.box;
	const YOLOv11ONNX::BoundingBox& b = detection2.box;
	return (a.x < b.x + b.width &&
		a.x + a.width > b.x &&
		a.y < b.y + b.height &&
		a.y + a.height > b.y);
}

ObjectTracker::HandInteractions ObjectTracker::CheckHandTouches(const std::vector<std::pair<YOLOv11ONNX::Detection, Object>>& items)
{
	ObjectTracker::HandInteractions handInteractions = { false, false };

	std::vector<YOLOv11ONNX::Detection> handDetections;
	std::vector<YOLOv11ONNX::Detection> petriDishDetections;
	std::vector<YOLOv11ONNX::Detection> bottleDetections;

	for (const std::pair<YOLOv11ONNX::Detection, Object>& pair : items) 
	{
		switch (pair.second.type)
		{
		case(DetectionTypes::DetectionType::HANDS):
			handDetections.push_back(pair.first);
			break;
		case(DetectionTypes::DetectionType::BOTTLES):
			bottleDetections.push_back(pair.first);
			break;
		case(DetectionTypes::DetectionType::PETRI_DISHES):
			petriDishDetections.push_back(pair.first);
			break;
		}
	}

	for (const YOLOv11ONNX::Detection& hand : handDetections)
	{
		for (const YOLOv11ONNX::Detection& petri : petriDishDetections)
		{
			if (BoxesOverlap(hand, petri))
			{
				handInteractions.touchingPetriDish = true;
				break;
			}
		}
		if (handInteractions.touchingPetriDish)
		{
			break;
		}
	}

	for (const YOLOv11ONNX::Detection& hand : handDetections)
	{
		for (const YOLOv11ONNX::Detection& bottle : bottleDetections)
		{
			if (BoxesOverlap(hand, bottle))
			{
				handInteractions.touchingPetriDish = true;
				break;
			}
		}
		if (handInteractions.touchingPetriDish)
		{
			break;
		}
	}

	return handInteractions;
}
