#include <QtOpenGL>
class QWidget;

#include "VideoManControl.h"
#include "VideoManInputFormat.h"

class GLWindow : public QGLWidget
{
	Q_OBJECT

public:
	GLWindow( QWidget *parent=0, QGLWidget *shareWidget=0 );
	~GLWindow();

	bool openVideoFile();
	bool openCamera();

protected:
	void initializeGL();
	void resizeGL( int width, int height );
	void paintGL();

protected slots:

	virtual void timeOutSlot();

	void frameCallbackSlot( const char *pixelBuffer, int input );

	friend void frameCallback( char *pixelBuffer, size_t input, double timeStamp, void *data );

signals:
	void frameCallbackSignal( const char *pixelBuffer, int input );

private:
	QTimer m_timer;
	VideoMan::VideoManControl *m_videoMan;
	int m_inputID;
};