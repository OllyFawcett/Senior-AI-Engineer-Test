#include "IOUtils.h"

const std::string IOUtils::HAND_DETECTOR_RELATIVE_PATH = "/HandDetector.onnx";
const std::string IOUtils::BOTTLE_DETECTOR_RELATIVE_PATH = "/BottleDetector.onnx";
const std::string IOUtils::PETRI_DISH_DETECTOR_RELATIVE_PATH = "/PetriDishDetector.onnx";
const std::string IOUtils::VIDEO_RELATIVE_PATH = "/AICandidateTest-FINAL.mp4";
const std::string IOUtils::CSV_RELATIVE_PATH = "/LabResults.csv";

bool IOUtils::LoadInputPaths(const std::string& inputPath, InputPaths& inputPaths)
{
    bool success = false;
	inputPaths.handDetectorPath = inputPath + HAND_DETECTOR_RELATIVE_PATH;
	inputPaths.bottleDetectorPath = inputPath + BOTTLE_DETECTOR_RELATIVE_PATH;
	inputPaths.petriDishDetectorPath = inputPath + PETRI_DISH_DETECTOR_RELATIVE_PATH;
    inputPaths.videoPath = inputPath + VIDEO_RELATIVE_PATH;
    if (FileExists(inputPaths.handDetectorPath) and 
        FileExists(inputPaths.bottleDetectorPath) and 
        FileExists(inputPaths.petriDishDetectorPath) and 
        FileExists(inputPaths.videoPath))
    {
        success = true;
    }
    return success;
}

bool IOUtils::LoadOutputPaths(const std::string& outputPath, OutputPaths& outputPaths)
{
    bool success = false;
    if (CreateDirectoryRecursively(outputPath))
    {
        outputPaths.csvPath = outputPath + CSV_RELATIVE_PATH;
        success = true;
    }
    return success;
}

bool IOUtils::FileExists(const std::filesystem::path& filename)
{
    bool exists = false;
    std::filesystem::path file(filename);
    exists = std::filesystem::exists(file);
    return exists;
}

bool IOUtils::CreateDirectoryRecursively(const std::filesystem::path& directoryName)
{
    bool success = false;

    if (!directoryName.empty())
    {
        if(std::filesystem::is_directory(directoryName) and 
            std::filesystem::exists(directoryName))
        {
            success = true;
        }
        else
        {
            std::filesystem::path dir(directoryName);
            std::error_code errorCode;
            if (std::filesystem::create_directories(dir, errorCode))
            {
                success = true;
            }
        }
    }
    return success;
}