#ifndef VIDEOMANINPUTCONTROLLER_H
#define VIDEOMANINPUTCONTROLLER_H

#include <string>

namespace VideoManPrivate
{
class VideoInput;

class VideoManInputController
{
public:
	VideoManInputController( const char *_identifier )
	{
		identifier = _identifier;
	}

	virtual ~VideoManInputController(void)
	{
	}

	virtual bool setInput( VideoInput *input ) = 0;

	const char *getIdentifier()
	{
		return identifier.c_str();
	}


private:
	std::string identifier;
};
};
#endif