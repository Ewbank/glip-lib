#include "gameOfLife.hpp"
#include "WindowRendering.hpp"
#include <cstdlib>
#include <ctime>

// Src :
	GameOfLife::GameOfLife(int _w, int _h, int argc, char** argv) : QApplication(argc,argv), w(_w), h(_h)
	{
		try
		{
			window = new WindowRenderer(100,w, h);

			// Create the pipeline :
				std::fstream file;
				file.open("./Filters/game.glsl");
				if(!file.is_open())
				    throw Exception("UPPER LEVEL - File not opened", __FILE__, __LINE__);
				HdlTextureFormat fmt(w, h, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST);
				ShaderSource src(file, "./Filters/game.glsl");

				FilterLayout fl("GameOfLife_Layout", fmt, src);

				PipelineLayout pl("MainPipeline");
				pl.addInput("Input");
				pl.addOutput("Output");
				pl.add(fl, "GameOfLife");

				pl.connectToInput("Input", "GameOfLife", "inText");
				pl.connectToOutput("GameOfLife", "outText", "Output");

				p1 = new Pipeline(pl, "Ping");
				p2 = new Pipeline(pl, "Pong");

			// Create a random starting point :
				fmt.setSWrapping(GL_REPEAT); // pac-man repetition
				fmt.setTWrapping(GL_REPEAT);
				t = new HdlTexture(fmt);

				randomTexture(0.6);
				window->giveTexture(t);

			// Do the first pass :
				(*p1) << (*t) << Process;
				(*p2) << (*t) << Process;

			timer = new QTimer;
			timer->setInterval(100);
			connect(timer, SIGNAL(timeout()),this, SLOT(compute()));
			timer->start();
		}
		catch(Exception& e)
		{
			std::cout << "Exception caught : " << std::endl;
			std::cout << e.what() << std::endl;
			std::cout << "(Will be rethrown)" << std::endl;
			throw e;
		}

		std::cout << "--- STARTING ---" << std::endl;
	}

	void GameOfLife::randomTexture(float alpha)
	{
		srand(time(NULL));
		unsigned char *tmp = new unsigned char [3*w*h];
		for(int i=0; i<3*w*h; i++)
		{
			float a = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
			tmp[i] = 255*(a>alpha);
		}

		t->write(tmp);
		delete[] tmp;
	}

	void GameOfLife::compute(void)
	{
		static int i = 0;
		//std::cout << "Computing..." << std::endl;

		if(i==10)
		{
			std::cout << "> Reset" << std::endl;
			//reset :
			randomTexture(0.6);
			(*p1) << (*t) << Process;
			(*p2) << (*t) << Process;
			i = 0;
		}
		else
		{
			if(i%2==0)
			{
				(*p2) << p1->out(0) << Process;
				window->giveTexture(&p2->out(0));
			}
			else
			{
				(*p1) << p2->out(0) << Process;
				window->giveTexture(&p1->out(0));
			}
		}

		i++;
	}
