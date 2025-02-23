#include "DetectorsHandler.h"

DetectorsHandler::DetectorsHandler()
{
	m_boxColours[DetectionTypes::DetectorType::HANDS] = cv::Scalar(255, 0, 0);
	m_boxColours[DetectionTypes::DetectorType::PETRI_DISHES] = cv::Scalar(0, 255, 0);
	m_boxColours[DetectionTypes::DetectorType::BOTTLES] = cv::Scalar(0, 0, 255);
	m_detectorThresholds[DetectionTypes::DetectorType::HANDS] = std::make_pair<float, float>(0.4, 0.4);
	m_detectorThresholds[DetectionTypes::DetectorType::BOTTLES] = std::make_pair<float, float>(0.1, 0.7);
	m_detectorThresholds[DetectionTypes::DetectorType::PETRI_DISHES] = std::make_pair<float, float>(0.1, 0.5);
	m_detectorLabels[DetectionTypes::DetectorType::HANDS] = { {0, "Right Hand"}, {1, "Right Hand"}, {2, "Left Hand"}, {3, "Left Hand"} };
	m_detectorLabels[DetectionTypes::DetectorType::BOTTLES] = { {0, "Bottle"}};
	m_detectorLabels[DetectionTypes::DetectorType::PETRI_DISHES] = { {0, "Petri Dish"}};
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

bool DetectorsHandler::DetectAndCreateDisplayImage(cv::Mat& image, std::map<DetectionTypes::DetectorType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	const std::lock_guard lg(m_mutex);

	bool success = false;
	if (!image.empty() and !m_detectors.empty())
	{
		for (const std::pair<DetectionTypes::DetectorType, std::shared_ptr<YOLOv11ONNX>> detector : m_detectors)
		{
			if (detector.second)
			{
				detections[detector.first] = detector.second->Detect(image, m_detectorThresholds[detector.first].first, m_detectorThresholds[detector.first].second);
				if (!detections[detector.first].empty())
				{
					//Successful read if any models have detected anything
					success = true;
				}
			}
		}
	}
	if (success)
	{
		DrawDetectionsOnImage(image, objectsToDisplay, detections);
	}
	return success;
}

bool DetectorsHandler::FilterDetections(const std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections,
	std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& filteredDetections)
{
	bool success = false;
	if (!detections.empty())
	{
		filteredDetections = detections;

		auto petriIt = filteredDetections.find(DetectionTypes::DetectorType::PETRI_DISHES);
		auto bottleIt = filteredDetections.find(DetectionTypes::DetectorType::BOTTLES);

		if (petriIt != filteredDetections.end() && bottleIt != filteredDetections.end())
		{
			auto boxesOverlap = [](const YOLOv11ONNX::Detection& d1, const YOLOv11ONNX::Detection& d2) -> bool {
				const auto& a = d1.box;
				const auto& b = d2.box;
				return (a.x < b.x + b.width &&
					a.x + a.width > b.x &&
					a.y < b.y + b.height &&
					a.y + a.height > b.y);
				};

			std::vector<bool> removePetri(petriIt->second.size(), false);
			std::vector<bool> removeBottle(bottleIt->second.size(), false);

			for (size_t i = 0; i < petriIt->second.size(); ++i)
			{
				for (size_t j = 0; j < bottleIt->second.size(); ++j)
				{
					if (boxesOverlap(petriIt->second[i], bottleIt->second[j]))
					{
						if (petriIt->second[i].conf < bottleIt->second[j].conf)
							removePetri[i] = true;
						else
							removeBottle[j] = true;
					}
				}
			}

			std::vector<YOLOv11ONNX::Detection> filteredPetri;
			for (size_t i = 0; i < petriIt->second.size(); ++i)
			{
				if (!removePetri[i])
					filteredPetri.push_back(petriIt->second[i]);
			}
			std::vector<YOLOv11ONNX::Detection> filteredBottle;
			for (size_t j = 0; j < bottleIt->second.size(); ++j)
			{
				if (!removeBottle[j])
					filteredBottle.push_back(bottleIt->second[j]);
			}

			petriIt->second = filteredPetri;
			bottleIt->second = filteredBottle;
		}

		success = true;
	}
	return success;
}


bool DetectorsHandler::DrawDetectionsOnImage(cv::Mat& image, std::map<DetectionTypes::DetectorType, bool>& objectsToDisplay, std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = true;
	if (!image.empty() and !detections.empty())
	{
		for (const std::pair<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections : detections)
		{
			if (objectsToDisplay.find(detections.first) != objectsToDisplay.end())
			{
				if (objectsToDisplay[detections.first])
				{
					for (const YOLOv11ONNX::Detection detection : detections.second)
					{
						cv::rectangle(image, cv::Point(detection.box.x, detection.box.y),
							cv::Point(detection.box.x + detection.box.width, detection.box.y + detection.box.height),
							m_boxColours[detections.first], 2, cv::LINE_AA);
						cv::putText(image, m_detectorLabels[detections.first][detection.classId] + ": " + std::to_string(detection.conf), cv::Point(detection.box.x, detection.box.y + 40), 0, 1, m_boxColours[detections.first], 2);
					}
				}
			}
		}
		success = true;
	}
	return success;
}