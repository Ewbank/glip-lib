
// Includes
	#include "WindowRendering.hpp"

// Class
	WindowRenderer::WindowRenderer(int w, int h, double _fmtImg) : QGLWidget(), OutputDevice("Display")
	{
		QWidget::setGeometry(10,10,w,h);

		if(_fmtImg==-1)
			fmtImg = static_cast<double>(w)/static_cast<double>(h);
		else
			fmtImg = _fmtImg;

		makeCurrent();
		HandleOpenGL::init();

		glViewport(0, 0, w, h);

		try
		{
			vbo = HdlVBO::generate2DStandardQuad();
		}
		catch(std::exception& e)
		{
			Exception m("WindowRenderer::WindowRenderer - Error while creating geometry", __FILE__, __LINE__);
			throw m+e;
		}

		show();
	}

	WindowRenderer::~WindowRenderer(void)
	{
		this->hide();
		delete vbo;
	}

	void WindowRenderer::resizeGL(int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void WindowRenderer::process(HdlTexture& t)
	{
		glLoadIdentity();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glScalef(1.0,1.0,1.0);
		t.bind();
		vbo->draw();
		swapBuffers();
	}

