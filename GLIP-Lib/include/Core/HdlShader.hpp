/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. Kerviche                                                                               */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : glip-lib.net                                                                              */
/*                                                                                                               */
/*     File          : HdlShader.hpp                                                                             */
/*     Original Date : August 7th 2010                                                                           */
/*                                                                                                               */
/*     Description   : OpenGL Pixel and Fragment Shader Handle                                                   */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    HdlShader.hpp
 * \brief   OpenGL Pixel and Fragment Shader Handle
 * \author  R. KERVICHE
 * \date    August 7th 2010
**/

#ifndef __HDLSHADER_INCLUDE__
#define __HDLSHADER_INCLUDE__

	// GL include
	#include "Core/LibTools.hpp"
        #include "Core/OglInclude.hpp"
        #include "Core/ShaderSource.hpp"
	#include "Core/HdlDynamicData.hpp"
	#include <string>
        #include <fstream>

namespace Glip
{
	namespace CoreGL
	{
		// Shader Handle
		/**
		\class HdlShader
		\brief Object handle for OpenGL Shaders Objects (both vertex and fragment shaders).
		**/
		class GLIP_API HdlShader : public ShaderSource
		{
			private :
				// Data
				GLuint shader;
				GLenum type;

			public :
				// Functions
				HdlShader(GLenum _type, const ShaderSource& src);
				~HdlShader(void);

				GLuint getShaderID(void) const;
				GLenum getType(void)     const;
		};

		// Program Handle
		/**
		\class HdlProgram
		\brief Object handle for OpenGL Program (A vertex and a fragment program).
		**/
		class GLIP_API HdlProgram
		{
			public :
				static const int numShaderTypes = 6; 

			private :
				// Data
				bool 				valid;
				GLuint 				program;
				GLuint 				attachedShaders[HandleOpenGL::numShaderTypes], 
								attachedFragmentShader;
				std::vector<std::string>	activeUniforms;
				std::vector<GLenum>		activeTypes;

			public :
				// Functions
				HdlProgram(void);
				~HdlProgram(void);

				bool isValid(void) const;
				void updateShader(const HdlShader& shader, bool linkNow = true);
				void link(void);
				void use(void);

				const std::vector<std::string>& getUniformsNames(void) const;
				const std::vector<GLenum>& getUniformsTypes(void) const;

				void setFragmentLocation(const std::string& fragName, int frag);
				void setVar(const std::string& varName, GLenum type, int v0, int v1=0, int v2=0, int v3=0);
				void setVar(const std::string& varName, GLenum type, int* v);
				void setVar(const std::string& varName, GLenum type, unsigned int v0, unsigned int v1=0, unsigned int v2=0, unsigned int v3=0);
				void setVar(const std::string& varName, GLenum type, unsigned int* v);
				void setVar(const std::string& varName, GLenum type, float v0, float v1=0, float v2=0, float v3=0);
				void setVar(const std::string& varName, GLenum type, float* v);
				void setVar(const std::string& varName, const HdlDynamicData& data);

				void getVar(const std::string& varName, int* ptr);
				void getVar(const std::string& varName, unsigned int* ptr);
				void getVar(const std::string& varName, float* ptr);
				void getVar(const std::string& varName, HdlDynamicData& data);

				bool isUniformVariableValid(const std::string& varName);
	
				// Static tools :
				static int maxVaryingVar(void);
				static void stopProgram(void);
		};
	}
}

#endif

