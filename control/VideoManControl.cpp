#include <iostream>
#include <string>
#include <assert.h>

#include "VideoManControlPrivate.h"
#include "VideoManFactory.h"
#include "renderers/VideoManRenderer.h"

#ifdef VM_OGLRenderer
	#include "renderers/VideoManRendererOGL.h"
#endif

#ifdef linux
	#include <stdio.h>
	//#include <dlfcn.h>
	#include <link.h>
#endif

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

VideoManControlPrivate::VideoManControlPrivate( VideoMan::VideoManControl::VMRendererType useRenderer )
{
	renderer = NULL;
	if ( useRenderer == VideoManControl::OPENGL_RENDERER )
	{
		#ifdef VM_OGLRenderer			
			renderer = new VideoManRendererOGL();
		#endif
	}
	nextInputID = 0;
}

VideoManControlPrivate::~VideoManControlPrivate(void)
{
	deleteInputs();
	factory.freeModules(); //delete video inputs & controllers
	if ( renderer != NULL )
	{
		delete renderer;
		renderer = NULL;
	}	
}

VideoManControl::VideoManControl(VMRendererType rendererType) 
{
	videoManPrivate	= new VideoManControlPrivate( rendererType );
}


VideoManControl::~VideoManControl(void)
{	
	delete videoManPrivate;
}

int VideoManControl::addVideoInput( const VMInputIdentification &identifier, VMInputFormat *format )
{
	VideoInput *video = videoManPrivate->factory.createVideoInput( identifier, format );	
	if ( video == NULL )
	{
		cerr << "Error creating video input" << endl;
		return -1;
	}
	if ( videoManPrivate->renderer != NULL )
	{
		if ( !videoManPrivate->renderer->addVideoInput( videoManPrivate->nextInputID, video ) )
		{
			cerr << "Failed renderer->addVideoInput" << endl;
			videoManPrivate->factory.deleteInput( &video );
			return -1;
		}
	}

    videoManPrivate->videoList[videoManPrivate->nextInputID] = video;	
	video->setInputID( videoManPrivate->nextInputID );
	videoManPrivate->nextInputID++;	
	return videoManPrivate->nextInputID - 1;
}

void VideoManControlPrivate::deleteInput( VIDEO_LIST::iterator it )
{
	if ( it->second != NULL )
	{
		factory.deleteInput( &it->second );
		if ( renderer != NULL )
			renderer->deleteVideoInput( it->first );		
	}
}

void VideoManControl::deleteInput( size_t input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "deleteInput: Index out of range");
	VideoManControlPrivate::VIDEO_LIST::iterator it = videoManPrivate->videoList.find( input );
	videoManPrivate->deleteInput( it );	
	videoManPrivate->videoList.erase( it );
}

void VideoManControlPrivate::deleteInputs()
{
	for ( VIDEO_LIST::iterator it = videoList.begin(); it != videoList.end(); it++ )
		deleteInput( it );
	videoList.clear();
}

void VideoManControl::deleteInputs()
{
	videoManPrivate->deleteInputs();
}

void VideoManControl::activateAllVideoInputs()
{
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->activateAllVideoInputs( );
}

void VideoManControl::activateVideoInput( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "activateVideoInput: Index out of range");
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->activateVideoInput( input );	
}


void VideoManControl::deactivateAllVideoInputs()
{
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->deactivateAllVideoInputs();
}


void VideoManControl::deactivateVideoInput( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "deactivateVideoInput: Index out of range");
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->deactivateVideoInput( input );
}

bool VideoManControl::isActivated( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "isActivated: Index out of range");
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		return videoManPrivate->renderer->isActivated( input );	
	return false;
}


VideoManInputController *VideoManControl::getController( const size_t &input ) const
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getController: Index out of range");
	return videoManPrivate->videoList[input]->getController();
}


VideoManInputController *VideoManControl::createController( const size_t &input, const char *controllerIdentifier )
{	
	VideoManInputController *cont = videoManPrivate->factory.createController( controllerIdentifier );
	if ( cont )
	{
		if ( linkController( input, cont ) )
			return cont;
		else
			delete cont;
	}
	return NULL;
}

void VideoManControl::deleteController( VideoManInputController **controller )
{
	videoManPrivate->factory.deleteController( controller );
}

bool VideoManControl::linkController( const size_t &input, VideoManInputController *controller  )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "linkController: Index out of range");
	if ( controller && videoManPrivate->videoList[input]->linkController( controller ) )
		return true;
	return false;
}


void VideoManControl::showPropertyPage( const size_t &input ) const
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getFrame: Index out of range");
	videoManPrivate->videoList[input]->showPropertyPage();
}


void VideoManControl::changeScreenSize( const int &left, const int &up, const int &width, const int &height )
{
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->changeScreenSize( left, up, width, height );
}

void VideoManControl::changeVisualizationMode( const int &vM )
{
	assert( videoManPrivate->renderer !=NULL && "changeVisualizationMode(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->changeVisualizationMode( vM );
}

void VideoManControl::changeMainVisualizationInput( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "changeMainVisualizationInput: Index out of range");
	assert( videoManPrivate->renderer !=NULL && "changeMainVisualizationInput(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->changeMainVisualizationInput( input );
}

int VideoManControl::getMainVisualizationInput()
{
	assert( videoManPrivate->renderer !=NULL && "getMainVisualizationInput(): Renderer not created");
	if ( videoManPrivate->renderer != NULL )
		return videoManPrivate->renderer->getMainVisualizationInput();
	return -1;
}

void VideoManControl::setVerticalFlip( const size_t &input, bool value )
{
	assert( videoManPrivate->renderer !=NULL && "setVerticalFlip(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setVerticalFlip: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->setVerticalFlip( input, value );
}

bool VideoManControl::getVerticalFlip( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "getVerticalFlip(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getVerticalFlip: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		return videoManPrivate->renderer->getVerticalFlip( input );
	return false;
}

void VideoManControl::setHorizontalFlip( const size_t &input, bool value )
{
	assert( videoManPrivate->renderer !=NULL && "setHorizontalFlip(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setHorizontalFlip: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->setHorizontalFlip( input, value );
}

bool VideoManControl::getHorizontalFlip( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "getHorizontalFlip(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getHorizontalFlip: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		return videoManPrivate->renderer->getHorizontalFlip( input );
	return false;
}

void  VideoManControl::setRendererZoom( const size_t &input, const float &x, const float &y, const float &zoomFactor, bool center )
{
	assert( videoManPrivate->renderer !=NULL && "setRendererZoom(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setRendererZoom: Index out of range");
	if ( videoManPrivate->renderer != NULL )
	{
		if ( center )
			videoManPrivate->renderer->setZoom( input, x, y, zoomFactor );
		else
		{
			float zx,zy,zf;
			zx = x;
			zy = y;
			videoManPrivate->renderer->getZoom( input, zx, zy, zf );
			VMInputFormat format = videoManPrivate->videoList[input]->getVideoManInputFormat();
			if ( zf != 0 && zoomFactor != 0 )
			{
				zx = x + (zx - x ) / ( zoomFactor / zf );
				zy = y + (zy - y ) / ( zoomFactor / zf );
				videoManPrivate->renderer->setZoom( input, zx, zy, zoomFactor );
			}
		}
	}
}
	
void  VideoManControl::getRendererZoom( const size_t &input, float &x, float &y, float &zoomFactor )
{
    assert( videoManPrivate->renderer !=NULL && "getRendererZoom(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getRendererZoom: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->getZoom( input, x, y, zoomFactor );
}

void VideoManControl::resetRendererZoom( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "resetRendererZoom(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "resetRendererZoom: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->resetZoom( input );
}

void VideoManControl::setRendererRotation( const size_t &input, float angle )
{
	assert( videoManPrivate->renderer !=NULL && "setRendererRotation(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setRendererRotation: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->setRotation( input, angle );
}

float VideoManControl::getRendererRotation( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "getRendererRotation(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getRendererRotation: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		return videoManPrivate->renderer->getRotation( input );
	return 0.0f;
}

void VideoManControl::activate2DDrawingSetup( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "activate2DDrawingSetup(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "activate2DDrawingSetup: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->activate2DDrawingSetup( input );
}

void VideoManControl::drawInputBorder( const size_t &input, const float &thickness, const float &r, const float &g, const float &b )
{
	assert( videoManPrivate->renderer !=NULL && "drawInputBorder(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "drawInputBorder: Index out of range");
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->drawInputBorder( input, thickness, r, g, b );
}

void VideoManControl::stopVideo( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "stopVideo: Index out of range");
	videoManPrivate->videoList[input]->stop();
}

void VideoManControl::pauseVideo( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "pauseVideo: Index out of range");
	videoManPrivate->videoList[input]->pause();
}

void VideoManControl::playVideo( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "playVideo: Index out of range");	
	videoManPrivate->videoList[input]->play();	
}

void VideoManControl::goToFrame( const size_t &input, int frame )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "goToFrame: Index out of range");
	videoManPrivate->videoList[input]->goToFrame( frame );
}

void VideoManControl::goToMilisecond( size_t input, double milisecond )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "goToMilisecond: Index out of range");
	videoManPrivate->videoList[input]->goToMilisecond( milisecond );	
}

int VideoManControl::getLengthFrames( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getLengthFrames: Index out of range");
	return videoManPrivate->videoList[input]->getLengthFrames();	
}

double VideoManControl::getLengthSeconds( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getLengthSeconds: Index out of range");
	return videoManPrivate->videoList[input]->getLengthSeconds();	
}

int VideoManControl::getPositionFrames( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getPositionFrames: Index out of range");
	return videoManPrivate->videoList[input]->getPositionFrames();	
}


double VideoManControl::getPositionSeconds( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getPositionSeconds: Index out of range");
	return videoManPrivate->videoList[input]->getPositionSeconds();	
}



void VideoManControl::getFormat( size_t input, VMInputFormat &format )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getFormat: Index out of range");
	format = videoManPrivate->videoList[input]->getVideoManInputFormat();
}

void VideoManControl::getInputIdentification( size_t input, VMInputIdentification &identification )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getInputIdentification: Index out of range");
	identification = videoManPrivate->videoList[input]->getIdentification();
}


void VideoManControl::updateTexture( size_t input, const char *image )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "updateTexture: Index out of range");
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");	
	videoManPrivate->renderer->updateTexture( input, image );
}

void VideoManControl::updateTexture( size_t input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "updateTexture: Index out of range");
	assert( videoManPrivate->renderer !=NULL && "updateTexure(): Renderer not created");
	videoManPrivate->renderer->updateTexture( input );
}

char *VideoManControl::getFrame( size_t input, bool wait )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getFrame: Index out of range");
	return videoManPrivate->videoList[input]->getFrame( wait );
}

void VideoManControl::releaseFrame( size_t input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "releaseFrame: Index out of range");
	videoManPrivate->videoList[input]->releaseFrame();
}

void VideoManControl::renderFrames() const
{
	assert( videoManPrivate->renderer !=NULL && "renderFrames(): Renderer not created");
	videoManPrivate->renderer->renderInputs();
}

void VideoManControl::renderFrame( size_t input ) const
{
	assert( videoManPrivate->renderer !=NULL && "renderFrames(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "renderFrame: Index out of range");
	videoManPrivate->renderer->renderInput( input );
}

int VideoManControl::screenToImageCoords( float &x, float &y )
{
	assert( videoManPrivate->renderer !=NULL && "inputCoords(): Renderer not created");
	return videoManPrivate->renderer->screenToImageCoords( x, y );
}

void VideoManControl::imageToScreenCoords( const size_t &input, float &x, float &y )
{
	assert( videoManPrivate->renderer !=NULL && "imageToScreenCoords(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "imageToScreenCoords: Index out of range");
	videoManPrivate->renderer->imageToScreenCoords( input, x, y );
}

bool VideoManControl::getScreenCoords( const size_t &input, int &left, int &bottom, int &width, int &height )
{
	assert( videoManPrivate->renderer !=NULL && "getScreenCoords(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getScreenCoords: Index out of range");
	return videoManPrivate->renderer->getScreenCoords( input, left, bottom, width, height );
}

bool VideoManControl::getScreenCoordsArea( const size_t &input, int &left, int &bottom, int &width, int &height )
{
	assert( videoManPrivate->renderer !=NULL && "getScreenCoordsArea(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getScreenCoordsArea: Index out of range");
	return videoManPrivate->renderer->getScreenCoordsArea( input, left, bottom, width, height );
}

void VideoManControl::activateViewport( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "activateViewport(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "activateViewport: Index out of range");
	videoManPrivate->renderer->activateViewport( input );
}


void VideoManControl::activateTexture( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "activateTexture(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "activateTexture: Index out of range");
	videoManPrivate->renderer->activateTexture( input );
}

void VideoManControl::getTextureCoords( const size_t &input, float &left, float &bottom, float &right, float &up )
{
	assert( videoManPrivate->renderer !=NULL && "getTextureCoords(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getTextureCoords: Index out of range");
	videoManPrivate->renderer->getTextureCoords( input, left, bottom, right, up );
}

void VideoManControl::setTextureFiltering( int textureFiltering )
{
	assert( videoManPrivate->renderer !=NULL && "setTextureFiltering(): Renderer not created");
	videoManPrivate->renderer->setTextureFiltering( VideoManRenderer::TEXTURE_FILTERING(textureFiltering) );
}

int VideoManControl::getTextureFiltering()
{
	assert( videoManPrivate->renderer !=NULL && "getTextureFiltering(): Renderer not created");
	return videoManPrivate->renderer->getTextureFiltering();
}

void VideoManControl::setKeepAspectRatio( const size_t &input, bool keep )
{
	assert( videoManPrivate->renderer !=NULL && "setKeepAspectRatio(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setKeepAspectRatio: Index out of range");
	videoManPrivate->renderer->setKeepAspectRatio( input, keep );
}

bool VideoManControl::getKeepAspectRatio( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "getKeepAspectRatio(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getKeepAspectRatio: Index out of range");
	return videoManPrivate->renderer->getKeepAspectRatio( input );
}

void VideoManControl::setImageROI( const size_t &input, const int &x, const int &y, const int &width, const int &height )
{
	assert( videoManPrivate->renderer !=NULL && "setImageROI(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setImageROI: Index out of range");
	videoManPrivate->renderer->setImageROI( input, x, y, width, height );
}

void VideoManControl::getImageROI( const size_t &input, int &x, int &y, int &width, int &height )
{
	assert( videoManPrivate->renderer !=NULL && "getImageROI(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getImageROI: Index out of range");
	videoManPrivate->renderer->getImageROI( input, x, y, width, height );
}

void VideoManControl::resetImageROI( const size_t &input )
{
	assert( videoManPrivate->renderer !=NULL && "resetImageROI(): Renderer not created");
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "resetImageROI: Index out of range");
	videoManPrivate->renderer->resetImageROI( input );
}

void VideoManControl::getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	videoManPrivate->factory.getAvailableDevices( deviceList, numDevices );
}

void VideoManControl::getAvailableDevices( const char *identifier, VMInputIdentification **deviceList, int &numDevices )
{
	videoManPrivate->factory.getAvailableDevices( identifier, deviceList, numDevices );
}

void VideoManControl::freeAvailableDevicesList( VMInputIdentification **deviceList, int &numDevices )
{
	videoManPrivate->factory.freeAvailableDevicesList( deviceList, numDevices );
}

int VideoManControl::getNumberOfInputs()
{
	return static_cast<int>( videoManPrivate->videoList.size() );
}

void VideoManControl::setUserInputImage(  const size_t &input, char* image )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setUserInputImage: Index out of range");
	videoManPrivate->videoList[input]->setImage( image );
	if ( videoManPrivate->renderer != NULL )
		videoManPrivate->renderer->updateTexture( input );

}

char *VideoManControl::getUserInputImage(  const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "getUserInputImage: Index out of range");
	return videoManPrivate->videoList[input]->pixelBuffer;
}

void VideoManControl::setFrameCallback( const size_t &input, frameCallback callback, void *data )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "setFrameCallback: Index out of range");
	videoManPrivate->videoList[input]->setFrameCallback( callback, data );
}

bool VideoManControl::supportFrameCallback( const size_t &input )
{
	assert( videoManPrivate->videoList.find( input ) != videoManPrivate->videoList.end() && "supportFrameCallback: Index out of range");
	return videoManPrivate->videoList[input]->supportFrameCallback();
}

void VideoManControl::getSupportedIdentifiers( char **&identifiers, int &numIdentifiers )
{
	videoManPrivate->factory.getSupportedIdentifiers( identifiers, numIdentifiers );
}

void VideoManControl::freeSupportedIdentifiersList( char **&identifiers, int &numIdentifiers )
{
	videoManPrivate->factory.freeSupportedIdentifiersList( identifiers, numIdentifiers );
}

bool VideoManControl::supportedIdentifier( const char *identifier )
{
	return videoManPrivate->factory.supportedIdentifier( identifier );
}
