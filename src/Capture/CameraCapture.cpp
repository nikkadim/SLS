#include "Capture/CameraCapture.h"

CameraCapture::CameraCapture()
{	
  m_camera       = nullptr;
  m_frameGrabber = nullptr;
}

void CameraCapture::init(ImageBuffer *buffer)
{
  m_buffer = buffer;
}

void CameraCapture::start()
{
  if(nullptr != m_camera && nullptr != m_frameGrabber)
  {
	m_frameGrabber->open();
  }
}

void CameraCapture::stop()
{
  if(nullptr != m_camera)
  {
	m_frameGrabber->close();
	m_camera->close();
  }
  m_frameGrabber = nullptr;
  m_camera       = nullptr;
}

void CameraCapture::newFrame(IplImage* frame)
{
  if(nullptr != m_buffer)
  {
	m_buffer->pushFrame(frame);
  }
  else
  {
	//	Log an error and drop the frame
	Logger::logError("CameraCapture - newFrame: Receiving new frames but dont have an image buffer. Just dropping them :(");
  }
}

void CameraCapture::setCamera(unique_ptr<lens::ICamera> camera)
{
  if(nullptr != camera)
  {
	m_camera       = ::move(camera);
	m_frameGrabber = unique_ptr<lens::FrameGrabber>( new lens::FrameGrabber( *m_camera ) );
	m_frameGrabber->addObserver(this);
  }
}

bool CameraCapture::hasCamera(void)
{ 
  return nullptr != m_camera;
}