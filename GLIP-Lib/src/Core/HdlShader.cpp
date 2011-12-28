/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : GPLv3                                                                                     */
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
 * \version 0.6
 * \date    August 7th 2010
**/

#include <iostream>
#include <string>
#include <sstream>
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
	HdlShader::HdlShader(GLenum _type, ShaderSource& src) : ShaderSource(src)
	{
		HandleOpenGL::init();

		type = _type; //either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER

		// create the shader
		shader = glCreateShader(type);

		if(shader==0)
		{
			throw Exception("HdlShader::HdlShader - Unable to create the shader " + getSourceName(), __FILE__, __LINE__);
		}

		// send the source code
		const char* data = getSourceCstr();
		glShaderSource(shader, 1, (const GLchar**)&data, NULL);
		// compile the shader source code
		glCompileShader(shader);

		// check the compilation
		GLint compile_status = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

		if(compile_status != GL_TRUE)
		{
			GLint logSize;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

			char *log = new char[logSize]; // +1 <=> '/0'

			glGetShaderInfoLog(shader, logSize, &logSize, log);

			std::string err = src.errorLog(std::string(log));

			delete[] log;

			throw Exception("HdlShader::HdlShader - error while compiling the shader " + getSourceName() + " : \n" + err, __FILE__, __LINE__);
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

		attachedFragmentShader = attachedVertexShader = 0;

		//attach the two Shaders if there aren't NULL
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

		// Look for some error during the linking
		GLint link_status = GL_TRUE;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);
		if(link_status!=GL_TRUE)
		{
			std::cout << "Error during Program linking" << std::endl;

			// get the log
			GLint logSize;

			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

			char *log = new char[logSize]; // +1 <=> '/0'

			glGetProgramInfoLog(program, logSize, &logSize, log);

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

			throw Exception("HdlProgram::link - Error during Program linking : " + logstr, __FILE__, __LINE__);
			return false;
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
			if( attachedVertexShader !=0 ) glDetachShader(program, attachedVertexShader);
			attachedVertexShader = shader.getShaderID();
			glAttachShader(program, attachedVertexShader);
		}
		else //shader.getType()==GL_FRAGMENT_SHADER
		{
			if( attachedFragmentShader !=0 ) glDetachShader(program, attachedFragmentShader);
			attachedFragmentShader = shader.getShaderID();
			glAttachShader(program, attachedFragmentShader);
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
	}

	/**
	\fn    bool HdlProgram::setFragmentLocation(const std::string& fragName, int frag)
	\brief Link the name of a fragment output variable to a fragment unit.

	\param fragName Name of the fragment output variable.
	\param frag     Index of the desired fragment unit.

	\return False in case of failure, true otherwise.
	**/
	bool HdlProgram::setFragmentLocation(const std::string& fragName, int frag)
	{
		glErrors(); // clean error buffer
		#ifdef __DEVELOPMENT_VERBOSE__
			std::cout << "HdlProgram::setFragmentLocation - FragName : " << fragName << std::endl;
		#endif
		glBindFragDataLocation(program, frag, fragName.c_str());
		return !glErrors(); //return false if any error
	}

	/**
	\fn    bool HdlProgram::modifyVar(const std::string& varName, SHADER_DATA_TYPE type, int val1, int val2, int val3, int val4)
	\brief Change a uniform variable in a shader (Integer version).

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in : SHADER_VAR, SHADER_VEC2, SHADER_VEC3, SHADER_VEC4.
	\param val1    Corresponding value to assign.
	\param val2    Corresponding value to assign.
	\param val3    Corresponding value to assign.
	\param val4    Corresponding value to assign.

	\return False in case of failure, true otherwise.
	**/
	bool HdlProgram::modifyVar(const std::string& varName, SHADER_DATA_TYPE type, int val1, int val2, int val3, int val4)
	{
		use();
		GLint loc = glGetUniformLocationARB(program, varName.c_str());
		if (loc==-1)
		{
			throw Exception("HdlProgram::modifyVar - Wrong location, does this var exist : \"" + varName + "\"? Is it used in the program? (May be the GLCompiler swapped it)", __FILE__, __LINE__);
			return false;
		}

		/*GLint curPrgm = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &curPrgm);
		if(curPrgm!=program || curPrgm==0)
		{
		std::cout << "You are trying to modify a program you aren't using (call void use(void) for this) - glError > GL_INVALID_OPERATION" << std::endl;
		return false;
		}*/

		switch(type)
		{
			case SHADER_VAR  : 	glUniform1iARB(loc, val1);                   break;
			case SHADER_VEC2 : 	glUniform2iARB(loc, val1, val2);             break;
			case SHADER_VEC3 : 	glUniform3iARB(loc, val1, val2, val3);       break;
			case SHADER_VEC4 : 	glUniform4iARB(loc, val1, val2, val3, val4); break;
			default :		throw Exception("HdlProgram::modifyVar - Unknown variable type");
		}

		return true;
	}

	/**
	\fn    bool HdlProgram::modifyVar(const std::string& varName, SHADER_DATA_TYPE type, float val1, float val2, float val3, float val4)
	\brief Change a uniform variable in a shader (Floating point version).

	\param varName Name of the fragment output variable.
	\param type    Kind of variable in : SHADER_VAR, SHADER_VEC2, SHADER_VEC3, SHADER_VEC4.
	\param val1    Corresponding value to assign.
	\param val2    Corresponding value to assign.
	\param val3    Corresponding value to assign.
	\param val4    Corresponding value to assign.

	\return False in case of failure, true otherwise.
	**/
	bool HdlProgram::modifyVar(const std::string& varName, SHADER_DATA_TYPE type, float val1, float val2, float val3, float val4)
	{
		use();
		GLint loc = glGetUniformLocationARB(program, varName.c_str());
		if (loc==-1)
		{
			throw Exception("HdlProgram::modifyVar - Wrong location, does this var exist : \"" + varName + "\"? Is it used in the program? (May be the GLCompiler swapped it)", __FILE__, __LINE__);
			return false;
		}

		/*GLint curPrgm = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &curPrgm);
		if(curPrgm!=program || curPrgm==0)
		{
		std::cout << "You are trying to modify a program you aren't using (call void use(void) for this) - glError > GL_INVALID_OPERATION" << std::endl;
		return false;
		}*/

		switch(type)
		{
			case SHADER_VAR  : 	glUniform1fARB(loc, val1);                   break;
			case SHADER_VEC2 : 	glUniform2fARB(loc, val1, val2);             break;
			case SHADER_VEC3 : 	glUniform3fARB(loc, val1, val2, val3);       break;
			case SHADER_VEC4 : 	glUniform4fARB(loc, val1, val2, val3, val4); break;
			default :		throw Exception("HdlProgram::modifyVar - Unknown variable type");
		}

		return true;
	}

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

		return param;
	}

	HdlProgram::~HdlProgram(void)
	{
		glDetachShader(program, attachedFragmentShader);
		glDetachShader(program, attachedVertexShader);
		glDeleteProgram(program);
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
