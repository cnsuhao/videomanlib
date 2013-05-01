#include "VideoManInputFormat.h"
#include "VideoInput.h"

#include <dc1394/dc1394.h>

class DC1394DeviceInput : public VideoManPrivate::VideoInput
{
public:
	DC1394DeviceInput(dc1394_t *d);
	virtual ~DC1394DeviceInput(void);

	bool initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );

	char *getFrame( bool wait = false);

	void releaseFrame( );

private:
	void cleanup_and_exit(dc1394camera_t *camera);

	dc1394_t *d;
	dc1394camera_t *camera;
	dc1394video_frame_t *frame;
};
