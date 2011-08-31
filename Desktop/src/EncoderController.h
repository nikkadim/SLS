/*
 Filename:		EncoderController.h
 Author:		Nikolaus Karpinsky
 Date Created:	03/23/11
 Last Edited:	03/23/11
 
 Revision Log:
 03/23/11 - Nik Karpinsky - Original creation.
 */

#ifndef _ENCODER_CONTROLLER_H_
#define _ENCODER_CONTROLLER_H_

//	Qt auto generated headers
#include "ui_Encode.h"

#include <QWidget>

#include "OpenGLWidget.h"
#include "Holoencoder.h"
#include "XYZFileIO.h"

#include "Codecs/DepthCodec.h"
#include "Codecs/MultiWavelengthCodec.h"
#include "MultiWavelengthCapture.h"
#include "Holodecoder.h"

#include "VideoIO.h"

class EncoderController : public QWidget, private Ui::Encode
{
  Q_OBJECT

private:
  Holoencoder m_encoder;

  //MultiWavelengthCapture m_decoder;

public:
  EncoderController(QWidget* parent = 0);
  ~EncoderController();

  void exportCurrentFrame(void);
  void exportEntireVideo(QListWidget* fileList);
  void selectXYZM(const string &filename);

  void init(void);

public slots:
  void encode(void);
  void selectSourceFile(void);
  void selectDestinationFile(void);

private:
  void _updateGL(void);
  void _connectSignalsWithController(void);
};

#endif	// _ENCODER_CONTROLLER_H_
