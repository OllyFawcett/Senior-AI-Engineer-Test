#include "VideoPlayer.h"
#include <QImage>
#include <QPixmap>
#include <chrono>
#include <iomanip>
#include <sstream>

const std::string VideoPlayer::CSV_COLUMN_HEADER_TIME = "Time";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS = "Hand Detections";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS = "Bottle Detections";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_PETRI_DISH_DETECTIONS = "Petri Dish Detections";

VideoPlayer::VideoPlayer(const std::string& videoPath, QLabel* displayLabel, QCheckBox* displayBottles, QCheckBox* displayHands,
    QCheckBox* displayPetriDishes, const std::shared_ptr<DetectorsHandler> spDetectorsHandler, const std::shared_ptr<CSVWriter> spCSVWriter, QObject* parent)
    : QObject(parent)
    , m_label(displayLabel)
    , m_displayBottles(displayBottles)
    , m_displayHands(displayHands)
    , m_displayPetriDishes(displayPetriDishes)
    , m_spDetectorsHandler(spDetectorsHandler)
    , m_spCSVWriter(spCSVWriter)
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
    InitialiseCSVWriter();
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
    std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>> detections;
    if (m_spDetectorsHandler)
    {
        std::map<DetectionTypes::DetectorType, bool> objectsToDisplay;
        objectsToDisplay[DetectionTypes::DetectorType::HANDS] = m_displayHands->isChecked();
        objectsToDisplay[DetectionTypes::DetectorType::BOTTLES] = m_displayBottles->isChecked();
        objectsToDisplay[DetectionTypes::DetectorType::PETRI_DISHES] = m_displayPetriDishes->isChecked();
        m_spDetectorsHandler->DetectAndCreateDisplayImage(frame, objectsToDisplay, detections);
        if (m_spCSVWriter)
        {
            UpdateCSV(detections);
        }
    }

    QImage qimg(frame.data, frame.cols, frame.rows, static_cast<uint32_t>(frame.step), QImage::Format_RGB888);

    QPixmap pix = QPixmap::fromImage(qimg).scaled(m_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    m_label->setPixmap(pix);
}

bool VideoPlayer::InitialiseCSVWriter()
{
    bool success = false;
    if (m_spCSVWriter)
    {
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_TIME);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_PETRI_DISH_DETECTIONS);
        success = true;
    }
    return success;
}

bool VideoPlayer::UpdateCSV(std::map<DetectionTypes::DetectorType, std::vector<YOLOv11ONNX::Detection>>& detections)
{
    bool success = false;
    if (m_spCSVWriter)
    {
        uint32_t handDetections = 0;
        uint32_t bottleDetections = 0;
        uint32_t petriDishDetections = 0;
        std::string dateTimeStr = GetCurrentDataTimeStr();

        if (detections.find(DetectionTypes::DetectorType::HANDS) != detections.end())
        {
            handDetections = detections[DetectionTypes::DetectorType::HANDS].size();
        }
        if (detections.find(DetectionTypes::DetectorType::BOTTLES) != detections.end())
        {
            handDetections = detections[DetectionTypes::DetectorType::BOTTLES].size();
        }
        if (detections.find(DetectionTypes::DetectorType::PETRI_DISHES) != detections.end())
        {
            handDetections = detections[DetectionTypes::DetectorType::PETRI_DISHES].size();
        }

        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_TIME, dateTimeStr);
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS, std::to_string(handDetections));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS, std::to_string(bottleDetections));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_PETRI_DISH_DETECTIONS, std::to_string(petriDishDetections));

        m_spCSVWriter->WriteRow();
    }
    return success;
}

std::string VideoPlayer::GetCurrentDataTimeStr()
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    long long milliseconds = value.count() % 1000;

    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    struct tm timeinfo;
    localtime_s(&timeinfo, &now_time);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setw(3) << std::setfill('0') << milliseconds;

    return oss.str();
}
