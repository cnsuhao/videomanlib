#ifndef falconFrameGrabberController_H
#define falconFrameGrabberController_H

#include "controllers/IfalconFrameGrabberController.h"
#ifdef WIN32
	#include <windows.h>
#endif
#include "IDSFalconFrameGrabber.h"

class FalconFrameGrabberController :
	public  IfalconFrameGrabberController
{
public:
	FalconFrameGrabberController( const char *_identifier );
	virtual ~FalconFrameGrabberController(void);

	bool setInput( VideoManPrivate::VideoInput *input );
			
protected:

	IDSFalconFrameGrabber *m_frameGrabber;


};
#endif
