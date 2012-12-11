#include "PGRController.h"
#include "PGRCamera.h"
#include "pgrerror.h"


PGRController::PGRController(const char *identifier) : IPointGreyController( identifier )
{
}

PGRController::~PGRController(void)
{
	cameraList.clear();
	contexts.clear();
}

bool PGRController::setInput( VideoInput * input )
{
	PGRCamera *camera = (PGRCamera*)input;
	cameraList.push_back( camera );
	return true;
}

void PGRController::startRecording()
{
	for ( size_t i=0; i<cameraList.size(); i++)
	{
		cameraList[i]->startGrabRecord();		
	}
}


void PGRController::stopRecording()
{
	/*for ( size_t i=0; i<cameraList.size(); i++)
	{
		cameraList[i]->stopGrabRecord();		
	}*/
}

int PGRController::getNumberOfCameras()
{
	return (int)cameraList.size();
}


bool PGRController::setRegister( size_t cameraNum, unsigned long reg, unsigned long value )
{
	if ( cameraNum < 0  ||  cameraNum >= (int)cameraList.size() )
		return false;
	return cameraList[cameraNum]->setRegister( reg, value );	
}

bool PGRController::setRegisterBroadcast( unsigned long reg, unsigned long value )
{	
	bool ret = false;
	for ( size_t i=0; i<cameraList.size(); i++)
	{
		ret = cameraList[i]->setRegisterBroadcast( reg, value );
	}
	return ret;
}


void PGRController::SynchronizeCameras()
{
	if ( cameraList.size() > 1 )
	{
		FlyCaptureError capError;
		UnlockBuffers();
		capError = flycaptureSyncForLockNext( &contexts[0], (int)contexts.size() );
		if( capError != FLYCAPTURE_OK )
		{
			std::string textError = "flycaptureSyncForLockNext";
			textError = textError + flycaptureErrorToString( capError );
			PGR_ERROR_MESSAGE( textError.c_str() );			
		}		
	}
}

void PGRController::UnlockBuffers()
{
	FlyCaptureError capError;
	for ( size_t i=0; i<cameraList.size(); i++)
	{
		capError = flycaptureUnlockAll( contexts[i] );
		if( capError != FLYCAPTURE_OK )
		{
			std::string textError = "flycaptureSyncForLockNext";
			textError = textError + flycaptureErrorToString( capError );
			PGR_ERROR_MESSAGE( textError.c_str() );
		}
	}
}