#include "DSPointGreyController.h"
#include "DirectShowHelpers.h"
#include <string>
using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

DSPointGreyController::DSPointGreyController( const char *_identifier ) : IDSPointGreyController( _identifier )
{
	pProperties = NULL;
}

DSPointGreyController::~DSPointGreyController(void)
{	
	SAFE_RELEASE( pProperties );
}

bool DSPointGreyController::setInput( VideoInput *input )
{
	string identifier = input->getIdentification().identifier;
	if ( identifier == "DSHOW_CAPTURE_DEVICE" )
	{
		captureDevice = (CaptureDeviceDShow*)input;
		if ( !captureDevice )
			return false;
		IBaseFilter *videoSource;
		videoSource = captureDevice->getVideoSourceFilter();
		if ( !videoSource )
			return false;
		videoSource->QueryInterface(IID_IFlyCaptureProperties,  (void **)&pProperties);
		if ( !pProperties )
		{
			SAFE_RELEASE( pProperties );
			return false;
		}
		/*bool fip;
		pProperties->GetOutputVerticalFlip( &fip );
		HRESULT hr = pProperties->SetOutputVerticalFlip( false );
		pProperties->GetOutputVerticalFlip( &fip );
		//pProperties->SetCustomImage( 0, 0, 0, 200, 200, );
		unsigned int mode, l, t, w, h, pf ;
		pProperties->GetCustomImage( &mode, &l, &t, &w, &h, &pf );
		long gain;
		pProperties->GetGain( &gain, &fip );
		pProperties->SetGain( gain, false );*/
		return true;
	}	
	return false;
}