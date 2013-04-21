#ifndef VIDEOMANINRENDERER_H
#define VIDEOMANINRENDERER_H

#include "VideoInput.h"
#include "VideoManControl.h"
#include <list>

namespace VideoManPrivate
{
typedef struct 
{
	int left, bottom, width, height;
	void init(int aleft, int abottom, int awidth, int aheight)
	{
		left=aleft;
		bottom=abottom;
		width=awidth;
		height=aheight;
	}
}SCREEN_SIZE;

struct IMAGE_ROI
{
	int x,y,width,height;
};

class RendererInput
{
public:
	RendererInput( void );
	virtual ~RendererInput( void );

	VideoInput *videoInput;
	SCREEN_SIZE screenCoords;
	SCREEN_SIZE viewport;	
	int width;
	int height;
	int depth;
	int align;
	int nChannels;
	bool activated; //If the input is activated it will be rendered
	bool supported; //The input format is or not supported
	VideoMan::VMPixelFormat pixelFormat;
	bool verticalFlip; //If the input is displayed vertically fliped
	bool horizontalFlip; //If the input is displayed horizontally fliped
	float zoomX, zoomY, zoomFactor; //zoom values
	float angle;
	bool keepAspectRatio;
	IMAGE_ROI imageROI;
};

class VideoManRenderer
{
public:
	VideoManRenderer(void);
	virtual ~VideoManRenderer(void);

	virtual bool addVideoInput( size_t inpuID, VideoInput *video ) = 0;
	virtual void renderInputs() = 0;
	virtual void renderInput( size_t inpuID ) = 0;
	virtual void updateTexture( size_t inpuID, const char *image ) = 0;
	virtual void updateTexture( size_t inpuID ) = 0;

	virtual void deleteVideoInput( size_t inputID ) = 0;
		
	/** \brief Activate all the video inputs. In the next render all the inputs will be shown		
	*/
	virtual void activateAllVideoInputs() = 0;

	/** \brief Activate this video input. In the next render only this input will be shown
		\param inpuID [in] the video input index		
	*/
	virtual void activateVideoInput( size_t inpuID ) = 0;

	/** \brief Deactivate all the video inputs. In the next render any of the inputs will be shown
	*/
	virtual void deactivateAllVideoInputs() = 0;

	/** \brief Deactivate the video input number v. In the next render this input will not be shown
		\param inpuID [in] the video input index
	*/
	virtual void deactivateVideoInput( size_t inpuID ) = 0;

	/** \brief Check if this video input is activated or not
		\param inpuID [in] the video input index
	*/
	virtual bool isActivated( size_t inpuID ) = 0;
	

	/** \brief Notify to the renderer the change of the screen
		\param left [in] the x coordinate of the top left corner of the region where the renderer must draw
		\param up [in] the y coordinate of the top left corner of the region where the renderer must draw
		\param width [in] the width of the region where the renderer must draw
		\param height [in] the height of the region where the renderer must draw
	*/
	virtual void changeScreenSize( int left, int up, int width, int height ) = 0;

	/** \brief Transform screen coordinates to image coordinates
		\param x [in\out] the x or horizontal coordinate
		\param y [in\out] the y or vertical coordinate
		\return if the coordinates are inside the image of any of the inputs, the index of that input is returned, -1 otherwise		
	*/
	virtual int screenToImageCoords( float &x, float &y ) = 0;

	/** \brief Transform image (corresponding to a specific input) coordinates to screen coordinates
		\param inpuID [in] the video input index
		\param x,y [in\out] the x,y (horizontal,vertical) coordinates	
	*/
	virtual void imageToScreenCoords( const size_t &inpuID, float &x, float &y ) = 0;	
	
	
	/** \brief Get the screen coordinates of the input image coordinates
		\param inpuID [in] the video input index
		\param left, up, width, height [out] the returned screen coordinates
		\return true if the input is activated, false otherwise
	*/
	virtual bool getScreenCoords( const size_t &inpuID, int &left, int &up, int &width, int &height ) = 0;

	/** \brief Get the screen coordinates of the area where the input is being displayed
		\param inpuID [in] the video input index
		\param left, up, width, height [out] the returned screen coordinates
		\return true if the input is activated, false otherwise
	*/
	virtual bool getScreenCoordsArea( const size_t &input, int &left, int &bottom, int &width, int &height ) = 0;

	/** \brief Get the coordinates of the input texture
		\param input [in] the video input index
		\param left,up,right,bottom [out] the texture coordinates
	*/
	virtual void getTextureCoords( const size_t &inpuID, float &left, float &bottom, float &right, float &up ) = 0;

	/** \brief Activate the renderer viewport of this video input
		\param inpuID [in] the video input index
	*/
	virtual void activateViewport( const size_t &inpuID ) = 0;

	/** \brief Activate the renderer texture of this video input
		\param inpuID [in] the video input index
	*/
	virtual void activateTexture( const size_t &inpuID ) = 0;

	/** \brief Change the visualization layout, i.e., the way the inputs are displayed
		\param layout [in] The layout number
	*/
	virtual void changeVisualizationMode( int vM ) = 0;

	/** \brief Change the input that is shown in the principal location of the layout, the effect depends on the visualization layout
		\param inpuID [in] the video input index
	*/
	virtual void changeMainVisualizationInput( const size_t &inpuID ) = 0;

	/**	\brief Get the index of the main visualization input
		\return the index of the main visualization input
	*/
	int getMainVisualizationInput();

	/** \brief  Turn the vertical flip (visualization) on or off
		\param inpuID [in] the video input index
		\param value [in] true turn on\false turn off
	*/
	virtual void setVerticalFlip( const size_t &inpuID, bool value ) = 0;
	
	/** \brief  Check if the vertical flip is on or off
		\param inpuID [in] the video input index		
		\return true if the vertical flip is on
	*/
	virtual bool getVerticalFlip( const size_t &inpuID ) = 0;
	
	/** \brief  Turn the horizontal flip (visualization) on or off
		\param inpuID [in] the video input index
		\param value [in] true turn on\false turn off
	*/
	virtual void setHorizontalFlip( const size_t &inpuID, bool value ) = 0;
	
	/** \brief  Check if the horizontal flip is on or off
		\param inpuID [in] the video input index		
		\return true if the horizontal flip is on
	*/
	virtual bool getHorizontalFlip( const size_t &inpuID ) = 0;

	/** \brief Set the zoom values. Following renders will show the zoom
		\param input [in] the video input index	
		\param x,y [in] image coordinates of the zoom center
		\param zoomFactor [in] the zoom scale ( 1.0 no zoom )
	*/
	virtual void setZoom( const size_t &input, const float &x, const float &y, const float &zoomFactor ) = 0;
	
	/** \brief Get the zoom values
		\param input [in] the video input index	
		\param x,y,zoomFactor [out] the zoom values
	*/
	virtual void getZoom( const size_t &input, float &x, float &y, float &zoomFactor ) = 0;

	/** \brief Reset the zoom value
		\param input [in] the video input index	
	*/
	virtual void resetZoom( const size_t &input ) = 0;

	/** \brief Se the rotation angle
		\param input [in] the video input index	
		\param angle [in] Angle in degrees
	*/
	virtual void setRotation( const size_t &input, const float &angle ) = 0;

	/** \brief Get the rotation angle
		\return the angle in degrees
	*/
	virtual float getRotation( const size_t &input ) = 0;

	/** \brief Activate the 2D drawing setup for the input in order to draw 2D elemnts in image coordinates
		\param input [in] the video input index
	*/
	virtual void activate2DDrawingSetup( const size_t &input ) = 0;

	/** \brief Draw a line border around the input
		\param input [in] the video input index
		\param thickness [in] thickness of the line
		\param r,g,b [in] color of the line
	*/
	virtual void drawInputBorder( const size_t &input, const float &thickness, const float &r, const float &g, const float &b ) = 0;

	virtual void setTextureFiltering( VideoMan::VMTextureFiltering textureFiltering ) = 0;
	
	virtual VideoMan::VMTextureFiltering getTextureFiltering() = 0;

	virtual void setKeepAspectRatio( const size_t &input, bool Keep ) = 0;	
	
	virtual bool getKeepAspectRatio( const size_t &input ) = 0;

	virtual void setImageROI( const size_t &input, const int &x, const int &y, const int &width, const int &height ) = 0;
	
	virtual void getImageROI( const size_t &input, int &x, int &y, int &width, int &height ) = 0;

	virtual void resetImageROI( const size_t &input ) = 0;

protected:

	bool supportedFormat( VideoMan::VMPixelFormat pixelFormat );

	std::list<VideoMan::VMPixelFormat> supportedFormats;

	int activatedInputs; //The number of the inputs that are activated

	SCREEN_SIZE screenSize;

	int visualizationMode; 

	size_t mainVisualizationInput;
};
};
#endif