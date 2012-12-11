#pragma once
#include "VideoManInputController.h"

/** \brief If you have a PointGrey camera you can access more features with this interface trough VMDirectShow
\par Demo Code:
\code
	IDSPointGreyController *controller = (IDSPointGreyController.h*)videoMan.createController( "DSHOW_POINTGREY_CONTROLLER" );
	if ( controller )			
	{
		...
		videoMan.deleteController( &controller ); //not required
	}
\endcode
*/
class IDSPointGreyController : public VideoManPrivate::VideoManInputController
{
public:
	IDSPointGreyController( const char *_identifier ) : VideoManPrivate::VideoManInputController( _identifier ){}
	virtual ~IDSPointGreyController(void){};


};
