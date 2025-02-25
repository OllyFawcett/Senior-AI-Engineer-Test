#include "DetectionUtils.h"

bool DetectionUtils::StringToDetectionType(const std::string& detectionStr, DetectionTypes::DetectionType& detectionType)
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
		else if (detectionStr == "Bottles")
		{
			detectionType = DetectionTypes::DetectionType::BOTTLES;
			success = true;
		}
	}
	return success;
}

std::string DetectionUtils::DetectionTypeToString(const DetectionTypes::DetectionType& detectionType)
{
	std::string detectionTypeStr;
	switch (detectionType)
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