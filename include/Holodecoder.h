/*
 Filename:		Holodecoder.h
 Author:		Nikolaus Karpinsky
 Date Created:	09/01/09
 Last Edited:	09/01/09
 
 Revision Log:
 09/01/09 - Nik Karpinsky - Original creation.
 */

#ifndef _HOLODECODER_H_
#define _HOLODECODER_H_

#ifdef __APPLE__
	#include <glew.h>
	#include <OpenGL/gl.h>
#elif _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <GL/glew.h>
	#include <GL/gl.h>
#else
	#include <GL/glew.h>
	#include <GL/gl.h>
#endif

#include <cv.h>

#include "TriMesh.h"
#include "PointCloudMesh.h"
#include "ImageIO.h"
#include "IGLContext.h"
#include "IDecoder.h"

#include <reactor/MediaFrame.h>

#include <wrench/gl/Camera.h>
#include <wrench/gl/ShaderProgram.h>
#include <wrench/gl/Shader.h>
#include <wrench/gl/Texture.h>
#include <wrench/gl/FBO.h>

#include <wrench/gl/OGLStatus.h>

#include <wrench/gl/utils/Arcball.h>
#include <wrench/gl/utils/GradientBackground.h>

using namespace wrench;
using namespace wrench::gl;
using namespace wrench::gl::utils;

class Holodecoder : public IGLContext, public IDecoder
{
private:
    ShaderProgram m_phaseCalculator;
    ShaderProgram m_phaseFilter;
    ShaderProgram m_depthCalculator;
    ShaderProgram m_normalCalculator;
    ShaderProgram m_finalRender;

	GLenum m_phaseMap0AttachPoint;
	GLenum m_phaseMap1AttachPoint;
    GLenum m_depthMapAttachPoint;
	GLenum m_normalMapAttachPoint;
	
	Texture* m_holoImages[2];
	Texture m_holoImage0;
	Texture m_holoImage1;
	
	Texture m_phaseMap0;
	Texture m_phaseMap1;
    Texture m_depthMap;
	Texture m_normalMap;
	
	FBO m_imageProcessor;
	
	Camera m_camera;
	Arcball m_controller;
    GradientBackground m_background;
	
    TriMesh* m_mesh;
	
	bool haveHoloImage;
	
	bool m_hasBeenInit;
	
	int m_frontBufferIndex;		//	Index of the front buffer in m_bufferIds
	
public:
    Holodecoder();
	
	virtual void init(void);
    virtual void init(int width, int height);
	virtual void draw(void);
	virtual void resize(int width, int height);
	virtual void cameraSelectMode(int mode);
	virtual void mousePressEvent(int mouseX, int mouseY);
	virtual void mouseMoveEvent(int mouseX, int mouseY);
	
	void setBackHoloBuffer(IplImage* image);
	void setBackHoloBuffer(reactor::MediaFrame& frame);
	void swapBuffers(void);
	
	void decode(void);
	Texture& getDepthMap(void);

private:
	void initShaders(float width, float height);
	void _initTextures(GLuint width, GLuint height);
	void _initLighting(void);
	
	int DATA_SIZE;
	GLubyte* imageData;

    void _drawCalculatePhase();
    void _drawFilterPhase();
    void _drawCalculateDepthMap();
};

#endif	// _HOLODECODER_H_
