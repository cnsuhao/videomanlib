#pragma once
#include "VideoManInputController.h"

/** \brief Advanced Controller for capture devices using DirectShow
\par Demo Code:
\code
	ICaptureDeviceDSController *controller = (ICaptureDeviceDSController*)videoMan.createController( inputID, "DSHOW_CAPTURE_DEVICE_CONTROLLER" );
	if ( controller && controller->supportImageProperties() )
		controller->setImageProperty( ICaptureDeviceDSController::ImageProperty::Brightness, 150 );
	if ( controller && controller->supportCameraControl() )
		controller->setCameraControl( ICaptureDeviceDSController::CameraControl::Exposure, 150 );
	...
	videoMan.deleteController( &controller ); //not required
\endcode
*/
class ICaptureDeviceDSController : public VideoManPrivate::VideoManInputController
{
public:

	enum ImageProperty
	{
		Brightness,
		Contrast,
		Hue,
		Saturation,
		Sharpness,
		Gamma,
		ColorEnable,
		WhiteBalance,
		BacklightCompensation,
		Gain,
	};

	enum CameraControl
	{
		Pan,
		Tilt,
		Roll,
		Zoom,
		Exposure,
		Iris,
		Focus
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
		\param ImageProperty [in] the image property
		\param value [in] the property value 
		\param flagAuto [in] The setting must be controlled manually or automatically
	*/
	virtual void setImageProperty( ImageProperty, const long &value, const bool &flagAuto ) = 0;	
	
	/** \brief Get an image property value
		\param ImageProperty [in] the image property
		\param value [out] the property value 
		\param flagAuto [out] The setting is controlled manually or automatically
		\return true If the device supports the image property
	*/
	virtual bool getImageProperty( ImageProperty imageProp, long &value, bool &flagAuto ) = 0;	

	/** \brief Get an image property range
		\param ImageProperty [in] the image property
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
		\param CameraControl [in] the camera control setting 
		\param value [in] the setting value 
		\param flagAuto [in] The setting must be controlled manually or automatically
	*/
	virtual void setCameraControl( CameraControl, const long &value, const bool &flagAuto ) = 0;	
	
	/** \brief Get a camera control setting value
		\param CameraControl [in] the camera control setting
		\param value [out] the property value 
		\param flags [out] The returned value indicates whether the setting is controlled manually or automatically (IAMVideoProcAmp)
		\param flagAuto [out] The setting is controlled manually or automatically
		\return true If the device supports the camera control property
	*/
	virtual bool getCameraControl( CameraControl, long &value, bool &flagAuto ) = 0;	
	
	/** \brief Get a camera control setting range values
		\param CameraControl [in] the camera control setting
		\param min [out] the setting minimum value 
		\param max [out] the setting maximum value 
		\param steppingDelta [out] smallest increment by which the setting can change
		\param defaultVal [out] default value
		\param flagAuto [out] The setting is controlled manually or automatically
		\return true If the device supports the camera control property
	*/
	virtual bool getCameraControlRange( CameraControl, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto ) = 0;	
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
