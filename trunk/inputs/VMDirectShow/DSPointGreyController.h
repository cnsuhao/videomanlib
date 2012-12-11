#ifndef DSPOINTGREYCONTROLLER_H
#define DSPOINTGREYCONTROLLER_H

#include "controllers/IDSPointGreyController.h"
#include "CaptureDeviceDShow.h"
#include "PGRDirectShow.h"


class DSPointGreyController :
	public  IDSPointGreyController
{
public:
	DSPointGreyController( const char *_identifier );
	virtual ~DSPointGreyController(void);

	bool setInput( VideoManPrivate::VideoInput *input );
			
protected:
	IFlyCaptureProperties *pProperties;
	CaptureDeviceDShow *captureDevice;
};
#endif
