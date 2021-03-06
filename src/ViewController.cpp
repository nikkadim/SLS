#include "ViewController.h"

ViewController::ViewController(QWidget* parent) : QWidget(parent)
{
  setupUi(this);
  _connectSignalsWithController();

  m_reader = nullptr;
}

void ViewController::init(void)
{
  viewGLWidget->setGLContext(&m_decoder);
  viewGLWidget->m_holoDecoder = &m_decoder;
}

void ViewController::cameraSelectMode(int mode)
{
  m_decoder.cameraSelectMode(mode);
}

void ViewController::openHoloImage(void)
{
  QString file = QFileDialog::getOpenFileName(this, "Select Holoimage to Open", "/", "Images (*.png *.jpg)");

  if(!file.isEmpty())
  {
    ImageIO io;
	string filename = file.toLocal8Bit().constData();
    IplImage* image = io.readImage(filename.c_str());

    if(nullptr != image)
	{
      m_decoder.setBackHoloBuffer(image);
      m_decoder.swapBuffers();

      _updateGL();
    }
  }
}

void ViewController::playMovie(string movieFile)
{
  if(!m_aviIO.readStreamIsOpen())
  {
	reactor::VideoFileReader* reader = new reactor::VideoFileReader();
	bool fileOpened = reader->openFile(movieFile); 
	
	m_reader = unique_ptr<reactor::MediaFrameReader>(new reactor::DeplanarReaderFilter(new reactor::ColorSpaceReaderFilter(reader, PIX_FMT_YUV444P)));

    if(fileOpened)
    {
        reactor::MediaFrame frame = m_reader->readFrame();
      m_decoder.setBackHoloBuffer(frame);
      m_decoder.swapBuffers();
      m_movieTimer.start();
      startTimer(0);
    }
  }

  _updateGL();
}

void ViewController::pauseMovie()
{
}

void ViewController::timerEvent(QTimerEvent* event)
{
  int elapsed = m_movieTimer.elapsed();

  if(elapsed >= 33)
  {
    m_movieTimer.restart();
    //	Need to fetch the next frame

    //IplImage* frame = m_aviIO.readStream();
	
	//float position = m_aviIO.readStreamPosition() * 100.0;
	//positionSlider->setValue(position);

      reactor::MediaFrame frame = m_reader->readFrame();
      
    m_decoder.setBackHoloBuffer(frame);
    m_decoder.swapBuffers();
  }
  _updateGL();
}

void ViewController::changeMoviePosition(int position)
{
  m_aviIO.setReadStreamPosition((float)position / 100.0f);
}

void ViewController::_updateGL(void)
{
  OpenGLWidget* glContext = findChild<OpenGLWidget*>(QString::fromUtf8("viewGLWidget"));

  if(nullptr != glContext)
  {
    glContext->updateScene();
  }
  else
  {
    Logger::logError("ViewController - _updateGL: Unable to find OpenGL Widget");
  }
}

void ViewController::_connectSignalsWithController(void)
{
  connect(positionSlider, SIGNAL(sliderMoved(int)),	this, SLOT(changeMoviePosition(int)));
  connect(positionSlider, SIGNAL(sliderPressed()),	this, SLOT(pauseMovie()));
}