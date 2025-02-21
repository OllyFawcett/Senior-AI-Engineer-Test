#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <memory>
#include "CSVWriter.h"
#include "DetectorsHandler.h"
#include "LabTracking.h"

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(const std::string& videoPath, QLabel* displayLabel, QCheckBox* displayBottles, QCheckBox* displayHands, 
                         QCheckBox* displayPetriDishes, const std::shared_ptr<DetectorsHandler> spDetectorsHandler,
                         QObject* parent = nullptr);
    ~VideoPlayer();

    void Start();
    void Stop();

private slots:
    void UpdateFrame();

private:
    std::shared_ptr<DetectorsHandler> m_spDetectorsHandler;
    std::shared_ptr<CSVWriter> m_spCSVWriter;
    cv::VideoCapture m_cap;
    QLabel* m_label;
    QCheckBox* m_displayBottles;
    QCheckBox* m_displayHands;
    QCheckBox* m_displayPetriDishes;
    QTimer m_timer;
    float m_fps;
};

