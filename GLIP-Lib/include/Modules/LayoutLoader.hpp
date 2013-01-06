/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : LayoutLoader.hpp                                                                          */
/*     Original Date : December 22th 2011                                                                        */
/*                                                                                                               */
/*     Description   : Layout Loader from files or strings.                                                      */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    LayoutLoader.hpp
 * \brief   Layout Loader from files or strings.
 * \author  R. KERVICHE
 * \date    December 22th 2011
**/

#ifndef __LAYOUT_LOADER_INCLUDE__
#define __LAYOUT_LOADER_INCLUDE__

	#include <map>
	#include "../Core/HdlTexture.hpp"
	#include "../Core/ShaderSource.hpp"
	#include "../Core/Filter.hpp"
	#include "../Core/Pipeline.hpp"

namespace Glip
{
	using namespace CoreGL;
	using namespace CorePipeline;

	namespace Modules
	{

/**
\class LayoutLoader
\brief Load and save pipelines layouts to file

The LayoutLoader module enables you to use dynamic pipeline saved in a file or a standard string. It will create either a Glip::Core::PipelineLayout or a Glip::Core::Pipeline that you can use directly or combined with other pipeline structures.

The script must be structured with the following commands (but no special order is needed except standard declaration order) :

- Format for the texture : <BR>
<b>TEXTURE_FORMAT</b>:<i>format_name</i>(<i>integer width</i>, <i>integer height</i>, <i>GLEnum mode</i>, <i>GLEnum depth</i>, <i>GLEnum minFiltering</i>, <i>GLEnum maxFiltering</i> [, <i>GLEnum sWrapping</i>, <i>GLEnum TWrapping</i>, <i>integer maximum_mipmap_level</i> ]);

- Required format to be provided by the application (for dynamic sizes). See LayoutLoader::addRequiredElement() : <BR>
<b>REQUIRED_FORMAT</b>:<i>format_name</i>();

- Shader source code, from the same file : <BR>
<b>SHADER_SOURCE</b>:<i>source_name</i>()
{
	<i>source code</i>
}

- Shader source code, from the another file : <BR>
<b>SHADER_SOURCE</b>:<i>source_name</i>(<i>string filename</i>);

- Filter layout :
<b>FILTER_LAYOUT</b>:<i>filter_layout_name</i>(<i>format_name</i>, <i>fragment_shader_source</i> [, <i>vertex_shader_source</i>/<b>DEFAULT_VERTEX_SHADER</b>, <b>CLEARING_ON</b>/<b>CLEARING_OFF</b>, <b>BLENDING_ON</b>/<b>BLENDING_OFF</b>]);

- Pipeline layout : <BR>
<b>PIPELINE_LAYOUT</b>:<i>pipeline_layout_name</i>() <BR>
{ <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>INPUT_PORTS</b>(<i>input_port_name_1</i> [,<i>input_port_name_2</i>, ..., <i>input_port_name_n</i>] ); <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>OUTPUT_PORTS</b>(<i>output_port_name_1</i> [,<i>output_port_name_2</i>, ..., <i>output_port_name_n</i>] ); <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>FILTER_INSTANCE</b>:<i>filter_instance_name</i>(<i>filter_layout_name</i>); <BR>
&nbsp;&nbsp;&nbsp;&nbsp; ... <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>PIPELINE_INSTANCE</b>:<i>pipeline_instance_name</i>(<i>pipeline_layout_name</i>); <BR>
&nbsp;&nbsp;&nbsp;&nbsp; ... <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>CONNECTION</b>(THIS,<i>this_port_name</i>,<i>element_name</i>,<i>e_port_name</i>); // Connection to input. <BR>
&nbsp;&nbsp;&nbsp;&nbsp; ... <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>CONNECTION</b>(<i>element_1_name</i>, <i>e1_port_name</i>, <i>element_2_name</i>, <i>e2_port_name</i>); // Connection between two elements (filter, pipeline), the connection goes from <i>element_1_name</i>::<i>e1_port_name</i> to <i>element_2_name</i>::<i>e2_port_name</i>. <BR>
&nbsp;&nbsp;&nbsp;&nbsp; ... <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>CONNECTION</b>(<i>element_name</i>,<i>e_port_name</i>,THIS,<i>this_port_name</i>); // Connection to output. <BR>
&nbsp;&nbsp;&nbsp;&nbsp; ... <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <i>If you don't declare any connection, the loader will try to connect elements by himself using PipelineLayout::autoConnect(), make sure that the pipeline is compliant with the corresponding rules.</i> <BR>
}

- Required pipeline layout to be provided by the application (for decorators). See LayoutLoader::addRequiredElement() : <BR>
<b>REQUIRED_PIPELINE</b>:<i>pipeline_layout_name</i>();

- Main pipeline layout (the layout at the end of the loading stage) : <BR>
<i>Same description as PIPELINE_LAYOUT, but starting with </i> <b>PIPELINE_MAIN</b>.

- Include another script, in order to use some of its definition (format, source, filter or pipeline layout) : <BR>
<b>INCLUDE_FILE</b>(<i>string filename</i>);

- Distribute code to the shaders code with the SHARED_CODE marker. Insert them in the SHADER_SOURCE elements by adding INSERT_SHARED_CODE marker : <BR>
<b>SHARED_CODE</b> <BR>
{ <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <i>// shared code goes here...</i> <BR>
} <BR>
&nbsp;&nbsp; Then use : <BR>
<b>SHADER_SOURCE</b>:<i>source_name</i>() <BR>
{ <BR>
&nbsp;&nbsp;&nbsp;&nbsp; <b>INSERT_SHARED_CODE</b> // The shared code will be inserted here... <BR>
} <BR>

- Comments : C++ style.

<i>Hello world pipeline example </i> (no processing, just resizing to <i>format</i>) :
	\code
	// The output format (for the output texture). Note that the filtering parameters are of no use in this pipeline. They will be use in the next processing/display step :
	TEXTURE_FORMAT:format(640,480,GL_RGB,GL_UNSIGNED_BYTE,GL_LINEAR,GL_LINEAR);

	// The shader source (for more information, check the GLSL language specifications at http://www.opengl.org/documentation/glsl/) :
	SHADER_SOURCE:HelloWorld()
	{
		#version 130

		uniform sampler2D	texInput;
		out     vec4 		texOutput;

		void main()
		{
			// Get the input data :
			vec4 col  = textureLod(texInput, gl_TexCoord[0].st, 0.0);

			// Write the output data :
			texOutput = col;
		}
	}

	// Declare the filter layout :
	FILTER_LAYOUT:helloFilter(format,HelloWorld);
	// The filter layout will have one input port and one output port, which names are respectively texInput and texOutput.
	// This information is gathered from the shader source HelloWorld, by analyzing the variables declared as uniform sampler2D for inputs and out vec4 for outputs.

	PIPELINE_MAIN:pMainGradient()
	{
		// Declare some input and output ports for this pipeline ::
		INPUT_PORTS(texInput);
		OUTPUT_PORTS(texOutput);

		FILTER_INSTANCE:instHello(helloFilter);

		// Since the input and output port names we chose for the pipeline are the same than for the filter
		// (as described in the shader source) then we don't need to do the connections (it will be made automatically).
		// However one can imagine replacing the previous code by :
		//
		//INPUT_PORTS(input);
		//OUTPUT_PORTS(output);
		//
		// In that case, we would have to declare the connections as :
		//
		//CONNECTION(THIS,input,instHello,texInput);
		//CONNECTION(instHello,texOutput,THIS,texOutput);
		//
	}
	\endcode

Loading Example :
	\code
	try
	{
		// Load a PipelineLayout :
		Loader loader;
		PipelineLayout* myLayout = loader("./path/pipeline.ppl");

		// use it :
		Pipeline* myPipeline1 = new Pipeline(*myLayout,"Pipeline1");
		Pipeline* myPipeline2 = new Pipeline(*myLayout,"Pipeline1");

		// Then clean :
		delete myLayout;

		// For a single pipeline :
		Pipeline* myPipelineU = loader("./path/otherPipeline.ppl","myPipelineName");

		// use them, see Glip::Core::Pipeline documentation...

		// Clean all :
		delete myPipeline1;
		delete myPipeline2;
		delete myPipelineU;
	}
	catch(Exception& e)
	{
		std::cout << "An exception was caught : " << std::endl;
		std::cout << e.what() << std::endl;
	}
	\endcode

**/
		class LayoutLoader
		{
			public :
				enum LoaderKeyword
				{
					FORMAT_LAYOUT,
					SHADER_SOURCE,
					FILTER_LAYOUT,
					PIPELINE_LAYOUT,
					PIPELINE_MAIN,
					INCLUDE_FILE,
					FILTER_INSTANCE,
					PIPELINE_INSTANCE,
					CONNECTION,
					INPUT_PORTS,
					OUTPUT_PORTS,
					THIS_PIPELINE,
					DEFAULT_VERTEX_SHADER,
					CLEARING_ON,
					CLEARING_OFF,
					BLENDING_ON,
					BLENDING_OFF,
					REQUIRED_FORMAT,
					REQUIRED_PIPELINE,
					SHARED_CODE,
					INSERT_SHARED_CODE,
					NumKeywords
				};

			private :
				static const char* 				keywords[NumKeywords];
				// Reading :
				std::string					sharedCode;
				std::vector<LoaderKeyword> 			entryType;
				std::vector<std::string> 			entryName;
				std::vector<std::string>			entryCode;
				std::vector<std::string>			entryPath;
				std::map<std::string,HdlTextureFormat*> 	formatList;
				std::map<std::string,ShaderSource*> 		sourceList;
				std::map<std::string,FilterLayout*> 		filterList;
				std::map<std::string,PipelineLayout*> 		pipelineList;
				std::map<std::string,HdlTextureFormat*>		requiredFormatList;
				std::map<std::string,PipelineLayout*>		requiredPipelineList;

				// Writing :
				std::string formatCode;
				std::string sourceCode;
				std::string filterCode;
				std::string pipelineCode;

				LoaderKeyword 			getKeyword(const std::string& str);
				void				removeCommentary(std::string& source, std::string start, std::string end, bool cumulative);
				std::string 			getSource(const std::string& sourceName, std::string& path);
				std::vector<std::string> 	getArguments(const std::string& code);
				std::string 			getBody(const std::string& code);
				void				updateEntriesLists(const std::string& sourceName, bool slave=false);
				HdlTextureFormat*		buildFormat(const std::string& code, const std::string& name);
				ShaderSource*			buildShaderSource(const std::string& code, const std::string& name, const std::string& path);
				FilterLayout* 			buildFilter(const std::string& code, const std::string& name);
				PipelineLayout*			buildPipeline(std::string code, const std::string& name);
				void 				clean(void);
				void 				writeFormatCode(const __ReadOnly_HdlTextureFormat& hLayout, std::string name);
				void 				writeSourceCode(const ShaderSource& source, std::string name);
				void 				writeFilterCode(const __ReadOnly_FilterLayout& fLayout);
				void 				writePipelineCode(const __ReadOnly_PipelineLayout& pLayout, bool main=false);
			public :
				LayoutLoader(void);
				~LayoutLoader(void);

				PipelineLayout* operator()(const std::string& source); //can be a file or directly the source
				Pipeline* operator()(const std::string& source, const std::string& pipelineName);
				std::string write(const __ReadOnly_PipelineLayout& pLayout, std::string filename="");

				void addRequiredElement(const std::string& name, const __ReadOnly_HdlTextureFormat& fmt);
				void addRequiredElement(const std::string& name, __ReadOnly_PipelineLayout& layout);
				int  clearRequiredElements(const std::string& name = "");
		};
	}
}

#endif // LAYOUTLOADER_HPP_INCLUDED
