#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LabTracking.h"
#include "DetectionTypes.h"

class LabTracking : public QMainWindow
{
    Q_OBJECT

public:
    LabTracking(QWidget *parent = nullptr);
    ~LabTracking();

    QLabel* GetCameraViewLabel() const;
    QLabel* GetBottleCountLabel() const;
    QLabel* GetPetriDishCountLabel() const;
    QCheckBox* GetCheckBox(const DetectionTypes::DetectorType detectionType) const;

private:
    Ui::LabTrackingClass ui;
};
