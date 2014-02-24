#include "PointGreyController.h"
#include "FlyCapture2Defs.h"
#include "Error.h"

using namespace FlyCapture2;
using namespace VideoMan;
using namespace VideoManPrivate;

void PrintInfo( CameraInfo* pCamInfo )
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
}

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
	Error error = cameraList[0]->m_camera->WriteRegister( reg, value ); 	
    if ( error != PGRERROR_OK )
		printf( "%s\n", error.GetDescription() );
	return false;
}

bool PointGreyController::setRegisterBroadcast( unsigned long reg, unsigned long value )
{	
	/*bool ret = false;
	for ( size_t i=0; i<cameraList.size(); i++)
	{
		ret = cameraList[i]->setRegisterBroadcast( reg, value );
	}
	return ret;*/
	return false;
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
	CameraInfo camInfo;
	Error error = cameraList[0]->m_camera->GetCameraInfo(&camInfo);
    if (error == PGRERROR_OK)    
		PrintInfo(&camInfo);
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

bool PointGreyController::setShutterControl( bool autoShutter )
{
	return cameraList[0]->setShutterControl( autoShutter );
}

bool PointGreyController::setFrameRate( float frameRate )
{
	return cameraList[0]->setFrameRate( frameRate );
}

bool PointGreyController::moveImageROI( int x, int y )
{
	return cameraList[0]->moveImageROI( x, y );
}

bool PointGreyController::setImageROI( int x, int y, int width, int height )
{
	return cameraList[0]->setImageROI( x, y, width, height );
}
bool PointGreyController::getImageROI( int &x, int &y, int &width, int &height )
{
	cameraList[0]->getImageRoi( x, y, width, height );
	int w,h;
	w = cameraList[0]->getVideoManInputFormat().width;
	h = cameraList[0]->getVideoManInputFormat().height;
	return (width!=w || height!=h);
}

bool PointGreyController::getROIUnits( int &Hmax, int &Vmax, int &imageHStepSize, int &imageVStepSize, int &offsetHStepSize, int &offsetVStepSize )
{
	return cameraList[0]->getROIUnits( Hmax, Vmax, imageHStepSize, imageVStepSize, offsetHStepSize, offsetVStepSize );
}

bool PointGreyController::resetImageROI()
{
	return cameraList[0]->resetImageROI();
}

bool PointGreyController::setTrigger( bool triggerOn, int source, int mode )
{
	return cameraList[0]->setTrigger( triggerOn, source, mode );
}

bool PointGreyController::fireSoftwareTrigger( bool broadcast )
{
	return cameraList[0]->fireSoftwareTrigger( broadcast );
}

void PointGreyController::convertRawToColor( const unsigned char* imgSrc, unsigned char *imgDst, int width, int height )
{
	FlyCapture2::Image rawImage;
	rawImage.SetData( imgSrc, width * height );
	rawImage.SetDimensions( height, width, width, PIXEL_FORMAT_RAW8, cameraList[0]->m_bayerTileFormat );
	rawImage.SetColorProcessing( NEAREST_NEIGHBOR);//EDGE_SENSING );
	
	FlyCapture2::Image rgbImage;
	rgbImage.SetData( imgDst, width * height * 3 );
	rgbImage.SetDimensions( height, width, width * 3, PIXEL_FORMAT_BGR, NONE );
	rawImage.Convert( PIXEL_FORMAT_BGR, &rgbImage );

}

bool PointGreyController::getBayerFormat(size_t &btf)
{
	btf = cameraList[0]->m_bayerTileFormat;
	return true;
}

bool PointGreyController::setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source )
{
	return cameraList[0]->setStrobeOutput( onOff, delay, duration, polarity, source );
}