#include <iostream>
#include "PointGreyController.h"
//#include "FlyCapture2.h"

//using namespace FlyCapture2;

/*void PrintInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

void PrintError2( Error error )
{
    printf( "%s\n", error.GetDescription() );
}*/

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

PointGreyController::PointGreyController(const char *identifier) : IPointGreyController( identifier )
{
}

PointGreyController::~PointGreyController(void)
{
	cameraList.clear();	
}

bool PointGreyController::setInput( VideoInput * input )
{
	PGRCamera *camera = (PGRCamera*)input;
	cameraList.push_back( camera );
	return true;
}

void PointGreyController::startRecording()
{
	/*for ( size_t i=0; i<cameraList.size(); i++)
	{
		cameraList[i]->startGrabRecord();		
	}*/
}


void PointGreyController::stopRecording()
{
	/*for ( size_t i=0; i<cameraList.size(); i++)
	{
		cameraList[i]->stopGrabRecord();		
	}*/
}

int PointGreyController::getNumberOfCameras()
{
	return (int)cameraList.size();
}


bool PointGreyController::setRegister( unsigned int reg, unsigned int value )
{
	if ( !cameraList[0]->setRegister( reg, value ) )
	{
		printf( "Error setting register %i", reg );	
		return false;
	}
	return true;
}

bool PointGreyController::setRegisterBroadcast( unsigned long reg, unsigned long value )
{	
	if ( !cameraList[0]->setRegisterBroadcast( reg, value ) )
	{
		printf( "Error setting register %i", reg );
		return false;
	}
	return true;
}


void PointGreyController::SynchronizeCameras()
{
/*	if ( cameraList.size() > 1 )
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
	}*/
}

void PointGreyController::UnlockBuffers()
{
/*	FlyCaptureError capError;
	for ( size_t i=0; i<cameraList.size(); i++)
	{
		capError = flycaptureUnlockAll( contexts[i] );
		if( capError != FLYCAPTURE_OK )
		{
			std::string textError = "flycaptureSyncForLockNext";
			textError = textError + flycaptureErrorToString( capError );
			PGR_ERROR_MESSAGE( textError.c_str() );
		}
	}*/
}

void PointGreyController::printCameraInfo()
{
	cameraList[0]->printInfo();
}

bool PointGreyController::setGainControl(bool autoGain)
{
	return cameraList[0]->setGainControl( autoGain );	
}

bool PointGreyController::getGainControl()
{
	return cameraList[0]->getGainControl();
}

bool PointGreyController::setExposureControl( bool autoExp )
{
	return cameraList[0]->setExposureControl( autoExp );
}

bool PointGreyController::setSharpnessControlValue(float value)
{
	return cameraList[0]->setSharpnessControlValue( value );
}

bool PointGreyController::setSharpnessControl( bool autpoSharp )
{
	return cameraList[0]->setSharpnessControl( autpoSharp );
}

bool PointGreyController::setShutterTime( float shutterTime )
{
	return cameraList[0]->setShutterTime( shutterTime );
}

bool PointGreyController::moveImageROI( int x, int y )
{
	return cameraList[0]->moveImageROI( x, y );
}

bool PointGreyController::setImageROI( int x, int y, int width, int height, int videoMode  )
{
	return cameraList[0]->setImageROI( x, y, width, height, videoMode );
}

bool PointGreyController::getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit, int videoMode )
{
	return cameraList[0]->getROIUnits( Hmax, Vmax, Hunit, Vunit, HPosUnit, VPosUnit, videoMode );
}

bool PointGreyController::resetImageROI()
{
	return cameraList[0]->resetImageROI();
}

bool PointGreyController::setTrigger( bool triggerOn, int source, int mode )
{
	cout << "PointGreyController::setTrigger not implemented" << endl;
	return false;
}
	
bool PointGreyController::fireSoftwareTrigger( bool broadcast )
{
	cout << "PointGreyController::fireSoftwareTrigger not implemented" << endl;
	return false;
}

bool PointGreyController::getBayerFormat(size_t &btf)
{
	cout << "PointGreyController::getBayerFormat not implemented" << endl;
	return false;
}

bool PointGreyController::setFrameRate( float frameRate )
{
	cout << "PointGreyController::setFrameRate not implemented" << endl;
	return false;
}