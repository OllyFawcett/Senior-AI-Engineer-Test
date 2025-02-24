#include "DetectorsHandler.h"

DetectorsHandler::DetectorsHandler()
{
	m_boxColours[DetectionTypes::DetectionType::HANDS] = cv::Scalar(255, 0, 0);
	m_boxColours[DetectionTypes::DetectionType::PETRI_DISHES] = cv::Scalar(0, 255, 0);
	m_boxColours[DetectionTypes::DetectionType::BOTTLES] = cv::Scalar(0, 0, 255);
	m_detectorThresholds[DetectionTypes::DetectorType::HANDS] = std::make_pair<float, float>(0.4, 0.4);
	m_detectorThresholds[DetectionTypes::DetectorType::PETRI_DISHES_AND_BOTTLES] = std::make_pair<float, float>(0.1, 0.4);
	m_detectorLabels[DetectionTypes::DetectorType::HANDS] = { {0, "Right Hand"}, {1, "Right Hand"}, {2, "Left Hand"}, {3, "Left Hand"} };
	m_detectorLabels[DetectionTypes::DetectorType::PETRI_DISHES_AND_BOTTLES] = { {0, "Petri Dish"}, {1, "Bottles"} };
}

bool DetectorsHandler::AddDetector(const DetectionTypes::DetectorType& type, const std::string& modelPath)
{
	const std::lock_guard lg(m_mutex);
	bool success = false;
	if (!modelPath.empty())
	{
		std::shared_ptr<YOLOv11ONNX> spModel = std::make_shared<YOLOv11ONNX>(modelPath, false);
		m_detectors[type] = spModel;
	}
	return success;
}

bool DetectorsHandler::DetectAndCreateDisplayImage(cv::Mat& image, std::map<DetectionTypes::DetectionType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	const std::lock_guard lg(m_mutex);

	bool success = false;
	if (!image.empty() and !m_detectors.empty())
	{
		for (const std::pair<DetectionTypes::DetectorType, std::shared_ptr<YOLOv11ONNX>> detector : m_detectors)
		{
			if (detector.second)
			{
				std::vector<YOLOv11ONNX::Detection> modelDetections = detector.second->Detect(image, m_detectorThresholds[detector.first].first, m_detectorThresholds[detector.first].second);
				if (!modelDetections.empty())
				{
					for (const YOLOv11ONNX::Detection& detection : modelDetections)
					{
						DetectionTypes::DetectionType detectionType;
						if (StringToDetectionType(m_detectorLabels[detector.first][detection.classId], detectionType))
						{
							detections[detectionType].push_back(detection);
						}
					}
					success = true;
				}
			}
		}
		if (success)
		{
			DrawDetectionsOnImage(image, objectsToDisplay, detections);
		}
	}
	return success;
}



bool DetectorsHandler::DrawDetectionsOnImage(cv::Mat& image, std::map<DetectionTypes::DetectionType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = true;
	if (!image.empty() and !detections.empty())
	{
		for (const std::pair<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detectionsSameType : detections)
		{
			if (objectsToDisplay.find(detectionsSameType.first) != objectsToDisplay.end())
			{
				if (objectsToDisplay[detectionsSameType.first])
				{
					for (const YOLOv11ONNX::Detection detection : detectionsSameType.second)
					{
						std::string detectionTypeStr = DetectionTypeToString(detectionsSameType.first);
						cv::rectangle(image, cv::Point(detection.box.x, detection.box.y),
							cv::Point(detection.box.x + detection.box.width, detection.box.y + detection.box.height),
							m_boxColours[detectionsSameType.first], 2, cv::LINE_AA);
						cv::putText(image, detectionTypeStr + ": " + std::to_string(detection.conf), cv::Point(detection.box.x, detection.box.y + 40), 0, 1, m_boxColours[detectionsSameType.first], 2);
						
					}
				}
			}
		}
		success = true;
	}
	return success;
}

bool DetectorsHandler::StringToDetectionType(const std::string& detectionStr, DetectionTypes::DetectionType& detectionType)
{
	bool success = false;
	if (!detectionStr.empty())
	{
		if ((detectionStr == "Right Hand") or (detectionStr == "Left Hand"))
		{
			detectionType = DetectionTypes::DetectionType::HANDS;
			success = true;
		}
		else if (detectionStr == "Petri Dish")
		{
			detectionType = DetectionTypes::DetectionType::PETRI_DISHES;
			success = true;
		}
		else if (detectionStr == "Bottle")
		{
			detectionType = DetectionTypes::DetectionType::BOTTLES;
			success = true;
		}
	}
	return success;
}

std::string DetectorsHandler::DetectionTypeToString(const DetectionTypes::DetectionType& detectionType)
{
	std::string detectionTypeStr;
	switch(detectionType)
	{
	case(DetectionTypes::DetectionType::BOTTLES):
		detectionTypeStr = "Bottles";
		break;
	case(DetectionTypes::DetectionType::HANDS):
		detectionTypeStr = "Hands";
		break;
	case(DetectionTypes::DetectionType::PETRI_DISHES):
		detectionTypeStr = "Petri Dish";
		break;
	}
	return detectionTypeStr;
}