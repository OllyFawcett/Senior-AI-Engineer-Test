#include "ObjectTracker.h"

ObjectTracker::ObjectTracker(const std::map<DetectionTypes::DetectionType, bool>& objectsToTrack, const float maxDistanceForMatch)
	: m_objectsToTrack(objectsToTrack)
	, m_maxDistanceForMatch(maxDistanceForMatch)
{
}

bool ObjectTracker::AddNewDetections(const cv::Mat& image, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = false;
	if (!detections.empty() and !image.empty())
	{
		for (std::pair<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>> detectorAndDetection : detections)
		{
			for (YOLOv11ONNX::Detection& detection : detectorAndDetection.second)
			{
				bool addedDetection = false;
				Object object = GenerateObject(image, detectorAndDetection.first, detection);
				if (m_uniqueDetectionCentroidsAndColour[detectorAndDetection.first].empty() and
					m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].empty())
				{
					m_unmatchedDetectionCentroidsAndColour[detectorAndDetection.first].push_back(object);
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
							it->centroid.x = object.centroid.x;
							it->centroid.y = object.centroid.y;
							addedDetection = true;
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
					}
				}
			}
		}
	}
	return success;
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
