/*
 Filename:		OpenGLWidget.h
 Author:		Nikolaus Karpinsky
 Date Created:	09/30/10
 Last Edited:	09/30/10
 
 Revision Log:
 09/30/10 - Nik Karpinsky - Original creation.
 */

#ifndef _ENCODING_OPEN_GL_WIDGET_H_
#define _ENCODING_OPEN_GL_WIDGET_H_

#ifdef __APPLE__
#include <glew.h>
#include <QtOpenGL/QGLWidget>
#elif _WIN32
#include <windows.h>
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>
#else
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>
#endif

#include <QtCore/QTimer>
#include <QSemaphore>
#include <QtGui>
#include <iostream>

#include "AbstractGLContext.h"
#include "EncodingGLContext.h"
#include "DecodingGLContext.h"

class EncodingOpenGLWidget : public QGLWidget
{
  Q_OBJECT

private:
  QColor		m_clearColor;
  QSemaphore	m_codecLock;

  bool m_encode;
  bool m_decode;

  float m_width;
  float m_height;

  EncodingGLContext* m_encodingContext;
  DecodingGLContext* m_decodingContext;

public:
  EncodingOpenGLWidget(QWidget *parent);
  EncodingOpenGLWidget(QWidget* parent, AbstractGLContext* glContext, QColor clearColor);
  virtual ~EncodingOpenGLWidget();

  void reinit(float width, float height);
  void initializeGL();
  void updateScene();
  void setGLContext(AbstractGLContext* glContext);
  void setEncodingContext(EncodingGLContext* encodingContext);
  void setDecodingContext(DecodingGLContext* decodingContext);
  void cameraSelectMode(int mode);
  MeshInterchange* decode();
  MeshInterchange* encode();

  AbstractGLContext*    m_glContext;

protected:
  void paintGL();
  void resizeGL(int width, int height);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent* event);
};

#endif	// _ENCODING_OPEN_GL_WIDGET_H_
