#pragma once
#include "DetectionTypes.h"
#include "string"

class DetectionUtils
{
public:
	static bool StringToDetectionType(const std::string& detectionStr, DetectionTypes::DetectionType& detectionType);
	static std::string DetectionTypeToString(const DetectionTypes::DetectionType& detectionType);
};