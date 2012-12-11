#ifndef DSUEYECAMERACONTROLLER_H
#define DSUEYECAMERACONTROLLER_H

#include "controllers/IDSuEyeCameraController.h"
#include "CaptureDeviceDShow.h"
#include "uEyeCaptureInterface.h"

class DSuEyeCameraController :
	public  IDSuEyeCameraController
{
public:
	DSuEyeCameraController( const char *_identifier );
	virtual ~DSuEyeCameraController(void);

	bool setInput( VideoManPrivate::VideoInput *input );

	//pPin methods
	void setExposureTime( LONG time );	

	void getExposureTime( long &val );

	void GetUsedBandwith(long &bandWidth);
	
	void GetPixelClock(long &plClock);

	void GetPixelClockRange(long &plMin, long &plMax, long &plDefault);

	void SetPixelClock(long lClock);

	void GetRGB8ColorMode(long &plMode);

	void SetRGB8ColorMode(long lMode);

	//pUEye Methods

	void getExposureRange(long &min, long &max, long &iInterval);

	//pAutoFeatures methods

	//pUEyeEx methods

	void SaveParameters( const char* cszFileName );
	
	void LoadParameters( const char* cszFileName );

	void SetGainBoost( long lGainBoost);
	
	void GetGainBoost(long &plGainBoost);
		
	void SetHardwareGamma(long lHWGamma);

	void GetHardwareGamma(long &plHWGamma);

	void GetDeviceInfo(SENSORINFO &psInfo, CAMERAINFO &pcInfo);
	
	void GetDLLVersion(long &pVersion);
	
	void SetBadPixelCorrection(long lEnable);
	
	void GetBadPixelCorrection(long &plEnable);

	void LoadSettings(void);

	void SaveSettings(void);

	void ResetDefaults(void);
	
	void SetAutoBrightnessReference(long lReference);
	
	void GetAutoBrightnessReference(long &plReference);

	void SetAutoBrightnessMaxExposure(long lMaxExposure);

	void GetAutoBrightnessMaxExposure(long& plMaxExposure);

	void SetAutoBrightnessMaxGain(long lMaxGain);

	void GetAutoBrightnessMaxGain(long& plMaxGain);

	void SetAutoBrightnessSpeed(long lSpeed);

	void GetAutoBrightnessSpeed(long& plSpeed);

	void SetAutoBrightnessAOI(long lXPos, long lYPos, long lWidth, long lHeight);

	void GetAutoBrightnessAOI(long& plXPos, long& plYPos, long& plWidth, long& plHeight);

	void SetAutoWBGainOffsets(long lRedOffset, long lBlueOffset);

	void GetAutoWBGainOffsets(long& plRedOffset, long& plBlueOffset);

	void SetAutoWBGainRange(long lMinRGBGain, long lMaxRGBGain);

	void GetAutoWBGainRange(long& plMinRGBGain, long& plMaxRGBGain);

	void SetAutoWBSpeed(long lSpeed);

	void GetAutoWBSpeed(long& plSpeed);

	void SetAutoWBAOI(long lXPos, long lYPos, long lWidth, long lHeight);

	void GetAutoWBAOI(long& plXPos, long& plYPos, long& plWidth, long& plHeight);
			
protected:	
	IuEyeCapture *pUEye;
	IuEyeCaptureEx *pUEyeEx;
	IuEyeAutoFeatures *pAutoFeatures;
	IuEyeCapturePin *pPin;	
	CaptureDeviceDShow *captureDevice;
};
#endif
