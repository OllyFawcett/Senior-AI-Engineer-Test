#include "ObjectTracker.h"

ObjectTracker::ObjectTracker(const std::map<DetectionTypes::DetectionType, bool>& objectsToTrack, const float maxDistanceForMatch)
	: m_objectsToTrack(objectsToTrack)
	, m_maxDistanceForMatch(maxDistanceForMatch)
{
}

bool ObjectTracker::AddNewDetections(std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = false;
	if (!detections.empty())
	{
		for (std::pair<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>> detectorAndDetection : detections)
		{
			for (YOLOv11ONNX::Detection& detection : detectorAndDetection.second)
			{
				bool addedDetection = false;
				cv::Point2f centroid = GenerateCentroid(detection);
				if (m_uniqueDetectionCentroids[detectorAndDetection.first].empty() and
					m_unMatchedDetectionCentroids[detectorAndDetection.first].empty())
				{
					m_unMatchedDetectionCentroids[detectorAndDetection.first].push_back(centroid);
					addedDetection = true;
				}
				else if (!m_uniqueDetectionCentroids[detectorAndDetection.first].empty())
				{
					for (auto it = m_uniqueDetectionCentroids[detectorAndDetection.first].begin();
						it != m_uniqueDetectionCentroids[detectorAndDetection.first].end(); ++it)
					{
						float dx = it->x - centroid.x;
						float dy = it->y - centroid.y;
						float dist = std::sqrt(dx * dx + dy * dy);

						if (dist < m_maxDistanceForMatch)
						{
							it->x = centroid.x;
							it->y = centroid.y;
							addedDetection = true;
							break;
						}
					}
				}
				if (!addedDetection)
				{
					bool foundMatch = false;
					for (auto it = m_unMatchedDetectionCentroids[detectorAndDetection.first].begin();
						it != m_unMatchedDetectionCentroids[detectorAndDetection.first].end();)
					{
						float dx = it->x - centroid.x;
						float dy = it->y - centroid.y;
						float dist = std::sqrt(dx * dx + dy * dy);

						if (dist < m_maxDistanceForMatch)
						{
							m_uniqueDetectionCentroids[detectorAndDetection.first].push_back(centroid);
							it = m_unMatchedDetectionCentroids[detectorAndDetection.first].erase(it);
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
						m_unMatchedDetectionCentroids[detectorAndDetection.first].push_back(centroid);
					}
				}
			}
		}
	}
	return success;
}

uint32_t ObjectTracker::GetUniqueDetectionCount(const DetectionTypes::DetectionType DetectionType)
{
	return m_uniqueDetectionCentroids[DetectionType].size();
}

uint32_t ObjectTracker::GetUnMatchedDetectionCount(const DetectionTypes::DetectionType DetectionType)
{
	return m_unMatchedDetectionCentroids[DetectionType].size();
}

cv::Point2f ObjectTracker::GenerateCentroid(const YOLOv11ONNX::Detection& detection)
{
	cv::Point2f centroid;
	centroid.x = detection.box.x + (detection.box.width / 2);
	centroid.y = detection.box.y + (detection.box.height / 2);
	return centroid;
}