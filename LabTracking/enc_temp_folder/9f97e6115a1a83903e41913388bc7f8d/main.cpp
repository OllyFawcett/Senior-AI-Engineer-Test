#include "LabTracking.h"
#include "VideoPlayer.h" 
#include <QtWidgets/QApplication>
#include <QString>
#include "YOLOv11ONNX.h"
#include "DetectorsHandler.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    LabTracking w;
    w.show();

    std::string videoFilePath = "C:/Users/OliverFawcett/Downloads/AICandidateTest-FINAL.mp4";
    std::string handDetectorModelPath = "C:/Users/OliverFawcett/Downloads/HandDetector.onnx";
    std::string petriDishDetectorModelPath = "C:/Users/OliverFawcett/Downloads/PetriDishDetector.onnx";
    std::string bottleDetectorModelPath = "C:/Users/OliverFawcett/Downloads/BottleDetector.onnx";
    std::string labelsPaths = "C:/Users/OliverFawcett/Downloads/labels.txt";
    std::shared_ptr<DetectorsHandler> spDetectorsHandler = std::make_shared<DetectorsHandler>();
    spDetectorsHandler->AddDetector(DetectorsHandler::DetectorType::HANDS, handDetectorModelPath);
    //spDetectorsHandler->AddDetector(DetectorsHandler::DetectorType::PETRI_DISHES, petriDishDetectorModelPath);
    spDetectorsHandler->AddDetector(DetectorsHandler::DetectorType::BOTTLES, bottleDetectorModelPath);
    VideoPlayer* player = new VideoPlayer(videoFilePath, w.GetCameraViewLabel(),spDetectorsHandler, &w);
    player->Start();

    return a.exec();
}
