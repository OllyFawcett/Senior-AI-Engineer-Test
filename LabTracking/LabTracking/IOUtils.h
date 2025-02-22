#pragma once
#include <string>
#include <filesystem>

class IOUtils
{
public:
	struct InputPaths
	{
		std::string handDetectorPath;
		std::string bottleDetectorPath;
		std::string petriDishDetectorPath;
	};
	//Currently not really worth having a struct for one thing but left in so I can potentially add more output paths
	struct OutputPaths
	{
		std::string csvPath;
	};

	static bool LoadInputPaths(const std::string& inputFolder, InputPaths& inputPaths);
	static bool LoadOutputPaths(const std::string& outputFolder, OutputPaths& outputPaths);
	static bool FileExists(const std::filesystem::path& filename);


private:
	static const std::string HAND_DETECTOR_RELATIVE_PATH;
	static const std::string BOTTLE_DETECTOR_RELATIVE_PATH;
	static const std::string PETRI_DISH_DETECTOR_RELATIVE_PATH;
	static const std::string CSV_RELATIVE_PATH;
};