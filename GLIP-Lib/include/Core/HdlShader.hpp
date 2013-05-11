/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
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
        #include "OglInclude.hpp"
        #include "ShaderSource.hpp"
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
		class HdlShader : public ShaderSource
		{
			private :
				// Data
				GLuint shader;
				GLenum type;

			public :
				// Functions
				HdlShader(GLenum _type, ShaderSource& src);
				~HdlShader(void);

				GLuint getShaderID(void) const;
				GLenum getType(void)     const;
		};

		// Program Handle
		/**
		\class HdlProgram
		\brief Object handle for OpenGL Program (A vertex and a fragment program).
		**/
		class HdlProgram
		{
			private :
				// Data
				bool 				valid;
				GLuint 				program;
				GLuint 				attachedVertexShader, 
								attachedFragmentShader;
				std::vector<std::string>	activeUniforms;
				std::vector<GLenum>		activeTypes;

				// Functions
				bool link(void);
			public :
				// Functions
				HdlProgram(void);
				HdlProgram(const HdlShader& shd1, const HdlShader& shd2);
				~HdlProgram(void);

				bool isValid(void);

				void update(const HdlShader& shader, bool lnk = true);
				void use(void);

				const std::vector<std::string>& getUniformVarsNames(void) const;
				const std::vector<GLenum>& getUniformVarsTypes(void) const;

				void setFragmentLocation(const std::string& fragName, int frag);
				void modifyVar(const std::string& varName, GLenum type, int v0, int v1=0, int v2=0, int v3=0);
				void modifyVar(const std::string& varName, GLenum type, int* v);
				void modifyVar(const std::string& varName, GLenum type, unsigned int v0, unsigned int v1=0, unsigned int v2=0, unsigned int v3=0);
				void modifyVar(const std::string& varName, GLenum type, unsigned int* v);
				void modifyVar(const std::string& varName, GLenum type, float v0, float v1=0, float v2=0, float v3=0);
				void modifyVar(const std::string& varName, GLenum type, float* v);

				// Static tools :
				static int maxVaryingVar(void);
				static void stopProgram(void);
		};
	}
}

#endif

