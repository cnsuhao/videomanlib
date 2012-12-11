#include <QtOpenGL>
#include <highgui.h>
class QWidget;

#include "VideoManControl.h"
#include "VideoManInputFormat.h"


class GLWindow : public QGLWidget
{
	Q_OBJECT

private:
	typedef std::pair<int,QString> IND_TEXT;

public:
	GLWindow( QMainWindow *window, int timerInterval=30, QWidget *parent=0, const char *name=0 );//: QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {}
	~GLWindow();

	void openImageFile( QString fileName );

	void setListInputs( QComboBox *list, const std::vector<IND_TEXT> &inputs, int currentIndex );

protected:
	void initializeGL();
	void resizeGL( int width, int height );
	void paintGL();

	void processImage();


	bool event( QEvent *event );

	void keyPressEvent( QKeyEvent *e );

	void mousePressEvent( QMouseEvent *e );

private:
	void createToolbar();
  
protected slots:
	virtual void timeOutSlot();
	virtual void openVideoFile();
	virtual void openImageFile();
	virtual void openCamera();
	virtual void changeMode0();
	virtual void changeMode1();
	void sltListFindModesChanged( int index );
	void sltListApproxModesChanged( int index );
	void sltListMorphologyOpChanged( int index );
	void sltMinThreshChanged( int value );
	void sltMaxThreshChanged( int value );
	void sltInvertThreshChanged( int state );
	
  
private:


	QTimer *m_timer;
	IplImage *m_image;
	IplImage *m_binary;
	IplImage *m_binaryAux;
	IplImage *m_grey;
	CvMemStorage *m_storage;

	struct BLOB{
	  CvRect rect;
	  float area;
	  float perimeter;
	  float roundness;	  
	  float elongation;
	  CvPoint2D32f centroide;
	  CvBox2D ellipse;
	};

	std::vector<BLOB> m_blobs;
	int videoInputID, binaryInputID;
	int m_selectedBlob;

	std::vector<IND_TEXT> m_findContoursModes;
	int m_findContoursMode;

	std::vector<IND_TEXT> m_contoursApproximationModes;
	int m_contoursApproximationMode;

	std::vector<IND_TEXT> m_morphologyOps;
	int m_morphologyOp;

	int m_minThreshold, m_maxThreshold;
	bool m_invertThresh;

	QMainWindow *m_mainWindow;
	QTableWidget *m_blobPropertiesTable;
 
};