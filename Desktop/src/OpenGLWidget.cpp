#include "OpenGLWidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_clearColor = QColor::fromRgb(0, 0, 0, 0);
    m_glContext = NULL;
}

OpenGLWidget::OpenGLWidget(QWidget *parent, AbstractGLContext* glContext, QColor clearColor) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_glContext = glContext;
	m_clearColor = clearColor;
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{	
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << "Failed to init GLEW: " << glewGetErrorString(err) << endl;
	}
	std::cout << "Using GLEW Version: " << glewGetString(GLEW_VERSION) << endl;

        m_glContext->init();
	
	// Set the clear color
	qglClearColor(m_clearColor);
	
	// Set up the rest of the rendering
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
}

void OpenGLWidget::setNewGLContext(AbstractGLContext* glContext)
{
    //  If we have an old one delete it
    if(NULL != m_glContext)
    {
        delete m_glContext;
    }

    m_glContext = glContext;
    m_glContext->init();
    resizeGL(512, 512);
}

void OpenGLWidget::updateScene()
{
	updateGL();
}

void OpenGLWidget::paintGL()
{
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	
	m_glContext->draw();
	
	glPopMatrix();
	
	//	Print any errors if we have them
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
		cout << "OpenGL Error: " << gluErrorString(error) << endl;
    }
}

void OpenGLWidget::resizeGL(int width, int height)
{
	glViewport(0, 0, 512, 512);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	m_glContext->resize(512, 512);
	
	glMatrixMode(GL_MODELVIEW);
}

void OpenGLWidget::cameraSelectMode(int mode)
{
	m_glContext->cameraSelectMode(mode);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
	std::cout << "Mouse Pressed: (" << event->pos().x() << "," << event->pos().y() << ")" << std::endl;
	m_glContext->mousePressEvent(event->pos().x(), event->pos().y());
	updateGL();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	std::cout << "Mouse Moved: (" << event->pos().x() << "," << event->pos().y() << ")" << std::endl;
	m_glContext->mouseMoveEvent(event->pos().x(), event->pos().y());
	updateGL();
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	std::cout << "Mouse Released: (" << event->pos().x() << "," << event->pos().y() << ")" << std::endl;
	updateGL();
}

void OpenGLWidget::openHoloImage(string filename)
{
	ImageIO io;
	IplImage* image = io.readImage(filename.c_str());
        m_glContext = m_holoDecoder;

	if(NULL != image)
	{
			
		m_holoDecoder->setBackHoloBuffer(image);
		m_holoDecoder->swapBuffers();
	}
	
	updateGL();
}

void OpenGLWidget::timerEvent(QTimerEvent* event)
{
	int elapsed = movieTimer.elapsed();
	
	if(elapsed >= 33)
	{
		movieTimer.restart();
		//	Need to fetch the next frame
		
		IplImage* frame = m_aviIO.readAviFileFrame();

		if(frame)
		{
			m_holoDecoder->setBackHoloBuffer(frame);
			m_holoDecoder->swapBuffers();		
		}
		else 
		{
			//killTimer(event->id);
		}

	}
	updateGL();
}

void OpenGLWidget::playMovie(string movieFile)
{
	m_movieFilename = movieFile;
	
	if(!m_aviIO.aviFileOpen())
	{
		bool fileOpened = m_aviIO.readAviFile(m_movieFilename.c_str());
		
		if(fileOpened)
		{
			IplImage *frame = m_aviIO.readAviFileFrame();
			if(frame)
			{
				m_holoDecoder->setBackHoloBuffer(frame);
				m_holoDecoder->swapBuffers();
				movieTimer.start();
				startTimer(0);
			}
		}
	}
	
	updateGL();
}