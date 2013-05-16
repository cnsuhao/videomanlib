#pragma once
#include "VideoManInputController.h"

/** \brief Advanced controller for capture devices using DirectShow
\par Demo Code:
\code
	videoMan.getAvailableDevices( "DSHOW_CAPTURE_DEVICE", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format );
	...
	ICaptureDeviceDSController *controller = (ICaptureDeviceDSController*)videoMan.createController( inputID, "DSHOW_CAPTURE_DEVICE_CONTROLLER" );
	if ( controller )
	{
		if (controller->supportImageProperties() )
			controller->setImageProperty( ICaptureDeviceDSController::ImageProperty::Brightness, 150 );
		if ( controller->supportCameraControl() )
			controller->setCameraControl( ICaptureDeviceDSController::CameraControl::Exposure, 150 );
	}
	...
	videoMan.deleteController( &controller ); 
\endcode
*/
class ICaptureDeviceDSController : public VideoManPrivate::VideoManInputController
{
public:

	/** \brief Image properties
	*/
	enum ImageProperty
	{
		Brightness, /** Brightness image property */
		Contrast, /** Contrast image property */
		Hue, /** Hue image property */
		Saturation, /** Saturation image property */
		Sharpness, /** Sharpness image property */
		Gamma, /** Gamma image property */
		ColorEnable, /** ColorEnable image property */
		WhiteBalance, /** WhiteBlance image property */
		BacklightCompensation, /** BacklightCompensation image property */
		Gain, /** Gain image property */
	};

	/** \brief Camera controls
	*/
	enum CameraControl
	{
		Pan, /** Pan camera control */
		Tilt, /** Tilt camera control */
		Roll, /** Roll camera control */
		Zoom, /** Zoom camera control */
		Exposure, /** Exposure camera control */
		Iris, /** Iris camera control */
		Focus /** Focus camera control */
	};

	ICaptureDeviceDSController(const char *_identifier) : VideoManPrivate::VideoManInputController( _identifier )
	{}
	virtual ~ICaptureDeviceDSController(void){};

	//! \name Crossbar Control for frame grabbers
	//@{

	/** \brief Get the numberl of channels
		\return the numberl of channels
	*/
	virtual int getInputChannelsCount() = 0;
	/** \brief Change the input channel
		\param channel [in] the channel index
		\return true if the channel was changed
	*/
	virtual bool setInputChannel( int channel ) = 0;
	//@}

	//! \name Image properties
	//@{
	/** \brief Check if the input supports image properties adjustments
		\return true if the input supports image properties adjustments
	*/
	virtual bool supportImageProperties() = 0;

	/** \brief change an image property
		\param imageProp [in] the image property
		\param value [in] the property value 
		\param flagAuto [in] The setting must be controlled manually or automatically
	*/
	virtual void setImageProperty( ImageProperty imageProp, const long &value, const bool &flagAuto ) = 0;	
	
	/** \brief Get an image property value
		\param imageProp [in] the image property
		\param value [out] the property value 
		\param flagAuto [out] The setting is controlled manually or automatically
		\return true If the device supports the image property
	*/
	virtual bool getImageProperty( ImageProperty imageProp, long &value, bool &flagAuto ) = 0;	

	/** \brief Get an image property range
		\param imageProp [in] the image property
		\param min [out] the image property minimum value 
		\param max [out] the image property maximum value 
		\param steppingDelta [out] smallest increment by which the property can change
		\param defaultVal [out] default value
		\param flagAuto [out] The setting is controlled manually or automatically
		\return true If the device supports the image property
	*/
	virtual bool getImagePropertyRange( ImageProperty imageProp, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto ) = 0;	
	//@}

	//! \name Camera control
	//@{
	/** \brief Check if the input supports camera control adjustments
		\return true if the input supports camera control adjustments
	*/
	virtual bool supportCameraControl() = 0;

	/** \brief change a camera control setting
		\param control [in] the camera control setting 
		\param value [in] the setting value 
		\param flagAuto [in] The setting must be controlled manually or automatically
	*/
	virtual void setCameraControl( CameraControl control, const long &value, const bool &flagAuto ) = 0;	
	
	/** \brief Get a camera control setting value
		\param control [in] the camera control setting
		\param value [out] the property value 
		\param flagAuto [out] The returned value indicates whether the setting is controlled manually or automatically (IAMVideoProcAmp)		
		\return true If the device supports the camera control property
	*/
	virtual bool getCameraControl( CameraControl control, long &value, bool &flagAuto ) = 0;	
	
	/** \brief Get a camera control setting range values
		\param control [in] the camera control setting
		\param min [out] the setting minimum value 
		\param max [out] the setting maximum value 
		\param steppingDelta [out] smallest increment by which the setting can change
		\param defaultVal [out] default value
		\param flagAuto [out] The setting is controlled manually or automatically
		\return true If the device supports the camera control property
	*/
	virtual bool getCameraControlRange( CameraControl control, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto ) = 0;	
	//@}

	//! \name Capture Dialogs
	//@{
	/** \brief Show Video display dialog box that might exist in a Video for Windows capture driver
	*/
	virtual void showCaptureDialogDisplay() = 0;
	/** \brief Show Video source dialog box that might exist in a Video for Windows capture driver
	*/
	virtual void showCaptureDialogSource() = 0;
	/** \brief Show Video format dialog box that might exist in a Video for Windows capture driver
	*/
	virtual void showCaptureDialogFormat() = 0;
	//@}
};
