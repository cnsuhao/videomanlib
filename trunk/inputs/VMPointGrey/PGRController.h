#pragma once
#include <windows.h>
#include <vector>
#include "VideoManInputController.h"
#include "controllers/IPointGreyController.h"
#include "PGRCamera.h"

class PGRController :
	public IPointGreyController
{
public:
	PGRController(const char *identifier);
	virtual ~PGRController(void);

	bool setInput( VideoInput * input );

	void startRecording();
	void stopRecording();
	int  getNumberOfCameras();	

	bool setRegister( size_t cameraNum, unsigned long reg, unsigned long value );
	bool setRegisterBroadcast( unsigned long reg, unsigned long value );

	static PGRController &getInstance();

	void SynchronizeCameras();

	void UnlockBuffers();

private:
	std::vector< PGRCamera* > cameraList;

	std::vector<FlyCaptureContext> contexts;

	
};
