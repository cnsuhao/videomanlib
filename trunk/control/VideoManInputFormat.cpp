#include "VideoManInputFormat.h"

using namespace VideoMan;

VMInputFormat::VMInputFormat(void)
{
	align = 4;
	SetFormat( 320, 240, 60, VM_UNKNOWN, VM_RGB24 );	
	showDlg = false;
	clock = true;
	renderAudio = true;	
	memset(identification, 0, sizeof(identification));
}


VMInputFormat::~VMInputFormat(void)
{
}

VMInputFormat::VMInputFormat(VMInputFormat const &format)
{
	*this = format;
}

VMInputFormat &VMInputFormat::operator=( VMInputFormat const &format )
{
	this->nChannels = format.nChannels;
	this->depth = format.depth;
	this->width = format.width;
	this->height = format.height;
	this->fps = format.fps;
	this->showDlg = format.showDlg;
	this->clock = format.clock;
	this->renderAudio = format.renderAudio;
	this->formatIn = format.formatIn;
	this->formatOut = format.formatOut;
	this->align = format.align;
	return *this;	
}


bool VMInputFormat::SetFormat( int awidth, int aheight, double afps, VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut )
{
	width = awidth;
	height = aheight;
	fps = afps;		
	if ( !setPixelFormat( apixelFormatIn, apixelFormatOut ) )
		return false;
	if ( !validFormat() )
		return false;
	return true;
}

bool VMInputFormat::setPixelFormat( VMPixelFormat apixelFormatIn, VMPixelFormat apixelFormatOut )
{
	formatIn = apixelFormatIn;
	formatOut = apixelFormatOut;
	switch ( apixelFormatOut )
	{
		case VM_RGB24:
		case VM_BGR24:
		{
			depth = 8;
			nChannels = 3;
			break;
		}
		case VM_RGB32:
		case VM_BGR32:
		{
			depth = 8;
			nChannels = 4;		
			break;
		}
		case VM_GREY16:
		{
			depth = 16;
			nChannels = 1;
			break;
		}
		case VM_GREY8:
		{
			depth = 8;
			nChannels = 1;
			break;
		}
		case VM_RAW8:
		{
			depth = 8;
			nChannels = 1;
			break;
		}
		case VM_YUV422:
		{
			depth = 16;
			nChannels = 3;
			break;
		}
	}
	/*BppCalc = calculateBpp( pixelFormats.formatOut );
	if ( BppCalc == -1 )
		return false;
	else
		BppOut = BppCalc;*/
	return true;
}

/*float VMInputFormat::calculateBpp( VMPixelFormat apixelFormat )
{
	float BPPret = -1;
	switch ( apixelFormat )
	{
		case RGB24:
		case BGR24:
		{
			BPPret = 3;
			break;
		}
		case RGB32:
		case BGR32:
		{
			BPPret = 4;				
			break;
		}
		case GREY16:
		{
			BPPret = 2;                
			break;
		}
		case GREY8:
		{
			BPPret = 1;				
			break;
		}
		case RAW8:
		{
			BPPret = 1;				
			break;
		}
		case YUV422:
		{
			BPPret = 2;				
			break;
		}
	}
	return BPPret;
}*/


bool VMInputFormat::validFormat()
{
	if ( width <= 0 || height <= 0)
		return false;
	if ( depth <= 0 || nChannels <= 0)
		return false;
/*	float BppCalc = calculateBpp( pixelFormats.formatIn );
	if ( BppCalc == -1 )
		return false;
	BppCalc = calculateBpp( pixelFormats.formatOut );
	if ( BppCalc == -1 )
		return false;*/
	return true;
}

/*float VMInputFormat::getBpp()
{
	return BppOut;
}*/

VMPixelFormat VMInputFormat::getPixelFormatIn() const
{
	return formatIn;
}

VMPixelFormat VMInputFormat::getPixelFormatOut() const
{
	return formatOut;
}
