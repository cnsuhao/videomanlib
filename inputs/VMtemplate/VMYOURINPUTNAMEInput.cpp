#include <iostream>
#include <vector>
#include <assert.h>

#include "VMYOURINPUTNAMEInput.h"

using namespace std;

VMYOURINPUTNAMEInput::VMYOURINPUTNAMEInput(void)
{	
	//-- initialization --//
}

VMYOURINPUTNAMEInput::~VMYOURINPUTNAMEInput(void)
{
} 

bool VMYOURINPUTNAMEInput::init( const char *friendlyName, const char *devicePath, VideoMan::VMInputFormat *aFormat )
{
  return false;
}

char *VMYOURINPUTNAMEInput::getFrame( bool wait)
{
//    if ( there is image )
		return pixelBuffer;	
//	else
//		return NULL;
}


void VMYOURINPUTNAMEInput::releaseFrame()
{
	
}


void VMYOURINPUTNAMEInput::play()
{

}


void VMYOURINPUTNAMEInput::pause()
{
}


void VMYOURINPUTNAMEInput::stop()
{
	
}

