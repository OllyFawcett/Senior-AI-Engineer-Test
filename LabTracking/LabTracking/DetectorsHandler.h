#pragma once
#include "YOLOv11ONNX.h"

class DetectorsHandler
{
public:
	enum class DetectorType
	{
		HANDS,
		PETRI_DISHES,
		BOTTLES
	};
	DetectorsHandler();

private:
	std::map<DetectorType, std::shared_ptr<YOLOv11ONNX>> m_detectors;
};