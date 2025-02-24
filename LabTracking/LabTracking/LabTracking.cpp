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

QLabel* LabTracking::GetBottleCountLabel() const
{
    return ui.UniqueBottles;
}

QLabel* LabTracking::GetPetriDishCountLabel() const
{
    return ui.UniquePetriDishes;
}

QCheckBox* LabTracking::GetCheckBox(const DetectionTypes::DetectionType detectionType) const
{
    QCheckBox* selectedCheckBox = nullptr;
    switch (detectionType)
    {
    case(DetectionTypes::DetectionType::HANDS):
        selectedCheckBox = ui.displayHands;
        break;
    case(DetectionTypes::DetectionType::PETRI_DISHES):
        selectedCheckBox = ui.displayPetriDishes;
        break;
    case(DetectionTypes::DetectionType::BOTTLES):
        selectedCheckBox = ui.displayBottles;
        break;
    }
    return selectedCheckBox;
}