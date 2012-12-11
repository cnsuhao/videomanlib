#pragma once

#include "VideoManInputController.h"


#ifdef VIDEOMAN_EXPORTS
#define VIDEOMAN_API __declspec(dllexport)
#else
#define VIDEOMAN_API __declspec(dllimport)
#endif

class VIDEOMAN_API IPGRController 
	: public VideoManInputController
{
public:
	//virtual ~IPGRController() = 0;
	virtual void startRecording() = 0;
	virtual void stopRecording() = 0;
	virtual int  getNumberOfCameras() = 0;	
	virtual bool setRegister( size_t cameraNum, unsigned long reg, unsigned long value ) = 0;
	virtual bool setRegisterBroadcast( unsigned long reg, unsigned long value ) = 0;
	virtual void SynchronizeCameras() = 0;
	virtual void UnlockBuffers() = 0;
};
