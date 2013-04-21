#pragma once
#include "VideoManInputController.h"

/** \brief Advanced controller for IDS uEye cameras running with VMDirectShow
\par Demo Code:
\code
	videoMan.getAvailableDevices( "DSHOW_CAPTURE_DEVICE", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format ); //Suppose list[0] is a uEye camera
	...
	IDSuEyeCameraController *controller = (IDSuEyeCameraController.h*)videoMan.createController( inputID, "DSHOW_uEYE_CAMERA_CONTROLLER" );
	if ( controller )			
	{
		controller->SetExposureTime( 100 );				
	}
	...
	videoMan.deleteController( &controller );
\endcode
*/
class IDSuEyeCameraController :public VideoManPrivate::VideoManInputController
{
public:
	IDSuEyeCameraController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IDSuEyeCameraController(void){};

	/** \brief Set the exposure time of the camera
		\param time [in] exposure time in us
	*/
	virtual void setExposureTime( long time ) = 0;

	/** \brief Get the current possible min, max and interval for exposure time.		
	 	\param 	min [out] Receives the minimum possible exposure time.
		\param 	max [out] Receives the minimum possible exposure time.
     	\param 	interval [out] Receives the current possible step width.     
	*/
	virtual void getExposureRange(long &min, long &max, long &interval) = 0;

	/** \brief Get the current exposure time.		
	 	\param 	val [out] Receives the exposure time.		
	*/
	virtual void getExposureTime( long &val ) = 0;

	/** \brief Returns the used bandwith
		\param 	bandWidth [out] Receives the used bandwitdh
	 */
	virtual void GetUsedBandwith(long &bandWidth) = 0;

	/** \brief the pixelclock for the connected camera.
		\param 	plClock     Receives the current pixel clock.
	*/
	virtual void GetPixelClock(long &plClock) = 0;

	/** \brief Returns the min, max and default value for the pixelclock.
		\param 	plMin       Receives the minimum possible pixel clock.
		\param 	plMax       Receives the maximum possible pixel clock.
		\param 	plDefault   Receives the default pixel clock value.
	*/
	virtual void GetPixelClockRange(long &plMin, long &plMax, long &plDefault) = 0;

	/** \brief Sets the Pixelclock (in MHz) for the connected device.
		\param 	lClock      The pixel clock in MHz to set.
	*/
	virtual void SetPixelClock(long lClock) = 0;

	/** \brief Queries which color mode to use when RGB8 mediatype is selected.
		\param 	plMode      Receives the currently selected RGB8 colormode.
	*/
	virtual void GetRGB8ColorMode(long &plMode) = 0;

	/** \brief Determines which color mode to use when RGB8 mediatype is selected.
		Specifies whether Y8 or raw bayer pattern is used with RGB8 mode
		possible values are 11 for raw bayer pattern (on bayer color
		cameras), or 6 for monochrome images.
		\param 	lMode       Specifies the color mode used for RGB8 mode.
	*/
	virtual void SetRGB8ColorMode(long lMode) = 0;

	/** \brief Stores the current parameters to file or camera EEPRom userset.
		\param cszFileName Filename or EEPRom userset to store parameters to
	*/
	virtual void SaveParameters( const char* cszFileName ) = 0;
	
	/** \brief Load the parameters from a file or camera EEPRom userset.
		\param cszFileName Filename or EEPRom userset to store parameters to
	*/
	virtual void LoadParameters( const char* cszFileName ) = 0;
	
	/** \brief Activates or deactivates the additional gain amplification
		\param  lGainBoost Set to 1 to activate or 0 to deactivate extra amplification.
	*/
	virtual void SetGainBoost( long lGainBoost) = 0;

	/** \brief Queries the current state of the extra amplification.
		\param  plGainBoost Receives 1 if extra amplification is enabled.
    */
	virtual void GetGainBoost(long &plGainBoost) = 0;
	
	/** \brief Activates or deactivates the hardware gamma.
		\param  lHWGamma    Set to 1 to activate or 0 to deactivate hw gamma.
    */
	virtual void SetHardwareGamma(long lHWGamma) = 0;

	/** \brief Queries the current state of hardware gamma.
		\param  plHWGamma    Receives 1 if hw gamma is enabled.
    */
	virtual void GetHardwareGamma(long &plHWGamma) = 0;
	  	
	/** \brief Queries the pVersion of the installed uEye Driver files
		\param 	pVersion    Receives the pVersion of connected cameras.
		\note   This is not the pVersion of the uEye capture device filter but
		pVersion of the underlying driver files.
	*/
	virtual void GetDLLVersion(long &pVersion) = 0;

	/** \brief Activates or deactivates the hot pixel correction.
		\param 	lEnable     Set to 1 to activate or 0 to deactivate correction.
	*/
	virtual void SetBadPixelCorrection(long lEnable) = 0;

	/** \brief Queries the current state of the hot pixel correction unit.
		\param 	plEnable    Receives 1 if hot pixel correction is enabled.
	*/
	virtual void GetBadPixelCorrection(long &plEnable) = 0;

	/** \brief Loads previous stored camera settings
	*/
	virtual void LoadSettings(void) = 0;

	/** \brief Stores the current set camera settings in the registry.
			Data will be stored individual for each uEye UI model (e.g. UI1410-C)
	*/
	virtual void SaveSettings(void) = 0;

	/** \brief Resets the camera parameters to the driver defaults.
		\note   You may not be able to reset parameters while the filter is 
				connected or running.
	*/
	virtual void ResetDefaults(void) = 0;

	/** \brief Specifies the brightness reference value which should be achieved by auto gain and auto exposure.
		\param  lReference  The reference value the controller should reach.
	*/
	virtual void SetAutoBrightnessReference(long lReference) = 0;

	/** \brief Queries the actual set reference value for auto brightness control.
		\param  plReference Receives the current value for reference.
	*/
	virtual void GetAutoBrightnessReference(long &plReference) = 0;

	/** \brief Upper limit of the exposure time when used to control the image brightness automatically.  
		\param  lMaxExposure Maximum exposure time (in us Units) the controller is allowed to set.
	*/
	virtual void SetAutoBrightnessMaxExposure(long lMaxExposure) = 0;

	/** \brief Queries the actual set upper limit of automatic controlled exposure time.
		\param  plMaxExposure Receives the currently allowed maximum exposure time (us Units)
	*/
	virtual void GetAutoBrightnessMaxExposure(long& plMaxExposure) = 0;

	/** \brief Upper limit of gain when used to control the image brightness automatically.
		\param  lMaxGain    Maximum master gain value the controller is allowed to set.
	*/
	virtual void SetAutoBrightnessMaxGain(long lMaxGain) = 0;

	/** \brief Queries the actual set upper limit of automatic controlled master gain amplifier. 
		\param  plMaxGain   Receives the currently allowed maximum gain value.
	*/
	virtual void GetAutoBrightnessMaxGain(long& plMaxGain) = 0;

	/** \brief Controls the percentage of examined images for the automatic brightness control unit.
		\param  lSpeed      The desired speed in a range of 1%(slow) to 100%(fast).
	*/
	virtual void SetAutoBrightnessSpeed(long lSpeed) = 0;

	/** \brief Queries the actual set rate at which image brightness is examined. 
		\param  plSpeed     Receives the currently set examination speed.
	*/
	virtual void GetAutoBrightnessSpeed(long& plSpeed) = 0;

	/** \brief Specifies the area of interest within the image in which the brightness should be examined.
		\param  lXPos       Left bound of the area of interest.
		\param  lYPos       Upper bound of the area of interest.
		\param  lWidth      Width of the area of interest.
		\param  lHeight     Height of the area of interest.
	*/
	virtual void SetAutoBrightnessAOI(long lXPos, long lYPos, long lWidth, long lHeight) = 0;

	/** \brief Queries the actual used area of interest in which the brightness is examined.
		\param  plXPos      Receives the left bound.
		\param  plYPos      Receives the upper bound.
		\param  plWidth     Receives the width.
		\param  plHeight    Receives the height.
	*/
	virtual void GetAutoBrightnessAOI(long& plXPos, long& plYPos, long& plWidth, long& plHeight) = 0;

	/** \brief Specifies relative offsets between the individual color channels when used by the automatic whitebalance control unit.
		\param  lRedOffset  Offset for the red gain channel relative to the green one.
		\param  lBlueOffset Offset for the blue gain channel relative to the green one.
	*/
	virtual void SetAutoWBGainOffsets(long lRedOffset, long lBlueOffset) = 0;

	/** \brief Queries the actual set color channel offsets for automatic whitebalance.
		\param  plRedOffset  Receives the red to green channel offset.
		\param  plBlueOffset  Receives the blue to green channel offset.
	*/
	virtual void GetAutoWBGainOffsets(long& plRedOffset, long& plBlueOffset) = 0;

	/** \brief Limits the range the automatic whitebalance controller unit is allowed to use when adjusting the RGB gain channels.
		\param  lMinRGBGain  Minimum allowed gain value.
		\param  lMaxRGBGain  Maximum allowed gain value.
	*/
	virtual void SetAutoWBGainRange(long lMinRGBGain, long lMaxRGBGain) = 0;

	/** \brief Queries the actual allowed gain range for the automatic whitebalance controller unit.
		\param  plMinRGBGain  Receives the currently allowed minimal gain value.
		\param  plMaxRGBGain  Receives the currently allowed maximal gain value.
	*/
	virtual void GetAutoWBGainRange(long& plMinRGBGain, long& plMaxRGBGain) = 0;

	/** \brief Controls the percentage of examined images for the automatic whitebalance control unit.
		\param  lSpeed      The desired speed in a range of 1%(slow) to 100%(fast).
	*/
	virtual void SetAutoWBSpeed(long lSpeed) = 0;

	/** \brief Queries the actual set rate at which the images whitebalance is examined. 
		\param  plSpeed     Receives the currently set examination speed.
	*/
	virtual void GetAutoWBSpeed(long& plSpeed) = 0;

	/** \brief Specifies the area of interest within the image in which the whitebalance should be examined.
		\param  lXPos       Left bound of the area of interest.
		\param  lYPos       Upper bound of the area of interest.
		\param  lWidth      Width of the area of interest.
		\param  lHeight     Height of the area of interest.
	*/
	virtual void SetAutoWBAOI(long lXPos, long lYPos, long lWidth, long lHeight) = 0;

	/** \brief Queries the actual used area of interest in which the whitebalance is examined.
		\param  plXPos      Receives the left bound.
		\param  plYPos      Receives the upper bound.
		\param  plWidth     Receives the width.
		\param  plHeight    Receives the height.
	*/
	virtual void GetAutoWBAOI(long& plXPos, long& plYPos, long& plWidth, long& plHeight) = 0;

};
