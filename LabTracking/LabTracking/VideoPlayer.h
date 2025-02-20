#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <memory>
#include "DetectorsHandler.h"
#include "LabTracking.h"

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(const std::string& videoPath, QLabel* displayLabel, 
                         const std::shared_ptr<DetectorsHandler> spDetectorsHandler,
                         QObject* parent = nullptr);
    ~VideoPlayer();

    void Start();
    void Stop();

private slots:
    void UpdateFrame();

private:
    std::shared_ptr<DetectorsHandler> m_spDetectorsHandler;
    cv::VideoCapture m_cap;
    QLabel* m_label;
    QTimer m_timer;
    float m_fps;
};

