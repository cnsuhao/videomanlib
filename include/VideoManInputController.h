#ifndef VIDEOMANINPUTCONTROLLER_H
#define VIDEOMANINPUTCONTROLLER_H

#ifdef WIN32
#ifdef VideoMan_EXPORTS
#define VIDEOMAN_API __declspec(dllexport)
#else
#define VIDEOMAN_API __declspec(dllimport)
#endif
#endif
#ifdef linux
#define VIDEOMAN_API 
#endif

#include <string>

namespace VideoManPrivate
{
class VideoInput;

class VIDEOMAN_API VideoManInputController
{
public:
	VideoManInputController( const char *identifier );
	virtual ~VideoManInputController(void);

	virtual bool setInput( VideoInput *input ) = 0;

	const char *getIdentifier();

private:
	std::string identifier;
};
};
#endif