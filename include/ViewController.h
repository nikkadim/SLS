/*
 Filename:		CameraController.h
 Author:		Nikolaus Karpinsky
 Date Created:	03/23/11
 Last Edited:	03/23/11
 
 Revision Log:
 03/23/11 - Nik Karpinsky - Original creation.
 */

#ifndef _VIEW_CONTROLLER_H_
#define _VIEW_CONTROLLER_H_

//	Qt auto generated headers
#include "ui_View.h"

#include <QWidget>
#include <QtCore/QTimer>

#include "OpenGLWidget.h"

#include "VideoIO.h"

#include <reactor/MediaFrame.h>
#include <reactor/VideoFileReader.h>
#include <reactor/FormatConverter.h>
#include <reactor/MediaFrameReader.h>
#include <reactor/ColorSpaceReaderFilter.h>
#include <reactor/DeplanarReaderFilter.h>

class ViewController : public QWidget, private Ui::View
{
  Q_OBJECT

private:
  Holodecoder m_decoder;
  QTime m_movieTimer;
  VideoIO m_aviIO;

  unique_ptr<reactor::MediaFrameReader> m_reader;

public:
  ViewController(QWidget* parent = 0);
 
  void init(void);
  void cameraSelectMode(int mode);
  void playMovie(string movieFile);
  void openHoloImage(void);

  public slots:
	void changeMoviePosition(int position);
	void pauseMovie(void);

protected:
  void timerEvent(QTimerEvent* event);

private:
  void _updateGL(void);
  void _connectSignalsWithController(void);
};

#endif	// _VIEW_CONTROLLER_H_
