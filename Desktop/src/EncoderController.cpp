#include "EncoderController.h"

EncoderController::EncoderController(QWidget* parent) : QWidget(parent)
{
  setupUi(this);
  _connectSignalsWithController();
}

EncoderController::~EncoderController()
{
}

void EncoderController::init(void)
{
  encoderGLWidget->setGLContext(&m_encoder);
}

void EncoderController::exportCurrentFrame(void)
{
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", "/", "Images (*.png)");

  if(!fileName.isEmpty())
  {
    ImageIO io;
    Texture holoimage = m_encoder.encode();
    io.saveTexture(fileName.toAscii().constData(), holoimage);
  }
}

void EncoderController::exportEntireVideo(QListWidget* fileList)
{
  Logger::logDebug("EncoderController - exportEntireVideo: Enter");

  QString fileName = QFileDialog::getSaveFileName(this, "Save File", "/", "Video (*.avi)");

  if(!fileName.isEmpty())
  {
      VideoIO io;
      bool canSaveFile = io.openSaveStream(fileName.toAscii().constData(), 512, 512, 30);

      if(canSaveFile)
      {
          XYZFileIO fileIO;

          //	Inform the user of the progress
          QProgressDialog progress("Encoding frames...", 0, 0, fileList->count(), this);

          Logger::logDebug("EncoderController - exportEntireVideo: Encoding frames");
          for(int itemNumber = 0; itemNumber < fileList->count(); itemNumber++)
          {
              //	Increase the progress
              progress.setValue(itemNumber);

              QListWidgetItem *item = fileList->item(itemNumber);

              AbstractMesh* currentMesh = fileIO.newMeshFromFile(item->text().toAscii().constData());
              m_encoder.setCurrentMesh(currentMesh);

              Texture holoimage = m_encoder.encode();
              io.saveStream(holoimage);
          }

          //	Last one done!
          progress.setValue(fileList->count());

          Logger::logDebug("EncoderController - exportEntireVideo: Encoding complete!");
          io.closeSaveStream();
      }
  }
}

void EncoderController::selectXYZM(const string &filename)
{
  XYZFileIO fileIO;
  AbstractMesh* currentMesh = fileIO.newMeshFromFile(filename);
  m_encoder.setCurrentMesh(currentMesh);
}

void EncoderController::_updateGL(void)
{
  Logger::logDebug("ViewController - _updateGL: Enter");

  OpenGLWidget* glContext = findChild<OpenGLWidget*>(QString::fromUtf8("encoderGLWidget"));

  if(NULL != glContext)
  {
    glContext->updateScene();
  }
  else
  {
    Logger::logError("ViewController - _updateGL: Unable to find OpenGL Widget");
  }
}

void EncoderController::selectSourceFile(void)
{
  QString file = QFileDialog::getOpenFileName(this, "Select source file to Open", "/", "Video (*.avi)");

  if(!file.isEmpty())
  {
	sourceFileBox->setText(file);
  }
}

void EncoderController::selectDestinationFile(void)
{
  QString file = QFileDialog::getOpenFileName(this, "Select destination file to save to", "/", "Video (*.avi)");

  if(!file.isEmpty())
  {
	destFileBox->setText(file);
  }
}

void EncoderController::encode(void)
{
  //  Open source media
  QString sourceFilename = sourceFileBox->text();
  VideoIO io;
  io.openReadStream(sourceFilename.toStdString());
  IplImage* frame = io.readStream();

  //  Get the decoder
  Holodecoder* decoder = new Holodecoder();
  encoderGLWidget->setGLContext(decoder);
  encoderGLWidget->reinit(frame->width, frame->height);

  //  Get the encoder
  DepthCodec* encoder = new DepthCodec();

  bool calculateReference = true;
  if(NULL != frame)
  {
	QString destFilename = destFileBox->text();
    encoder->openEncodeStream(destFilename.toStdString(), frame->width, frame->height);

    while(NULL != frame)
    {
      //  Actual encoding
      IplImage *im_gray = frame;
      bool releaseGray = false;

	  /*
      if(frame->nChannels > 1)
      {
        im_gray = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        cvCvtColor(frame, im_gray, CV_RGB2GRAY);
        releaseGray = true;
      }*/

	  decoder->setBackHoloBuffer(im_gray);
	  decoder->swapBuffers();
      //if() //  Check if we have decoded a new 3D frame
      //{
        //if(calculateReference)
        //{
         // calculateReference = false;
          //decoder->captureReferencePlane();
        //}
        //else
        //{
          MeshInterchange mesh;
          //mesh.setData(&decoder.decode());
          encoderGLWidget->encode();
          mesh.setData(&decoder->m_depthMap);
          encoder->encode(mesh);
        //}
      //}

      if(releaseGray)
      {
        cvReleaseImage(&im_gray);
      }

      frame = io.readStream();
    }

    encoder->closeEncodeStream();
  }

  io.closeReadStream();

  //  Reset the context
  encoderGLWidget->setGLContext(&m_encoder);

  //  Get rid of our encoder and decoder
  delete decoder;
  delete encoder;
}

void EncoderController::_connectSignalsWithController(void)
{
  connect(encodeButton,				SIGNAL(clicked()), this, SLOT(encode()));
  connect(sourceFileChooseButton,	SIGNAL(clicked()), this, SLOT(selectSourceFile()));
  connect(destFileChooseButton,		SIGNAL(clicked()), this, SLOT(selectDestinationFile()));
}
