#include <QObject>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <memory>
#include "YOLOv11ONNX.h"

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(const std::string& videoPath, QLabel* displayLabel, 
                         const std::shared_ptr<YOLOv11ONNX> spHandDetector,
                         QObject* parent = nullptr);
    ~VideoPlayer();

    void Start();
    void Stop();

private slots:
    void UpdateFrame();

private:
    std::shared_ptr<YOLOv11ONNX> m_spHandDetector;
    cv::VideoCapture m_cap;
    QLabel* m_label;
    QTimer m_timer;
    float m_fps;
};

