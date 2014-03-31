#ifndef VIDEOMANCONTROLPRIVATE_H
#define VIDEOMANCONTROLPRIVATE_H

#include <map>
#include <list>
#include "VideoManControl.h"
#include "VideoManFactory.h"
#include "VideoInput.h"
#include "renderers/VideoManRenderer.h"

namespace VideoManPrivate
{
class  VideoManControlPrivate
{
	friend class VideoMan::VideoManControl;

protected:
	VideoManControlPrivate( VideoMan::VideoManControl::VMRendererType useRenderer );

	~VideoManControlPrivate(void);

	typedef std::map < size_t, VideoInput * > VIDEO_LIST;

	void deleteInput( VIDEO_LIST::iterator );	

	void deleteInputs();

	VIDEO_LIST videoList;		//The list of video inputs
	size_t nextInputID;
	VideoManRenderer *renderer;		//The object that draws the image in the screen
	VideoManFactory factory;		//The object that creates the video inputs
	bool verbose;
};
};
#endif