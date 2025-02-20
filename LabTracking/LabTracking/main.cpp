#include "LabTracking.h"
#include "VideoPlayer.h" 
#include <QtWidgets/QApplication>
#include <QString>
#include "YOLOv11ONNX.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    LabTracking w;
    w.show();

    std::string videoFilePath = "C:/Users/OliverFawcett/Downloads/AICandidateTest-FINAL.mp4";
    std::filesystem::path modelPath = "C:/Users/OliverFawcett/Downloads/YOLOv11HandDetector.onnx";
    std::shared_ptr<YOLOv11ONNX> spHandDetector = std::make_shared<YOLOv11ONNX>(modelPath);
    VideoPlayer* player = new VideoPlayer(videoFilePath, w.GetCameraViewLabel(),spHandDetector, &w);
    player->Start();

    return a.exec();
}
