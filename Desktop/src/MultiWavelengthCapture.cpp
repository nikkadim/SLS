#include "MultiWavelengthCapture.h"

MultiWavelengthCapture::MultiWavelengthCapture(void)
{
	m_hasBeenInit = false;
        m_haveReferencePhase = false;
}

void MultiWavelengthCapture::init()
{
	if(!m_hasBeenInit)
	{
                _initShaders();
                _initTextures(576, 576);
		
                m_axis.init();

		m_controller.init(512, 512);
		m_camera.init(0.0f, 0.75f, 1.0f, 0.0f, 0.75f, 0.0f, 0.0f, 1.0f, 0.0f);
		m_camera.setMode(1);
		
		m_mesh = new TriMesh(512, 512);
		
		m_mesh->initMesh();
		m_hasBeenInit = true;
	}
}

void MultiWavelengthCapture::_initShaders(void)
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

        m_phaseFilter.init();
        m_phaseFilter.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MedianFilter3x3.vert"));
        m_phaseFilter.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MedianFilter3x3.frag"));
        m_phaseCalculator.bindAttributeLocation("vert", 0);
        m_phaseCalculator.bindAttributeLocation("vertTexCoord", 1);

        m_phaseFilter.link();
        m_phaseFilter.uniform("image", 0);
        m_phaseFilter.uniform("width", 576.0f);
        m_phaseFilter.uniform("height", 576.0f);

        m_normalCalculator.init();
        m_normalCalculator.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/NormalCalculator.vert"));
        m_normalCalculator.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/NormalCalculator.frag"));
        m_phaseCalculator.bindAttributeLocation("vert", 0);
        m_phaseCalculator.bindAttributeLocation("vertTexCoord", 1);

        m_normalCalculator.link();
        m_normalCalculator.uniform("phaseA", 0);
        m_normalCalculator.uniform("width", 576.0f);
        m_normalCalculator.uniform("height", 576.0f);

        m_finalRender.init();
        m_finalRender.attachShader(new Shader(GL_VERTEX_SHADER, "Shaders/MultiWavelength/FinalRender.vert"));
        m_finalRender.attachShader(new Shader(GL_FRAGMENT_SHADER, "Shaders/MultiWavelength/FinalRender.frag"));
        m_finalRender.link();
        m_finalRender.uniform("normals", 0);
	m_finalRender.uniform("phaseMap", 1);
        m_finalRender.uniform("referencePhaseMap", 2);
	
        OGLStatus::logOGLErrors("MultiWavelengthCapture - initShaders()");
}

void MultiWavelengthCapture::_initTextures(GLuint width, GLuint height)
{
        Logger::logDebug("MultiWavelengthCapture - initTextures(): Creating textures for phase map and normal map");
	
        m_imageProcessor.init(width, height);
	m_imageProcessor.unbind();
	
        m_phaseMap0AttachPoint      = GL_COLOR_ATTACHMENT0_EXT;
        m_phaseMap1AttachPoint      = GL_COLOR_ATTACHMENT1_EXT;
        m_normalMapAttachPoint      = GL_COLOR_ATTACHMENT2_EXT;
        m_referencePhaseAttachPoint = GL_COLOR_ATTACHMENT3_EXT;

        m_fringeImage1.init(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        m_fringeImage2.init(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        m_fringeImage3.init(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

        m_phaseMap0.init        (width, height, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT);
        m_phaseMap1.init        (width, height, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT);
        m_normalMap.init        (width, height, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT);
        m_referencePhase.init   (width, height, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT);

	m_imageProcessor.setTextureAttachPoint(m_phaseMap0, m_phaseMap0AttachPoint);
	m_imageProcessor.setTextureAttachPoint(m_phaseMap1, m_phaseMap1AttachPoint);
	m_imageProcessor.setTextureAttachPoint(m_normalMap, m_normalMapAttachPoint);
        m_imageProcessor.setTextureAttachPoint(m_referencePhase, m_referencePhaseAttachPoint);

        OGLStatus::logOGLErrors("MultiWavelengthCapture - initTextures()");
}

void MultiWavelengthCapture::draw(void)
{
    if(!m_haveReferencePhase)
    {
        //  If we dont have the reference phase then we are calculating it and we redraw
        m_imageProcessor.bind();
        {
            m_imageProcessor.bindDrawBuffer(m_referencePhaseAttachPoint);
            m_phaseCalculator.bind();
            m_fringeImage1.bind(GL_TEXTURE0);
            m_fringeImage2.bind(GL_TEXTURE1);
            m_fringeImage3.bind(GL_TEXTURE2);
            m_imageProcessor.process();
        }
        m_imageProcessor.unbind();

        m_haveReferencePhase = true;
    }
    else
    {
	m_imageProcessor.bind();
	{
		//	Pass 1
		m_imageProcessor.bindDrawBuffer(m_phaseMap0AttachPoint);
		m_phaseCalculator.bind();
                m_fringeImage1.bind(GL_TEXTURE0);
                m_fringeImage2.bind(GL_TEXTURE1);
                m_fringeImage3.bind(GL_TEXTURE2);
                m_imageProcessor.process();
		
		//	Pass 2
		m_imageProcessor.bindDrawBuffer(m_phaseMap1AttachPoint);
		m_phaseFilter.bind();
                m_phaseMap0.bind(GL_TEXTURE0);
		m_imageProcessor.process();
		
		//	Pass 3
		m_imageProcessor.bindDrawBuffer(m_normalMapAttachPoint);
		m_normalCalculator.bind();
                m_phaseMap1.bind(GL_TEXTURE0);
		m_imageProcessor.process();
	}
	m_imageProcessor.unbind();
	
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glLoadIdentity();
	
	m_camera.applyMatrix();
        m_controller.applyTransform();

	glColor3f(0.0f, 1.0f, 0.0f);

        glm::mat4 mvMatrix;
        glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(mvMatrix));
        m_axis.draw(mvMatrix);

	m_finalRender.bind();
	{
                m_normalMap.bind(GL_TEXTURE0);
                m_phaseMap1.bind(GL_TEXTURE1);
                m_referencePhase.bind(GL_TEXTURE2);

                // Draw a plane of pixels
		m_mesh->draw();
	}
	m_finalRender.unbind();

	glPopMatrix();
    }
        OGLStatus::logOGLErrors("MultiWavelengthCapture - draw()");
}

void MultiWavelengthCapture::resize(int width, int height)
{
	m_camera.reshape(width, height);
	gluPerspective(45.0, 1.0, .00001, 10.0);
}

void MultiWavelengthCapture::cameraSelectMode(int mode)
{
	m_camera.setMode(mode);
}

void MultiWavelengthCapture::mousePressEvent(int mouseX, int mouseY)
{
	m_camera.mousePressed(mouseX, mouseY);
}

void MultiWavelengthCapture::mouseMoveEvent(int mouseX, int mouseY)
{
	m_camera.mouseMotion(mouseX, mouseY);
}

void MultiWavelengthCapture::setBackBuffer(IplImage* image)
{
        //int backBufferIndex = (m_frontBufferIndex + 1) % 2;
        //m_holoImages[backBufferIndex]->transferToTexture(image);
	
	//	Make sure we dont have any errors
	OGLStatus::logOGLErrors("Holodecoder - setBackHoloBuffer()");
}

void MultiWavelengthCapture::swapBuffers(void)
{
	//	Switch the front and back buffer
	m_frontBufferIndex = (m_frontBufferIndex + 1) % 2;
	
	//	Make sure we dont have any errors
	OGLStatus::logOGLErrors("Holodecoder - swapBuffers()");
}

void MultiWavelengthCapture::loadTestData(void)
{
    if(m_haveReferencePhase)
    {
        //  Load the test data
        const string path("/home/karpinsn/Dropbox/Research/Data/MultiwaveLength/");

        ImageIO io;

        m_fringeImage1.transferToTexture(io.readImage(path + "fringe1.png"));
        m_fringeImage2.transferToTexture(io.readImage(path + "fringe2.png"));
        m_fringeImage3.transferToTexture(io.readImage(path + "fringe3.png"));
    }
    else
    {
        //  Load the test data
        const string path("/home/karpinsn/Dropbox/Research/Data/MultiwaveLength/Reference/");

        ImageIO io;

        m_fringeImage1.transferToTexture(io.readImage(path + "Reference1.png"));
        m_fringeImage2.transferToTexture(io.readImage(path + "Reference2.png"));
        m_fringeImage3.transferToTexture(io.readImage(path + "Reference3.png"));
    }
}