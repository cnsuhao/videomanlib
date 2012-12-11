#include <QMainWindow>
#include <QMdiArea>
#include <QGLWidget>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

protected:
	
protected slots:	
	virtual void openVideoFile();
	virtual void openCamera();	

private:
	QMdiArea m_mdiArea;
	QGLWidget *m_mainGLwidget;	
};