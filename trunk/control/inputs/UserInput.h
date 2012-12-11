#ifndef USERINPUT_H
#define USERINPUT_H

#include "VideoInput.h"

namespace VideoManPrivate
{
class UserInput :
	public VideoInput
{
public:
	UserInput(void);
	virtual ~UserInput(void);
	
	char *getFrame( bool wait = false );
	bool init( const VideoMan::VMInputFormat &aFormat );
	void setImage( char *image );
};
};
#endif