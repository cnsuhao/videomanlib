#include "VideoManRenderer.h"
#include <algorithm>

using namespace VideoMan;
using namespace VideoManPrivate;

RendererInput::RendererInput(void)
{
	videoInput = NULL;
	activated = false;	
	screenCoords.left = 0;
	screenCoords.width= 0;
	screenCoords.bottom = 0;
	screenCoords.height = 0;
	align = 4;
	verticalFlip = false;
	horizontalFlip = false;	
}

RendererInput::~RendererInput(void)
{
}

VideoManRenderer::VideoManRenderer(void)
{
	activatedInputs = 0;
	visualizationMode = 0;
	mainVisualizationInput = 0;	
}

VideoManRenderer::~VideoManRenderer(void)
{
}

bool VideoManRenderer::supportedFormat( VMPixelFormat pixelFormat )
{
	return ( std::find( supportedFormats.begin(), supportedFormats.end(), pixelFormat ) != supportedFormats.end() );
}

int VideoManRenderer::getMainVisualizationInput()
{
	//if ( activatedInputs > 0 )
		return static_cast<int>( mainVisualizationInput );
	//return -1;
}
