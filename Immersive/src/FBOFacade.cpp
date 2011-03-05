/*
 *  RenderTextureFacade.cpp
 *  Holoencoder
 *
 *  Created by Nikolaus Karpinsky on 9/20/10.
 *  Copyright 2010 ISU. All rights reserved.
 *
 */

#include "FBOFacade.h"
FBOFacade::FBOFacade(void)
{
    m_framebuffer = 0;
    m_renderingQuad = 0;
}

FBOFacade::~FBOFacade()
{
    glDeleteFramebuffersEXT(1, &m_framebuffer);
}

bool FBOFacade::init()
{
    bool init = false;

    glewInit();
    if (!GLEW_EXT_framebuffer_object)
    {
        Logger::logError( "Framebuffer objects are not supported!");
        return false;
    }
    else
    {
        _cacheQuad();
        _initFBO();
        init = true;
    }

    OGLStatus::logOGLErrors("FBOFacade - init()");
    return init;
}

void FBOFacade::bind()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_framebuffer);
    OGLStatus::logOGLErrors("FBOFacade - bind()");
}

void FBOFacade::process(void)
{
    glPushAttrib(GL_VIEWPORT_BIT);
    {
        glMatrixMode (GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity ();
        gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
	
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity();
        glViewport (0, 0, 512, 512);

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glPolygonMode(GL_FRONT,GL_FILL);
	
        glColor3f(1.0f, 1.0f, 1.0f);
        glCallList(m_renderingQuad);
	
        // Pop back matricies as if nothing happened
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    glPopAttrib();

    OGLStatus::logOGLErrors("FBOFacade - process()");
}

void FBOFacade::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    glDrawBuffer(GL_BACK);

    OGLStatus::logOGLErrors("FBOFacade - unbind()");
}

void FBOFacade::bindDrawBuffer(GLenum attachmentPoint)
{
    glDrawBuffer(attachmentPoint);
}

void FBOFacade::setTextureAttachPoint(const TextureFacade &texture, const GLenum attachmentPoint)
{
    bind();
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentPoint, texture.getTextureTarget(), texture.getTextureId(), 0);
}

void FBOFacade::_cacheQuad(void)
{
    //	Quad used to render image operations
    m_renderingQuad = glGenLists(1);

    glNewList(m_renderingQuad, GL_COMPILE);
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin (GL_QUADS);
        {
            glTexCoord2f (0.0f,0.0f); // lower left corner of image
            glVertex3f (-1.0f, -1.0f, 0.0f);
            glTexCoord2f (1.0f, 0.0f); // lower right corner of image
            glVertex3f (1.0f, -1.0f, 0.0f);
            glTexCoord2f (1.0f, 1.0f); // upper right corner of image
            glVertex3f (1.0f, 1.0f, 0.0f);
            glTexCoord2f (0.0f, 1.0f); // upper left corner of image
            glVertex3f (-1.0f, 1.0f, 0.0f);
        }
        glEnd ();
    }
    glEndList();
}

void FBOFacade::_initFBO(void)
{
    glGenFramebuffersEXT(1, &m_framebuffer);
    OGLStatus::logOGLErrors("FBOFacade - _initFBO()");
}