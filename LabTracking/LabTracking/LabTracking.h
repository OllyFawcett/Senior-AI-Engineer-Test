#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LabTracking.h"

class LabTracking : public QMainWindow
{
    Q_OBJECT

public:
    LabTracking(QWidget *parent = nullptr);
    ~LabTracking();

    QLabel* GetCameraViewLabel() const { return ui.cameraViewLabel; }
    

private:
    Ui::LabTrackingClass ui;
};
