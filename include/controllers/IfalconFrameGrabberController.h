#pragma once
#include "VideoManInputController.h"

/** \brief If you have an IDS Falcon frame grabber you can access more features with this interface trough IDS SDK
\par Demo Code:
\code
	IfalconFrameGrabberController *controller = (IfalconFrameGrabberController*)videoMan.createController( "IDS_FALCON_FRAME_GRABBER_CONTROLLER" );
	if ( controller )			
	{		
		...
		videoMan.deleteController( &controller ); //not required
	}
\endcode
*/
class IfalconFrameGrabberController :public VideoManPrivate::VideoManInputController
{
public:
	IfalconFrameGrabberController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IfalconFrameGrabberController(void){};

};
