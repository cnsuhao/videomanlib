#include "falconFrameGrabberController.h"
#include <assert.h>
#include <string>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

FalconFrameGrabberController::FalconFrameGrabberController( const char *_identifier ) : IfalconFrameGrabberController( _identifier )
{
	//uEyeCamera = NULL;
}

FalconFrameGrabberController::~FalconFrameGrabberController(void)
{
	
}

bool FalconFrameGrabberController::setInput( VideoInput *input )
{
	string identifier = input->getIdentification().identifier;
	if ( identifier == "IDS_FALCON_FRAME_GRABBER" )
	{
		m_frameGrabber = (IDSFalconFrameGrabber*)input;
		if ( m_frameGrabber )
			return true;
	}
	return false;
}

