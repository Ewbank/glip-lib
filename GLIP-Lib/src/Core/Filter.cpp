/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. Kerviche                                                                               */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : glip-lib.net                                                                              */
/*                                                                                                               */
/*     File          : Filter.cpp                                                                                */
/*     Original Date : August 15th 2011                                                                          */
/*                                                                                                               */
/*     Description   : Filter object                                                                             */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    Filter.cpp
 * \brief   Filter object
 * \author  R. KERVICHE
 * \date    August 15th 2011
**/

#include <algorithm>
#include <set>
#include "Core/Exception.hpp"
#include "Core/Filter.hpp"
#include "Core/HdlTexture.hpp"
#include "Core/HdlShader.hpp"
#include "Core/HdlVBO.hpp"
#include "Core/HdlFBO.hpp"
#include "devDebugTools.hpp"
#include "Core/Geometry.hpp"

    using namespace Glip::CoreGL;
    using namespace Glip::CorePipeline;

// Tools
	// AbstractFilterLayout
	/**
	\fn AbstractFilterLayout::AbstractFilterLayout(const std::string& type, const HdlAbstractTextureFormat& f)
	\brief AbstractFilterLayout constructor.
	\param type The typename of the filter layout.
	\param f The texture format associated to all outputs of the filter.
	**/
	AbstractFilterLayout::AbstractFilterLayout(const std::string& type, const HdlAbstractTextureFormat& f)
	 : 	AbstractComponentLayout(type), 
		HdlAbstractTextureFormat(f),
		clearing(true),
		blending(false),
		depthTesting(false),
		isStandardGeometry(true),
		sFactor(GL_ONE),
		dFactor(GL_ONE),
		blendingEquation(GL_FUNC_ADD),
		depthTestingFunction(GL_LESS)
	{
		for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
			shaderSources[k] = NULL;
	}

	/**
	\fn AbstractFilterLayout::AbstractFilterLayout(const AbstractFilterLayout& c)
	\brief AbstractFilterLayout constructor.
	\param c Copy.
	**/
	AbstractFilterLayout::AbstractFilterLayout(const AbstractFilterLayout& c)
	 :	AbstractComponentLayout(c), 
		HdlAbstractTextureFormat(c),
		clearing(c.clearing),
		blending(c.blending), 
		depthTesting(c.depthTesting),
		isStandardGeometry(c.isStandardGeometry),
		sFactor(c.sFactor),
		dFactor(c.dFactor),
		blendingEquation(c.blendingEquation),
		depthTestingFunction(c.depthTestingFunction)
	{
		for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
			shaderSources[k] = NULL;

		try
		{
			for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
			{
				if(c.shaderSources[k]!=NULL)
					shaderSources[k] = new ShaderSource(*c.shaderSources[k]);
			}

			if(!c.geometryModelsList.empty())
			{
				for(std::list<GeometryModel>::const_iterator it=c.geometryModelsList.begin(); it!=c.geometryModelsList.end(); it++)
					geometryModelsList.push_back(*it);
			}
			else
				throw Exception("AbstractFilterLayout::AbstractFilterLayout - No geometry models registered for " + getFullName(), __FILE__, __LINE__, Exception::CoreException);
		}
		catch(Exception& e)
		{
			clean();
			throw e;
		}
		
	}

	AbstractFilterLayout::~AbstractFilterLayout(void)
	{
		clean();
	}

	void AbstractFilterLayout::clean(void)
	{
		for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
		{
			delete shaderSources[k];
			shaderSources[k] = NULL;
		}
		geometryModelsList.clear();
	}

	/**
	\fn const ShaderSource* AbstractFilterLayout::getShaderSource(GLenum shaderType) const
	\brief Access the shader source of a particular type of shader. 
	\param shaderType The type of the targeted shader source, among GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER.
	\return Pointer to the ShaderSource or NULL if no source was defined.
	**/
	const ShaderSource* AbstractFilterLayout::getShaderSource(GLenum shaderType) const
	{
		return shaderSources[HandleOpenGL::getShaderTypeIndex(shaderType)];
	}

	/**
	\fn unsigned int AbstractFilterLayout::getNumGeometryModels(void) const
	\brief Get the number of geometry models to be rendered.
	\return The number of geometry models contained.
	**/
	unsigned int AbstractFilterLayout::getNumGeometryModels(void) const
	{
		return geometryModelsList.size();
	}

	/**
	\fn std::list<GeometryModel>::const_iterator AbstractFilterLayout::modelsListBegin(void) const
	\brief Get the beginning iterator of the models list.
	\return The constant iterator at the beginning of the models list.
	**/
	std::list<GeometryModel>::const_iterator AbstractFilterLayout::modelsListBegin(void) const
	{
		return geometryModelsList.begin();	
	}

	/**
	\fn std::list<GeometryModel>::const_iterator AbstractFilterLayout::modelsListEnd(void) const
	\brief Get the ending iterator of the models list.
	\return The constant iterator at the end of the models list.
	**/
	std::list<GeometryModel>::const_iterator AbstractFilterLayout::modelsListEnd(void) const
	{
		return geometryModelsList.end();
	}	

	/**
	\fn bool AbstractFilterLayout::isStandardGeometryModel(void) const
	\return true if the filter will be using a standard quad as geometry.
	**/
	bool AbstractFilterLayout::isStandardGeometryModel(void) const
	{
		return isStandardGeometry;
	}

	/**
	\fn bool AbstractFilterLayout::isClearingEnabled(void) const
	\return true if clearing is enabled.		
	**/
	bool AbstractFilterLayout::isClearingEnabled(void) const
	{
		return clearing;
	}

	/**
	\fn int AbstractFilterLayout::getNumUniformVars(void) const
	\return The cumulative number of uniform variables.
	**/
	int AbstractFilterLayout::getNumUniformVars(void) const
	{
		std::set<std::string> uniformVarsSet;
		for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
		{
			if(shaderSources[k]!=NULL)
			{
				const std::vector<std::string>& uniformVars = shaderSources[k]->getUniformVars();
				uniformVarsSet.insert(uniformVars.begin(), uniformVars.end());
			}
		}

		return uniformVarsSet.size();
	}

	/**
	\fn void AbstractFilterLayout::enableClearing(void)
	\brief Enables clearing operation (the destination buffer is cleared before each operation).
	**/
	void AbstractFilterLayout::enableClearing(void)
	{ 
		clearing = true;
	}
	
	/**
	\fn void AbstractFilterLayout::disableClearing(void)
	\brief Disables clearing operation.
	**/
	void AbstractFilterLayout::disableClearing(void)
	{
		clearing = false; 
	}

	/**
	\fn bool AbstractFilterLayout::isBlendingEnabled(void) const
	\return true if blending is enabled.	
	**/
	bool AbstractFilterLayout::isBlendingEnabled(void) const
	{ 
		return blending;  
	}

	/**
	\fn const GLenum& AbstractFilterLayout::getSFactor(void) const
	\return Get the blending factor to be applied to the source (only if blending is enabled).
	**/
	const GLenum& AbstractFilterLayout::getSFactor(void) const
	{
		return sFactor;
	}

	/**
	\fn const GLenum& AbstractFilterLayout::getDFactor(void) const
	\return Get the blending factor to be applied to the destination (only if blending is enabled).
	**/
	const GLenum& AbstractFilterLayout::getDFactor(void) const
	{
		return dFactor;
	}

	/**
	\fn const GLenum& AbstractFilterLayout::getBlendingEquation(void) const
	\return Get the equation used for the blending operation.
	**/
	const GLenum& AbstractFilterLayout::getBlendingEquation(void) const
	{
		return blendingEquation;
	}

	/**
	\fn void AbstractFilterLayout::enableBlending(const GLenum& _sFactor, const GLenum& _dFactor, const GLenum& _blendingEquation)
	\brief Enables blending operation (the alpha channel is used).
	\param _sFactor Blending factor to be applied to the source; among GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA and GL_SRC_ALPHA_SATURATE (see https://www.opengl.org/sdk/docs/man2/xhtml/glBlendFunc.xml for more information).
	\param _dFactor Blending factor to be applied to the destination; among GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA. GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA and GL_ONE_MINUS_CONSTANT_ALPHA (see https://www.opengl.org/sdk/docs/man2/xhtml/glBlendFunc.xml for more information).
	\param _blendingEquation Setting the blending function, for mixing the source and destination color; among GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX (see https://www.opengl.org/sdk/docs/man2/xhtml/glBlendEquation.xml for more information).
	**/
	void AbstractFilterLayout::enableBlending(const GLenum& _sFactor, const GLenum& _dFactor, const GLenum& _blendingEquation)   		
	{
		sFactor = _sFactor;
		dFactor = _dFactor;
		blendingEquation = _blendingEquation;
		blending = true;  
	}

	/**
	\fn void AbstractFilterLayout::disableBlending(void)
	\brief Disables blending operation.
	**/
	void AbstractFilterLayout::disableBlending(void)
	{
		blending = false;
	}

	/**
	\fn bool AbstractFilterLayout::isDepthTestingEnabled(void) const
	\return True if the depth testing is enabled.
	**/
	bool AbstractFilterLayout::isDepthTestingEnabled(void) const
	{
		return depthTesting;
	}

	/**
	\fn const GLenum& AbstractFilterLayout::getDepthTestingFunction(void) const
	\return The depth testing function used if depth testing is enabled;
	**/
	const GLenum& AbstractFilterLayout::getDepthTestingFunction(void) const
	{
		return depthTestingFunction;
	}

	/**
	\fn void AbstractFilterLayout::enableDepthTesting(const GLenum& _depthTestingFunction)
	\brief Enable depth testing during filter processing.
	\param _depthTestingFunction Function used the depth testing; among GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL and GL_ALWAYS (see https://www.opengl.org/sdk/docs/man2/xhtml/glDepthFunc.xml for more information).
	**/
	void AbstractFilterLayout::enableDepthTesting(const GLenum& _depthTestingFunction)
	{
		depthTestingFunction = _depthTestingFunction;
		depthTesting = true;
	}	

	/**
	\fn void AbstractFilterLayout::disableDepthTesting(void)
	\brief Disable depth testing operation.
	**/
	void AbstractFilterLayout::disableDepthTesting(void)
	{
		depthTesting = false;
	}	

// FilterLayout
	/**
	\fn FilterLayout::FilterLayout(const std::string& type, const HdlAbstractTextureFormat& fout, const ShaderSource& fragmentSource, const std::list<GeometryModel>& geometries)
	\brief FilterLayout constructor.
	\param type The typename of the filter layout.
	\param fout The texture format of all the outputs.
	\param fragmentSource The ShaderSource of the fragement shader.
	\param geometries The list of geometry models to use in this filter (if left empty, the standard quad will be used).
	**/
	FilterLayout::FilterLayout(const std::string& type, const HdlAbstractTextureFormat& fout, const ShaderSource& fragmentSource, const std::list<GeometryModel>& geometries)
	 : 	AbstractComponentLayout(type),
		ComponentLayout(type),
		HdlAbstractTextureFormat(fout),
		AbstractFilterLayout(type, fout)
	{
		// Copy the source : 
		ShaderSource* _fragmentSource = new ShaderSource(fragmentSource);
		shaderSources[HandleOpenGL::getShaderTypeIndex(GL_FRAGMENT_SHADER)] = _fragmentSource;

		// Create the ports : 
		std::vector<std::string> varsIn = _fragmentSource->getInputVars(),
					 varsOut = _fragmentSource->getOutputVars();
		for(std::vector<std::string>::const_iterator it=varsIn.begin(); it!=varsIn.end(); it++)
			addInputPort(*it);
		for(std::vector<std::string>::const_iterator it=varsOut.begin(); it!=varsOut.end(); it++)
			addOutputPort(*it);

		// Add the geometries :
		if(geometries.empty())
		{
			geometryModelsList.push_back(GeometryPrimitives::StandardQuad());
			isStandardGeometry = true;
		}
		else
		{
			for(std::list<GeometryModel>::const_iterator it=geometries.begin(); it!=geometries.end(); it++)
				geometryModelsList.push_back(*it);
			isStandardGeometry = false;
		}
	}

	/**
	\fn FilterLayout::FilterLayout(const std::string& type, const HdlAbstractTextureFormat& fout, const std::map<GLenum, ShaderSource*>& sources, const std::list<GeometryModel>& geometries)
	\brief FilterLayout constructor.
	\param type The typename of the filter layout.
	\param fout The texture format of all the outputs.
	\param sources List of all the sources, map to their respective types (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER), the objects will be copied.
	\param geometries The list of geometry models to use in this filter (if left empty, the standard quad will be used).
	**/
	FilterLayout::FilterLayout(const std::string& type, const HdlAbstractTextureFormat& fout, const std::map<GLenum, ShaderSource*>& sources, const std::list<GeometryModel>& geometries)
	 : 	AbstractComponentLayout(type),
		ComponentLayout(type),
		HdlAbstractTextureFormat(fout),
		AbstractFilterLayout(type, fout)
	{
		if(sources.empty())
			throw Exception("FilterLayout::FilterLayout - No ShaderSource provided.", __FILE__, __LINE__, Exception::CoreException);

		// Copy the sources, and the find the input/output variables :
		std::set<std::string>	varsIn,
					varsOut;
		for(std::map<GLenum, ShaderSource*>::const_iterator it=sources.begin(); it!=sources.end(); it++)
		{
			if(it->second!=NULL)
			{
				shaderSources[HandleOpenGL::getShaderTypeIndex(it->first)] = new ShaderSource(*it->second);
			
				const std::vector<std::string>  currentVarsIn = it->second->getInputVars(),
							 	currentVarsOut = it->second->getOutputVars();
				varsIn.insert(currentVarsIn.begin(), currentVarsIn.end());
				varsOut.insert(currentVarsOut.begin(), currentVarsOut.end());
			}
		}
		// Build Ports :
		for(std::set<std::string>::const_iterator it=varsIn.begin(); it!=varsIn.end(); it++)
			addInputPort(*it);

		for(std::set<std::string>::const_iterator it=varsOut.begin(); it!=varsOut.end(); it++)
			addOutputPort(*it);

		// Add the geometries :
		if(geometries.empty())
		{
			geometryModelsList.push_back(GeometryPrimitives::StandardQuad());
			isStandardGeometry = true;
		}
		else
		{
			for(std::list<GeometryModel>::const_iterator it=geometries.begin(); it!=geometries.end(); it++)
				geometryModelsList.push_back(*it);
			isStandardGeometry = false;
		}
	}

// Filter
	/**
	\fn Filter::Filter(const AbstractFilterLayout& c, const std::string& name)
	\brief Filter constructor.
	\param c Filter layout.
	\param name The instance name.
	**/
	Filter::Filter(const AbstractFilterLayout& c, const std::string& name)
	:	AbstractComponentLayout(c), 
		Component(c, name),
		HdlAbstractTextureFormat(c), 
		AbstractFilterLayout(c),
		prgm(NULL) 
	{
		const int 	limInput  = HdlTexture::getMaxImageUnits(),
				limOutput = HdlFBO::getMaximumColorAttachment();

		for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
			shaders[k] = NULL;

		firstRun	= true;
		broken		= true; // Wait for complete initialization.

		// Check for the number of input
		if(getNumInputPort()>limInput)
			throw Exception("Filter::Filter - Filter " + getFullName() + " has too many input port for hardware (Max : " + toString(limInput) + ", Current : " + toString(getNumInputPort()) + ").", __FILE__, __LINE__, Exception::CoreException);

		if(getNumOutputPort()>limOutput)
			throw Exception("Filter::Filter - Filter " + getFullName() + " has too many output port for hardware (Max : " + toString(limOutput) + ", Current : " + toString(getNumInputPort()) + ").", __FILE__, __LINE__, Exception::CoreException);

		// Build arguments table :
		arguments.assign(getNumInputPort(), reinterpret_cast<HdlTexture*>(NULL));
		try
		{
			// Build the shaders and the program : 
			prgm 	= new HdlProgram;

			#ifdef GLIP_USE_GL
				const GLenum listShaderTypeEnum[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER};
			#else
				const GLenum listShaderTypeEnum[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER};
			#endif
			for(unsigned int k=0; k<(sizeof(listShaderTypeEnum)/sizeof(GLenum)); k++)
			{
				const ShaderSource* ptr = getShaderSource(listShaderTypeEnum[k]);
				if(ptr!=NULL)
				{
					shaders[k] = new HdlShader(listShaderTypeEnum[k], *ptr);
					prgm->updateShader(*shaders[k], false);
				}
			}
			prgm->link();
		}
		catch(Exception& e)
		{
			clean();
			Exception m("Filter::Filter - Caught an exception while creating the shaders for " + getFullName(), __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}

		try
		{
			// Test if this filter is using out vec4's : 
			bool allRequireCompatibility = true;
			for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
				allRequireCompatibility = allRequireCompatibility && (shaders[k]==NULL || shaders[k]->requiresCompatibility());

			if(!allRequireCompatibility)
			{
				for(int i=0; i<getNumOutputPort(); i++)
					prgm->setFragmentLocation(getOutputPortName(i), i);

				// Now link to apply the change of locations (link must be done before setting any uniform values, including input sampler2D) : 
				prgm->link();
			}

			// Set the names of the samplers :
			for(int i=0; i<getNumInputPort(); i++)
				prgm->setVar(getInputPortName(i), GL_INT, i);
			
			prgm->stopProgram();
		}
		catch(Exception& e)
		{
			clean();
			Exception m("Filter::Filter - Caught an exception while editing the samplers for " + getFullName(), __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}

		// Build the geometries :
		for(std::list<GeometryModel>::const_iterator it=modelsListBegin(); it!=modelsListEnd(); it++)
			geometries.push_back(new GeometryInstance(*it, GL_STATIC_DRAW));

		// Finally : 
		broken = false;
	}

	Filter::~Filter(void)
	{
		clean();
	}

	void Filter::clean(void)
	{
		delete prgm;
		prgm = NULL;
		for(unsigned int k=0; k<HandleOpenGL::numShaderTypes; k++)
		{
			delete shaders[k];
			shaders[k] = NULL;
		}
		for(std::vector<GeometryInstance*>::iterator it=geometries.begin(); it!=geometries.end(); it++)
			delete *it;
		geometries.clear();
	}

	/**
	\fn void Filter::setInputForNextRendering(int id, HdlTexture* ptr)
	\brief Sets input texture for next rendering.
	\param id Index of the input.
	\param ptr Pointer to a valid texture object.
	**/
	void Filter::setInputForNextRendering(int id, HdlTexture* ptr)
	{
		if(id<0 || id>getNumInputPort())
			throw Exception("Filter::setInputForNextRendering - Index out of range", __FILE__, __LINE__, Exception::CoreException);
		arguments[id] = ptr;
	}

	/**
	\fn void Filter::process(HdlFBO& renderer)
	\brief Start the rendering process.
	\param renderer The FBO to use as target.
	**/
	void Filter::process(HdlFBO& renderer)
	{
		if(renderer.getAttachmentCount()<getNumOutputPort())
			throw Exception("Filter::process - Renderer doesn't have as many texture targets as Filter " + getFullName() + " has outputs.", __FILE__, __LINE__, Exception::CoreException);
		
		// Pre-depth test : 
			if(isDepthTestingEnabled() && !renderer.hasDepthBuffer())
				renderer.addDepthBuffer();

		// Prepare the renderer :
			renderer.beginRendering(getNumOutputPort(), isDepthTestingEnabled());
	
		// Enable states :
			if(isDepthTestingEnabled())
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(getDepthTestingFunction());
			}
			else
				glDisable(GL_DEPTH_TEST);

			if(isBlendingEnabled())
			{
				glEnable(GL_BLEND);
				glBlendFunc(getSFactor(), getDFactor());
				glBlendEquation(getBlendingEquation());
			}
			else
				glDisable(GL_BLEND);

			if(isClearingEnabled())
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

		// Link the textures :
			for(int i=0; i<getNumInputPort(); i++)
				arguments[i]->bind(i);

		// Prepare the transform :
		#ifdef GLIP_USE_GL
			glLoadIdentity();
		#endif

		// Load the shader :
			prgm->use();
			
		// Test on first run : 
			if(firstRun)
			{
				const GLenum err = glGetError();

				if(err!=GL_NO_ERROR)
				{
					// Force end rendering : 
					HdlProgram::stopProgram();
					renderer.endRendering();

					firstRun 	= false;
					broken 		= true;
					throw Exception("Filter::process : Exception caught on first run of filter " + getFullName() + ". The error occured after initialization, GL error : " + getGLEnumNameSafe(err) + " - " + getGLErrorDescription(err), __FILE__, __LINE__, Exception::CoreException);
				}
			}

		// Draw :
			int renderingCount = 0;
			for(std::vector<GeometryInstance*>::iterator it=geometries.begin(); it!=geometries.end(); it++)
			{
				(*it)->draw();
				renderingCount = (renderingCount+1) % 16;
				if(renderingCount==0)
					glFinish(); // Wait for all operations to complete in order to appease the watchdog from time to time.
			}

		// Test on first run :
			if(firstRun)
			{
				const GLenum err = glGetError();

				if(err!=GL_NO_ERROR)
				{
					// Force end rendering : 
					HdlProgram::stopProgram();
					renderer.endRendering();

					firstRun 	= false;
					broken 		= true;
					throw Exception("Filter::process : Exception caught on first run of filter " + getFullName() + ". The error occured after drawing operation, GL error : " + getGLEnumNameSafe(err) + " - " + getGLErrorDescription(err), __FILE__, __LINE__, Exception::CoreException);
				}
			}

		// Stop using the shader :
			HdlProgram::stopProgram();

		// Remove from stack :
			if(isDepthTestingEnabled())
				glDisable(GL_DEPTH_TEST);

			if(isBlendingEnabled())
				glDisable(GL_BLEND);

		// Unload :
			for(int i=0; i<getNumInputPort(); i++)
				HdlTexture::unbind(i);

		// End rendering :
			renderer.endRendering();

		// Test on first run : 
			if(firstRun)
			{
				const GLenum err = glGetError();
				if(err!=GL_NO_ERROR)
				{
					firstRun 	= false;
					broken 		= true;
					throw Exception("Filter::process : Exception caught on first run of filter " + getFullName() + ". The error occured after deinitialization, GL error : " + getGLEnumNameSafe(err) + " - " + getGLErrorDescription(err) + ".", __FILE__, __LINE__, Exception::CoreException);
				}
				else
					firstRun 	= false; // First run completed successfully.
			}
	}

	/**
	\fn HdlProgram& Filter::program(void)
	\brief Access the program carried.
	\return Reference to the program.
	**/
	HdlProgram& Filter::program(void)
	{
		return *prgm;
	}

	/**
	\fn bool Filter::wentThroughFirstRun(void) const
	\brief Check if the filter was already applied, at least once.
	\return True if the filter was already applied.
	**/
	bool Filter::wentThroughFirstRun(void) const
	{
		return !firstRun;
	}

	/**
	\fn bool Filter::isBroken(void) const
	\brief Check if the filter is broken (its initialization failed, or an error occured during its first run).
	\return True if the filter is broken and should not be used.
	**/
	bool Filter::isBroken(void) const
	{
		return broken;
	}

