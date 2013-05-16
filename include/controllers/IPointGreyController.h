#pragma once

#include "VideoManInputController.h"

/** \brief Advanced controller for PointGrey cameras running with VMPointGrey2
\par Demo Code:
\code
	videoMan.getAvailableDevices( "PGR_CAMERA2", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format );
	...
	IPointGreyController *controller = (IPointGreyController.h*)videoMan.createController( inputID, "POINTGREY_CONTROLLER2" );
	if ( controller )			
	{
		controller->setGainControl( true );				
		controller->setShutterTime( 4 );
	}
	...
	videoMan.deleteController( &controller );	 
\endcode
*/
class IPointGreyController : public VideoManPrivate::VideoManInputController
{
public:
	IPointGreyController( const char *identifier ) : VideoManPrivate::VideoManInputController(identifier){}
	virtual ~IPointGreyController(){}

	/** \brief Set a value in a camera's register
		\param reg [in] Register
		\param value [in] Value
		\return true if the register value was changed
	*/
	virtual bool setRegister( unsigned int reg, unsigned int value ) = 0;
	
	/** \brief Print the camera's info
	*/
	virtual void printCameraInfo() = 0;
	
	/** \brief Enable or disable the auto gain control
		\param autoGain [in] Enable the gain control
		\return True if the gain control was correctly changed
	*/
	virtual bool setGainControl(bool autoGain) = 0;
	
	/** \brief Check if the auto gain control is enabled or not
		\return True if the gain control is enabled
	*/
	virtual bool getGainControl() = 0;	
	
	/** \brief Enable or disable the auto exposure control
		\param autoExp [in] Enable the exposure control
		\return True if the exposure control was correctly changed
	*/
	virtual bool setExposureControl( bool autoExp ) = 0;
	
	/** \brief Set a sharpness value
		\param value [in] The sharpness value
		\return True if the sharpness value was changed
	*/
	virtual bool setSharpnessControlValue(float value) = 0;
	
	/** \brief Enable or disable the auto sharpness control
		\param autpoSharp [in] Enable the sharpness control
		\return True if the sharpness control was correctly changed
	*/
	virtual bool setSharpnessControl( bool autpoSharp ) = 0;
	
	/** \brief Set a shutter time
		\param shutterTime [in] The shutter time
		\return True if the shutter time was changed
	*/
	virtual bool setShutterTime( float shutterTime ) = 0;
	
	/** \brief Set a framerate
		\param frameRate [in] The framerate
		\return True if the framerate was changed
	*/
	virtual bool setFrameRate( float frameRate ) = 0;	
	
	
	virtual bool setTrigger( bool triggerOn, int source, int mode ) = 0;
	virtual bool fireSoftwareTrigger( bool broadcast ) = 0;
	virtual bool setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source ) = 0;
	
	/** \brief Move the region of interest
		\param x [in] X coordinate of the top left corner of the ROI
		\param y [in] Y coordinate of the top left corner of the ROI
		\return True if the ROI was moved (the camera can take some frames to use the new ROI coordinates)
	*/
	virtual bool moveImageROI( int x, int y ) = 0;
	
	/** \brief Set a region of interest
		\param x [in] X coordinate of the ROI's top left corner
		\param y [in] Y coordinate of the ROI's top left corner
		\param width [in] Width of the ROI
		\param height [in] Height of the ROI
		\return True if the ROI was set (the camera can take some frames to use the new ROI coordinates)
	*/
	virtual bool setImageROI( int x, int y, int width, int height ) = 0;
	
	/** \brief Get the units of the region of interest
		\param Hmax [out] Maximum image width
		\param Vmax [out] Maximum image height
		\param imageHStepSize [out] Horizontal step size for the offset
		\param imageVStepSize [out] Vertical step size for the offset
		\param offsetHStepSize [out] Horizontal step size for the image
		\param offsetVStepSize [out] Vertical step size for the image	
		\return True if the returned values are correct
	*/
	virtual bool getROIUnits( int &Hmax, int &Vmax, int &Hunit, int &Vunit, int &HPosUnit, int &VPosUnit ) = 0;
	
	/** \brief Reset the region of interest to the maximum image size in the current format
		\return True if the ROI was reset
	*/
	virtual bool resetImageROI() = 0;
	
	/** \brief Configure the camera's trigger
		\param triggerOn [in] Set the trigger on or off
		\param source [in] GPIO pin numberd source
		\param mode [in] Trigger mode
		\return True if the trigger was correctly configured
	*/
	virtual void convertRawToColor( const unsigned char* imgSrc, unsigned char *imgDst, int width, int height ){};
	
	/** \brief Get the raw bayer format of the camera
		\return True if the returned bayer format is correct
	*/
	virtual bool getBayerFormat(size_t &btf)=0;
};
