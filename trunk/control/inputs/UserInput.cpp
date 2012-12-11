#define _CRT_SECURE_NO_DEPRECATE

#include "UserInput.h"
#include <string.h>

using namespace VideoMan;
using namespace VideoManPrivate;

UserInput::UserInput( void )
{
	pixelBuffer = NULL;
}


UserInput::~UserInput(void)
{
}

bool UserInput::init( const VMInputFormat &aFormat )
{
	identification.fileName = NULL;
	identification.friendlyName = NULL;
	identification.uniqueName = NULL;

	const std::string id = "USER_INPUT";
	identification.identifier = new char[id.length() + 1];
	strcpy( identification.identifier, id.c_str() );

	format = aFormat;
	return format.validFormat();
}

char *UserInput::getFrame( bool wait )
{
	return pixelBuffer;
}


void UserInput::setImage( char *image )
{
	pixelBuffer = image;
}
