#include "CaptureDeviceDSController.h"
#include <string>
using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

CaptureDeviceDSController::CaptureDeviceDSController( const char *_identifier ) : ICaptureDeviceDSController( _identifier )
{
	propertiesMap[Brightness] = VideoProcAmp_Brightness;
	propertiesMap[Contrast] = VideoProcAmp_Contrast;
	propertiesMap[Hue] = VideoProcAmp_Hue;
	propertiesMap[Saturation] = VideoProcAmp_Saturation;
	propertiesMap[Gamma] = VideoProcAmp_Gamma;
	propertiesMap[ColorEnable] = VideoProcAmp_ColorEnable;
	propertiesMap[WhiteBalance] = VideoProcAmp_WhiteBalance;
	propertiesMap[BacklightCompensation] = VideoProcAmp_BacklightCompensation;
	propertiesMap[Gain] = VideoProcAmp_Gain;
	propertiesMap[Sharpness] = VideoProcAmp_Sharpness; 

	cameraControlsMap[Pan] = CameraControl_Pan;
	cameraControlsMap[Tilt] = CameraControl_Tilt;
	cameraControlsMap[Roll] = CameraControl_Roll;
	cameraControlsMap[Zoom] = CameraControl_Zoom;
	cameraControlsMap[Exposure] = CameraControl_Exposure;
	cameraControlsMap[Iris] = CameraControl_Iris;
	cameraControlsMap[Focus] = CameraControl_Focus;
}

CaptureDeviceDSController::~CaptureDeviceDSController(void)
{
}

bool CaptureDeviceDSController::setInput( VideoInput *input )
{
	string identifier = input->getIdentification().identifier;
	if ( identifier == "DSHOW_CAPTURE_DEVICE" )
	{
		captureDevice = (CaptureDeviceDShow*)input;
		if ( captureDevice ) 
			return true;		
	}
	return false;
}
