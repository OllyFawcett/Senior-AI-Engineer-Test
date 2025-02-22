#include <QtWidgets/QApplication>
#include <QString>

#include "CSVWriter.h"
#include "DetectorsHandler.h"
#include "DetectionTypes.h"
#include "LabTracking.h"
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
    }
    QApplication a(argc, argv);
    LabTracking w;
    w.show();

    //TODO : consider loading from ini or from arguments when running
    std::string videoFilePath = "C:/Users/OliverFawcett/Downloads/AICandidateTest-FINAL.mp4";
    std::string handDetectorModelPath = "C:/Users/OliverFawcett/Downloads/HandDetector.onnx";
    std::string petriDishDetectorModelPath = "C:/Users/OliverFawcett/Downloads/PetriDishDetector2.onnx";
    std::string bottleDetectorModelPath = "C:/Users/OliverFawcett/Downloads/BottleDetector2.onnx";
    std::string outputCSVName = "LabResults.csv";

    std::shared_ptr<CSVWriter> spCSVWriter = std::make_shared<CSVWriter>(outputCSVName);
    std::shared_ptr<DetectorsHandler> spDetectorsHandler = std::make_shared<DetectorsHandler>();
    spDetectorsHandler->AddDetector(DetectionTypes::DetectorType::HANDS, handDetectorModelPath);
    spDetectorsHandler->AddDetector(DetectionTypes::DetectorType::PETRI_DISHES, petriDishDetectorModelPath);
    spDetectorsHandler->AddDetector(DetectionTypes::DetectorType::BOTTLES, bottleDetectorModelPath);
    VideoPlayer* player = new VideoPlayer(videoFilePath, w.GetCameraViewLabel(), w.GetCheckBox(DetectionTypes::DetectorType::BOTTLES), 
                                          w.GetCheckBox(DetectionTypes::DetectorType::HANDS), 
                                          w.GetCheckBox(DetectionTypes::DetectorType::PETRI_DISHES), spDetectorsHandler, spCSVWriter, &w);
    player->Start();

    return a.exec();
}
