#include "CaptureController.h"

CaptureController::CaptureController(QWidget* parent) : QWidget(parent)
{
  setupUi(this);                    //  Creates the UI objects
  _connectSignalsWithController();  //  Connects the UI objects to the slots in the controller
  _readSettings();                  //  Reads the persisted settings

  m_dropFrame = false;
  m_fpsLabel.setText(QString(""));
  m_infoBar = NULL;
}

CaptureController::~CaptureController()
{
}

void CaptureController::showEvent(QShowEvent *event)
{  
  //  Connect to camera
  m_frameCapture.start();
  m_frameRateTimer.start(1000);

  //  Display the current FPS
  m_infoBar->addPermanentWidget(&m_fpsLabel);

  captureGLWidget->updateScene();
}

void CaptureController::hideEvent(QHideEvent *)
{
  //  Remove the FPS counter
  if(NULL != m_infoBar)
  {
    m_infoBar->removeWidget(&m_fpsLabel);
  }
}

void CaptureController::init(void)
{
  captureGLWidget->setGLContext(&m_gl3DContext);

  m_camera.init(&m_buffer);
  m_frameCapture.init(&m_buffer);
}

void CaptureController::setInfoBar(QStatusBar* infoBar)
{
  m_infoBar = infoBar;
}

void CaptureController::captureReference(void)
{
  m_gl3DContext.captureReferencePlane();
}

void CaptureController::cameraSelectMode(int mode)
{
  m_gl3DContext.cameraSelectMode(mode);
}

void CaptureController::connectCamera(void)
{
  m_infoBar->showMessage("Connecting to camera...");

  CameraConnectDialog dialog;
  lens::Camera *camera = dialog.getCamera();

  //  Make sure that we have a new camera
  if(NULL != camera)
  {
    m_camera.setCamera(camera);

    //  Reinitalize OpenGL stuff
    m_gl3DContext.resizeInput(camera->getWidth(), camera->getHeight());

    m_camera.start();
    m_infoBar->showMessage("Connected to the camera");
  }
  else
  {
    m_infoBar->showMessage("Not connected");
  }
}

void CaptureController::disconnectCamera(void)
{
  m_camera.stop();
  m_infoBar->showMessage("Camera stopped");
}

void CaptureController::dropFrame(void)
{
  m_dropFrame = true;
}

void CaptureController::newGammaValue(double gammaValue)
{
  //  Set the new gamma value
  m_gl3DContext.setGammaCutoff(gammaValue);

  //  Persist the new gamma value
  m_settings.setValue(SettingsGammaValue, gammaValue);
}

void CaptureController::newScalingFactor(double scalingFactor)
{
  m_gl3DContext.setScalingFactor(scalingFactor);

  //  Persist the new scaling factor
  m_settings.setValue(SettingsScalingFactor, scalingFactor);
}

void CaptureController::newViewMode(QString viewMode)
{
  if(0 == viewMode.compare(QString("3D")))
  {
    m_gl3DContext.show3D();
  }
  else if(0 == viewMode.compare(QString("Phase")))
  {
    m_gl3DContext.showPhase();
  }
}

void CaptureController::updateFPS(void)
{
  double frameRate = m_gl3DContext.getFrameRate();
  QString frameRateMessage = QString("FPS: ");
  frameRateMessage.append(QString("%1").arg(frameRate, 0, 'f', 3));

  m_fpsLabel.setText(frameRateMessage);
}

void CaptureController::newFrame(IplImage *frame)
{
  if(!m_dropFrame)  //  If we dont drop a frame then process it
  {
    IplImage *im_gray = frame;
    bool releaseGray = false;

    if(frame->nChannels > 1)
    {
      im_gray = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
      cvCvtColor(frame, im_gray, CV_RGB2GRAY);
      releaseGray = true;
    }

    m_gl3DContext.newImage(im_gray);

    if(releaseGray)
    {
      cvReleaseImage(&im_gray);
    }

    cvReleaseImage(&frame);
    captureGLWidget->updateScene();
  }
  else  // Drop a frame
  {
    m_dropFrame = false;  //  We only want to drop one frame so set it back
  }
}

void CaptureController::_connectSignalsWithController(void)
{
  connect(&m_frameCapture,    SIGNAL(newFrame(IplImage*)),          this, SLOT(newFrame(IplImage*)));
  connect(openCameraButton,   SIGNAL(clicked()),                    this, SLOT(connectCamera()));
  connect(closeCameraButton,  SIGNAL(clicked()),                    this, SLOT(disconnectCamera()));
  connect(calibrateButton,    SIGNAL(clicked()),                    this, SLOT(captureReference()));
  connect(dropFrameButton,    SIGNAL(clicked()),                    this, SLOT(dropFrame()));
  connect(gammaBox,           SIGNAL(valueChanged(double)),         this, SLOT(newGammaValue(double)));
  connect(scalingFactorBox,   SIGNAL(valueChanged(double)),         this, SLOT(newScalingFactor(double)));
  connect(viewModeBox,        SIGNAL(currentIndexChanged(QString)), this, SLOT(newViewMode(QString)));
  connect(&m_frameRateTimer,  SIGNAL(timeout()),                    this, SLOT(updateFPS()));
}

void CaptureController::_readSettings(void)
{
  //  Read in the settings file and set the settings
  gammaBox->setValue(         m_settings.value(QString("CaptureGammaValue"),    .30).toDouble());
  scalingFactorBox->setValue( m_settings.value(QString("CaptureScalingFactor"), .04).toDouble());
}
