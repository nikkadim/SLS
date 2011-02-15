/*
 *  RenderTextureFacade.cpp
 *  Holoencoder
 *
 *  Created by Nikolaus Karpinsky on 9/20/10.
 *  Copyright 2010 ISU. All rights reserved.
 *
 */

#include "FBO.h"

wrench::gl::FBO::FBO(void)
{
}

wrench::gl::FBO::~FBO()
{
	glDeleteFramebuffersEXT(1, &m_framebuffer);
}

bool wrench::gl::FBO::init(int width, int height)
{
    m_width = width;
    m_height = height;

    _cacheQuad();
    _initFBO();
	
    OGLStatus::logOGLErrors("FBOFacade - init()");
	
    return true;
}

void wrench::gl::FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_framebuffer);
	OGLStatus::logOGLErrors("FBOFacade - bind()");
}

void wrench::gl::FBO::process(void)
{
	glPushAttrib(GL_VIEWPORT_BIT);
	{
		glMatrixMode (GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity ();
                gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
	
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity();
                glViewport (0, 0, m_width, m_height);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glPolygonMode(GL_FRONT,GL_FILL);

                glBindVertexArray(m_vaoID);
                glDrawArrays(GL_QUADS, 0, 4);
                glBindVertexArray(0);

		//	Pop back matricies as if nothing happened
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	glPopAttrib();
		
	OGLStatus::logOGLErrors("FBOFacade - process()");
}

void wrench::gl::FBO::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer(GL_BACK);
	
	OGLStatus::logOGLErrors("FBOFacade - unbind()");
}

void wrench::gl::FBO::bindDrawBuffer(GLenum attachmentPoint)
{
	glDrawBuffer(attachmentPoint);
}

void wrench::gl::FBO::setTextureAttachPoint(const Texture &texture, const GLenum attachmentPoint)
{
	bind();
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentPoint, texture.getTextureTarget(), texture.getTextureId(), 0);
}

void wrench::gl::FBO::_cacheQuad(void)
{	
        float* vertex   = new float[12];	// vertex array
        float* tex      = new float[8];         // texture coord array

        vertex[0] = -1.0f; vertex[1]   =-1.0f; vertex[2]  = -1.0f;
        vertex[3] =  1.0f; vertex[4]   =-1.0f; vertex[5]  = -1.0f;
        vertex[6] =  1.0f; vertex[7]   = 1.0f; vertex[8]  = -1.0f;
        vertex[9] = -1.0f; vertex[10]  = 1.0f; vertex[11] = -1.0f;

        tex[0] = 0.0f; tex[1] = 0.0f;
        tex[2] = 1.0f; tex[3] = 0.0f;
        tex[4] = 1.0f; tex[5] = 1.0f;
        tex[6] = 0.0f; tex[7] = 1.0f;

        glGenVertexArrays(1, &m_vaoID);
        glBindVertexArray(m_vaoID);

        glGenBuffers(2, m_vboID);

        glBindBuffer(GL_ARRAY_BUFFER, m_vboID[0]);
        glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), vertex, GL_STATIC_DRAW);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, m_vboID[1]);
        glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), tex, GL_STATIC_DRAW);
        glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        delete [] vertex;
        delete [] tex;
}

void wrench::gl::FBO::_initFBO(void)
{
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_framebuffer);

	OGLStatus::logOGLErrors("FBOFacade - _initFBO()");
}
