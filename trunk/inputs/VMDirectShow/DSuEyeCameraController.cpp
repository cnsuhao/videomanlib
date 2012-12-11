#include "DSuEyeCameraController.h"
#include "DirectShowHelpers.h"
#include <string>
using namespace std;
using namespace VideoManPrivate;

DSuEyeCameraController::DSuEyeCameraController( const char *_identifier ) : IDSuEyeCameraController( _identifier )
{
	pUEye = NULL;
	pAutoFeatures = NULL;
	pPin = NULL;
}

DSuEyeCameraController::~DSuEyeCameraController(void)
{	
	SAFE_RELEASE( pUEye );
	SAFE_RELEASE( pAutoFeatures );
	SAFE_RELEASE( pPin );
}


bool DSuEyeCameraController::setInput( VideoInput *input )
{
	string identifier = input->getIdentification().identifier;
	if ( identifier == "DSHOW_CAPTURE_DEVICE" )
	{
		captureDevice = (CaptureDeviceDShow*)input;
		if ( !captureDevice )
			return false;
		IBaseFilter *videoSource;
		videoSource = captureDevice->getVideoSourceFilter();	
		if ( !videoSource )
			return false;
		videoSource->QueryInterface(IID_IuEyeCapture,  (void **)&pUEye);
		if ( !pUEye )
			return false;
		videoSource->QueryInterface(IID_IuEyeAutoFeatures,  (void **)&pAutoFeatures);
		if ( !pAutoFeatures )
		{
			SAFE_RELEASE( pUEye );
			return false;	
		}
		videoSource->QueryInterface(IID_IuEyeCapturePin,  (void **)&pPin);	
		if ( !pPin )
		{
			SAFE_RELEASE( pUEye );
			SAFE_RELEASE( pAutoFeatures );
			return false;
		}
		videoSource->QueryInterface( IID_IuEyeCaptureEx, (void **)&pUEyeEx );
		if ( !pUEyeEx )
		{
			SAFE_RELEASE( pUEye );
			SAFE_RELEASE( pAutoFeatures );
			SAFE_RELEASE( pPin );
			return false;
		}
		return true;
	}
	return false;
}

void DSuEyeCameraController::setExposureTime( LONG time )
{
	if ( pPin )
		pPin->SetExposureTime( time );
}

void DSuEyeCameraController::getExposureTime( long &val )
{
	if ( pPin )
		pPin->GetExposureTime( &val );
}

void DSuEyeCameraController::GetUsedBandwith( long &bandWith )
{
	if ( pPin )
		pPin->GetUsedBandwith( &bandWith );
}
	
void DSuEyeCameraController::GetPixelClock(long &plClock)
{
	if ( pPin )
		pPin->GetPixelClock( &plClock );
}

void DSuEyeCameraController::GetPixelClockRange(long &plMin, long &plMax, long &plDefault)
{
	if ( pPin )
		pPin->GetPixelClockRange( &plMin, &plMax, &plDefault );
}

void DSuEyeCameraController::SetPixelClock(long lClock)
{
	if ( pPin )
		pPin->SetPixelClock( lClock );
}

void DSuEyeCameraController::GetRGB8ColorMode(long &plMode)
{
	if ( pPin )
		pPin->GetRGB8ColorMode( &plMode );
}

void DSuEyeCameraController::SetRGB8ColorMode(long lMode)
{
	if ( pPin )
		pPin->SetRGB8ColorMode( lMode );
}

void DSuEyeCameraController::getExposureRange(long &min, long &max, long &interval)
{
	if ( pPin )
		pPin->GetExposureRange( &min, &max, &interval );
}

void DSuEyeCameraController::SaveParameters( const char* cszFileName )
{
	if( pUEyeEx )
		pUEyeEx->SaveParameters( cszFileName );
}

void DSuEyeCameraController::LoadParameters( const char* cszFileName )
{
	if( pUEyeEx )
		pUEyeEx->LoadParameters( cszFileName );
}

void DSuEyeCameraController::SetGainBoost( long lGainBoost)
{
	if( pUEyeEx )
		pUEyeEx->SetGainBoost( lGainBoost );
}
	
void DSuEyeCameraController::GetGainBoost(long &plGainBoost)
{
	if( pUEyeEx )
		pUEyeEx->GetGainBoost( &plGainBoost );
}
		
void DSuEyeCameraController::SetHardwareGamma(long lHWGamma)
{
	if( pUEyeEx )
		pUEyeEx->SetHardwareGamma( lHWGamma );
}

void DSuEyeCameraController::GetHardwareGamma(long &plHWGamma)
{
	if( pUEyeEx )
		pUEyeEx->GetHardwareGamma( &plHWGamma );
}

void DSuEyeCameraController::GetDLLVersion(long &pVersion)
{
	if ( pUEye )
		pUEye->GetDLLVersion( &pVersion );
}

void DSuEyeCameraController::SetBadPixelCorrection(long lEnable)
{
	if ( pUEye )
		pUEye->SetBadPixelCorrection( lEnable );
}

void DSuEyeCameraController::GetBadPixelCorrection(long &plEnable)
{
	if ( pUEye )
		pUEye->GetBadPixelCorrection( &plEnable );
}

void DSuEyeCameraController::LoadSettings(void)
{
	if ( pUEye )
		pUEye->LoadSettings();
}

void DSuEyeCameraController::SaveSettings(void)
{
	if ( pUEye )
		pUEye->SaveSettings();
}

void DSuEyeCameraController::ResetDefaults(void)
{
	if ( pUEye )
		pUEye->ResetDefaults();
}

void DSuEyeCameraController::SetAutoBrightnessReference(long lReference)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoBrightnessReference( lReference );
}
	
void DSuEyeCameraController::GetAutoBrightnessReference(long &plReference)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoBrightnessReference( &plReference );
}

void DSuEyeCameraController::SetAutoBrightnessMaxExposure(long lMaxExposure)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoBrightnessMaxExposure( lMaxExposure );
}

void DSuEyeCameraController::GetAutoBrightnessMaxExposure(long& plMaxExposure)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoBrightnessMaxExposure( &plMaxExposure );
}

void DSuEyeCameraController::SetAutoBrightnessMaxGain(long lMaxGain)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoBrightnessMaxGain( lMaxGain );
}

void DSuEyeCameraController::GetAutoBrightnessMaxGain(long& plMaxGain)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoBrightnessMaxGain( &plMaxGain );
}

void DSuEyeCameraController::SetAutoBrightnessSpeed(long lSpeed)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoBrightnessSpeed( lSpeed );
}

void DSuEyeCameraController::GetAutoBrightnessSpeed(long& plSpeed)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoBrightnessSpeed( &plSpeed );
}

void DSuEyeCameraController::SetAutoBrightnessAOI(long lXPos, long lYPos, long lWidth, long lHeight)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoBrightnessAOI( lXPos, lYPos, lWidth, lHeight );
}

void DSuEyeCameraController::GetAutoBrightnessAOI(long& plXPos, long& plYPos, long& plWidth, long& plHeight)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoBrightnessAOI( &plXPos, &plYPos, &plWidth, &plHeight );
}

void DSuEyeCameraController::SetAutoWBGainOffsets(long lRedOffset, long lBlueOffset)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoWBGainOffsets( lRedOffset, lBlueOffset );
}

void DSuEyeCameraController::GetAutoWBGainOffsets(long& plRedOffset, long& plBlueOffset)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoWBGainOffsets( &plRedOffset, &plBlueOffset );
}

void DSuEyeCameraController::SetAutoWBGainRange(long lMinRGBGain, long lMaxRGBGain)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoWBGainRange( lMinRGBGain, lMaxRGBGain );
}

void DSuEyeCameraController::GetAutoWBGainRange(long& plMinRGBGain, long& plMaxRGBGain)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoWBGainRange( &plMinRGBGain, &plMaxRGBGain );
}

void DSuEyeCameraController::SetAutoWBSpeed(long lSpeed)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoWBSpeed( lSpeed );
}

void DSuEyeCameraController::GetAutoWBSpeed(long& plSpeed)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoWBSpeed( &plSpeed );
}

void DSuEyeCameraController::SetAutoWBAOI(long lXPos, long lYPos, long lWidth, long lHeight)
{
	if ( pAutoFeatures )
		pAutoFeatures->SetAutoWBAOI( lXPos, lYPos, lWidth, lHeight );
}

void DSuEyeCameraController::GetAutoWBAOI(long& plXPos, long& plYPos, long& plWidth, long& plHeight)
{
	if ( pAutoFeatures )
		pAutoFeatures->GetAutoWBAOI( &plXPos, &plYPos, &plWidth, &plHeight );
}