#include <QObject>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(const std::string& videoPath, QLabel* displayLabel, QObject* parent = nullptr);
    ~VideoPlayer();

    void Start();
    void Stop();

private slots:
    void UpdateFrame();

private:
    cv::VideoCapture m_cap;
    QLabel* m_label;
    QTimer m_timer;
    float m_fps;
};

