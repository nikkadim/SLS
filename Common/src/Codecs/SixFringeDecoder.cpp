#include "SixFringeDecoder.h"

SixFringeDecoder::SixFringeDecoder(string& filename)
{
  m_filename = filename;
  m_returnImage = -1;
}

void SixFringeDecoder::openCodec(EncodingOpenGLWidget* glWidget)
{
  if(nullptr == glWidget)
  {
    //  No OpenGL encoding widget. Cant open decode stream
    return;
  }

  m_io.openReadStream(m_filename);
  m_calculateReference = true;
  m_glWidget = glWidget;

  m_glWidget->setGLContext(&m_coder);
  m_glWidget->setDecodingContext(&m_coder);
  m_glWidget->reinit(getWidth(), getHeight());
}

void SixFringeDecoder::closeCodec(void)
{
  m_io.closeReadStream();
  m_glWidget = nullptr;
}

void SixFringeDecoder::process(MeshInterchange* data)
{
  if(nullptr == m_glWidget)
  {
    //  No OpenGL encoding widget. Return a nullptr MeshInterchange
    data = nullptr;
    return;
  }

  if(_streamUntilNewFrame())
  {
    //  End of the file. Return a nullptr mesh
    data->setTexture(nullptr, false);
    return;
  }

  if(m_calculateReference)
  {
	if(nullptr != m_shortReference && nullptr != m_longReference)
	{
	  m_coder.loadReferencePlane(this, &SixFringeDecoder::_referenceFrameCallback);
	  m_glWidget->decode();
	}
	else
	{
	  //	Capture the reference plane
	  m_coder.captureReferencePlane();
	  m_glWidget->decode();

	  //	Stream again until we get an actual frame
	  _streamUntilNewFrame();
	}
	m_calculateReference = false;
  }

  //  TODO - Come and fix
  MeshInterchange* change = m_glWidget->decode();

  if(nullptr != change && !change->isEmpty())
  {
	data->setTexture(change->getTexture(), false);
  }
}

void SixFringeDecoder::previewProcess(MeshInterchange* data)
{
}

bool SixFringeDecoder::_streamUntilNewFrame(void)
{
  IplImage* frame = m_io.readStream();

  //  While the frame is not null
  //  and we dont have a new 3D frame
  while(nullptr != frame && !m_coder.newImage(frame))
  {
    frame = m_io.readStream();
  }

  return frame == nullptr;
}

int SixFringeDecoder::getWidth(void)
{
  return m_io.readStreamWidth();
}

int SixFringeDecoder::getHeight(void)
{
  return m_io.readStreamHeight();
}

float SixFringeDecoder::getStreamLocation(void)
{
  return m_io.readStreamPosition();
}

void SixFringeDecoder::setReferencePlane(shared_ptr<IplImage> shortWavelength, shared_ptr<IplImage> longWavelength)
{
  m_shortReference = shortWavelength;
  m_longReference = longWavelength;
}

void SixFringeDecoder::setGammaCutoff(float gammaValue)
{
  m_coder.setGammaCutoff(gammaValue);
}

void SixFringeDecoder::setScalingFactor(float scaling)
{
  m_coder.setScalingFactor(scaling);
}

shared_ptr<IplImage> SixFringeDecoder::_referenceFrameCallback(void* callbackInstance)
{
  SixFringeDecoder* decoder = (SixFringeDecoder*)callbackInstance;
  decoder->m_returnImage = (decoder->m_returnImage + 1) % 2;
  return decoder->_referenceFrame(decoder->m_returnImage);
}

shared_ptr<IplImage> SixFringeDecoder::_referenceFrame(int returnImage)
{
	return 0 == returnImage ? m_shortReference : m_longReference;
}

string SixFringeDecoder::codecName(void)
{
  return "Six Fringe Decoder";
}