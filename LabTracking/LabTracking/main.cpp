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
    std::string modelPath = "C:/Users/OliverFawcett/Downloads/best.onnx";
    std::string labelsPaths = "C:/Users/OliverFawcett/Downloads/labels.txt";
    std::shared_ptr<YOLOv11ONNX> spHandDetector = std::make_shared<YOLOv11ONNX>(modelPath, false);
    VideoPlayer* player = new VideoPlayer(videoFilePath, w.GetCameraViewLabel(),spHandDetector, &w);
    player->Start();

    return a.exec();
}
