/*
 Filename:		TextureFacade.h
 Author:		Nikolaus Karpinsky
 Date Created:	09/20/10
 Last Edited:	10/27/10
 
 Revision Log:
 09/20/10 - Nik Karpinsky - Original creation.
 10/27/10 - Nik Karpinsky - Allows for VRJ context specific data
 */

#ifndef _TEXTURE_FACADE_H_
#define _TEXTURE_FACADE_H_

#ifdef __APPLE__
#include <glew.h>
#include <OpenGL/gl.h>
#elif _WIN32
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include "OGLStatus.h"

#ifdef USE_VRJ
#include <vrj/Draw/OGL/GlContextData.h>
#endif

class TextureFacade
{
private:
    GLuint m_width;		// Width of the texture
    GLuint m_height;		// Height of the texture

    //	Consult glTexImage2D for what these variables do
    GLint m_internalFormat;     // Internal format of the texture to render to
    GLenum m_format;		// Format of the texture
    GLenum m_dataType;		// Data type of the texture

#ifdef USE_VRJ
    vrj::GlContextData<GLuint> vrjTextureHandle;
    vrj::GlContextData<GLuint> vrjPBOHandle;
    #define m_textureId (*vrjTextureHandle)
    #define m_PBOId     (*vrjPBOHandle)
#else
    GLuint m_textureId;		// Texture identifier
    GLuint m_PBOId;		// PBO identifier. Used for fast DMA transfers
#endif

public:
    TextureFacade(void);
    ~TextureFacade();

    bool init(GLuint width, GLuint height, GLint internalFormat = GL_RGBA32F_ARB, GLenum format = GL_RGBA, GLenum dataType = GL_FLOAT);
    void bind();
    void unbind();

    const GLuint getTextureId(void) const;
    const GLenum getTextureTarget(void) const;

    void transferToTexture(const void* data);
    void transferFromTexture(void* data);
    //void* getDataPointer();						//	Caution should be used when using this method
};

#endif	// _TEXTURE_FACADE_H_