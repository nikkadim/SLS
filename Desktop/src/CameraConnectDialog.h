#ifndef CAMERA_CONNECT_DIALOG_H
#define CAMERA_CONNECT_DIALOG_H

#include "lens/Camera.h"
#include "lens/OpenCVCamera.h"

//	Qt auto generated headers
#include "ui_CameraConnectDialog.h"

class CameraConnectDialog : public QDialog, private Ui::CameraConnectDialog
{
    Q_OBJECT

public:
    CameraConnectDialog(QWidget *parent = 0);

    lens::Camera* getCamera(void);
};

#endif // CAMERA_CONNECT_DIALOG_H
