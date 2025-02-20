#include "DetectorsHandler.h"

DetectorsHandler::DetectorsHandler()
{
	m_boxColours[DetectorType::HANDS] = cv::Scalar(255, 0, 0);
	m_boxColours[DetectorType::PETRI_DISHES] = cv::Scalar(0, 255, 0);
	m_boxColours[DetectorType::BOTTLES] = cv::Scalar(0, 0, 255);
}

bool DetectorsHandler::AddDetector(const DetectorType& type, const std::string& modelPath)
{
	bool success = false;
	if (!modelPath.empty())
	{
		std::shared_ptr<YOLOv11ONNX> spModel = std::make_shared<YOLOv11ONNX>(modelPath, false);
		m_detectors[type] = spModel;
	}
	return success;
}

bool DetectorsHandler::DetectAndCreateDisplayImage(cv::Mat& image, std::map<DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = false;
	if (!image.empty() and !m_detectors.empty())
	{
		for (const std::pair<DetectorType, std::shared_ptr<YOLOv11ONNX>> detector : m_detectors)
		{
			if (detector.second)
			{
				detections[detector.first] = detector.second->Detect(image, 0.5, 0.5);
				if (!detections[detector.first].empty())
				{
					if (detector.first == DetectorType::BOTTLES)
					{
						std::cout << "found bottles" << std::endl;
					}
					//Successful read if any models have detected anything
					success = true;
				}
			}
		}
	}
	if (success)
	{
		DrawDetectionsOnImage(image, detections);
	}
	return success;
}

bool DetectorsHandler::DrawDetectionsOnImage(cv::Mat& image, std::map<DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
	bool success = true;
	if (!image.empty() and !detections.empty())
	{
		for (const std::pair<DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections : detections)
		{
			for (const YOLOv11ONNX::Detection detection : detections.second)
			{
				cv::rectangle(image, cv::Point(detection.box.x, detection.box.y),
					cv::Point(detection.box.x + detection.box.width, detection.box.y + detection.box.height),
					m_boxColours[detections.first], 2, cv::LINE_AA);
			}
		}
		success = true;
	}
	return success;
}