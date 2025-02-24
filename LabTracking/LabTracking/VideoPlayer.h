#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <memory>
#include "CSVWriter.h"
#include "DetectorsHandler.h"
#include "LabTracking.h"
#include "ObjectTracker.h"

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(const std::string& videoPath, QLabel* displayLabel, QLabel* bottleCount, QLabel* petriDishCount, QCheckBox* displayBottles,
        QCheckBox* displayHands, QCheckBox* displayPetriDishes, const std::shared_ptr<DetectorsHandler> spDetectorsHandler, const std::shared_ptr<CSVWriter> spCSVWriter,
        std::shared_ptr<ObjectTracker> spObjectTracker, QObject* parent = nullptr);
    ~VideoPlayer();

    void Start();
    void Stop();

private slots:
    void UpdateFrame();

private:
    static const std::string CSV_COLUMN_HEADER_TIME;
    static const std::string CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS;
    static const std::string CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS;
    static const std::string CSV_COLUMN_HEADER_NUMBER_OF_PETRI_DISH_DETECTIONS;

    bool InitialiseCSVWriter();
    bool UpdateCSV(std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections);
    std::string GetCurrentDataTimeStr();
    std::shared_ptr<DetectorsHandler> m_spDetectorsHandler;
    std::shared_ptr<CSVWriter> m_spCSVWriter;
    std::shared_ptr<ObjectTracker> m_spObjectTracker;
    cv::VideoCapture m_cap;
    QLabel* m_label;
    QLabel* m_bottleCountLabel;
    QLabel* m_petriDishCountLabel;
    QCheckBox* m_displayBottles;
    QCheckBox* m_displayHands;
    QCheckBox* m_displayPetriDishes;
    QTimer m_timer;
    float m_fps;
};

