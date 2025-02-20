#include "VideoPlayer.h"
#include <QImage>
#include <QPixmap>
#include <QDebug>

VideoPlayer::VideoPlayer(const std::string& videoPath, QLabel* displayLabel, const std::shared_ptr<DetectorsHandler> spDetectorsHandler, QObject* parent)
    : QObject(parent)
    , m_label(displayLabel)
    , m_spDetectorsHandler(spDetectorsHandler)
{
    m_cap.open(videoPath);
    if (!m_cap.isOpened()) {
        qDebug() << "Error: Could not open video file:" << QString::fromStdString(videoPath);
    }

    m_fps = m_cap.get(cv::CAP_PROP_FPS);
    if (m_fps <= 0) {
        m_fps = 30; 
    }

    connect(&m_timer, &QTimer::timeout, this, &VideoPlayer::UpdateFrame);
}

VideoPlayer::~VideoPlayer()
{
    Stop();
    m_cap.release();
}

void VideoPlayer::Start()
{
    if (m_cap.isOpened() and (m_fps > 0)) 
    {
        uint32_t interval = static_cast<uint32_t>(1000.0 / m_fps);
        m_timer.start(interval);
    }
}

void VideoPlayer::Stop()
{
    m_timer.stop();
}

void VideoPlayer::UpdateFrame()
{
    cv::Mat frame;
    if (!m_cap.read(frame)) {
        Stop();
        return;
    }

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    std::map<DetectorsHandler::DetectorType, std::vector<YOLOv11ONNX::Detection>> detections;
    if (m_spDetectorsHandler)
    {
        m_spDetectorsHandler->DetectAndCreateDisplayImage(frame, detections);
    }
    if (!detections.empty())
    {
        std::cout << "hello" << std::endl;
    }
    QImage qimg(frame.data, frame.cols, frame.rows, static_cast<uint32_t>(frame.step), QImage::Format_RGB888);

    QPixmap pix = QPixmap::fromImage(qimg).scaled(m_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    m_label->setPixmap(pix);
}
