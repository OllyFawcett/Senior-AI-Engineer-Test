#include "LabTracking.h"
#include "VideoPlayer.h" 
#include <QtWidgets/QApplication>
#include <QString>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    LabTracking w;
    w.show();
    QString videoFilePath = "C:/Users/OliverFawcett/Downloads/AICandidateTest-FINAL.mp4";
    VideoPlayer* player = new VideoPlayer(videoFilePath.toStdString(), w.GetCameraViewLabel(), &w);
    player->Start();

    return a.exec();
}
