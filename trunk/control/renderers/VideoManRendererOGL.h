#ifndef VIDEOMANINRENDEREROGL_H
#define VIDEOMANINRENDEREROGL_H

#include <GL/gl.h>
#include "VideoManRenderer.h"
#include "VideoInput.h"
#include <map>
#include <assert.h>

//#pragma comment(lib,"opengl32.lib")
//#pragma comment(lib,"glu32.lib")

/** 
	SUPPORTED FORMATS:
	-RGB24, BGR24
	-RGB32, BGR32
	-GREY16			
	-GREY8
*/
namespace VideoManPrivate
{
class RendererOGLInput : 
	public RendererInput
{
public:
	RendererOGLInput( void );
	virtual ~RendererOGLInput( void );

	GLuint texture;
	int textureWidth, textureHeight;
	float tu1,tu2,tv1,tv2; //texture coordinates
	int x1,x2,y1,y2; //quad coordinates
	unsigned int oglTextureFormat; //RGB24 RGB32
	unsigned int internalFormat;
	GLenum dataType;
	double modelViewMatrix[16];	
	double projectionMatrix[16];
};


class VideoManRendererOGL :
	public VideoManRenderer
{
public:
	VideoManRendererOGL(void);
	virtual ~VideoManRendererOGL(void);

	bool addVideoInput( size_t inpuID, VideoInput *video );
	void renderInputs();
	void renderInput( size_t v );
	void updateTexture( size_t v, const char *image );
	void updateTexture( size_t v );
	
	void deleteVideoInput( size_t inputID );
	
	/** \brief Activate all the video inputs. In the next render all the inputs will be shown		
	*/
	void activateAllVideoInputs();

	/** \brief Activate the video input number v. In the next render only this input will be shown
		\param v [in] The video input index		
	*/
	void activateVideoInput( size_t v );

	/** \brief Deactivate all the video inputs. In the next render any of the inputs will be shown
	*/
	void deactivateAllVideoInputs();

	/** \brief Deactivate the video input number v. In the next render this input will not be shown
		\param v [in] The video input index
	*/
	void deactivateVideoInput( size_t v );

	/** \brief Check if this video input is activated or not
		\param v [in] the video input index
	*/
	bool isActivated( size_t v );

	void changeVisualizationMode( int vM );

	void changeMainVisualizationInput( const size_t &v );
	
	/** \brief Notify to the renderer the change of the screen
		\param left [in] the x coordinate of the top left corner of the region where the renderer must draw
		\param bottom [in] the y coordinate of the bottom left corner of the region where the renderer must draw
		\param width [in] the width of the region where the renderer must draw
		\param height [in] the height of the region where the renderer must draw
	*/
	void changeScreenSize( int left, int bottom, int width, int height );	 	

	int screenToImageCoords( float &x, float &y );	
	
	void imageToScreenCoords( const size_t &v, float &x, float &y );
	
	bool getScreenCoords( const size_t &v, int &left, int &bottom, int &width, int &height );

	bool getScreenCoordsArea( const size_t &input, int &left, int &bottom, int &width, int &height );

	/** \brief Get the coordinates of the input texture
		\param input [in] the video input index
		\param left,up,right,bottom [out] the texture coordinates
	*/
	void getTextureCoords( const size_t &input, float &left, float &bottom, float &right, float &up );

	/** \brief Activate the renderer viewport of this video input
		\param v [in] the video input index
	*/
	void activateViewport( const size_t &v );

	/** \brief Activate the renderer texture of this video input
		\param v [in] the video input index
	*/
	void activateTexture( const size_t &v );

	void setVerticalFlip( const size_t &v, bool value );

	bool getVerticalFlip( const size_t &v );

	void setHorizontalFlip( const size_t &v, bool value );

	bool getHorizontalFlip( const size_t &v );

	void setZoom( const size_t &input, const float &x, const float &y, const float &zoomFactor );
	
	void getZoom( const size_t &input, float &x, float &y, float &zoomFactor );

	void resetZoom( const size_t &input );

	void setRotation( const size_t &input, const float &angle );

	float getRotation( const size_t &input );

	void activate2DDrawingSetup( const size_t &input );

	void drawInputBorder( const size_t &input, const float &thickness, const float &r, const float &g, const float &b );

	void setTextureFiltering( VideoMan::VMTextureFiltering textureFiltering );

	VideoMan::VMTextureFiltering getTextureFiltering();
	
	void setKeepAspectRatio( const size_t &input, bool keep );

	bool getKeepAspectRatio( const size_t &input );

	void setImageROI( const size_t &input, const int &x, const int &y, const int &width, const int &height );
	
	void getImageROI( const size_t &input, int &x, int &y, int &width, int &height );

	void resetImageROI( const size_t &input );

private:
		
	void changeScreenSizeV0( int left, int bottom, int width, int height );

	void changeScreenSizeV1L( int left, int bottom, int width, int height, bool keepPosition );
	
	void changeScreenSizeV1R( int left, int bottom, int width, int height, bool keepPosition );
	
	void changeScreenSizeV1B( int left, int bottom, int width, int height, bool keepPosition );

	void changeScreenSizeV1T( int left, int bottom, int width, int height, bool keepPosition );

	void setScreenCoordinates( int left, int bottom, int width, int height, RendererOGLInput &input );

	typedef std::map<size_t,RendererOGLInput> INPUT_LIST;
	INPUT_LIST inputList;

	void deleteVideoInput( INPUT_LIST::iterator );

	bool generateTexture( RendererOGLInput &input );

	void updateProjection( RendererOGLInput &theInput );

	void updateModelview( RendererOGLInput &theInput );

	GLuint textureFiltering;
};
};
#endif