#ifndef CAPTUREDEVICEDSCONTROLLER_H
#define CAPTUREDEVICEDSCONTROLLER_H

#include "controllers/ICaptureDeviceDSController.h"
#include "VideoInput.h"
#include <map>
#include <assert.h>


#include "CaptureDeviceDShow.h"

/** \brief Controller for capture devices using DirectShow
\par Demo Code:

\code
	ICaptureDeviceDSController *controller = (ICaptureDeviceDSController*)videoMan.createController( "DSHOW_CAPTURE_DEVICE_CONTROLLER" );
	if ( controller && videoMan.linkController( inputID, controller ) )			
		controller->setImageProperty( ICaptureDeviceDSController::Brightness, 150 );
\endcode
*/
class CaptureDeviceDSController :
	public ICaptureDeviceDSController
{

	friend class CaptureDeviceDShow;

public:
	CaptureDeviceDSController( const char *identifier );
	virtual ~CaptureDeviceDSController(void);
		
	//! \name Crossbar Control for frame grabbers
	//@{

	/** \brief Get the numberl of channels
		\return the number of channels
	*/
	int getInputChannelsCount()
	{
		return captureDevice->getInputChannelsCount();
	}	

	/** \brief Change the input channel
		\param channel [in] the channel index
		\return true if the channel was changed
	*/
	bool setInputChannel( int channel )
	{
		return captureDevice->setInputChannel( channel );
	}
	//@}

	//! \name Image properties
	//@{

	/** \brief Check if the input supports image properties adjustments
		\return true if the input supports image properties adjustments
	*/
	 bool supportImageProperties()
	 {
		 return captureDevice->supportImageProperties();
	 }

	void setImageProperty( ImageProperty imageProp, const long &value, const bool &flagAuto )
	{
		assert( propertiesMap.find(imageProp) != propertiesMap.end() && "setImageProperty: ImageProperty unknown" );
		captureDevice->setImageProperty( propertiesMap[imageProp], value, flagAuto );
	}

	bool getImageProperty( ImageProperty imageProp, long &value, bool &flagAuto )
	{
		assert( propertiesMap.find(imageProp) != propertiesMap.end() && "getImageProperty: ImageProperty unknown" );
		return captureDevice->getImageProperty( propertiesMap[imageProp], value, flagAuto );
	}

	bool getImagePropertyRange( ImageProperty imageProp, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto )
	{
		assert( propertiesMap.find(imageProp) != propertiesMap.end() && "getImagePropertyRange: ImageProperty unknown" );
		return captureDevice->getImagePropertyRange( propertiesMap[imageProp], min, max, steppingDelta, defaultVal, flagAuto );
	}	
	//@}

	//! \name Camera control
	//@{
	
	/** \brief Check if the input supports camera control adjustments
		\return true if the input supports camera control adjustments
	*/
	bool supportCameraControl()
	{
		return captureDevice->supportCameraControl();
	}

	/** \brief change a camera control setting
		\param CameraControl [in] the camera control setting 
		\param value [in] the setting value 
	*/
	void setCameraControl( CameraControl camControl, const long &value, const bool &flagAuto )
	{
		assert( cameraControlsMap.find(camControl) != cameraControlsMap.end() && "setCameraControl: CameraControl unknown" );
		captureDevice->setCameraControl( cameraControlsMap[camControl], value, flagAuto );
	}
	
	/** \brief Get a camera control setting value
		\param CameraControl [in] the camera control setting
		\param value [out] the property value 
		\param flags [out] The returned value indicates whether the setting is controlled manually or automatically (IAMVideoProcAmp)
	*/
	bool getCameraControl( CameraControl camControl, long &value, bool &flagAuto )
	{
		assert( cameraControlsMap.find(camControl) != cameraControlsMap.end() && "getCameraControl: CameraControl unknown" );
		return captureDevice->getCameraControl( cameraControlsMap[camControl], value, flagAuto );
	}

	/** \brief Get a camera control setting range values
		\param CameraControl [in] the camera control setting
		\param min [out] the setting minimum value 
		\param max [out] the setting maximum value 
		\param steppingDelta [out] smallest increment by which the setting can change
		\param defaultVal [out] default value
		\param flags [out] The returned value indicates whether the setting is controlled manually or automatically (IAMVideoProcAmp)
	*/
	bool getCameraControlRange( CameraControl camControl, long &min, long &max, long &steppingDelta, long &defaultVal, bool &flagAuto )
	{
		assert( cameraControlsMap.find(camControl) != cameraControlsMap.end() && "getCameraControlRange: CameraControl unknown" );
		return captureDevice->getCameraControlRange( cameraControlsMap[camControl], min, max, steppingDelta, defaultVal, flagAuto );
	}
	//@}

	
	//! \name Capture Dialogs
	//@{
	void showCaptureDialogDisplay()
	{
		captureDevice->showCaptureDialogDisplay();
	}
	void showCaptureDialogSource()
	{
		captureDevice->showCaptureDialogSource();
	}
	void showCaptureDialogFormat()
	{
		captureDevice->showCaptureDialogFormat();
	}
	//@}

protected:

	/** \brief Set the input that will be controlled
		\param input [in] the input
		\return true if the device is acepted
	*/
	bool setInput( VideoManPrivate::VideoInput *input );
	
	CaptureDeviceDShow *captureDevice; //The controlled device 

	std::map < ImageProperty, VideoProcAmpProperty > propertiesMap; 
	std::map < CameraControl, CameraControlProperty > cameraControlsMap; 
};

#endif