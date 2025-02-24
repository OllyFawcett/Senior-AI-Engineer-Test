#include <QtWidgets/QApplication>
#include <QString>

#include "CSVWriter.h"
#include "DetectorsHandler.h"
#include "DetectionTypes.h"
#include "IOUtils.h"
#include "LabTracking.h"
#include "ObjectTracker.h"
#include "VideoPlayer.h" 
#include "YOLOv11ONNX.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: <Input Directory> <Output Directory>" << std::endl;;
    }
    else
    {
        std::string inputPath = argv[1];
        std::string outputPath = argv[2];
        IOUtils::InputPaths inputPaths;
        IOUtils::OutputPaths outputPaths;
        if (IOUtils::LoadInputPaths(inputPath, inputPaths) and IOUtils::LoadOutputPaths(outputPath, outputPaths))
        {
            QApplication a(argc, argv);
            LabTracking w;
            w.show();

            const std::map<DetectionTypes::DetectionType, bool> objectsToTrack = { {DetectionTypes::DetectionType::BOTTLES, true},
                                                                                  {DetectionTypes::DetectionType::PETRI_DISHES, true},
                                                                                  {DetectionTypes::DetectionType::HANDS, false} };
            const float maxDistanceForMatch = 30;

            std::shared_ptr<CSVWriter> spCSVWriter = std::make_shared<CSVWriter>(outputPaths.csvPath);
            std::shared_ptr<DetectorsHandler> spDetectorsHandler = std::make_shared<DetectorsHandler>();
            std::shared_ptr<ObjectTracker> spObjectTracker = std::make_shared<ObjectTracker>(objectsToTrack, maxDistanceForMatch);
            spDetectorsHandler->AddDetector(DetectionTypes::DetectorType::HANDS, inputPaths.handDetectorPath);
            spDetectorsHandler->AddDetector(DetectionTypes::DetectorType::PETRI_DISHES_AND_BOTTLES, inputPaths.petriDishAndBottleDetectorPath);
            VideoPlayer* player = new VideoPlayer(inputPaths.videoPath, w.GetCameraViewLabel(), w.GetBottleCountLabel(), w.GetPetriDishCountLabel(),  w.GetCheckBox(DetectionTypes::DetectionType::BOTTLES),
                w.GetCheckBox(DetectionTypes::DetectionType::HANDS),
                w.GetCheckBox(DetectionTypes::DetectionType::PETRI_DISHES), spDetectorsHandler, spCSVWriter, spObjectTracker, &w);
            player->Start();

            return a.exec();
        }
    }
    return 0;
}
