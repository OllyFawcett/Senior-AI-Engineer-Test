#include "IOUtils.h"

const std::string IOUtils::HAND_DETECTOR_RELATIVE_PATH = "/HandDetector.onnx";
const std::string IOUtils::BOTTLE_DETECTOR_RELATIVE_PATH = "/BottleDetector.onnx";
const std::string IOUtils::PETRI_DISH_DETECTOR_RELATIVE_PATH = "/PetriDishDetector.onnx";
const std::string IOUtils::CSV_RELATIVE_PATH = "/LabResults.csv";

bool IOUtils::LoadInputPaths(const std::string& inputPath, InputPaths& inputPaths)
{
    bool success = false;
	inputPaths.handDetectorPath = inputPath + HAND_DETECTOR_RELATIVE_PATH;
	inputPaths.bottleDetectorPath = inputPath + BOTTLE_DETECTOR_RELATIVE_PATH;
	inputPaths.petriDishDetectorPath = inputPath + PETRI_DISH_DETECTOR_RELATIVE_PATH;
    if (FileExists(inputPaths.handDetectorPath) and 
        FileExists(inputPaths.bottleDetectorPath) and 
        FileExists(inputPaths.petriDishDetectorPath))
    {
        success = true;
    }
    return success;
}

bool IOUtils::LoadOutputPaths(const std::string& outputPath, OutputPaths& outputPaths)
{
    bool success = false;
}

bool IOUtils::FileExists(const std::filesystem::path& filename)
{
    bool exists = false;
    std::filesystem::path file(filename);
    exists = std::filesystem::exists(file);
    return exists;
}