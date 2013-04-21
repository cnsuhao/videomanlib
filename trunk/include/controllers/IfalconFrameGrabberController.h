#pragma once
#include "VideoManInputController.h"

/** \brief Advanced controller for IDS Falcon frame grabber running with VMIDSFalcon
\par Demo Code:
\code
	videoMan.getAvailableDevices( "IDS_FALCON_FRAME_GRABBER", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format );
	...
	IfalconFrameGrabberController *controller = (IfalconFrameGrabberController.h*)videoMan.createController( inputID, "IDS_FALCON_FRAME_GRABBER_CONTROLLER" );
	if ( controller )			
	{
		controller->SetExposureTime( 100 );				
	}
	...
	videoMan.deleteController( &controller );
\endcode
*/
class IfalconFrameGrabberController :public VideoManPrivate::VideoManInputController
{
public:
	IfalconFrameGrabberController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IfalconFrameGrabberController(void){};

};
