/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE 			                                                         */
/*     LICENSE       : GPLv3                                                                                     */
/*     Website       : http://glip-lib.net/                                                                      */
/*                                                                                                               */
/*     File          : mainGLFW.cpp                                                                              */
/*     Original Date : September 1st 2011                                                                        */
/*                                                                                                               */
/*     Description   : GLFW Based Hello World                                                                    */
/*                                                                                                               */
/* ************************************************************************************************************* */

// Compile on Linux with :
// g++ -o mainGLFW src/*.cpp -I<your path>/GLIP-Lib/include/ -L<your path>/GLIP-Lib/lib/ -lXext -lX11 -lGL -lGLU -lm -lglfw -lglip

// Includes
	#include <fstream>
	#include <cmath>
	#include "GLIPLib.hpp"
	#include <GL/glfw.h>
	#include <stdlib.h>

// Namespace :
	using namespace Glip;
	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;
	using namespace Glip::Modules;

// Resizing callback :
	void GLFWCALL WindowResize( int width, int height )
	{
		glViewport(0,0,width,height);
	}

// Main
	int main(int argc, char** argv)
	{
		bool 	running 		= true,
			showConvolved 		= false,
			waitingForRelease 	= false;

		std::fstream log;

		log.open("./log.txt", std::fstream::out | std::fstream::trunc);
		if(!log.is_open())
			throw Exception("TestFFT1D - Cannot open log file.", __FILE__, __LINE__);

		log << "> TestFFT1D" << std::endl;

		// Initialize GLFW
		if( !glfwInit() )
		{
			log << "Failed to start GLFW." << std::endl;
			log << "> Abort" << std::endl;
			log.close();
			exit( EXIT_FAILURE );
		}

		// Open an OpenGL window
		if( !glfwOpenWindow(512,512,0,0,0,0,0,0, GLFW_WINDOW ) )
		{
			glfwTerminate();
			log << "Failed to open GLFW window." << std::endl;
			log << "> Abort" << std::endl;
			log.close();
			exit( EXIT_FAILURE );
		}

		// Set resizing callback :
		glfwSetWindowSizeCallback( WindowResize );

		Pipeline* fft1D = NULL;
		Pipeline* ifft1D = NULL;

		try
		{
			int i=0;

			// Initialize GLIP-LIB
			HandleOpenGL::init();

			// Print info :
			log << "Vendor name   : " << HandleOpenGL::getVendorName() << std::endl;
			log << "Renderer name : " << HandleOpenGL::getRendererName() << std::endl;
			log << "GL version    : " << HandleOpenGL::getVersion() << std::endl;
			log << "GLSL version  : " << HandleOpenGL::getGLSLVersion() << std::endl;

			// Create a format for the filters
			HdlTextureFormat fmt(512, 1, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
			
			// FFT pipelines :
			fft1D = new Pipeline(GenerateFFT1DPipeline::generate(fmt.getWidth(), FFTModules::Shifted), "FFT1D");
			ifft1D = new Pipeline(GenerateFFT1DPipeline::generate(fmt.getWidth(), FFTModules::Inversed | FFTModules::Shifted), "IFFT1D");

			LayoutLoader loader;

			PipelineLayout inputPipelineLayout = loader.getPipelineLayout("./Filters/gen1D.ppl");
			Pipeline inputPipeline(inputPipelineLayout, "Input");
			

			PipelineLayout ppl = loader.getPipelineLayout("./Filters/visu1D.ppl");
			Pipeline visualization(ppl,"Visualization");

			//fft1D->enablePerfsMonitoring();
			//ifft1D->enablePerfsMonitoring();

			// Geometry : 
			GeometryInstance quad(GeometryPrimitives::StandardQuad(), GL_STATIC_DRAW_ARB);

			// Main loop
			while( running )
			{
				if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS && !waitingForRelease)
				{
					showConvolved = !showConvolved;
					waitingForRelease = true;
				}

				if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)==GLFW_RELEASE && waitingForRelease)
					waitingForRelease = false;

				glClear( GL_COLOR_BUFFER_BIT );
				glLoadIdentity();

				inputPipeline << Pipeline::Process;
				(*fft1D) << inputPipeline.out() << Pipeline::Process;
				(*ifft1D) << fft1D->out() << Pipeline::Process;

				visualization << inputPipeline.out() << fft1D->out() << ifft1D->out() << Pipeline::Process;
				visualization.out().bind();

				quad.draw();

				i++;

				// Swap front and back rendering buffers
				glfwSwapBuffers();

				// Check if ESC key was pressed or window was closed
				running = !glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );

				glfwSleep(0.05);
			}

			/*log << "FFT1D" << std::endl;
			log << "    Number of processes : " << fft1D->getNumProcesses() << std::endl;
			log << "    Mean time           : " << fft1D->getMeanTime() << " ms" << std::endl;
			log << "    Std Dev on time     : " << fft1D->getStdDevTime() << " ms" << std::endl;
			log << "iFFT1D" << std::endl;
			log << "    Number of processes : " << ifft1D->getNumProcesses() << std::endl;
			log << "    Mean time           : " << ifft1D->getMeanTime() << " ms" << std::endl;
			log << "    Std Dev on time     : " << ifft1D->getStdDevTime() << " ms" << std::endl;
			fft1D->disablePerfsMonitoring();
			ifft1D->disablePerfsMonitoring();*/

			log << "> End" << std::endl;
			log.close();

			HandleOpenGL::deinit();

			// Close window and terminate GLFW
			glfwTerminate();

			// Exit program
			exit( EXIT_SUCCESS );
		}
		catch(Exception& e)
		{
			log << "Exception caught : " << std::endl;
			log << e.what() << std::endl;
			log << "> Abort" << std::endl;
			log.close();

			std::cerr << "Exception caught : " << std::endl;
			std::cerr << e.what() << std::endl;
		}

		delete fft1D;
		delete ifft1D;

		return 0;
	}

