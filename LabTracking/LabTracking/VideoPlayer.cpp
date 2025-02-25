#include "VideoPlayer.h"
#include "DetectionUtils.h"
#include <QImage>
#include <QPixmap>
#include <QString>
#include <chrono>
#include <iomanip>
#include <sstream>

const std::string VideoPlayer::CSV_COLUMN_HEADER_TIME = "Time";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS = "Hand Detections";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS = "Bottle Detections";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_UNFILLED_PETRI_DISH_DETECTIONS = "Unfilled Petri Dish Detections";
const std::string VideoPlayer::CSV_COLUMN_HEADER_NUMBER_OF_FILLED_PETRI_DISH_DETECTIONS = "Filled Petri Dish Detections";
const std::string VideoPlayer::CSV_COLUMN_HEADER_HAND_TOUCHING_PETRI_DISH = "Hand Touching Petri Dish";
const std::string VideoPlayer::CSV_COLUMN_HEADER_HAND_TOUCHING_BOTTLE = "Hand Touching Bottle";

VideoPlayer::VideoPlayer(const std::string& videoPath, QLabel* displayLabel, QLabel* bottleCount, QLabel* petriDishCount, QCheckBox* displayBottles, QCheckBox* displayHands,
    QCheckBox* displayPetriDishes, const std::shared_ptr<DetectorsHandler> spDetectorsHandler, const std::shared_ptr<CSVWriter> spCSVWriter,
    std::shared_ptr<ObjectTracker> spObjectTracker, QObject* parent)
    : QObject(parent)
    , m_bottleCountLabel(bottleCount)
    , m_petriDishCountLabel(petriDishCount)
    , m_label(displayLabel)
    , m_displayBottles(displayBottles)
    , m_displayHands(displayHands)
    , m_displayPetriDishes(displayPetriDishes)
    , m_spDetectorsHandler(spDetectorsHandler)
    , m_spCSVWriter(spCSVWriter)
    , m_spObjectTracker(spObjectTracker)
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
    m_boxColours[DetectionTypes::DetectionType::HANDS] = cv::Scalar(255, 0, 0);
    m_boxColours[DetectionTypes::DetectionType::PETRI_DISHES] = cv::Scalar(0, 255, 0);
    m_boxColours[DetectionTypes::DetectionType::BOTTLES] = cv::Scalar(0, 0, 255);
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
    if (!frame.empty())
    {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        std::map<DetectionTypes::DetectionType, std::vector<YOLOv11ONNX::Detection>> detections;
        if (m_spDetectorsHandler)
        {
            std::map<DetectionTypes::DetectionType, bool> objectsToDisplay;
            objectsToDisplay[DetectionTypes::DetectionType::HANDS] = m_displayHands->isChecked();
            objectsToDisplay[DetectionTypes::DetectionType::BOTTLES] = m_displayBottles->isChecked();
            objectsToDisplay[DetectionTypes::DetectionType::PETRI_DISHES] = m_displayPetriDishes->isChecked();
            m_spDetectorsHandler->Detect(frame, objectsToDisplay, detections);
            if (m_spObjectTracker)
            {
                std::vector<std::pair<YOLOv11ONNX::Detection, ObjectTracker::Object>> trackedDetections = m_spObjectTracker->AddNewDetections(frame, detections);
                ObjectTracker::HandInteractions handInteractions = m_spObjectTracker->CheckHandTouches(trackedDetections);
                DrawDetectionsOnImage(frame, objectsToDisplay, trackedDetections);
                uint32_t bottleCount = m_spObjectTracker->GetUniqueDetectionCount(DetectionTypes::DetectionType::BOTTLES);
                uint32_t petriDishCount = m_spObjectTracker->GetUniqueDetectionCount(DetectionTypes::DetectionType::PETRI_DISHES);
                std::string bottleCountStr = "Bottles: " + std::to_string(bottleCount);
                std::string petriDishCountStr = "Petri Dishes: " + std::to_string(petriDishCount);
                if (m_spCSVWriter)
                {
                    UpdateCSV(trackedDetections, handInteractions);
                }
                if (m_bottleCountLabel)
                {
                    m_bottleCountLabel->setText(QString::fromStdString(bottleCountStr));
                }
                if (m_petriDishCountLabel)
                {
                    m_petriDishCountLabel->setText(QString::fromStdString(petriDishCountStr));
                }
            }
        }
        QImage qimg(frame.data, frame.cols, frame.rows, static_cast<uint32_t>(frame.step), QImage::Format_RGB888);

        QPixmap pix = QPixmap::fromImage(qimg).scaled(m_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        m_label->setPixmap(pix);
    }
}

bool VideoPlayer::InitialiseCSVWriter()
{
    bool success = false;
    if (m_spCSVWriter)
    {
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_TIME);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_UNFILLED_PETRI_DISH_DETECTIONS);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_NUMBER_OF_FILLED_PETRI_DISH_DETECTIONS);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_HAND_TOUCHING_PETRI_DISH);
        m_spCSVWriter->AddColumnHeader(CSV_COLUMN_HEADER_HAND_TOUCHING_BOTTLE);
        success = true;
    }
    return success;
}

bool VideoPlayer::DrawDetectionsOnImage(cv::Mat& image, std::map<DetectionTypes::DetectionType, bool>& objectsToDisplay, std::vector<std::pair<YOLOv11ONNX::Detection, ObjectTracker::Object>>& detections)
{
    bool success = true;
    if (!image.empty() and !detections.empty())
    {
        for (const std::pair<YOLOv11ONNX::Detection, ObjectTracker::Object>& detection : detections)
        {
            if (objectsToDisplay.find(detection.second.type) != objectsToDisplay.end())
            {
                if (objectsToDisplay[detection.second.type])
                {
                    std::string detectionTypeStr = DetectionUtils::DetectionTypeToString(detection.second.type);
                    cv::rectangle(image, cv::Point(detection.first.box.x, detection.first.box.y),
                        cv::Point(detection.first.box.x + detection.first.box.width, detection.first.box.y + detection.first.box.height),
                        m_boxColours[detection.second.type], 2, cv::LINE_AA);
                    std::string displayString = detectionTypeStr;
                    if (detection.second.state == ObjectTracker::ObjectState::UN_FILLED)
                    {
                        displayString += ": UNFILLED";
                    }
                    else if (detection.second.state == ObjectTracker::ObjectState::FILLED)
                    {
                        displayString += ": FILLED";
                    }
                    cv::putText(image, displayString, cv::Point(detection.first.box.x, detection.first.box.y + 40), 0, 1, m_boxColours[detection.second.type], 2);
                }
            }
        }
        success = true;
    }
    return success;
}

bool VideoPlayer::UpdateCSV(std::vector<std::pair<YOLOv11ONNX::Detection, ObjectTracker::Object>>& detections, ObjectTracker::HandInteractions& handInteractions)
{
    bool success = false;
    if (m_spCSVWriter)
    {
        uint32_t handDetections = 0;
        uint32_t bottleDetections = 0;
        uint32_t filledPetriDishDetections = 0;
        uint32_t unfilledPetriDishDetections = 0;
        bool handTouchingBottle = handInteractions.touchingBottle;
        bool handTouchingPetriDish = handInteractions.touchingPetriDish;
        std::string dateTimeStr = GetCurrentDataTimeStr();

        for (const std::pair<YOLOv11ONNX::Detection, ObjectTracker::Object> detection : detections)
        {
            if (detection.second.type == DetectionTypes::DetectionType::BOTTLES)
            {
                bottleDetections += 1;
            }
            else if (detection.second.type == DetectionTypes::DetectionType::HANDS)
            {
                handDetections += 1;
            }
            else if (detection.second.type == DetectionTypes::DetectionType::PETRI_DISHES)
            {
                if (detection.second.state == ObjectTracker::ObjectState::FILLED)
                {
                    filledPetriDishDetections += 1;
                }
                else if (detection.second.state == ObjectTracker::ObjectState::UNKNOWN)
                {
                    unfilledPetriDishDetections += 1;
                }

            }
        }
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_TIME, dateTimeStr);
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_HAND_DETECTIONS, std::to_string(handDetections));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_BOTTLE_DETECTIONS, std::to_string(bottleDetections));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_UNFILLED_PETRI_DISH_DETECTIONS, std::to_string(unfilledPetriDishDetections));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_NUMBER_OF_FILLED_PETRI_DISH_DETECTIONS, std::to_string(filledPetriDishDetections));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_HAND_TOUCHING_BOTTLE, std::to_string(handTouchingBottle));
        m_spCSVWriter->AddRowValue(CSV_COLUMN_HEADER_HAND_TOUCHING_PETRI_DISH, std::to_string(handTouchingPetriDish));

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
