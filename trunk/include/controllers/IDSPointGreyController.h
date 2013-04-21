#pragma once
#include "VideoManInputController.h"

/** \brief Advanced controller for PointGrey cameras running with VMDirectShow
\par Demo Code:
\code
	videoMan.getAvailableDevices( "DSHOW_CAPTURE_DEVICE", list, numDevices );
	inputID = videoMan.addVideoInput( list[0], &format ); //Suppose list[0] is a PointGrey camera
	...
	IDSPointGreyController *controller = (IDSPointGreyController.h*)videoMan.createController( inputID, "DSHOW_POINTGREY_CONTROLLER" );
	if ( controller )			
	{
		...		
	}
	...
	videoMan.deleteController( &controller );
\endcode
*/
class IDSPointGreyController : public VideoManPrivate::VideoManInputController
{
public:
	IDSPointGreyController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IDSPointGreyController(void){};


};
