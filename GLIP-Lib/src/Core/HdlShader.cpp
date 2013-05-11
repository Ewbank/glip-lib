/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : HdlShader.cpp                                                                             */
/*     Original Date : August 7th 2010                                                                           */
/*                                                                                                               */
/*     Description   : OpenGL Pixel and Fragment Shader Handle                                                   */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    HdlShader.cpp
 * \brief   OpenGL Pixel and Fragment Shader Handle
 * \author  R. KERVICHE
 * \date    August 7th 2010
**/

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include "Exception.hpp"
#include "HdlShader.hpp"
#include "../include/GLIPLib.hpp"
#include "devDebugTools.hpp"

using namespace Glip::CoreGL;

// HdlShader - Functions
	/**
	\fn    HdlShader::HdlShader(GLenum _type, ShaderSource& src)
	\brief HdlShader constructor.
	\param _type The kind of shader it will be : either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
	\param src   The source code used.
	**/
	HdlShader::HdlShader(GLenum _type, ShaderSource& src)
	 : ShaderSource(src)
	{
		NEED_EXTENSION(GLEW_ARB_vertex_shader)
		NEED_EXTENSION(GLEW_ARB_fragment_shader)
		NEED_EXTENSION(GLEW_ARB_shader_objects)
		NEED_EXTENSION(GLEW_ARB_shading_language_100)
		NEED_EXTENSION(GLEW_ARB_vertex_program)
		NEED_EXTENSION(GLEW_ARB_fragment_program)

		if(!src.requiresCompatibility())
			FIX_MISSING_GLEW_CALL(glBindFragDataLocation, glBindFragDataLocationEXT)

		FIX_MISSING_GLEW_CALL(glCompileShader, glCompileShaderARB)
		FIX_MISSING_GLEW_CALL(glLinkProgram, glLinkProgramARB)
		FIX_MISSING_GLEW_CALL(glGetUniformLocation, glGetUniformLocationARB)
		FIX_MISSING_GLEW_CALL(glUniform1i,  glUniform1iARB)
		FIX_MISSING_GLEW_CALL(glUniform2i,  glUniform2iARB)
		FIX_MISSING_GLEW_CALL(glUniform3i,  glUniform3iARB)
		FIX_MISSING_GLEW_CALL(glUniform4i,  glUniform4iARB)
		FIX_MISSING_GLEW_CALL(glUniform1f,  glUniform1fARB)
		FIX_MISSING_GLEW_CALL(glUniform2f,  glUniform2fARB)
		FIX_MISSING_GLEW_CALL(glUniform3f,  glUniform3fARB)
		FIX_MISSING_GLEW_CALL(glUniform4f,  glUniform4fARB)

		type = _type; //either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER

		// create the shader
		shader = glCreateShader(type);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlShader::HdlShader", "glCreateShader()")
		#endif

		if(shader==0)
		{
			throw Exception("HdlShader::HdlShader - Unable to create the shader from " + getSourceName() + ". Last OpenGL error : " + glErrorToString(), __FILE__, __LINE__);
		}

		// send the source code
		const char* data = getSourceCstr();
		glShaderSource(shader, 1, (const GLchar**)&data, NULL);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlShader::HdlShader", "glShaderSource()")
		#endif

		// compile the shader source code
		glCompileShader(shader);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlShader::HdlShader", "glCompileShader()")
		#endif

		// check the compilation
		GLint compile_status = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlShader::HdlShader", "glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status)")
		#endif

		if(compile_status != GL_TRUE)
		{
			GLint logSize;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlShader::HdlShader", "glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize)")
			#endif

			char *log = new char[logSize+1]; // +1 <=> '/0'
			memset(log, 0, logSize+1);

			glGetShaderInfoLog(shader, logSize, &logSize, log);

			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlShader::HdlShader", "glGetShaderInfoLog()")
			#endif

			log[logSize] = 0;

			std::string err = src.errorLog(std::string(log));

			delete[] log;

			throw Exception("HdlShader::HdlShader - error while compiling the shader from " + getSourceName() + " : \n" + err, __FILE__, __LINE__);
		}
	}

	/**
	\fn    GLuint HdlShader::getShaderID(void) const
	\brief Returns the ID of the shader for OpenGL.

	\return The ID handled by the driver.
	**/
	GLuint HdlShader::getShaderID(void) const
	{
		return shader;
	}

	/**
	\fn    GLenum HdlShader::getType(void) const
	\brief Returns the kind of the shader for OpenGL (either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).

	\return The Kind.
	**/
	GLenum HdlShader::getType(void) const
	{
		return type;
	}

	HdlShader::~HdlShader(void)
	{
		glDeleteShader(shader);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlShader::~HdlShader", "glDeleteShader()")
		#endif
	}


// HdlProgram - Functions
	/**
	\fn HdlProgram::HdlProgram(void)
	\brief HdlProgram constructor.
	**/
	HdlProgram::HdlProgram(void)
	 : valid(false)
	{
		// create the program
		program = glCreateProgram();

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::HdlProgram(void)", "glCreateProgram()")
		#endif

		if(program==0)
			throw Exception("HdlProgram::HdlProgram - Program can't be created. Last OpenGL error : " + glErrorToString(), __FILE__, __LINE__);

		attachedFragmentShader = attachedVertexShader = 0;
	}

	/**
	\fn    HdlProgram::HdlProgram(const HdlShader& shd1, const HdlShader& shd2)
	\brief HdlProgram constructor. Note that the shaders must be of different kinds.

	\param shd1 The first shader to link.
	\param shd2 The second shader to link.
	**/
	HdlProgram::HdlProgram(const HdlShader& shd1, const HdlShader& shd2)
	 : valid(false)
	{
		// create the program
		program = glCreateProgram();

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::HdlProgram(const HdlShader& shd1, const HdlShader& shd2)", "glCreateProgram()")
		#endif

		attachedFragmentShader = attachedVertexShader = 0;

		//attach the two Shaders
		update(shd1, false);
		update(shd2, false);

		// Link the program
		valid = link();
	}

	/**
	\fn    bool HdlProgram::isValid(void)
	\brief Check if the program is valid.
	\return True if the Program is valid, false otherwise.
	**/
	bool HdlProgram::isValid(void)
	{
		return valid;
	}

	/**
	\fn    bool HdlProgram::link(void)
	\brief Link the program.

	\return False in case of failure, true otherwise.
	**/
	bool HdlProgram::link(void)
	{
		// Link them
		glLinkProgram(program);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::link", "glLinkProgram()")
		#endif

		// Look for some error during the linking
		GLint link_status = GL_TRUE;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::link", "glGetProgramiv(program, GL_LINK_STATUS, &link_status)")
		#endif

		if(link_status!=GL_TRUE)
		{
			//std::cout << "Error during Program linking" << std::endl;

			// get the log
			GLint logSize;

			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlProgram::link", "glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize)")
			#endif

			char *log = new char[logSize+1]; // +1 <=> '/0'
			memset(log, 0, logSize+1);

			glGetProgramInfoLog(program, logSize, &logSize, log);

			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlProgram::link", "glGetProgramInfoLog()")
			#endif

			log[logSize] = 0;

			// Write the log in a file
			//std::cout << "Information written in programLog.txt" << std::endl;
			//std::fstream logFile;
			//logFile.open("./programLog.txt", std::fstream::out | std::fstream::app);
			//logFile << "Linking log from GlProgram" << std::endl;
			//logFile.write(log, logSize);
			//logFile << std::endl << std::endl;
			//logFile.close();

			std::string logstr(log);
			delete[] log;

			throw Exception("HdlProgram::link - Error during Program linking : \n" + logstr, __FILE__, __LINE__);
			return false;
		}
		else
		{	
			use();
		
			// Update available uniforms of the following types : 
			const GLenum interestTypes[] = {GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_DOUBLE, GL_DOUBLE_VEC2, GL_DOUBLE_VEC3, GL_DOUBLE_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_DOUBLE_MAT2, GL_DOUBLE_MAT3, GL_DOUBLE_MAT4, GL_UNSIGNED_INT};
			const int numAllowedTypes = sizeof(interestTypes) / sizeof(GLenum);

			// Get number of uniforms :  
			GLint numUniforms = 0;
			glGetProgramiv(	program, GL_ACTIVE_UNIFORMS, &numUniforms);
 	
			char buffer[1024];	
			GLenum type;
			GLint actualSize, actualSizeName;
			for(int k=0; k<numUniforms; k++)
			{
				glGetActiveUniform( program, k, 1024, &actualSizeName, &actualSize, &type, buffer);

				if( std::find(interestTypes, interestTypes + numAllowedTypes, type) != interestTypes + numAllowedTypes)
				{
 					activeUniforms.push_back(buffer);
					activeTypes.push_back(type);
				}
			}
		}

		return true;
	}

	/**
	\fn    void HdlProgram::update(const HdlShader& shader, bool lnk)
	\brief Change a shader in the program.

	\param shader The shader to add.
	\param lnk Set to true if you want the program to be linked again.
	**/
	void HdlProgram::update(const HdlShader& shader, bool lnk)
	{
		if( shader.getType()==GL_VERTEX_SHADER )
		{
			if( attachedVertexShader !=0 )
			{
				glDetachShader(program, attachedVertexShader);

				#ifdef __GLIPLIB_TRACK_GL_ERRORS__
					OPENGL_ERROR_TRACKER("HdlProgram::update", "glDetachShader(program, attachedVertexShader)")
				#endif
			}

			attachedVertexShader = shader.getShaderID();
			glAttachShader(program, attachedVertexShader);

			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlProgram::update", "glAttachShader(program, attachedVertexShader)")
			#endif
		}
		else //shader.getType()==GL_FRAGMENT_SHADER
		{
			if( attachedFragmentShader !=0 )
			{
				glDetachShader(program, attachedFragmentShader);

				#ifdef __GLIPLIB_TRACK_GL_ERRORS__
					OPENGL_ERROR_TRACKER("HdlProgram::update", "glDetachShader(program, attachedFragmentShader)")
				#endif
			}

			attachedFragmentShader = shader.getShaderID();
			glAttachShader(program, attachedFragmentShader);

			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlProgram::update", "glAttachShader(program, attachedFragmentShader)")
			#endif
		}

		// Link the program
		if(lnk) valid = link();
	}

	/**
	\fn    void HdlProgram::use(void)
	\brief Start using the program with OpenGL.
	**/
	void HdlProgram::use(void)
	{
		glUseProgram(program);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::use", "glUseProgram()")
		#endif
	}

	/**
	\fn    const std::vector<std::string>& getUniformVarsNames(void) const
	\brief Get access to the list of uniform variables names of supported types managed by the program (GL based).
	\return Access to a sting based vector.
	**/
	const std::vector<std::string>& HdlProgram::getUniformVarsNames(void) const
	{
		return activeUniforms;
	}

	/**
	\fn    const std::vector<std::string>& getUniformVarsTypes(void) const
	\brief Get access to the list of uniform variables types corresponding to the names provided by HdlProgram::getUniformVarsNames (GL based).
	\return Access to a sting based vector.
	**/
	const std::vector<GLenum>& HdlProgram::getUniformVarsTypes(void) const
	{
		return activeTypes;
	}

	/**
	\fn    void HdlProgram::setFragmentLocation(const std::string& fragName, int frag)
	\brief Link the name of a fragment output variable to a fragment unit.

	\param fragName Name of the fragment output variable.
	\param frag     Index of the desired fragment unit.
	**/
	void HdlProgram::setFragmentLocation(const std::string& fragName, int frag)
	{
		glErrors(); // clean error buffer
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlProgram::setFragmentLocation - FragName : " << fragName << std::endl;
		#endif
		glBindFragDataLocation(program, frag, fragName.c_str());

		bool test = glErrors();

		if(test)
			throw Exception("HdlProgram::setFragmentLocation - Error while setting fragment location \"" + fragName + "\".", __FILE__, __LINE__);
	}

	/**
	\fn    void HdlProgram::modifyVar(const std::string& varName, GLenum type, int v0, int v1=0, int v2=0, int v3=0)
	\brief Change a uniform variable in a shader. Raise an exception if any error occur.

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in, see http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml for possible types.
	\param v0      Corresponding value to assign.
	\param v1      Corresponding value to assign.
	\param v2      Corresponding value to assign.
	\param v3      Corresponding value to assign.
	**/
	/**
	\fn    void HdlProgram::modifyVar(const std::string& varName, GLenum type, unsigned int v0, unsigned int v1=0, unsigned int v2=0, unsigned int v3=0)
	\brief Change a uniform variable in a shader. Raise an exception if any error occur.

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in, see http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml for possible types.
	\param v0      Corresponding value to assign.
	\param v1      Corresponding value to assign.
	\param v2      Corresponding value to assign.
	\param v3      Corresponding value to assign.
	**/
	/**
	\fn    void HdlProgram::modifyVar(const std::string& varName, GLenum type, float v0, float v1=0, float v2=0, float v3=0)
	\brief Change a uniform variable in a shader. Raise an exception if any error occur.

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in, see http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml for possible types.
	\param v0      Corresponding value to assign.
	\param v1      Corresponding value to assign.
	\param v2      Corresponding value to assign.
	\param v3      Corresponding value to assign.
	**/

	/**
	\fn    void HdlProgram::modifyVar(const std::string& varName, GLenum t, int* v)
	\brief Change a uniform variable in a shader. Raise an exception if any error occur.

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in, see http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml for possible types.
	\param v       Pointer to the values to assign.
	**/
	/**
	\fn    void HdlProgram::modifyVar(const std::string& varName, GLenum t, unsigned int* v)
	\brief Change a uniform variable in a shader. Raise an exception if any error occur.

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in, see http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml for possible types.
	\param v       Pointer to the values to assign.
	**/
	/**
	\fn    void HdlProgram::modifyVar(const std::string& varName, GLenum t, float* v)
	\brief Change a uniform variable in a shader. Raise an exception if any error occur.

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in, see http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml for possible types.
	\param v       Pointer to the values to assign.
	**/

	#define GENmodifyVar( argT1, argT2, argT3)   \
		void HdlProgram::modifyVar(const std::string& varName, GLenum t, argT1 v0, argT1 v1, argT1 v2, argT1 v3) \
		{ \
			use(); \
			GLint loc = glGetUniformLocation(program, varName.c_str()); \
			 \
			if (loc==-1) \
				throw Exception("HdlProgram::modifyVar - Wrong location, does this var exist : \"" + varName + "\"? Is it used in the program? (May be the GLCompiler swapped it because it is unused).", __FILE__, __LINE__); \
			 \
			switch(t) \
			{ \
				case GL_##argT2 : 		glUniform1##argT3 (loc, v0);		break; \
				case GL_##argT2##_VEC2 : 	glUniform2##argT3 (loc, v0, v1);		break; \
				case GL_##argT2##_VEC3 : 	glUniform3##argT3 (loc, v0, v1, v2);	break; \
				case GL_##argT2##_VEC4 : 	glUniform4##argT3 (loc, v0, v1, v2, v3);	break; \
				default :		throw Exception("HdlProgram::modifyVar - Unknown variable type or type mismatch for \"" + glParamName(t) + "\" when modifying uniform variable \"" + varName + "\".", __FILE__, __LINE__); \
			} \
			 \
			bool test = glErrors(); \
			 \
			if(test) \
				throw Exception("HdlProgram::modifyVar - An error occurred when loading data of type \"" + glParamName(t) + "\" in variable \"" + varName + "\".", __FILE__, __LINE__); \
		} \
		 \
		void HdlProgram::modifyVar(const std::string& varName, GLenum t, argT1* v) \
		{ \
			use(); \
			GLint loc = glGetUniformLocation(program, varName.c_str()); \
			 \
			if (loc==-1) \
				throw Exception("HdlProgram::modifyVar - Wrong location, does this var exist : \"" + varName + "\"? Is it used in the program? (May be the GLCompiler swapped it because it is unused).", __FILE__, __LINE__); \
			 \
			switch(t) \
			{ \
				case GL_##argT2 : 		glUniform1##argT3##v(loc, 1, v);	break; \
				case GL_##argT2##_VEC2 : 	glUniform2##argT3##v(loc, 2, v);	break; \
				case GL_##argT2##_VEC3 : 	glUniform3##argT3##v(loc, 3, v);	break; \
				case GL_##argT2##_VEC4 : 	glUniform4##argT3##v(loc, 4, v);	break; \
				default :		throw Exception("HdlProgram::modifyVar - Unknown variable type or type mismatch for \"" + glParamName(t) + "\" when modifying uniform variable \"" + varName + "\".", __FILE__, __LINE__); \
			} \
			 \
			bool test = glErrors(); \
			 \
			if(test) \
				throw Exception("HdlProgram::modifyVar - An error occurred when loading data of type \"" + glParamName(t) + "\" in variable \"" + varName + "\".", __FILE__, __LINE__); \
		}

	GENmodifyVar( int, INT, i)
	GENmodifyVar( unsigned int, UNSIGNED_INT, ui)
	GENmodifyVar( float, FLOAT, f)

	#undef GENmodifyVar
		

	// tools
	/**
	\fn    int HdlProgram::maxVaryingVar(void)
	\brief Returns the maximum number of varying variables available.

	\return The maximum number of varying variables.
	**/
	int HdlProgram::maxVaryingVar(void)
	{
		GLint param;

		glGetIntegerv(GL_MAX_VARYING_FLOATS, &param);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::maxVaryingVar", "glGetIntegerv()")
		#endif

		return param;
	}

	HdlProgram::~HdlProgram(void)
	{
		glDetachShader(program, attachedFragmentShader);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::~HdlProgram", "glDetachShader(program, attachedFragmentShader)")
		#endif

		glDetachShader(program, attachedVertexShader);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::~HdlProgram", "glDetachShader(program, attachedVertexShader)")
		#endif

		glDeleteProgram(program);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlProgram::~HdlProgram", "glDeleteProgram(program)")
		#endif
	}

// Static tools :
	/**
	\fn    void HdlProgram::stopProgram(void)
	\brief Stop using a program
	**/
	void HdlProgram::stopProgram(void)
	{
		glUseProgram(0);
	}
