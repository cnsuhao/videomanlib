#include "VideoManInputController.h"

using namespace VideoManPrivate;

VideoManInputController::VideoManInputController(const char *_identifier )
{
	identifier = _identifier;
}

VideoManInputController::~VideoManInputController(void)
{
}

const char *VideoManInputController::getIdentifier()
{
	return identifier.c_str();
}
