#include "Filter.hpp"
#include "HdlTexture.hpp"
#include "HdlShader.hpp"
#include "HdlVBO.hpp"
#include "HdlFBO.hpp"

    using namespace Glip::CoreGL;
    using namespace Glip::CorePipeline;

// Tools
    std::string getStandardVertexSource(int nUnits)
    {
        std::stringstream str;

        str << "void main() \n { \n    gl_FrontColor  = gl_Color; \n";

        for(int i=0; i<nUnits; i++)
            str << "    gl_TexCoord[" << i << "] = gl_TextureMatrix[" << i << "] * gl_MultiTexCoord" << i << "; \n";

        str << "    gl_Position = gl_ModelViewMatrix * gl_Vertex; \n } \n";

        return std::string(str.str());
    }

// __ReadOnly_FilterLayout
    __ReadOnly_FilterLayout::__ReadOnly_FilterLayout(const std::string& type, const __ReadOnly_HdlTextureFormat& f)
     : __ReadOnly_ComponentLayout(type), __ReadOnly_HdlTextureFormat(f), vertexSource(NULL), fragmentSource(NULL), blending(false), clearing(true)
    { }

    __ReadOnly_FilterLayout::__ReadOnly_FilterLayout(const __ReadOnly_FilterLayout& c)
     : __ReadOnly_ComponentLayout(c), __ReadOnly_HdlTextureFormat(c), blending(c.blending), clearing(c.clearing)
    {
        if(c.vertexSource!=NULL)
            vertexSource   = new ShaderSource(*c.vertexSource);
        else
            throw Exception("__ReadOnly_FilterLayout::__ReadOnly_FilterLayout - vertexSource is NULL for " + getNameExtended(), __FILE__, __LINE__);

        if(c.fragmentSource!=NULL)
            fragmentSource = new ShaderSource(*c.fragmentSource);
        else
            throw Exception("__ReadOnly_FilterLayout::__ReadOnly_FilterLayout - fragmentSource is NULL for " + getNameExtended(), __FILE__, __LINE__);
    }

    __ReadOnly_FilterLayout::~__ReadOnly_FilterLayout(void)
    {
        delete vertexSource;
        delete fragmentSource;
    }

    ShaderSource& __ReadOnly_FilterLayout::getVertexSource(void) const
    {
        if(vertexSource==NULL)
            throw Exception("FilterLayout::getVertexSource - The source has not been defined yet for " + getNameExtended(), __FILE__, __LINE__);

        return *vertexSource;
    }

    ShaderSource& __ReadOnly_FilterLayout::getFragmentSource(void) const
    {
        if(fragmentSource==NULL)
            throw Exception("FilterLayout::getFragmentSource - The source has not been defined yet for " + getNameExtended(), __FILE__, __LINE__);

        return *fragmentSource;
    }

    bool __ReadOnly_FilterLayout::isBlendingEnabled(void) 	{ return blending;  }
    void __ReadOnly_FilterLayout::enableBlending(void)   	{ blending = true;  }
    void __ReadOnly_FilterLayout::disableBlending(void)  	{ blending = false; }
    bool __ReadOnly_FilterLayout::isClearingEnabled(void) 	{ return clearing;  }
    void __ReadOnly_FilterLayout::enableClearing(void)   	{ clearing = true;  }
    void __ReadOnly_FilterLayout::disableClearing(void)  	{ clearing = false; }

// FilterLayout
    FilterLayout::FilterLayout(const std::string& type, const __ReadOnly_HdlTextureFormat& fout, const ShaderSource& fragment, ShaderSource* vertex)
     : __ReadOnly_ComponentLayout(type), ComponentLayout(type), __ReadOnly_FilterLayout(type, fout), __ReadOnly_HdlTextureFormat(fout)
    {
        fragmentSource = new ShaderSource(fragment);

        if(vertex!=NULL)
            vertexSource = new ShaderSource(*vertex);

        // Analyze sources to get the variables and the outputs
        std::vector<std::string> varsIn  = fragmentSource->getInputVars();
        if(vertexSource!=NULL)
        {
            // Add also vertex inputs if needed
            std::vector<std::string> varsInVertex = vertexSource->getInputVars();

            // Push them!
            for(std::vector<std::string>::iterator it=varsInVertex.begin(); it!=varsInVertex.end(); it++)
                varsIn.push_back(*it);

            ///TODO Verify that variables nam in vertex and fragment are different
        }
        std::vector<std::string> varsOut = fragmentSource->getOutputVars();

        // Build Ports :
        for(std::vector<std::string>::iterator it=varsIn.begin(); it!=varsIn.end(); it++)
            addInputPort(*it);

        for(std::vector<std::string>::iterator it=varsOut.begin(); it!=varsOut.end(); it++)
            addOutputPort(*it);

        // If there is no vertexSource
        if(vertexSource==NULL)
        {
            // Build one :
            std::cout << "Using : " << getStandardVertexSource(varsIn.size()) << std::endl;
            vertexSource = new ShaderSource(getStandardVertexSource(varsIn.size()));
        }
    }

// Filter
	Filter::Filter(const __ReadOnly_FilterLayout& c)
	: __ReadOnly_FilterLayout(c), Component(c, c.getName()), __ReadOnly_ComponentLayout(c), __ReadOnly_HdlTextureFormat(c), program(NULL), vertexShader(NULL), fragmentShader(NULL), vbo(NULL)
	{
		// Build arguments table :
		arguments.assign(getNumInputPort(), NULL);

		try
		{
			// Build the shaders :
			vertexShader   = new HdlShader(GL_VERTEX_SHADER, getVertexSource());
			fragmentShader = new HdlShader(GL_FRAGMENT_SHADER, getFragmentSource());
			program        = new HdlProgram(*vertexShader, *fragmentShader);
		}
		catch(std::exception& e)
		{
			Exception m("Filter::Filter - Caught an exception while creating the shaders for " + getNameExtended(), __FILE__, __LINE__);
			throw m+e;
		}

		try
		{
			// Set the names of the samplers :
			for(int i=0; i<getNumInputPort(); i++)
				program->modifyVar(getInputPortName(i), HdlProgram::SHADER_VAR, i);

			for(int i=0; i<getNumOutputPort(); i++)
				program->setFragmentLocation(getOutputPortName(i), i);

			program->stopProgram();
		}
		catch(std::exception& e)
		{
			Exception m("Filter::Filter - Caught an exception while editing the samplers for " + getNameExtended(), __FILE__, __LINE__);
			throw m+e;
		}

		try
		{
			// Create a basic geometry :
			vbo = HdlVBO::generate2DStandardQuad();
		}
		catch(std::exception& e)
		{
			throw Exception("Filter::Filter - Caught an exception while creating the geometry for " + getNameExtended() + " : \n" + e.what(), __FILE__, __LINE__);
		}

		// Set up the data on the program :
		for(int i=0; i<getNumInputPort(); i++)
			program->setFragmentLocation(getInputPortName(i), i);
	}

	Filter::~Filter(void)
	{
		if(program==NULL)
			throw Exception("Filter::~Filter - Internal error : program is NULL", __FILE__, __LINE__);
		if(vertexShader==NULL)
			throw Exception("Filter::~Filter - Internal error : vertexShader is NULL", __FILE__, __LINE__);
		if(fragmentShader==NULL)
			throw Exception("Filter::~Filter - Internal error : fragmentShader is NULL", __FILE__, __LINE__);
		if(vbo==NULL)
			throw Exception("Filter::~Filter - Internal error : vbo is NULL", __FILE__, __LINE__);
		delete program;
		delete vertexShader;
		delete fragmentShader;
		delete vbo;
	}

	void Filter::setInputForNextRendering(int id, HdlTexture* ptr)
	{
		if(id<0 || id>getNumInputPort())
			throw Exception("Filter::setInputForNextRendering - Index out of range", __FILE__, __LINE__);
		arguments[id] = ptr;
		//std::cout << "Adding : " << arguments[id] << " at " << id << std::endl;
	}

	void Filter::process(HdlFBO& renderer)
	{
		// Prepare the renderer
			renderer.beginRendering();
			//std::cout << "Begin rendering 		: "; glErrors(true, false);

		// Enable states
			if(isBlendingEnabled())
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
			}
			else
				glDisable(GL_BLEND);

			if(isClearingEnabled())
			{
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
			//std::cout << "Clearing states 		: "; glErrors(true, false);

		// Link the textures
			for(int i=0; i<getNumInputPort(); i++)
			{
				//std::cout << "Using : " << arguments[i] << std::endl;
				/*if(arguments[i]==NULL)
					throw Exception("Argument is NULL", __FILE__, __LINE__);*/
				arguments[i]->bind(i);
			}
			//std::cout << "Binding	 		: "; glErrors(true, false);

		// Prepare geometry
			glLoadIdentity();

		// Load the shader
			program->use();
			//std::cout << "Using shader 		: "; glErrors(true, false);

		// Draw
			vbo->draw();
			//std::cout << "drawing VBO 		: "; glErrors(true, false);

		// Stop using the shader
			HdlProgram::stopProgram();
			//std::cout << "Stop program 		: "; glErrors(true, false);

		// Remove from stack
			if(isBlendingEnabled())
			    glDisable(GL_BLEND);

		// Unload
			for(int i=0; i<getNumInputPort(); i++)
			{
				//std::cout << "Unbinding " << i << std::endl;
				HdlTexture::unbind(i);
			}
			//std::cout << "Unbinding 		: "; glErrors(true, false);

		// End rendering
			renderer.endRendering();
			//std::cout << "End rendering 		: "; glErrors(true, false);


	}

	HdlProgram& Filter::operator->(void)
	{
		return *program;
	}

	void Filter::setGeometry(HdlVBO* v)
	{
		if(v==NULL)
			throw Exception("Filter::setGeometry - You can't set a pointer to NULL for the geometry!", __FILE__, __LINE__);
		delete vbo;
		vbo = v;
	}
