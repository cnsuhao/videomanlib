#ifdef WIN32
	#include "windows.h"
#endif

#include <math.h>
#include <iostream>
#include "VideoManRendererOGL.h"
#include <GL/glu.h>

#ifndef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#endif

#ifndef GL_BGR
#define GL_BGR GL_BGR_EXT
#endif

#include <stdlib.h>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

RendererOGLInput::RendererOGLInput(void)
{
}

RendererOGLInput::~RendererOGLInput(void)
{
}

void printGLerror( std::string message, GLenum glErr )
{
	switch( glErr )
	{
	case GL_INVALID_ENUM:
		{
			cout << message << " " << "GL_INVALID_ENUM" << endl;
			break;
		}
	case GL_INVALID_VALUE:
		{
			cout << message << " " << "GL_INVALID_VALUE" << endl;
			break;
		}
	case GL_INVALID_OPERATION:
		{
			cout << message << " " << "GL_INVALID_OPERATION" << endl;
			break;
		}
	case GL_STACK_OVERFLOW:
		{
			cout << message << " " << "GL_STACK_OVERFLOW" << endl;
			break;
		}
	case GL_STACK_UNDERFLOW:
		{
			cout << message << " " << "GL_STACK_UNDERFLOW" << endl;
			break;
		}
	case GL_OUT_OF_MEMORY:
		{
			cout << message << " " << "GL_OUT_OF_MEMORY" << endl;
			break;
		}
	}
}

VideoManRendererOGL::VideoManRendererOGL(void)
{
	//supportedFormats.push_back(VM_RGB15);
	supportedFormats.push_back(VM_RGB24);
	supportedFormats.push_back(VM_BGR24);
	supportedFormats.push_back(VM_RGB32);
	supportedFormats.push_back(VM_BGR32);
	supportedFormats.push_back(VM_GREY16);
	supportedFormats.push_back(VM_GREY8);
	supportedFormats.push_back(VM_RAW8);
	textureFiltering = GL_LINEAR;
}

VideoManRendererOGL::~VideoManRendererOGL(void)
{
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )	
		deleteVideoInput( it );
	inputList.clear();
}

bool VideoManRendererOGL::addVideoInput( size_t inpuID, VideoInput *video )
{
	RendererOGLInput input;

	input.videoInput = video;
	VMInputFormat format = video->getVideoManInputFormat();
	VMPixelFormat pF = format.getPixelFormatOut();
	input.depth = format.depth;
	input.nChannels = format.nChannels;
	input.width = format.width;
	input.height = format.height;
	input.align = format.align;
	input.pixelFormat = pF;
	input.supported = supportedFormat( pF );	
	input.keepAspectRatio = true;
	input.imageROI.x = 0;
	input.imageROI.y = 0;
	input.imageROI.width = input.width;
	input.imageROI.height = input.height;
	if ( !input.supported )
	{
		input.activated = false;
		inputList[inpuID] = input;
		return false;
	}
		
	if ( !generateTexture( input ) )
	{
		return false;
	}

	//Initialize screen coordinates
	input.screenCoords.bottom = 0;
	input.screenCoords.left = 0;
	input.screenCoords.width = input.width;
	input.screenCoords.height = input.height;

	input.zoomX = static_cast<float>( input.width ) * 0.5f;
	input.zoomY = static_cast<float>( input.height ) * 0.5f;
	input.zoomFactor = 1.0f;
	input.angle = 0.0f;

	input.x1 = 0;
	input.y1 = 0;
	input.x2 = input.width;
	input.y2 = input.height;
		
	updateModelview( input );
	updateProjection( input );

	inputList[inpuID] = input;
	activateVideoInput( inpuID );

	mainVisualizationInput = inpuID;

	return true;
}

void VideoManRendererOGL::deleteVideoInput( size_t inputID )
{
	assert( inputList.find(inputID) != inputList.end() && "deleteVideoInput: Index out of range");
	INPUT_LIST::iterator it = inputList.find(inputID);
	deleteVideoInput( it );
	inputList.erase( it );
}


void VideoManRendererOGL::deleteVideoInput( INPUT_LIST::iterator it )
{
	assert( it != inputList.end() && "deleteVideoInput: Index out of range");
	glDeleteTextures( 1, &it->second.texture );
	deactivateVideoInput( it->first );
	it->second.supported = false;	
	changeScreenSize( screenSize.left,screenSize.bottom,screenSize.width,screenSize.height  );	
}


bool VideoManRendererOGL::generateTexture( RendererOGLInput &inputOGL )
{
	GLenum glErr;
	glErr = glGetError();
	if ( glErr != GL_NO_ERROR)	
		printGLerror( "Error in OpenGL stack", glErr );
	const char *glVersion = (const char*)glGetString(GL_VERSION);
	if ( !glVersion )
	{
		cout << "Error generating texture OpenGL context not initialized" << endl;
		return false;
	}

	GLint texSize; 
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &texSize );

	bool gl2Supported = false;
	gl2Supported = atof(glVersion) >= 2.0;		
	if ( !gl2Supported )
	{
		int pow = 2;
		while( pow < inputOGL.width )//&& pow <= MAX_SIZE )
		{
			pow = pow * 2;
		}
		inputOGL.textureWidth = pow;
		pow = 2;
		while( pow<inputOGL.height )//&& pow<=MAX_SIZE )
		{
			pow = pow * 2;
		}
		inputOGL.textureHeight = pow;
	}
	else
	{
		//Texture non-power of two supported
		inputOGL.textureWidth = inputOGL.width;
		inputOGL.textureHeight = inputOGL.height;
	}
	if ( inputOGL.textureWidth > texSize || inputOGL.textureHeight > texSize )
	{
		cout << "Error generating texture GPU max texture size is " << texSize << endl;
		return false;
	}


	float Bpp = static_cast<float>( inputOGL.depth ) / 8.0f;
	//std::vector<char*> data(textureHeight * textureWidth * static_cast<size_t>( Bpp ), 0);	
	glGenTextures( 1, &inputOGL.texture );
	glBindTexture( GL_TEXTURE_2D, inputOGL.texture );

	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, textureFiltering );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, textureFiltering );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP );
	
	inputOGL.dataType = GL_UNSIGNED_BYTE;
	switch ( inputOGL.pixelFormat )
	{
		case VM_GREY16:
		case VM_RAW16:
		{
			inputOGL.oglTextureFormat = GL_LUMINANCE;
			inputOGL.internalFormat = GL_LUMINANCE16;
			inputOGL.dataType = GL_UNSIGNED_SHORT;			
			break;
		}
		case VM_RAW8:
		case VM_GREY8:	
		{
			inputOGL.oglTextureFormat = GL_LUMINANCE;
			inputOGL.internalFormat = 1;
			inputOGL.dataType = GL_UNSIGNED_BYTE;
			break;
		}
		case VM_BGR24:
		{			
			inputOGL.oglTextureFormat = GL_BGR;
			inputOGL.internalFormat = 3;
			inputOGL.dataType = GL_UNSIGNED_BYTE;
			break;
		}
		case VM_RGB24:
		{
			inputOGL.oglTextureFormat = GL_RGB;
			inputOGL.internalFormat = 3;
			inputOGL.dataType = GL_UNSIGNED_BYTE;
			break;
		}
		/*case VM_RGB15:
		{
			inputOGL.oglTextureFormat = GL_RGB;
			inputOGL.internalFormat = GL_RGB5;
			inputOGL.dataType = GL_UNSIGNED_BYTE;
			break;
		}*/
		case VM_BGR32:
		{
			inputOGL.oglTextureFormat = GL_BGRA;
			inputOGL.internalFormat = 3;
			inputOGL.dataType = GL_UNSIGNED_BYTE;
			break;
		}
		case VM_RGB32:
		{
			inputOGL.oglTextureFormat = GL_RGBA;
			inputOGL.internalFormat = 3;
			inputOGL.dataType = GL_UNSIGNED_BYTE;
			break;
		}
		default: 
			{
				return false;
			}
	}
	glTexImage2D( GL_TEXTURE_2D, 0, inputOGL.internalFormat, inputOGL.textureWidth, inputOGL.textureHeight, 0, inputOGL.oglTextureFormat, inputOGL.dataType, NULL );

	glErr = glGetError();
	if ( glErr != GL_NO_ERROR)
	{
		printGLerror( "Error generating OpenGL texture", glErr );
		return false;
	}

	//Calculate texture coordinates
	inputOGL.tu1 = 0.0f;
	inputOGL.tv1 = 0.0f;
	inputOGL.tu2 = static_cast<float>( inputOGL.width / (float)inputOGL.textureWidth );
	inputOGL.tv2 = static_cast<float>( inputOGL.height / (float)inputOGL.textureHeight );
	
	return true;
}


void VideoManRendererOGL::renderInputs()
{
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )	
		renderInput( it->first );
}


void VideoManRendererOGL::renderInput( size_t v )
{
	assert( inputList.find(v) != inputList.end() && "renderInput: Index out of range");
	const RendererOGLInput &theInput = inputList[v];
	if ( !theInput.activated )
		return;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, theInput.texture);
	activate2DDrawingSetup( v );
	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( theInput.tu1, theInput.tv1 );
		glVertex2i( theInput.x1,  theInput.y1 );
		glTexCoord2f( theInput.tu1, theInput.tv2 );
		glVertex2i( theInput.x1, theInput.y2 );
		glTexCoord2f( theInput.tu2, theInput.tv1 );
		glVertex2i( theInput.x2,  theInput.y1 );
		glTexCoord2f( theInput.tu2, theInput.tv2 );
		glVertex2i( theInput.x2, theInput.y2 );
	glEnd();
}

void VideoManRendererOGL::updateTexture( size_t v )
{
	assert( inputList.find(v) != inputList.end() && "updateTexture: Index out of range");
	const RendererOGLInput &theInput = inputList[v];
	glPixelStorei( GL_UNPACK_ALIGNMENT, theInput.align );
	glBindTexture( GL_TEXTURE_2D, theInput.texture );
	if ( theInput.videoInput->pixelBuffer != NULL )
	{
		glTexSubImage2D( GL_TEXTURE_2D, 0, theInput.imageROI.x, theInput.imageROI.y, theInput.imageROI.width, theInput.imageROI.height,
		theInput.oglTextureFormat, theInput.dataType, (void*)theInput.videoInput->pixelBuffer);
	}
}

void VideoManRendererOGL::updateTexture( size_t v, const char *image )
{
	assert( inputList.find(v) != inputList.end() && "updateTexture: Index out of range");
	assert( image != NULL && "updateTexture: Invalid Image");
	const RendererOGLInput &theInput = inputList[v];
	glPixelStorei( GL_UNPACK_ALIGNMENT, theInput.align );
	glBindTexture( GL_TEXTURE_2D, theInput.texture );	
	//glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, theInput.width, theInput.height,
	//	theInput.oglTextureFormat, theInput.dataType, (void*)image );
	glTexSubImage2D( GL_TEXTURE_2D, 0, theInput.imageROI.x, theInput.imageROI.y, theInput.imageROI.width, theInput.imageROI.height,
		theInput.oglTextureFormat, theInput.dataType, (void*)image );	
}

void VideoManRendererOGL::activateAllVideoInputs()
{
	activatedInputs = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )	
	{
		if ( it->second.supported )
		{
			it->second.activated = true;
			activatedInputs++;
		}
	}
	changeScreenSize( screenSize.left,screenSize.bottom,screenSize.width,screenSize.height  );
}

void VideoManRendererOGL::activateVideoInput( size_t v )
{
	assert( inputList.find(v) != inputList.end() && "activateVideoInput: Index out of range");
	RendererOGLInput &theInput = inputList[v];
	if ( theInput.activated || !theInput.supported )
		return;

	activatedInputs++;
	theInput.activated = true;
	changeScreenSize( screenSize.left,screenSize.bottom,screenSize.width,screenSize.height  );	
}

void VideoManRendererOGL::deactivateAllVideoInputs()
{
	activatedInputs = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
		it->second.activated = false;
	changeScreenSize( screenSize.left,screenSize.bottom,screenSize.width,screenSize.height  );
}

void VideoManRendererOGL::deactivateVideoInput( size_t v )
{
	assert( inputList.find(v) != inputList.end() && "deactivateVideoInput: Index out of range");
	RendererOGLInput &theInput = inputList[v];
	if ( !theInput.activated )
		return;
	activatedInputs--;
	theInput.activated=false;
	if ( mainVisualizationInput == v )
	{
		for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
		{
			if ( it->second.activated )
			{
				mainVisualizationInput = it->first;
				break;
			}
		}
	}
	changeScreenSize( screenSize.left,screenSize.bottom,screenSize.width,screenSize.height  );
}

bool VideoManRendererOGL::isActivated( size_t v )
{
	assert( inputList.find(v) != inputList.end() && "isActivated: Index out of range");
	return ( inputList[v].activated );
}

void VideoManRendererOGL::changeVisualizationMode( int vM )
{
	visualizationMode = vM;
	changeScreenSize( screenSize.left, screenSize.bottom, screenSize.width, screenSize.height  );	
}

void VideoManRendererOGL::changeMainVisualizationInput( const size_t &v )
{
	assert( inputList.find(v) != inputList.end() && "changeMainVisualizationInput: Index out of range");
	if ( inputList[v].activated )
	{
		mainVisualizationInput = v;
		changeScreenSize( screenSize.left, screenSize.bottom, screenSize.width, screenSize.height  );	
	}
}

void VideoManRendererOGL::changeScreenSize( int left, int bottom, int width, int height )
{
	screenSize.init( left, bottom, width, height );
	if ( screenSize.height <= 0 || screenSize.width <= 0 )
		return;
	if ( inputList.size() == 0 || activatedInputs == 0 )
		return;
	if ( activatedInputs <= 1 )
	{
		changeScreenSizeV0( left, bottom, width, height );
		return;
	}
	switch ( visualizationMode )
	{
		case 0:
		default:
		{
			changeScreenSizeV0( left, bottom, width, height );
			break;
		}
		case 1:
		{
			changeScreenSizeV1L( left, bottom, width, height, true );
			break;
		}
		case 2:
		{
			changeScreenSizeV1B( left, bottom, width, height, true );
			break;
		}
		case 3:
		{	
			changeScreenSizeV1R( left, bottom, width, height, true );
			break;		
		}
		case 4:
		{
			changeScreenSizeV1T( left, bottom, width, height, true );
			break;
		}
		case 5:
		{
			changeScreenSizeV1L( left, bottom, width, height, false );
			break;
		}
		case 6:
		{
			changeScreenSizeV1B( left, bottom, width, height, false );
			break;
		}
		case 7:
		{	
			changeScreenSizeV1R( left, bottom, width, height, false );
			break;		
		}
		case 8:
		{
			changeScreenSizeV1T( left, bottom, width, height, false );
			break;
		}
	}
}


void VideoManRendererOGL::changeScreenSizeV0( int left, int bottom, int width, int height )
{
	float numColsF = sqrt( ( (float)screenSize.width / (float)screenSize.height ) * 1.33333f * (float) activatedInputs );
	int numCols = static_cast<int>( floor( numColsF ) );
	if ( numCols == 0 ) numCols = 1;
	if ( numCols > activatedInputs ) numCols = activatedInputs;
	//if ((numColsF-numCols)>(numColsF-(numCols+1)))
	//	numCols=numCols+1;
	int numFils = static_cast<int>( ceil( (float)activatedInputs / (float)numCols ) );

	float despC = 0.0f;
	float despF = 0.0f;
	float despCs = 0.0f;
	float despFs = 0.0f;	
	int actual = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
	{
		if ( it->second.activated )
		{
			//Calculate Which is its row
			int row = static_cast<int>( floor( (float)actual / (float)numCols ) );
			//Calculate Which is its column
			int col = actual % numCols;
			despC = (float)col / (float)numCols;
			despF = (float)row / (float)numFils;
			despCs = (float)(col+1) / (float)numCols;
			despFs = (float)(row+1) / (float)numFils;
			int iniC = static_cast<int>( screenSize.width * despC );
			int finC = static_cast<int>( screenSize.width * despCs );
			int iniF = static_cast<int>( screenSize.height - screenSize.height * despFs );
			int finF = static_cast<int>( screenSize.height - screenSize.height * despF);
			int tamX = finC - iniC;
			int tamY = finF - iniF;

			setScreenCoordinates( iniC, iniF, tamX, tamY, it->second );			
			actual++;
		}
	}	
}

void VideoManRendererOGL::setKeepAspectRatio( const size_t &input, bool keep )
{
	assert( inputList.find(input) != inputList.end() && "setKeepAspectRatio: Index out of range");
	RendererOGLInput &theInput = inputList[input];
	if ( keep != theInput.keepAspectRatio )
	{
		theInput.keepAspectRatio = keep;
		updateProjection( theInput );
	}
}

bool VideoManRendererOGL::getKeepAspectRatio( const size_t &input )
{
	assert( inputList.find(input) != inputList.end() && "getKeepAspectRatio: Index out of range");
	return inputList[input].keepAspectRatio;
}

void VideoManRendererOGL::setScreenCoordinates( int left, int bottom, int width, int height, RendererOGLInput &input )
{
	input.screenCoords.bottom = bottom;
	input.screenCoords.height = height;
	input.screenCoords.width = width;
	input.screenCoords.left = left;
	updateProjection( input );
}


void VideoManRendererOGL::changeScreenSizeV1L( int left, int bottom, int width, int height, bool keepPosition )
{
	int despY;
	if ( keepPosition )
		despY = height / static_cast<int>( activatedInputs );
	else
		despY = height / static_cast<int>( activatedInputs - 1 );
	int numInput = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
	{
		if ( it->second.activated )
		{
			if ( mainVisualizationInput == it->first )
				setScreenCoordinates( left + width * 1 / 4, bottom, width * 3 / 4, height, it->second );								
			else
				setScreenCoordinates( left, bottom +  despY * numInput, width * 1 / 4, despY, it->second );				
			if ( keepPosition || mainVisualizationInput != it->first)
				++numInput;
		}
	}
}

void VideoManRendererOGL::changeScreenSizeV1R( int left, int bottom, int width, int height, bool keepPosition )
{
	int despY;
	if ( keepPosition )
		despY = height / static_cast<int>( activatedInputs );
	else
		despY = height / static_cast<int>( activatedInputs - 1 );
	int numInput = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
	{
		if ( it->second.activated )
		{
			if ( mainVisualizationInput == it->first )
				setScreenCoordinates( left, bottom, width * 3 / 4, height, it->second );								
			else
				setScreenCoordinates( left + width * 3 / 4, bottom +  despY * numInput, width * 1 / 4, despY, it->second );				
			if ( keepPosition || mainVisualizationInput != it->first)
				++numInput;
		}		
	}
}

void VideoManRendererOGL::changeScreenSizeV1B( int left, int bottom, int width, int height, bool keepPosition )
{
	int despY;
	if ( keepPosition )
		despY = width / static_cast<int>( activatedInputs );
	else
		despY = width / static_cast<int>( activatedInputs - 1 );
	int numInput = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
	{
		if ( it->second.activated )
		{
			if ( mainVisualizationInput == it->first )
				setScreenCoordinates( left, bottom + height * 1 / 4, width, height * 3 / 4, it->second );								
			else
				setScreenCoordinates( left + despY * numInput, bottom, despY, height * 1 / 4, it->second );				
			if ( keepPosition || mainVisualizationInput != it->first)
				++numInput;
		}
	}
}

void VideoManRendererOGL::changeScreenSizeV1T( int left, int bottom, int width, int height, bool keepPosition )
{
	int despY;
	if ( keepPosition )
		despY = width / static_cast<int>( activatedInputs );
	else
		despY = width / static_cast<int>( activatedInputs - 1 );
	int numInput = 0;
	for( INPUT_LIST::iterator it = inputList.begin(); it != inputList.end(); it++ )
	{
		if ( it->second.activated )
		{
			if ( mainVisualizationInput == it->first )
				setScreenCoordinates( left, bottom, width, height * 3 / 4, it->second );								
			else
				setScreenCoordinates( left + despY * numInput, bottom  + height * 3 / 4, despY, height * 1 / 4, it->second );				
			if ( keepPosition || mainVisualizationInput != it->first)
				++numInput;
		}
	}
}

int VideoManRendererOGL::screenToImageCoords( float &x, float &y )
{
	size_t i = 0;
	INPUT_LIST::iterator selectedIt;
	int selectedID = -1;
	y = screenSize.height - y;
	INPUT_LIST::iterator it = inputList.begin();
	while( selectedID == -1 && it != inputList.end() )
	{	
		const RendererOGLInput &theInput = it->second;
		if ( theInput.activated )
		{
			float dY = y - theInput.screenCoords.bottom;
			float dX = x - theInput.screenCoords.left;
			if ( dX >= 0 && dX <= theInput.screenCoords.width &&
				dY >= 0 && dY <= theInput.screenCoords.height )
			{
				selectedID = it->first;
				selectedIt = it;
			}
		}
		it++;
	}
	if ( selectedID == -1 )
		return -1;	
	const RendererOGLInput &selectedInput = selectedIt->second;
	double wx = (double)x;
	double wy = (double)y;
	//activate2DDrawingSetup( selectedID );
	int view[] = {selectedInput.screenCoords.left,selectedInput.screenCoords.bottom, selectedInput.screenCoords.width,selectedInput.screenCoords.height};
	//glGetIntegerv( GL_VIEWPORT, view );
	double ix,iy,iz;
	gluUnProject( wx, wy, 0.0, selectedInput.modelViewMatrix, selectedInput.projectionMatrix, view, &ix, &iy, &iz );
	x = (float)ix; 
	y = (float)iy;
	if ( x >= 0 && x < selectedInput.width && y >= 0 && y < selectedInput.height )
		return selectedID;
	else
		return -1;
}

void VideoManRendererOGL::imageToScreenCoords( const size_t &v, float &x, float &y )
{
	assert( inputList.find(v) != inputList.end() && "imageToScreenCoords: Index out of range");
	const RendererOGLInput &theInput = inputList[v];
	
	//activate2DDrawingSetup( v );
	//int view[4];
	int view[] = {theInput.screenCoords.left,theInput.screenCoords.bottom, theInput.screenCoords.width,theInput.screenCoords.height};
	//glGetIntegerv( GL_VIEWPORT, view );	
	double wx,wy,wz;	
	gluProject( x, y, 0.0, theInput.modelViewMatrix, theInput.projectionMatrix, view, &wx, &wy, &wz );
	x = (float)wx;
	y = (float)wy;
}

bool VideoManRendererOGL::getScreenCoords( const size_t &v, int &left, int &bottom, int &width, int &height )
{
	assert( inputList.find(v) != inputList.end() && "getScreenCoords: Index out of range");
	const RendererOGLInput &theInput = inputList[v];
	if ( !theInput.activated )
		return false;
	float l,b,t,r;
	l = b = 0;
	t = (float)theInput.height;
	r = (float)theInput.width;
	imageToScreenCoords( v, l, b );
	imageToScreenCoords( v, r, t );	
	if ( theInput.horizontalFlip )
	{
		width = static_cast<int>( l - r + 1 );
		left = (int)r;
	}
	else
	{
		width = static_cast<int>( r - l + 1 );		
		left = (int)l;		

	}
	if ( theInput.verticalFlip )	
	{
		bottom = (int)t;
		height = static_cast<int>( b - t + 1 );
	}
	else
	{				
		bottom = (int)b;
		height = static_cast<int>( t - b + 1 );
	}
	return true;
}

bool VideoManRendererOGL::getScreenCoordsArea( const size_t &v, int &left, int &bottom, int &width, int &height )
{
	assert( inputList.find(v) != inputList.end() && "getScreenCoordsArea: Index out of range");
    const RendererOGLInput &theInput = inputList[v];
    if ( !theInput.activated )
        return false;
    left = theInput.screenCoords.left;
    bottom = theInput.screenCoords.bottom;
    width = theInput.screenCoords.width;
    height = theInput.screenCoords.height;
    return true;
}

void VideoManRendererOGL::getTextureCoords( const size_t &input, float &left, float &bottom, float &right, float &up )
{
	assert( inputList.find( input ) != inputList.end() && "getTextureCoords: Index out of range");
	const RendererOGLInput &theInput = inputList[input];
	left = theInput.tu1;
	right = theInput.tu2;
	up = theInput.tv2;
	bottom = theInput.tv1;

}

void VideoManRendererOGL::activateViewport( const size_t &v )
{
	assert( inputList.find(v) != inputList.end() && "activateViewport: Index out of range");
	glViewport( inputList[v].screenCoords.left, inputList[v].screenCoords.bottom, inputList[v].screenCoords.width, inputList[v].screenCoords.height);
}

void VideoManRendererOGL::activateTexture( const size_t &v )
{
	assert( inputList.find(v) != inputList.end() && "activateTexture: Index out of range");
	glBindTexture(GL_TEXTURE_2D, inputList[v].texture);
}

void VideoManRendererOGL::setVerticalFlip( const size_t &v, bool value )
{
	assert( inputList.find(v) != inputList.end() && "setVerticalFlip: Index out of range");
	RendererOGLInput &theInput = inputList[v];
	theInput.verticalFlip = value;
	updateModelview( theInput );
}

bool VideoManRendererOGL::getVerticalFlip( const size_t &v )
{
	assert( inputList.find(v) != inputList.end() && "getVerticalFlip: Index out of range");	
	return inputList[v].verticalFlip;
}

void VideoManRendererOGL::setHorizontalFlip( const size_t &v, bool value )
{
	assert( inputList.find(v) != inputList.end() && "setHorizontalFlip: Index out of range");
	RendererOGLInput &theInput = inputList[v];
	theInput.horizontalFlip = value;
	updateModelview( theInput );
}

bool VideoManRendererOGL::getHorizontalFlip( const size_t &v )
{
	assert( inputList.find(v) != inputList.end() && "getHorizontalFlip: Index out of range");	
	return inputList[v].horizontalFlip;
}

void VideoManRendererOGL::updateModelview( RendererOGLInput &theInput )
{
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( theInput.width * 0.5f, theInput.height * 0.5f, 0.0f );	
	//zoom
	glScalef( theInput.zoomFactor, theInput.zoomFactor, theInput.zoomFactor );
	//rotation
	glRotatef( theInput.angle, 0, 0, 1 );
	//Flip
	if ( theInput.verticalFlip )
		glScalef( 1.0f, -1.0f, 1.0f );
	if ( theInput.horizontalFlip )
		glScalef( -1.0f, 1.0f, 1.0f );
	glTranslatef( -theInput.width * 0.5f, -theInput.height * 0.5f, 0.0f );	
	//translation
	float tx, ty;
	tx = theInput.zoomX - theInput.width * 0.5f;//1.0f - 2.0f * x / static_cast<float>( theInput.width );	
	ty = theInput.zoomY - theInput.height * 0.5f;//1.0f - 2.0f * y / static_cast<float>( theInput.height );
	glTranslatef( -tx, -ty, 0.0f );
	glGetDoublev( GL_MODELVIEW_MATRIX, theInput.modelViewMatrix );
	glPopMatrix();
}

void VideoManRendererOGL::setZoom( const size_t &input, const float &x, const float &y, const float &zoomFactor )
{
	assert( inputList.find(input) != inputList.end() && "setZoom: Index out of range");
	RendererOGLInput &theInput = inputList[input];
	if ( zoomFactor <= 0 )
		return;
	theInput.zoomX = x;
	theInput.zoomY = y;
	theInput.zoomFactor = zoomFactor;
	updateModelview( theInput );
}
	
void VideoManRendererOGL::getZoom( const size_t &input, float &x, float &y, float &zoomFactor )
{
	assert( inputList.find(input) != inputList.end() && "getZoom: Index out of range");
	const RendererOGLInput &theInput = inputList[input];
	x = theInput.zoomX;
	y = theInput.zoomY;
	zoomFactor = theInput.zoomFactor;    
}

void VideoManRendererOGL::resetZoom( const size_t &input )
{
	assert( inputList.find(input) != inputList.end() && "resetZoom: Index out of range");
	const RendererOGLInput &theInput = inputList[input];
	setZoom( input, theInput.width * 0.5f, theInput.height * 0.5f, 1.0 );
}

void VideoManRendererOGL::setRotation( const size_t &input, const float &angle )
{
	assert( inputList.find(input) != inputList.end() && "setRotation: Index out of range");
	RendererOGLInput &theInput = inputList[input];
	theInput.angle = angle;	
	updateModelview( theInput );
}

float VideoManRendererOGL::getRotation( const size_t &input )
{
	assert( inputList.find(input) != inputList.end() && "setRotation: Index out of range");
	RendererOGLInput &theInput = inputList[input];
	return theInput.angle;
}

void VideoManRendererOGL::updateProjection( RendererOGLInput &theInput )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	if ( theInput.keepAspectRatio )
	{
		const float screenAspect = (float)theInput.screenCoords.width / theInput.screenCoords.height;
		const float inputAspect = (float)theInput.width / theInput.height;
		if ( screenAspect > inputAspect )
		{
			const float margen = ( (float) theInput.screenCoords.width  * theInput.height / theInput.screenCoords.height ) - theInput.width;
			glOrtho( -0.5f*margen, theInput.width + 0.5f *margen, 0, theInput.height, -1.0, 1.0 );
		}
		else
		{
			const float margen = ( (float) theInput.screenCoords.height * theInput.width / theInput.screenCoords.width ) - theInput.height;
			glOrtho( 0, theInput.width, -0.5f * margen, theInput.height + 0.5f * margen, -1.0, 1.0 );
		}
	}
	else
		glOrtho( 0.0, theInput.width, 0.0, theInput.height, -1.0, 1.0 );
	glGetDoublev( GL_PROJECTION_MATRIX, theInput.projectionMatrix );
}

void VideoManRendererOGL::activate2DDrawingSetup( const size_t &input )
{
	assert( inputList.find(input) != inputList.end() && "activate2DDrawingSetup: Index out of range");
	const RendererOGLInput &theInput = inputList[input];
	activateViewport( input );
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixd( theInput.projectionMatrix );
	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixd( theInput.modelViewMatrix );
}

void VideoManRendererOGL::drawInputBorder( const size_t &input, const float &thickness, const float &r, const float &g, const float &b )
{
	assert( inputList.find(input) != inputList.end() && "drawInputBorder: Index out of range");
	const RendererOGLInput &theInput = inputList[input];
	if ( !theInput.activated )
		return;
	glPushAttrib( GL_ENABLE_BIT );
	glPushAttrib( GL_LINE_BIT );
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3f( r, g, b );
	glViewport( theInput.screenCoords.left, theInput.screenCoords.bottom, theInput.screenCoords.width, theInput.screenCoords.height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();	
	glLineWidth( thickness );		
	glBegin( GL_LINE_LOOP );
		glVertex2i( -1,  1 );
		glVertex2i( -1, -1 );
		glVertex2i(  1, -1 );
		glVertex2i(  1,  1 );
	glEnd();
	glPopAttrib();
	glPopAttrib();
}

void VideoManRendererOGL::setTextureFiltering( VMTextureFiltering _textureFiltering )
{
	switch ( _textureFiltering )
	{
	case VM_NEAREST:
		textureFiltering = GL_NEAREST;
		break;
	case VM_LINEAR:
		textureFiltering = GL_LINEAR;
		break;
	}
}

VMTextureFiltering VideoManRendererOGL::getTextureFiltering()
{
	switch ( textureFiltering )
	{
	case GL_NEAREST:
		return VM_NEAREST;		
	case GL_LINEAR:
		return VM_LINEAR;		
	}
	return VM_LINEAR;
}


void VideoManRendererOGL::setImageROI( const size_t &input, const int &x, const int &y, const int &width, const int &height )
{
	assert( inputList.find(input) != inputList.end() && "setImageROI: Index out of range");
	RendererOGLInput &theInput = inputList[input];
	assert( x >= 0 && y >= 0 && "setImageROI: ROI coordinates must be >= 0");
	assert( x + width <= theInput.width && y + height <= theInput.height && "setImageROI: ROI size too big");

	theInput.imageROI.x = x;
	theInput.imageROI.y = y;
	theInput.imageROI.width = width;
	theInput.imageROI.height = height;
	
	theInput.tu1 = static_cast<float>( theInput.imageROI.x ) / theInput.textureWidth;
	theInput.tv1 = static_cast<float>( theInput.imageROI.y ) / theInput.textureHeight;
	theInput.tu2 = theInput.tu1 + static_cast<float>( theInput.imageROI.width ) / theInput.textureWidth;
	theInput.tv2 = theInput.tv1 + static_cast<float>( theInput.imageROI.height ) / theInput.textureHeight;

	theInput.x1 = x;
	theInput.y1 = y;
	theInput.x2 = x + width - 1;
	theInput.y2 = y + height - 1;
}
	
void VideoManRendererOGL::getImageROI( const size_t &input, int &x, int &y, int &width, int &height )
{
	assert( inputList.find(input) != inputList.end() && "getImageROI: Index out of range");
	const RendererOGLInput &theInput = inputList[input];
	x = theInput.imageROI.x;
	y = theInput.imageROI.y;
	width = theInput.imageROI.width;
	height = theInput.imageROI.height;
}

void VideoManRendererOGL::resetImageROI( const size_t &input )
{
	assert( inputList.find(input) != inputList.end() && "resetImageROI: Index out of range");
	RendererOGLInput &theInput = inputList[input];
	theInput.imageROI.x = 0;
	theInput.imageROI.y = 0;
	theInput.imageROI.width = theInput.width;
	theInput.imageROI.height = theInput.height;
	
	theInput.tu1 = 0.0f;
	theInput.tv1 = 0.0f;
	theInput.tu2 = static_cast<float>( theInput.width / (float)theInput.textureWidth );
	theInput.tv2 = static_cast<float>( theInput.height / (float)theInput.textureHeight );		

	theInput.x1 = 0;
	theInput.x2 = theInput.width;
	theInput.y1 = 0;
	theInput.y2 = theInput.height;
}
