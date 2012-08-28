#include "ThreeFringeCapture.h"

ThreeFringeCapture::ThreeFringeCapture(void)
{
  m_hasBeenInit = false;
  m_haveReferencePhase = false;
  m_captureReferencePhase = false;
  m_currentFringeLoad = 0;
  m_currentChannelLoad = 0;
  m_frontBufferIndex = 0;
  m_gammaCutoff = 0.3f;
  m_scalingFactor = 0.04f;
  m_displayMode = Geometry;
}

ThreeFringeCapture::~ThreeFringeCapture()
{
  if(m_hasBeenInit)
  {
    delete m_mesh;
    cvReleaseImage(&m_fringeLoadingImage);
  }
}

void ThreeFringeCapture::init()
{
  init(256,256);
}

void ThreeFringeCapture::init(float width, float height)
{
  if(!m_hasBeenInit && width > 0 && height > 0)
  {
    _initShaders(width, height);
    _initTextures(width, height);
    m_textureDisplay.init();
    _initLighting();

    m_axis.init();

    //m_controller.init(512, 512);
	m_controller.init(0.0f, 0.0f, 0.0f, .5f);
    m_camera.init(0.0f, 0.75f, 1.0f, 0.0f, 0.75f, 0.0f, 0.0f, 1.0f, 0.0f);
    m_camera.setMode(1);

    m_mesh = new TriMesh(width, height);
    m_mesh->initMesh();

    m_fringeLoadingImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

    m_hasBeenInit = true;
  }
}

void ThreeFringeCapture::resizeInput(float width, float height)
{
  //  Make sure that it has been initalized first.
  if(m_hasBeenInit)
  {
    //  Resize all of the textures
    m_fringeImage1.reinit     (width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
    m_fringeImage2.reinit     (width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
 
    m_phaseMap0.reinit        (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
    m_phaseMap1.reinit        (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
    m_depthMap.reinit         (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
    m_normalMap.reinit        (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
    m_referencePhase.reinit   (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);

    //  Resize the image processor
    m_imageProcessor.reinit(width, height);
    m_imageProcessor.setTextureAttachPoint(m_phaseMap0, m_phaseMap0AttachPoint);
    m_imageProcessor.setTextureAttachPoint(m_phaseMap1, m_phaseMap1AttachPoint);
    m_imageProcessor.setTextureAttachPoint(m_depthMap, m_depthMapAttachPoint);
    m_imageProcessor.setTextureAttachPoint(m_normalMap, m_normalMapAttachPoint);
    m_imageProcessor.setTextureAttachPoint(m_referencePhase, m_referencePhaseAttachPoint);
    m_imageProcessor.unbind();

    //  Send the new size to all of the shaders
    m_phaseFilter.uniform("width", width);
    m_phaseFilter.uniform("height", height);
    m_normalCalculator.uniform("width", width);
    m_normalCalculator.uniform("height", height);

    //  Resize the display mesh
    delete m_mesh;
    m_mesh = new TriMesh(width, height);
    m_mesh->initMesh();

    //  Resize the fringe loader
    cvReleaseImage(&m_fringeLoadingImage);
    m_fringeLoadingImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  }

  OGLStatus::logOGLErrors("ThreeFringeCapture - resizeInput()");
}

void ThreeFringeCapture::_initShaders(float width, float height)
{
  // Create the shaders
  m_phaseCalculator.init();
  m_phaseCalculator.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MultiWavelength/PhaseCalculator.vert"));
  m_phaseCalculator.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MultiWavelength/PhaseCalculator.frag"));
  m_phaseCalculator.bindAttributeLocation("vert", 0);
  m_phaseCalculator.bindAttributeLocation("vertTexCoord", 1);

  m_phaseCalculator.link();
  m_phaseCalculator.uniform("fringeImage1", 0);
  m_phaseCalculator.uniform("fringeImage2", 1);
  m_phaseCalculator.uniform("fringeImage3", 2);
  m_phaseCalculator.uniform("gammaCutoff", m_gammaCutoff);

  m_depthCalculator.init();
  m_depthCalculator.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MultiWavelength/DepthCalculator.vert"));
  m_depthCalculator.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MultiWavelength/DepthCalculator.frag"));
  m_depthCalculator.bindAttributeLocation("vert", 0);
  m_depthCalculator.bindAttributeLocation("vertTexCoord", 1);

  m_depthCalculator.link();
  m_depthCalculator.uniform("actualPhase", 0);
  m_depthCalculator.uniform("referencePhase", 1);
  m_depthCalculator.uniform("scalingFactor", m_scalingFactor);

  m_phaseFilter.init();
  m_phaseFilter.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MultiWavelength/PhaseFilter.vert"));
  m_phaseFilter.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MultiWavelength/PhaseFilter.frag"));
  m_phaseFilter.bindAttributeLocation("vert", 0);
  m_phaseFilter.bindAttributeLocation("vertTexCoord", 1);

  m_phaseFilter.link();
  m_phaseFilter.uniform("image", 0);
  m_phaseFilter.uniform("width", width);
  m_phaseFilter.uniform("height", height);

  m_normalCalculator.init();
  m_normalCalculator.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MultiWavelength/NormalCalculator.vert"));
  m_normalCalculator.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MultiWavelength/NormalCalculator.frag"));
  m_normalCalculator.bindAttributeLocation("vert", 0);
  m_normalCalculator.bindAttributeLocation("vertTexCoord", 1);

  m_normalCalculator.link();
  m_normalCalculator.uniform("depthMap", 0);
  m_normalCalculator.uniform("width", width);
  m_normalCalculator.uniform("height", height);

  m_finalRender.init();
  m_finalRender.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MultiWavelength/FinalRender.vert"));
  m_finalRender.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MultiWavelength/FinalRender.frag"));
  m_finalRender.bindAttributeLocation("vert", 0);
  m_finalRender.bindAttributeLocation("vertTexCoord", 1);

  m_finalRender.link();
  m_finalRender.uniform("normals", 0);
  m_finalRender.uniform("depthMap", 1);
  m_finalRender.uniform("phaseMap", 2);

  OGLStatus::logOGLErrors("ThreeFringeCapture - initShaders()");
}

void ThreeFringeCapture::_initTextures(GLuint width, GLuint height)
{
  Logger::logDebug("ThreeFringeCapture - initTextures(): Creating textures for phase map and normal map");

  m_phaseMap0AttachPoint      = GL_COLOR_ATTACHMENT0_EXT;
  m_phaseMap1AttachPoint      = GL_COLOR_ATTACHMENT1_EXT;
  m_depthMapAttachPoint       = GL_COLOR_ATTACHMENT2_EXT;
  m_normalMapAttachPoint      = GL_COLOR_ATTACHMENT3_EXT;
  m_referencePhaseAttachPoint = GL_COLOR_ATTACHMENT4_EXT;

  m_fringeImage1.init(width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
  m_fringeImage2.init(width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
  
  m_fringeImages[0] = &m_fringeImage1;
  m_fringeImages[1] = &m_fringeImage2;

  m_phaseMap0.init        (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
  m_phaseMap1.init        (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
  m_depthMap.init         (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
  m_normalMap.init        (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);
  m_referencePhase.init   (width, height, GL_RGB32F_ARB, GL_RGB, GL_FLOAT);

  m_imageProcessor.init(width, height);
  m_imageProcessor.setTextureAttachPoint(m_phaseMap0, m_phaseMap0AttachPoint);
  m_imageProcessor.setTextureAttachPoint(m_phaseMap1, m_phaseMap1AttachPoint);
  m_imageProcessor.setTextureAttachPoint(m_depthMap, m_depthMapAttachPoint);
  m_imageProcessor.setTextureAttachPoint(m_normalMap, m_normalMapAttachPoint);
  m_imageProcessor.setTextureAttachPoint(m_referencePhase, m_referencePhaseAttachPoint);
  m_imageProcessor.unbind();

  OGLStatus::logOGLErrors("ThreeFringeCapture - initTextures()");
}

void ThreeFringeCapture::_initLighting(void)
{
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  m_finalRender.uniform("lightPosition", glm::vec3(0.5f, 0.5f, 4.0f));
  m_finalRender.uniform("ambientColor", glm::vec4(.1, .1, .1, 1.0));
  m_finalRender.uniform("diffuseColor", glm::vec4(1.0, 1.0, 1.0, 1.0));
  m_finalRender.uniform("specularColor", glm::vec4(1.0, 1.0, 1.0, 1.0));
}

void ThreeFringeCapture::setGammaCutoff(float gamma)
{
  m_gammaCutoff = gamma;
}

void ThreeFringeCapture::setScalingFactor(float scalingFactor)
{
  m_scalingFactor = scalingFactor;
}

MeshInterchange* ThreeFringeCapture::decode(void)
{
  OGLStatus::logOGLErrors("ThreeFringeCapture - decode()");
  return new MeshInterchange(m_depthMap);
}

void ThreeFringeCapture::draw(void)
{
  if(m_captureReferencePhase)
  {
    //  If we dont have the reference phase then we are calculating it and we redraw
    m_imageProcessor.bind();
    {
      m_imageProcessor.bindDrawBuffer(m_referencePhaseAttachPoint);
      m_phaseCalculator.bind();
      m_fringeImages[m_frontBufferIndex]->bind(GL_TEXTURE0);
      m_imageProcessor.process();
    }
    m_imageProcessor.unbind();

    m_haveReferencePhase = true;
    m_captureReferencePhase = false;
  }
  else if(m_haveReferencePhase && Geometry == m_displayMode)
  {
	m_imageProcessor.bind();
	{
      //	Pass 1 - Phase Calculation
      _drawCalculatePhase();

      //	Pass 2 - Phase filtering
      _drawFilterPhase();

      //    Pass 3 - Depth Calculator
      _drawCalculateDepthMap();

      //	Pass 4 - Normal Calculation
      _drawCalculateNormalMap();
	}
    m_imageProcessor.unbind();

	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glLoadIdentity();

    m_camera.applyMatrix();
    m_controller.applyTransform();

	glColor3f(0.0f, 1.0f, 0.0f);

    glm::mat4 modelViewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 normalMatrix;

    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelViewMatrix));
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projectionMatrix));
    normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));   //  This is needed for lighting calculations

    m_axis.draw(modelViewMatrix);

	m_finalRender.bind();
	{
      m_finalRender.uniform("modelViewMatrix", modelViewMatrix);
      m_finalRender.uniform("projectionMatrix", projectionMatrix);
      m_finalRender.uniform("normalMatrix", normalMatrix);

      m_normalMap.bind(GL_TEXTURE0);
      m_depthMap.bind(GL_TEXTURE1);
      m_phaseMap0.bind(GL_TEXTURE2);

      // Draw a plane of pixels
      m_mesh->draw();
	}
	m_finalRender.unbind();

    glPopMatrix();
  }
  else if(m_haveReferencePhase && Phase == m_displayMode)
  {
    m_textureDisplay.draw(&m_referencePhase);
  }

  m_fpsCalculator.frameUpdate();
  OGLStatus::logOGLErrors("ThreeFringeCapture - draw()");
}

void ThreeFringeCapture::resize(int width, int height)
{
  m_camera.reshape(width, height);
  m_textureDisplay.resize(width, height);

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, 1.0, .00001, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}

void ThreeFringeCapture::cameraSelectMode(int mode)
{
  m_camera.setMode(mode);
}

void ThreeFringeCapture::mousePressEvent(int mouseX, int mouseY)
{
  m_camera.mousePressed(mouseX, mouseY);
}

void ThreeFringeCapture::mouseMoveEvent(int mouseX, int mouseY)
{
  m_camera.mouseMotion(mouseX, mouseY);
}

bool ThreeFringeCapture::newImage(IplImage* image)
{
  bool needRedraw = false;

  cvSetImageCOI(image, 1);
  cvSetImageCOI(m_fringeLoadingImage, (m_currentChannelLoad + 1));
  cvCopy(image, m_fringeLoadingImage);
  cvSetImageCOI(m_fringeLoadingImage, 0);
  cvSetImageCOI(image, 0);

  m_currentChannelLoad++;

  if(m_currentChannelLoad == 3)
  {
    int backBufferIndex = (m_frontBufferIndex + 1) % 2;
    m_fringeImages[backBufferIndex]->transferToTexture(m_fringeLoadingImage);
    m_currentChannelLoad = 0;
    
	swapBuffers();
    m_3dpsCalculator.frameUpdate();
    needRedraw = true;
  }

  //	Make sure we dont have any errors
  OGLStatus::logOGLErrors("ThreeFringeCapture - setBackHoloBuffer()");
  return needRedraw;
}

void ThreeFringeCapture::swapBuffers(void)
{
  //	Switch the front and back buffer
  m_frontBufferIndex = (m_frontBufferIndex + 1) % 2;

  //	Make sure we dont have any errors
  OGLStatus::logOGLErrors("ThreeFringeCapture - swapBuffers()");
}

void ThreeFringeCapture::captureReferencePlane(void)
{
  m_captureReferencePhase = true;
}

void ThreeFringeCapture::show3D(void)
{
  m_displayMode = Geometry;
}

void ThreeFringeCapture::showPhase(void)
{
  m_displayMode = Phase;
}

double ThreeFringeCapture::getFrameRate(void)
{
  return m_fpsCalculator.getFrameRate();
}

double ThreeFringeCapture::get3DRate(void)
{
  return m_3dpsCalculator.getFrameRate();
}

string ThreeFringeCapture::getCaptureName(void)
{
  return "Three Fringe Capture";
}

void ThreeFringeCapture::_drawCalculatePhase()
{
  m_imageProcessor.bindDrawBuffer(m_phaseMap0AttachPoint);
  m_phaseCalculator.bind();
  m_phaseCalculator.uniform("gammaCutoff", m_gammaCutoff);
  m_fringeImages[m_frontBufferIndex]->bind(GL_TEXTURE0);
  m_fringeImages[m_frontBufferIndex]->bind(GL_TEXTURE1);
  m_fringeImages[m_frontBufferIndex]->bind(GL_TEXTURE2);
  m_imageProcessor.process();
}

void ThreeFringeCapture::_drawFilterPhase()
{
  m_imageProcessor.bindDrawBuffer(m_phaseMap1AttachPoint);
  m_phaseFilter.bind();
  m_phaseMap0.bind(GL_TEXTURE0);
  m_imageProcessor.process();

  //	Pass 2 - Phase filtering ... again
  m_imageProcessor.bindDrawBuffer(m_phaseMap0AttachPoint);
  m_phaseFilter.bind();
  m_phaseMap1.bind(GL_TEXTURE0);
  m_imageProcessor.process();
}

void ThreeFringeCapture::_drawCalculateDepthMap()
{
  m_imageProcessor.bindDrawBuffer(m_depthMapAttachPoint);
  m_depthCalculator.uniform("scalingFactor", m_scalingFactor);
  m_depthCalculator.bind();
  m_phaseMap0.bind(GL_TEXTURE0);
  m_referencePhase.bind(GL_TEXTURE1);
  m_imageProcessor.process();
}

void ThreeFringeCapture::_drawCalculateNormalMap()
{
  m_imageProcessor.bindDrawBuffer(m_normalMapAttachPoint);
  m_normalCalculator.bind();
  m_depthMap.bind(GL_TEXTURE0);
  m_imageProcessor.process();
}