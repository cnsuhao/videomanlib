#pragma once
//#include <windows.h>
#include <vector>
#include "VideoManInputController.h"
#include "controllers/IPointGreyController.h"
#include "PGRCamera.h"

class PointGreyController :
	public IPointGreyController
{
public:
	PointGreyController(const char *identifier);
	virtual ~PointGreyController(void);
	
	void startRecording();

	void stopRecording();

	int  getNumberOfCameras();

	/** \brief Set a value in a camera's register
		\param reg [in] Register
		\param value [in] Value
		\return true if the register value was changed
	*/
	bool setRegister( unsigned int reg, unsigned int value );

	bool setRegisterBroadcast( unsigned long reg, unsigned long value );

	void SynchronizeCameras();

	void UnlockBuffers();

	/** \brief Print the camera's info
	*/
	void printCameraInfo();

	bool setInput( VideoManPrivate::VideoInput *input );

	/** \brief Enable or disable the auto gain control
		\param autoGain [in] Enable the gain control
		\return True if the gain control was correctly changed
	*/
	bool setGainControl(bool autoGain);
	
	/** \brief Check if the auto gain control is enabled or not
		\return True if the gain control is enabled
	*/
	bool getGainControl();	

	/** \brief Enable or disable the auto exposure control
		\param autoExp [in] Enable the exposure control
		\return True if the exposure control was correctly changed
	*/
	bool setExposureControl( bool autoExp );

	/** \brief Set a sharpness value
		\param value [in] The sharpness value
		\return True if the sharpness value was changed
	*/
	bool setSharpnessControlValue(float value);
	
	/** \brief Enable or disable the auto sharpness control
		\param autpoSharp [in] Enable the sharpness control
		\return True if the sharpness control was correctly changed
	*/
	bool setSharpnessControl( bool autpoSharp );

	/** \brief Set a shutter time
		\param shutterTime [in] The shutter time
		\return True if the shutter time was changed
	*/
	bool setShutterTime( float shutterTime );

	/** \brief Enable or disable the auto shutter control
		\param autpoSharp [in] Enable the shutter control
		\return True if the shutter control was correctly changed
	*/
	bool setShutterControl( bool autoShutter );

	/** \brief Set a framerate
		\param frameRate [in] The framerate
		\return True if the framerate was changed
	*/
	bool setFrameRate( float frameRate );

	/** \brief Move the region of interest
		\param x [in] X coordinate of the top left corner of the ROI
		\param y [in] Y coordinate of the top left corner of the ROI
		\return True if the ROI was moved (the camera can take some frames to use the new ROI coordinates)
	*/
	bool moveImageROI( int x, int y );

	/** \brief Set a region of interest
		\param x [in] X coordinate of the ROI's top left corner
		\param y [in] Y coordinate of the ROI's top left corner
		\param width [in] Width of the ROI
		\param height [in] Height of the ROI
		\return True if the ROI was set (the camera can take some frames to use the new ROI coordinates)
	*/
	bool setImageROI( int x, int y, int width, int height );

	/** \brief Get the region of interest
		\param x [in] X coordinate of the ROI's top left corner
		\param y [in] Y coordinate of the ROI's top left corner
		\param width [in] Width of the ROI
		\param height [in] Height of the ROI
		\return True if the ROI was set (the camera can take some frames to use the new ROI coordinates)
	*/
	bool getImageROI( int &x, int &y, int &width, int &height );

	/** \brief Get the units of the region of interest
		\param Hmax [out] Maximum image width
		\param Vmax [out] Maximum image height
		\param imageHStepSize [out] Horizontal step size for the offset
		\param imageVStepSize [out] Vertical step size for the offset
		\param offsetHStepSize [out] Horizontal step size for the image
		\param offsetVStepSize [out] Vertical step size for the image	
		\return True if the returned values are correct
	*/
	bool getROIUnits( int &Hmax, int &Vmax, int &imageHStepSize, int &imageVStepSize, int &offsetHStepSize, int &offsetVStepSize );

	/** \brief Reset the region of interest to the maximum image size in the current format
		\return True if the ROI was reset
	*/
	bool resetImageROI();

	/** \brief Get the raw bayer format of the camera
		\return True if the returned bayer format is correct
	*/
	bool getBayerFormat(size_t &btf);

	/** \brief Configure the camera's trigger
		\param triggerOn [in] Set the trigger on or off
		\param source [in] GPIO pin numberd source
		\param mode [in] Trigger mode
		\return True if the trigger was correctly configured
	*/
	bool setTrigger( bool triggerOn, int source, int mode );

	/** \brief Fire a software trigger		
		\return True if the trigger was fired
	*/
	bool fireSoftwareTrigger( bool broadcast );

	/** \brief Convert a raw image to color
		\param imgSrc [in] Input image
		\param imgDst [in] Output image (enough memory should be already allocated)
		\param width [in] Width of the images 
		\param height [in] Height of the images
	*/
	void convertRawToColor( const unsigned char* imgSrc, unsigned char *imgDst, int width, int height );

	/** \brief Configure the camera's strobe output
		\param onOff [in] Set the output on or off
		\param delay [in] Delay time
		\param duration [in] Duration of the pulse
		\param polarity [in] High or low pulse
		\param source [in] GPIO pin number
	*/
	bool setStrobeOutput( bool onOff, float delay, float duration,	unsigned int polarity, unsigned int source );

private:
	std::vector< PGRCamera* > cameraList;
//	std::vector<FlyCaptureContext> contexts;
};