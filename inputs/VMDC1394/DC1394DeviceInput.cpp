#include "DC1394DeviceInput.h"
#include <iostream>
#include <string>
#include <string.h>

#include <dc1394/dc1394.h>

#define VIDEOMAN_DC1394_ERR_CLN_RTN(err,cleanup,message)  \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      dc1394_log_error("%s: in %s (%s, line %d): %s\n",   \
      dc1394_error_get_string(err),                       \
          __FUNCTION__, __FILE__, __LINE__, message);     \
      cleanup;                                            \
      return false;                                       \
    }                                                     \
  } while (0);

#define VIDEOMAN_DC1394_ERR_RTN(err,message)              \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      dc1394_log_error("%s: in %s (%s, line %d): %s\n",   \
      dc1394_error_get_string(err),                       \
          __FUNCTION__, __FILE__, __LINE__, message);     \
      return false;                                       \
    }                                                     \
  } while (0);


DC1394DeviceInput::DC1394DeviceInput(dc1394_t *d)
{
	this->d = d;
}

DC1394DeviceInput::~DC1394DeviceInput(void)
{
	dc1394_video_set_transmission(this->camera, DC1394_OFF);
	dc1394_capture_stop(this->camera);
	dc1394_camera_free(this->camera);
}

void DC1394DeviceInput::cleanup_and_exit(dc1394camera_t *camera)
{
	dc1394_video_set_transmission(this->camera, DC1394_OFF);
	dc1394_capture_stop(this->camera);
	dc1394_camera_free(this->camera);
}

bool DC1394DeviceInput::initInput( const inputIdentification &device, VideoManInputFormat *aformat )
{
	dc1394error_t err;

	this->camera = dc1394_camera_new (this->d, device.serialNumber);

	if (!this->camera) {
		dc1394_log_error("Failed to initialize camera with guid %llx", device.serialNumber);
		return false;
	}

	std::cout<<"Using camera with GUID "<<camera->guid<<std::endl;

	format.SetFormat( 640, 480, 100, RAW8, RAW8 );
	*aformat= format;


	dc1394video_modes_t modes;

	/*-----------------------------------------------------------------------
	*  list Capture Modes
	*-----------------------------------------------------------------------*/
	err=dc1394_video_get_supported_modes(camera, &modes);
	VIDEOMAN_DC1394_ERR_RTN(err,"Could not get list of modes");

	dc1394video_mode_t selected_mode = DC1394_VIDEO_MODE_FORMAT7_0;

	/*-----------------------------------------------------------------------
	*  setup capture
	*-----------------------------------------------------------------------*/

	err=dc1394_video_set_iso_speed(this->camera, DC1394_ISO_SPEED_400);
	VIDEOMAN_DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not set iso speed");

	err=dc1394_video_set_mode(this->camera, selected_mode);
	VIDEOMAN_DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not set video mode\n");

	err = dc1394_format7_set_image_size(this->camera, selected_mode , format.width, format.height);
	VIDEOMAN_DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not set iamge size\n");

	err=dc1394_video_set_framerate(this->camera, DC1394_FRAMERATE_60 );
	VIDEOMAN_DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not set framerate\n");


	err=dc1394_capture_setup(this->camera,4, DC1394_CAPTURE_FLAGS_DEFAULT);
	VIDEOMAN_DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera\n");

	/*-----------------------------------------------------------------------
	*  have the camera start sending us data
	*-----------------------------------------------------------------------*/
	err=dc1394_video_set_transmission(this->camera, DC1394_ON);
	VIDEOMAN_DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not start camera iso transmission\n");

	//std::cout<<"DC1394 OK"<<std::endl;
	//format.SetFormat( 1600, 1200, 15, GREY8, GREY8 );

	const std::string identifier = "DC1394_CAPTURE_DEVICE";
	identification.identifier = new char[identifier.length() + 1];
	strcpy( identification.identifier, identifier.c_str() );


	return true;
}

void DC1394DeviceInput::releaseFrame( )
{
	dc1394_capture_enqueue(this->camera, this->frame);
}

char *DC1394DeviceInput::getFrame( bool wait)
{
	dc1394error_t err;

	/*-----------------------------------------------------------------------
	*  capture one frame
	*-----------------------------------------------------------------------*/
	err=dc1394_capture_dequeue(this->camera, DC1394_CAPTURE_POLICY_WAIT, &this->frame);
	//DC1394_ERR_CLN_RTN(err,this->cleanup_and_exit(camera),"Could not capture a frame\n");

	if (this->frame!=NULL)
		pixelBuffer = (char*)this->frame->image;
	else
		pixelBuffer = NULL;

	return pixelBuffer;
}
