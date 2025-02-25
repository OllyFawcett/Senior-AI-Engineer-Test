#include "DetectorsHandler.h"
#include "DetectionUtils.h"

DetectorsHandler::DetectorsHandler()
{
	m_detectorThresholds[DetectionTypes::DetectorType::HANDS] = std::make_pair<float, float>(0.4, 0.4);
	m_detectorThresholds[DetectionTypes::DetectorType::PETRI_DISHES_AND_BOTTLES] = std::make_pair<float, float>(0.4, 0.4);
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

bool DetectorsHandler::Detect(cv::Mat& image, std::map<DetectionTypes::DetectionType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>>& detections)
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
						if (DetectionUtils::StringToDetectionType(m_detectorLabels[detector.first][detection.classId], detectionType))
						{
							detections[detectionType].push_back(detection);
						}
					}
					success = true;
				}
			}
		}
	}
	return success;
}
