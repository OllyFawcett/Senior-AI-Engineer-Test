#include "LabTracking.h"
#include <QImage>
#include <QPixmap>
LabTracking::LabTracking(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

LabTracking::~LabTracking()
{}

QLabel* LabTracking::GetCameraViewLabel() const 
{ 
    return ui.cameraViewLabel; 
}

QCheckBox* LabTracking::GetCheckBox(const DetectionTypes::DetectorType detectionType) const
{
    QCheckBox* selectedCheckBox = nullptr;
    switch (detectionType)
    {
    case(DetectionTypes::DetectorType::HANDS):
        selectedCheckBox = ui.displayHands;
        break;
    case(DetectionTypes::DetectorType::PETRI_DISHES):
        selectedCheckBox = ui.displayPetriDishes;
        break;
    case(DetectionTypes::DetectorType::BOTTLES):
        selectedCheckBox = ui.displayBottles;
        break;
    }
    return selectedCheckBox;
}