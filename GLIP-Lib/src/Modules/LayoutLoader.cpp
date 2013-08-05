/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : LayoutLoader.cpp                                                                          */
/*     Original Date : December 22th 2011                                                                        */
/*                                                                                                               */
/*     Description   : Layout Loader from files or strings.                                                      */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    LayoutLoader.cpp
 * \brief   Layout Loader from files or strings.
 * \author  R. KERVICHE
 * \date    December 22th 2011
**/

	// Includes :
	#include <sstream>
	#include <algorithm>
	#include "Core/Exception.hpp"
	#include "Modules/LayoutLoader.hpp"
	#include "devDebugTools.hpp"

	// Namespaces :
	using namespace Glip;
	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;
	using namespace Glip::Modules;
	using namespace Glip::Modules::VanillaParserSpace;

	// Static variables :
	const char* Glip::Modules::keywordsLayoutLoader[LL_NumKeywords] =  {	"TEXTURE_FORMAT",
										"SHADER_SOURCE",
										"FILTER_LAYOUT",
										"PIPELINE_LAYOUT",
										"PIPELINE_MAIN",
										"INCLUDE_FILE",
										"FILTER_INSTANCE",
										"PIPELINE_INSTANCE",
										"CONNECTION",
										"INPUT_PORTS",
										"OUTPUT_PORTS",
										"THIS",
										"DEFAULT_VERTEX_SHADER",
										"CLEARING_ON",
										"CLEARING_OFF",
										"BLENDING_ON",
										"BLENDING_OFF",
										"REQUIRED_FORMAT",
										"REQUIRED_PIPELINE",
										"SHARED_SOURCE",
										"INCLUDE_SHARED_SOURCE",
										"GEOMETRY",
										"GRID_2D",
										"GRID_3D",
										"CUSTOM_MODEL",
										"GEOMETRY_FROM_FILE",
										"STANDARD_QUAD",
										"VERTEX",
										"ELEMENT",
										"ADD_PATH"
									};

// LayoutLoader
	/**
	\fn LayoutLoader::LayoutLoader(void)
	\brief LayoutLoader constructor.
	**/
	LayoutLoader::LayoutLoader(void)
	 : isSubLoader(false)
	{
		clearPaths();
	}

	LayoutLoader::~LayoutLoader(void)
	{
		// Dynamic :
		clean();

		// And static :
		requiredFormatList.clear();
		requiredPipelineList.clear();
	}

	void LayoutLoader::clean(void)
	{
		// Dynamic only :
		currentPath.clear();
		dynamicPaths.clear();
		associatedKeyword.clear();
		formatList.clear();
		sourceList.clear();
		geometryList.clear();
		filterList.clear();
		pipelineList.clear();
		sharedCodeList.clear();
	}

	LayoutLoaderKeyword LayoutLoader::getKeyword(const std::string& str)
	{
		for(int i=0; i<LL_NumKeywords; i++)
			if(keywordsLayoutLoader[i]==str) return static_cast<LayoutLoaderKeyword>(i);

		return LL_UnknownKeyword;
	}

	void LayoutLoader::classify(const std::vector<VanillaParserSpace::Element>& elements, std::vector<LayoutLoaderKeyword>& associatedKeywords)
	{
		associatedKeywords.clear();

		for(std::vector<VanillaParserSpace::Element>::const_iterator it = elements.begin(); it!=elements.end(); it++)
			associatedKeywords.push_back( getKeyword( (*it).strKeyword ) );
	}

	bool LayoutLoader::fileExists(const std::string& filename, std::string& source, const bool test)
	{
		std::ifstream file;
	
		file.open(filename.c_str());

		if(file.is_open() && file.good() && !file.fail())
		{
			std::string 	buffer,	
					line;

			file.seekg(0, std::ios::beg);

			while(std::getline(file,line))
			{
				buffer += line;
				buffer += "\n";
			}

			file.close();

			if(test)
				return source!=buffer;
			else
			{
				source = buffer;
				return true;
			}
		}
		else
			return false;
	}

	void LayoutLoader::loadFile(const std::string& filename, std::string& content, std::string& usedPath)
	{
		bool oneLoaded = false;
		std::string source;

		// Check all path :
		std::vector<std::string> possiblePaths;

		// Blank :
		if( fileExists( filename, source, oneLoaded ) )
		{
			possiblePaths.push_back("");
			oneLoaded = true;
		}

		// From dynamic path :
		for(std::vector<std::string>::iterator it=dynamicPaths.begin(); it!=dynamicPaths.end(); it++)
		{
			if( fileExists( *it + filename, source, oneLoaded ) )
			{
				possiblePaths.push_back( *it );
				oneLoaded = true;
			}
		}

		if(possiblePaths.empty())
		{
			std::string msg = "Unable to load file \"" + filename + "\" from the following locations : ";

			for(std::vector<std::string>::iterator it=dynamicPaths.begin(); it!=dynamicPaths.end(); it++)
				msg  += "\n-> " + *it;

			throw Exception(msg, __FILE__, __LINE__);
		}
		else if(possiblePaths.size()>1)
		{
			std::string msg = "Ambiguous link : file \"" + filename + "\" was found in multiple locations, with different sources : ";

			for(std::vector<std::string>::iterator it=possiblePaths.begin(); it!=possiblePaths.end(); it++)
				msg  += "\n-> " + *it;

			throw Exception(msg, __FILE__, __LINE__);
		}
		//else

		usedPath = possiblePaths.front();
		//std::string realFilename = usedPath + filename;
		
		content.clear();
		content = source;
	}

	void LayoutLoader::preliminaryTests(const VanillaParserSpace::Element& e, char nameProperty, int minArguments, int maxArguments, char bodyProperty, const std::string& objectName)
	{
		// xxxProperty :
		//	1  : Must have.
		//	0  : Indiferent.
		//	-1 : Must not have.

		// Generate help :
		std::string nameDecorator;

		if(!e.name.empty())
			nameDecorator = ", with name \"" + e.name + "\",";

		// Tests :
		if((e.noName || e.name.empty()) && nameProperty>0)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + " does not have a name.", __FILE__, __LINE__);
		else if((!e.noName || !e.name.empty()) && nameProperty<0)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should not have a name.", __FILE__, __LINE__);

		if(maxArguments==0 && !e.arguments.empty())
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should not have arguments, but it has " + to_string(e.arguments.size()) + ".", __FILE__, __LINE__);

		if(minArguments>0 && e.arguments.empty())
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should have at least " + to_string(minArguments) + " argument(s).", __FILE__, __LINE__);

		if(e.arguments.size()!=minArguments && minArguments==maxArguments)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should have exactly "  + to_string(minArguments) + " argument(s).", __FILE__, __LINE__);

		if(e.arguments.size()<minArguments)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should have at least " + to_string(minArguments) + " argument(s), but it has only " + to_string(e.arguments.size()) + ".", __FILE__, __LINE__);

		if(e.arguments.size()>maxArguments)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should have at most " + to_string(maxArguments) + " argument(s), but it has " + to_string(e.arguments.size()) + ".", __FILE__, __LINE__);

		if(e.noBody && bodyProperty>0)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " does not have a body.", __FILE__, __LINE__);
		else if(!e.noBody && bodyProperty<0)
			throw Exception("From line " + to_string(e.startLine) + " : " + objectName + nameDecorator + " should not have a body.", __FILE__, __LINE__);
	}

	void LayoutLoader::enhanceShaderSource(std::string& str)
	{
		const std::string 	spacers 	= " \t\r\n\f\v",
					endSpacers 	= " \t\r\n\f\v;(){}[],./\\|+*",
					keyword 	= keywordsLayoutLoader[KW_LL_INCLUDE_SHARED_SOURCE];

		size_t pos = str.find(keyword);

		while(pos!=std::string::npos)
		{
			size_t k = pos + keyword.size();

			// Find the following trailing ':' :
			for(; k<str.size(); k++)
			{
				if(str[k]==':')
					break;
				else if(spacers.find(str[k])==std::string::npos)
				{
					k = str.size();
					break;
				}
			}

			// Possible errors :
			if(k>=str.size())
				throw Exception("Missing ':' after keyword \"" + keyword + "\" in ShaderSource object (or file).", __FILE__, __LINE__);

			size_t 	begin 	= str.find_first_not_of(spacers, k);

			if(begin==std::string::npos)
				throw Exception("Missing name after keyword \"" + keyword + "\" in ShaderSource object (or file).", __FILE__, __LINE__);

			size_t 	end	= str.find_first_of(endSpacers, begin);

			if(begin==std::string::npos)
				throw Exception("Missing end to name after keyword \"" + keyword + "\" in ShaderSource object (or file).", __FILE__, __LINE__);

			// Extract name :
			std::string sharedCodeName = str.substr(begin+1, end-begin-1);

			// Find it in the base :
			std::map<std::string,std::string>::iterator it = sharedCodeList.find(sharedCodeName);

			if(it==sharedCodeList.end())
				throw Exception("SharedCode object \"" + sharedCodeName + "\" is not reference in the current database.", __FILE__, __LINE__);

			// Replace :
			str.replace(str.begin() + pos, str.begin() + end, it->second.begin(), it->second.end());

			// Search next occurence :
			pos = str.find(keyword);
		}

		// Make sure there is at least one line :
		str += '\n';
	}

	void LayoutLoader::append(LayoutLoader& subLoader)
	{
		// Test for doubles :
		#define TEST_FOR_DOUBLES( varName, typeName, type) \
			for( std::map<std::string, type >::iterator it = subLoader.varName.begin(); it!=subLoader.varName.end(); it++ ) \
			{ \
				if( varName.find( it->first ) != varName.end() ) \
					throw Exception("The " + std::string(typeName) +  " \"" + it->first + "\" already exists in current pipeline.", __FILE__, __LINE__); \
			} \

			TEST_FOR_DOUBLES( sharedCodeList, 	"SharedCode",		std::string)
			TEST_FOR_DOUBLES( formatList,		"Format", 		HdlTextureFormat)
			TEST_FOR_DOUBLES( sourceList,		"ShaderSource", 	ShaderSource)
			TEST_FOR_DOUBLES( geometryList,		"Geometry",		GeometryModel)
			TEST_FOR_DOUBLES( filterList,		"FilterLayout", 	FilterLayout)
			TEST_FOR_DOUBLES( pipelineList,		"PipelineLayout",	PipelineLayout)

		#undef TEST_FOR_DOUBLES

		// Append :
		#define INSERTION( item ) item.insert( subLoader.item.begin(), subLoader.item.end() )

			INSERTION( sharedCodeList );
			INSERTION( formatList );
			INSERTION( sourceList );
			INSERTION( geometryList );
			INSERTION( filterList );
			INSERTION( pipelineList );

		#undef INSERTION
	}

	void LayoutLoader::appendPath(const VanillaParserSpace::Element& e)
	{
		preliminaryTests(e, -1, 1, 1, -1, "AppendPath");

		std::string resultingPath = currentPath + e.arguments[0];

		if( e.arguments[0].empty() )
			throw Exception("From line " + to_string(e.startLine) + " : Path is empty.", __FILE__, __LINE__);

		if( resultingPath[resultingPath.size()-1]!='/' )
			throw Exception("From line " + to_string(e.startLine) + " : Path should be ended by delimiter '/' (current : \"" + e.arguments[0] + "\").", __FILE__, __LINE__);

		// Test if it is already in the dynamic path list :
		std::vector<std::string>::iterator it = std::find(dynamicPaths.begin(), dynamicPaths.end(), resultingPath);

		if(it==dynamicPaths.end())
			dynamicPaths.push_back(resultingPath);
	}

	void LayoutLoader::includeFile(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, -1, 1, 1, -1, "IncludeFile");

		if(e.arguments[0].find('\n')!=std::string::npos)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot include file \"" + e.arguments[0] + "\" because its name contains a newline character.", __FILE__, __LINE__);
		else if(e.arguments[0].find('\\')!=std::string::npos)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot include file \"" + e.arguments[0] + "\" because its name contains the illegal character '\\'.", __FILE__, __LINE__);

		LayoutLoader subLoader;

		// Set it as a sub-loader :
		subLoader.isSubLoader = true;
		subLoader.requiredFormatList = requiredFormatList;
		subLoader.requiredPipelineList = requiredPipelineList;

		// Copy this path to the inner version :
		subLoader.dynamicPaths = dynamicPaths;

		// Load the file :
		std::string content;

		loadFile(e.arguments[0], content, subLoader.currentPath);

		try
		{
			// Build all the elements :
			std::string dummyMainPipelineName;
			subLoader.process(content, dummyMainPipelineName);

			// Append :
			append(subLoader);
		}
		catch(Exception& ex)
		{
			if(!subLoader.currentPath.empty())
			{
				Exception m("Exception caught while loading file \"" + e.arguments[0] + "\" (path : \"" + subLoader.currentPath + "\") : ", __FILE__, __LINE__);
				throw m + ex;
			}
			else
			{
				Exception m("Exception caught while loading file \"" + e.arguments[0] + "\" : ", __FILE__, __LINE__);
				throw m + ex;
			}
		}
	}

	void LayoutLoader::buildRequiredFormat(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 1, 10, -1, "RequiredFormat");

		// Identify the target :
		std::map<std::string,HdlTextureFormat>::iterator it = requiredFormatList.find(e.arguments[0]);

		if(it==requiredFormatList.end())
		{
			// Try in the current format list also :
			it = formatList.find(e.arguments[0]);

			if(it==formatList.end())
				throw Exception("From line " + to_string(e.startLine) + " : The required format \"" + e.arguments[0] + "\" was not found.", __FILE__, __LINE__);
		}

		std::map<std::string,HdlTextureFormat>::iterator it2 = formatList.find(e.name);

		if(it2!=formatList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A Format Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);

		// Check for possible arguments, to modify the texture :
		// Get the data :
		int w, h, mipmap = 0;
		GLenum mode, depth, minFilter, magFilter, sWrap, tWrap;

		if(e.arguments.size()>1)
		{
			if(e.arguments[1]=="*")
				w = it->second.getWidth();
			else if(!from_string(e.arguments[1], w))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read width for format \"" + e.name + "\". Token : \"" + e.arguments[0] + "\".", __FILE__, __LINE__);
		}
		else
			w = it->second.getWidth();

		if(e.arguments.size()>2)
		{
			if(e.arguments[2]=="*")
				h = it->second.getHeight();
			else if(!from_string(e.arguments[2], h))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read height for format \"" + e.name + "\". Token : \"" + e.arguments[0] + "\".", __FILE__, __LINE__);
		}
		else
			h = it->second.getHeight();

		if(e.arguments.size()>3)
		{
			if(e.arguments[3]=="*")
				mode = it->second.getGLMode();
			else
				mode = glFromString(e.arguments[3]);
		}
		else
			mode = it->second.getGLMode();

		if(e.arguments.size()>4)
		{
			if(e.arguments[4]=="*")
				depth = it->second.getGLDepth();
			else
				depth = glFromString(e.arguments[4]);
		}
		else
			depth = it->second.getGLDepth();

		if(e.arguments.size()>5)
		{
			if(e.arguments[5]=="*")
				minFilter = it->second.getMinFilter();
			else
				minFilter = glFromString(e.arguments[5]);
		}
		else
			minFilter = it->second.getMinFilter();

		if(e.arguments.size()>6)
		{
			if(e.arguments[6]=="*")
				magFilter = it->second.getMagFilter();
			else
				magFilter = glFromString(e.arguments[6]);
		}
		else
			magFilter = it->second.getMagFilter();

		if(e.arguments.size()>7)
		{
			if(e.arguments[7]=="*")
				sWrap = it->second.getSWrapping();
			else
				sWrap = glFromString(e.arguments[7]);
		}
		else
			sWrap = it->second.getSWrapping();

		if(e.arguments.size()>8)
		{
			if(e.arguments[8]=="*")
				tWrap = it->second.getTWrapping();
			else
				tWrap = glFromString(e.arguments[8]);
		}
		else
			tWrap = it->second.getTWrapping();

		if(e.arguments.size()>9)
		{
			if(e.arguments[9]=="*")
				mipmap = it->second.getMaxLevel();
			else if(!from_string(e.arguments[9], mipmap))
				throw Exception("From line " + to_string(e.startLine) + " : Canno read mipmap for format \"" + e.name + "\". Token : \"" + e.arguments[8] + "\".", __FILE__, __LINE__);
		}
		else
			mipmap = it->second.getMaxLevel();

		// Find possible errors :
		if(mode==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read mode for format \"" + e.name + "\". Token : \"" + e.arguments[2] + "\".", __FILE__, __LINE__);
		if(depth==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read depth for format \"" + e.name + "\". Token : \"" + e.arguments[3] + "\".", __FILE__, __LINE__);
		if(minFilter==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read MinFilter for format \"" + e.name + "\". Token : \"" + e.arguments[4] + "\".", __FILE__, __LINE__);
		if(magFilter==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read MagFilter for format \"" + e.name + "\". Token : \"" + e.arguments[5] + "\".", __FILE__, __LINE__);
		if(sWrap==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read SWrapping for format \"" + e.name + "\". Token : \"" + e.arguments[6] + "\".", __FILE__, __LINE__);
		if(sWrap==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read TWrapping for format \"" + e.name + "\". Token : \"" + e.arguments[7] + "\".", __FILE__, __LINE__);
		if(mipmap<0)
			throw Exception("From line " + to_string(e.startLine) + " : Mipmap cannot be negative for format \"" + e.name + "\". Token : \"" + e.arguments[4] + "\".", __FILE__, __LINE__);

		// Create and push (note the 0 base mipmap) :
		formatList.insert( std::pair<std::string, HdlTextureFormat>( e.name, HdlTextureFormat(w, h, mode, depth, minFilter, magFilter, sWrap, tWrap, 0, mipmap) ) );
	}

	void LayoutLoader::buildRequiredPipeline(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 1, 1, -1, "RequiredPipeline");

		// Identify the target :
		std::map<std::string,PipelineLayout>::iterator it = requiredPipelineList.find(e.arguments[0]);

		if(it==requiredPipelineList.end())
			throw Exception("From line " + to_string(e.startLine) + " : The required pipeline \"" + e.arguments[0] + "\" was not found.", __FILE__, __LINE__);

		std::map<std::string,PipelineLayout>::iterator it2 = pipelineList.find(e.name);

		if(it2!=pipelineList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A PipelineLayout Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);
		else
			pipelineList.insert( std::pair<std::string, PipelineLayout>(e.name, it->second) );
	}

	void LayoutLoader::buildSharedCode(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 0, 0, 1, "SharedCode");

		if(sharedCodeList.find(e.name)!=sharedCodeList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A SharedCode Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);

		sharedCodeList.insert( std::pair<std::string, std::string>(e.name, e.body) );
	}

	void LayoutLoader::buildFormat(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 4, 9, -1, "Format");

		// Get the data :
		int w, h, mipmap = 0;
		GLenum mode, depth, minFilter, magFilter, sWrap, tWrap;

		if(!from_string(e.arguments[0], w))
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read width for format \"" + e.name + "\". Token : \"" + e.arguments[0] + "\".", __FILE__, __LINE__);

		if(!from_string(e.arguments[1], h))
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read height for format \"" + e.name + "\". Token : \"" + e.arguments[0] + "\".", __FILE__, __LINE__);

		mode  = glFromString(e.arguments[2]);
		depth = glFromString(e.arguments[3]);

		if(e.arguments.size()>4)
			minFilter = glFromString(e.arguments[4]);
		else
			minFilter = GL_NEAREST;

		if(e.arguments.size()>5)
			magFilter = glFromString(e.arguments[5]);
		else
			magFilter = GL_NEAREST;

		if(e.arguments.size()>6)
			sWrap = glFromString(e.arguments[6]);
		else
			sWrap = GL_CLAMP;

		if(e.arguments.size()>7)
			tWrap = glFromString(e.arguments[7]);
		else
			tWrap = GL_CLAMP;

		if(e.arguments.size()>8)
		{
			if(!from_string(e.arguments[8], mipmap))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read mipmap for format \"" + e.name + "\". Token : \"" + e.arguments[8] + "\".", __FILE__, __LINE__);
		}
		else
			mipmap = 0;

		// Find possible errors :
		if(mode==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read mode for format \"" + e.name + "\". Token : \"" + e.arguments[2] + "\".", __FILE__, __LINE__);
		if(depth==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read depth for format \"" + e.name + "\". Token : \"" + e.arguments[3] + "\".", __FILE__, __LINE__);
		if(minFilter==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read MinFilter for format \"" + e.name + "\". Token : \"" + e.arguments[4] + "\".", __FILE__, __LINE__);
		if(magFilter==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read MagFilter for format \"" + e.name + "\". Token : \"" + e.arguments[5] + "\".", __FILE__, __LINE__);
		if(sWrap==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read SWrapping for format \"" + e.name + "\". Token : \"" + e.arguments[6] + "\".", __FILE__, __LINE__);
		if(sWrap==GL_FALSE)
			throw Exception("From line " + to_string(e.startLine) + " : Cannot read TWrapping for format \"" + e.name + "\". Token : \"" + e.arguments[7] + "\".", __FILE__, __LINE__);
		if(mipmap<0)
			throw Exception("From line " + to_string(e.startLine) + " : Mipmap cannot be negative for format \"" + e.name + "\". Token : \"" + e.arguments[4] + "\".", __FILE__, __LINE__);

		if(formatList.find(e.name)!=formatList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A Format Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);

		// Create and push (note the 0 base mipmap) :
		formatList.insert( std::pair<std::string, HdlTextureFormat>( e.name, HdlTextureFormat(w, h, mode, depth, minFilter, magFilter, sWrap, tWrap, 0, mipmap) ) );
	}

	void LayoutLoader::buildShaderSource(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 0, 1, 0, "ShaderSource");

		// Complementary tests :
		if(e.arguments.size()>0 && !e.noBody)
			throw Exception("From line " + to_string(e.startLine) + " : ShaderSource \"" + e.name + "\" cannot have both argument(s) and a body.", __FILE__, __LINE__);
		if(e.arguments.empty() && e.noBody)
			throw Exception("From line " + to_string(e.startLine) + " : ShaderSource \"" + e.name + "\" must have either an argument (filename) or a body (and not both).", __FILE__, __LINE__);

		if(formatList.find(e.name)!=formatList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A ShaderSource Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);

		// Load data :
		if(e.noBody)
		{
			std::string usedPath;

			try
			{
				std::string content;

				// Custom load :
				loadFile(e.arguments[0], content, usedPath);

				enhanceShaderSource(content);

				sourceList.insert( std::pair<std::string, ShaderSource>( e.name, ShaderSource(content, "ShaderSource:" + e.name, e.startLine) ) );
			}
			catch(Exception& ex)
			{
				Exception m("From line " + to_string(e.startLine) + " : An exception was caught while building ShaderSource \"" + e.name + "\" (loading from file \"" + e.arguments[0] + "\", (possibly incomplete) path : \"" + usedPath + "\") : ", __FILE__, __LINE__);
				throw m + ex;
			}
		}
		else
		{
			try
			{
				std::string content = e.body;

				enhanceShaderSource(content);

				sourceList.insert( std::pair<std::string, ShaderSource>( e.name, ShaderSource(content, "ShaderSource:" + e.name, e.startLine) ) );
			}
			catch(Exception& ex)
			{
				Exception m("From line " + to_string(e.startLine) + " : An exception was caught while building ShaderSource \"" + e.name + "\" : ", __FILE__, __LINE__);
				throw m + ex;
			}
		}
	}

	void LayoutLoader::buildGeometry(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 1, 4, 0, "Geometry");

		// Test for duplicata :
		if(geometryList.find(e.name)!=geometryList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A Geometry Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);

		// Find the first argument :
		if(e.arguments[0]==keywordsLayoutLoader[KW_LL_STANDARD_QUAD])
		{
			geometryList.insert( std::pair<std::string, GeometryModel>( e.name, GeometryPrimitives::StandardQuad() ) );
		}
		else if(e.arguments[0]==keywordsLayoutLoader[KW_LL_GRID_2D])
		{
			if(e.arguments.size()!=3)
				throw Exception("From line " + to_string(e.startLine) + " : The model \"" + keywordsLayoutLoader[KW_LL_GRID_2D] + "\" requires to have exactly 3 arguments (included) in geometry \"" + e.name + "\".", __FILE__, __LINE__);

			int w, h;

			if(!from_string(e.arguments[1], w))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read width for 2D grid geometry \"" + e.name + "\". Token : \"" + e.arguments[1] + "\".", __FILE__, __LINE__);

			if(!from_string(e.arguments[2], h))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read height for 2D grid geometry \"" + e.name + "\". Token : \"" + e.arguments[2] + "\".", __FILE__, __LINE__);

			geometryList.insert( std::pair<std::string, GeometryModel>( e.name, GeometryPrimitives::PointsGrid2D(w,h) ) );
		}
		else if(e.arguments[0]==keywordsLayoutLoader[KW_LL_GRID_3D])
		{
			if(e.arguments.size()!=4)
				throw Exception("From line " + to_string(e.startLine) + " : The model \"" + keywordsLayoutLoader[KW_LL_GRID_3D] + "\" requires to have exactly 4 arguments (included) in geometry \"" + e.name + "\".", __FILE__, __LINE__);

			int w, h, z;

			if(!from_string(e.arguments[1], w))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read width for 3D grid geometry \"" + e.name + "\". Token : \"" + e.arguments[1] + "\".", __FILE__, __LINE__);

			if(!from_string(e.arguments[2], h))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read height for 3D grid geometry \"" + e.name + "\". Token : \"" + e.arguments[2] + "\".", __FILE__, __LINE__);

			if(!from_string(e.arguments[3], z))
				throw Exception("From line " + to_string(e.startLine) + " : Cannot read height for 3D grid geometry \"" + e.name + "\". Token : \"" + e.arguments[3] + "\".", __FILE__, __LINE__);

			geometryList.insert( std::pair<std::string, GeometryModel>( e.name, GeometryPrimitives::PointsGrid3D(w,h,z)) );
		}
		else if(e.arguments[0]==keywordsLayoutLoader[KW_LL_CUSTOM_MODEL])
		{
			if(e.arguments.size()!=2 && e.arguments.size()!=3)
				throw Exception("From line " + to_string(e.startLine) + " : The model \"" + keywordsLayoutLoader[KW_LL_CUSTOM_MODEL] + "\" requires to have either 2 or 3 arguments (included) in geometry \"" + e.name + "\".", __FILE__, __LINE__);

			if(e.body.empty() || e.noBody)
				throw Exception("From line " + to_string(e.startLine) + " : The custom model \"" + e.name + "\" does not have a body.", __FILE__, __LINE__);

			try
			{
				GLenum primitive;

				primitive = glFromString(e.arguments[1]);

				bool hasTexCoord = false;

				if(e.arguments.size()>2)
				{
					if(e.arguments[2]=="true" || e.arguments[2]=="True" || e.arguments[2]=="TRUE")
						hasTexCoord = true;
				}

				GeometryPrimitives::CustomModel* g = NULL;

				// Parse the text : 
				VanillaParser parser(e.body, e.bodyLine);

				// Classify the new data :
				std::vector<LayoutLoaderKeyword> associatedKeywords;

				classify(parser.elements, associatedKeywords);

				// First pass is only for vertices : 
				GLfloat 	x = 0.0f, 
						y = 0.0f, 
						z = 0.0f, 
						u = 0.0f, 
						v = 0.0f;
				int 		pArg = 0;				
				for(int k=0; k<associatedKeywords.size(); k++)
				{
					// Test :
					if(associatedKeywords[k]==KW_LL_VERTEX)
					{
						if(g==NULL)
						{
							preliminaryTests(parser.elements[k], -1, 2, 5, -1, "Vertex (sub-parsing Geometry)");

							int dim = parser.elements[k].arguments.size();
							dim = (hasTexCoord ? dim-2 : dim);
							g = new GeometryPrimitives::CustomModel(dim, primitive, hasTexCoord);
						}
						else
						{
							const int n = g->hasTexCoord ? (g->dim + 2) : g->dim;
							preliminaryTests(parser.elements[k], -1, n, n, -1, "Vertex (sub-parsing Geometry)");
						}
	
						pArg = 0;

						if(!from_string(parser.elements[k].arguments[pArg], x))
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Cannot read X for vertex. Token : \"" + parser.elements[k].arguments[pArg] + "\".", __FILE__, __LINE__);

						pArg++;

						if(!from_string(parser.elements[k].arguments[pArg], y))
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Cannot read Y for vertex. Token : \"" + parser.elements[k].arguments[pArg] + "\".", __FILE__, __LINE__);

						pArg++;

						if(g->dim>=3)
						{
							if(!from_string(parser.elements[k].arguments[pArg], z))
								throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Cannot read Z for vertex. Token : \"" + parser.elements[k].arguments[pArg] + "\".", __FILE__, __LINE__);
							pArg++;
						}

						if(g->hasTexCoord)
						{
							if(!from_string(parser.elements[k].arguments[pArg], u))
								throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Cannot read U for vertex. Token : \"" + parser.elements[k].arguments[pArg] + "\".", __FILE__, __LINE__);
							
							pArg++;

							if(!from_string(parser.elements[k].arguments[pArg], v))
								throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Cannot read V for vertex. Token : \"" + parser.elements[k].arguments[pArg] + "\".", __FILE__, __LINE__);
							
							pArg++;
						}

						
						if(g->dim==2)
							g->newVertex2D(x, y, u, v);
						else if(g->dim==3)
							g->newVertex3D(x, y, z, u, v);
						else
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Internal error - unsupported number of dimensions.", __FILE__, __LINE__);
					}
					else if(associatedKeywords[k]!=KW_LL_ELEMENT)
					{
							if( associatedKeywords[k]<LL_NumKeywords )
								throw Exception("From line " + to_string(parser.elements[k].startLine) + " : The keyword " + keywordsLayoutLoader[associatedKeywords[k]] + " is not allowed in a Geometry definition (\"" + e.name + "\").", __FILE__, __LINE__);
							else
								throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Unknown keyword \"" + parser.elements[k].strKeyword + "\" in a Geometry definition (\"" + e.name + "\").", __FILE__, __LINE__);
					}
				}

				if(g==NULL)
					throw Exception("From line " + to_string(e.startLine) + " : The custom model \"" + e.name + "\" does not have any vertex declared.", __FILE__, __LINE__);

				// Second pass for elements : 
				std::vector<GLuint> indices(g->numVerticesPerEl);
				for(int k=0; k<associatedKeywords.size(); k++)
				{
					if(associatedKeywords[k]==KW_LL_ELEMENT)
					{
						preliminaryTests(parser.elements[k], -1, g->numVerticesPerEl, g->numVerticesPerEl, -1, "Element (sub-parsing Geometry)");

						// Load all the indices : 
						for(int pArg=0; pArg<g->numVerticesPerEl; pArg++)
						{
							if(!from_string(parser.elements[k].arguments[pArg], indices[pArg]))
								throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Cannot read vertex index " + to_string(pArg) + " for primitive element. Token : \"" + parser.elements[k].arguments[pArg] + "\".", __FILE__, __LINE__);
						}

						// Add it : 
						g->newElement(indices);
					}
				}

				geometryList.insert( std::pair<std::string, GeometryModel>( e.name, *g) );

				delete g;
			}
			catch(Exception& ex)
			{
				Exception m("From line " + to_string(e.startLine) + " : Exception caught while building Geometry \"" + e.name + "\".", __FILE__, __LINE__);
				throw m + ex;
			}
		}
		else
			throw Exception("From line " + to_string(e.startLine) + " : Unknown geometry argument \"" + e.arguments[0] + "\" (or not supported in current version) in Geometry \"" + e.name + "\".", __FILE__, __LINE__);
	}

	void LayoutLoader::buildFilter(const VanillaParserSpace::Element& e)
	{
		// Preliminary tests :
		preliminaryTests(e, 1, 2, 6, -1, "FilterLayout");

		// Find the format :
		std::map<std::string,HdlTextureFormat>::iterator 	format		 = formatList.find(e.arguments[0]);
		std::map<std::string,ShaderSource>::iterator 		fragmentSource	 = sourceList.find(e.arguments[1]),
							 		vertexSource;
		std::map<std::string, GeometryModel>::iterator		geometry;
		ShaderSource						*vertexSourcePtr = NULL;
		GeometryModel						*geometryPtr	 = NULL;

		if(format==formatList.end())
			throw Exception("From line " + to_string(e.startLine) + " : No Format with name \"" + e.arguments[0] + "\" was registered and can be use in Filter \"" + e.name + "\".", __FILE__, __LINE__);
		if(fragmentSource==sourceList.end())
			throw Exception("From line " + to_string(e.startLine) + " : No ShaderSource with name \"" + e.arguments[1] + "\" was registered and can be use in Filter \"" + e.name + "\".", __FILE__, __LINE__);
		if(filterList.find(e.name)!=filterList.end())
			throw Exception("From line " + to_string(e.startLine) + " : A FilterLayout Object with the name \"" + e.name + "\" was already registered.", __FILE__, __LINE__);

		if(e.arguments.size()>2)
		{
			if(e.arguments[2]!=keywordsLayoutLoader[KW_LL_DEFAULT_VERTEX_SHADER])
			{
				vertexSource = sourceList.find(e.arguments[2]);

				if(vertexSource==sourceList.end())
					throw Exception("From line " + to_string(e.startLine) + " : No ShaderSource with name \"" + e.arguments[2] + "\" was registered and can be use in Filter \"" + e.name + "\".", __FILE__, __LINE__);

				vertexSourcePtr = &vertexSource->second;
			}
		}

		if(e.arguments.size()>5)
		{
			if(e.arguments[5]!=keywordsLayoutLoader[KW_LL_STANDARD_QUAD])
			{
				geometry = geometryList.find(e.arguments[5]);

				if(geometry==geometryList.end())
					throw Exception("From line " + to_string(e.startLine) + " : No Geometry with name \"" + e.arguments[5] + "\" was registered and can be use in Filter \"" + e.name + "\".", __FILE__, __LINE__);

				geometryPtr = &geometry->second;
			}
		}

		filterList.insert( std::pair<std::string, FilterLayout>( e.name, FilterLayout(e.name, format->second, fragmentSource->second, vertexSourcePtr, geometryPtr) ) );

		std::map<std::string,FilterLayout>::iterator filterLayout = filterList.find(e.name);

		if(e.arguments.size()>3)
		{
			if(e.arguments[3]==keywordsLayoutLoader[KW_LL_CLEARING_ON])
				filterLayout->second.enableClearing();
			else if(e.arguments[3]==keywordsLayoutLoader[KW_LL_CLEARING_OFF])
				filterLayout->second.disableClearing();
			else
				throw Exception("From line " + to_string(e.startLine) + " : Unable to read clearing parameter (should be either \"" +  keywordsLayoutLoader[KW_LL_CLEARING_ON] + "\" or \"" + keywordsLayoutLoader[KW_LL_CLEARING_OFF] + "\"). Token : \"" + e.arguments[3] + "\".", __FILE__, __LINE__);
		}

		if(e.arguments.size()>4)
		{
			if(e.arguments[4]==keywordsLayoutLoader[KW_LL_BLENDING_ON])
				filterLayout->second.enableBlending();
			else if(e.arguments[4]==keywordsLayoutLoader[KW_LL_BLENDING_OFF])
				filterLayout->second.disableBlending();
			else
				throw Exception("From line " + to_string(e.startLine) + " : Unable to read clearing parameter (should be either \"" +  keywordsLayoutLoader[KW_LL_BLENDING_ON] + "\" or \"" + keywordsLayoutLoader[KW_LL_BLENDING_OFF] + "\"). Token : \"" + e.arguments[4] + "\".", __FILE__, __LINE__);
		}
	}

	void LayoutLoader::buildPipeline(const VanillaParserSpace::Element& e)
	{
		try
		{
			// Preliminary tests :
			preliminaryTests(e, 1, 0, 0, 1, "PipelineLayout");

			// Load the content of the body :
			VanillaParser parser(e.body, e.bodyLine);

			// Classify the new data :
			std::vector<LayoutLoaderKeyword> associatedKeywords;

			classify(parser.elements, associatedKeywords);

			// Test for possible external elements :
			int 	inputPorts = -1,
				outputPorts = -1,
				components = 0;
			for(int k=0; k<associatedKeywords.size(); k++)
			{
				switch(associatedKeywords[k])
				{
					case KW_LL_INPUT_PORTS :
						if(inputPorts>=0)
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : The InputPorts have already been declared for this PipelineLayout (\"" + e.name + "\").", __FILE__, __LINE__);
						else
							inputPorts = k;
						break;
					case KW_LL_OUTPUT_PORTS :
						if(outputPorts>=0)
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : The OutputPorts have already been declared for this PipelineLayout (\"" + e.name + "\").", __FILE__, __LINE__);
						else
							outputPorts = k;
						break;
					case KW_LL_FILTER_INSTANCE :
					case KW_LL_PIPELINE_INSTANCE :
						components++;
						break;
					case KW_LL_CONNECTION :
						break; //OK
					default :
						if( associatedKeywords[k]<LL_NumKeywords )
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : The keyword " + keywordsLayoutLoader[associatedKeywords[k]] + " is not allowed in a PipelineLayout definition (\"" + e.name + "\").", __FILE__, __LINE__);
						else
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Unknown keyword \"" + parser.elements[k].strKeyword + "\" in a PipelineLayout definition (\"" + e.name + "\").", __FILE__, __LINE__);
						break;
				}
			}

			if(inputPorts<0)
				throw Exception("From line " + to_string(e.startLine) + " : The InputPorts are not declared for the PipelineLayout \"" + e.name + "\".", __FILE__, __LINE__);

			if(outputPorts<0)
				throw Exception("From line " + to_string(e.startLine) + " : The OutputPorts are not declared for the PipelineLayout \"" + e.name + "\".", __FILE__, __LINE__);

			if(components==0)
				throw Exception("From line " + to_string(e.startLine) + " : The PipelineLayout \"" + e.name + "\" does not contain any sub-component (such as Filters or sub-Pipelines).", __FILE__, __LINE__);

			// Create the object :
			PipelineLayout layout(e.name);

			// Add the inputs :
			preliminaryTests(parser.elements[inputPorts], -1, 1, 256, -1, "InputPorts");

			for(int k=0; k<parser.elements[inputPorts].arguments.size(); k++)
				layout.addInput(parser.elements[inputPorts].arguments[k]);

			// Add the outputs :
			preliminaryTests(parser.elements[outputPorts], -1, 1, 256, -1, "OutputPorts");

			for(int k=0; k<parser.elements[outputPorts].arguments.size(); k++)
				layout.addOutput(parser.elements[outputPorts].arguments[k]);

			// Parse and add the Objects for the PipelineLayout :
			std::map<std::string,FilterLayout>::iterator	filter;
			std::map<std::string,PipelineLayout>::iterator	pipeline;

			for(int k=0; k<associatedKeywords.size(); k++)
			{
				switch(associatedKeywords[k])
				{
					case KW_LL_FILTER_INSTANCE :
						preliminaryTests(parser.elements[k], 1, 1, 1, -1, "FilterInstance");
						filter = filterList.find(parser.elements[k].arguments[0]);

						if(filter==filterList.end())
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : No FilterLayout with name \"" + parser.elements[k].arguments[0] + "\" was registered and can be use in PipelineLayout \"" + e.name + "\".", __FILE__, __LINE__);
						else
							layout.add(filter->second, parser.elements[k].name);
						break;

					case KW_LL_PIPELINE_INSTANCE :
						preliminaryTests(parser.elements[k], 1, 1, 1, -1, "PipelineInstance");
						pipeline = pipelineList.find(parser.elements[k].arguments[0]);

						if(pipeline==pipelineList.end())
							throw Exception("From line " + to_string(parser.elements[k].startLine) + " : No PipelineLayout with name \"" + parser.elements[k].arguments[0] + "\" was registered and can be use in PipelineLayout \"" + e.name + "\".", __FILE__, __LINE__);
						else
							layout.add(pipeline->second, parser.elements[k].name);
						break;

					default :
						break;
				}
			}

			// Install the connections :
			bool makeAutoConnect = true;
			for(int k=0; k<associatedKeywords.size(); k++)
			{
				if(associatedKeywords[k]==KW_LL_CONNECTION)
				{
					makeAutoConnect = false;

					preliminaryTests(parser.elements[k], -1, 4, 4, -1, "Connection");

					// Test the nature of the connection :
					if(parser.elements[k].arguments[0]==keywordsLayoutLoader[KW_LL_THIS_PIPELINE] && parser.elements[k].arguments[1]==keywordsLayoutLoader[KW_LL_THIS_PIPELINE])
						throw Exception("From line " + to_string(parser.elements[k].startLine) + " : Direct connections between input and output are not allowed.", __FILE__, __LINE__);
					else if(parser.elements[k].arguments[0]==keywordsLayoutLoader[KW_LL_THIS_PIPELINE])
						layout.connectToInput(parser.elements[k].arguments[1], parser.elements[k].arguments[2], parser.elements[k].arguments[3]);
					else if(parser.elements[k].arguments[2]==keywordsLayoutLoader[KW_LL_THIS_PIPELINE])
						layout.connectToOutput(parser.elements[k].arguments[0], parser.elements[k].arguments[1], parser.elements[k].arguments[3]);
					else
						layout.connect(parser.elements[k].arguments[0], parser.elements[k].arguments[1], parser.elements[k].arguments[2], parser.elements[k].arguments[3]);
				}
			}

			// If no connection was installed, try to autoConnect :
			if(makeAutoConnect)
				layout.autoConnect();

			// Save :
			pipelineList.insert( std::pair<std::string, PipelineLayout>(e.name, layout) );
		}
		catch(Exception& ex)
		{
			Exception m("From line " + to_string(e.startLine) + " : Exception caught while building PipelineLayout \"" + e.name + "\".", __FILE__, __LINE__);
			throw m + ex;
		}
	}

	void LayoutLoader::process(const std::string& code, std::string& mainPipelineName)
	{
		try
		{
			// Parse :
			VanillaParser rootParser(code);

			// Class the elements :
			classify(rootParser.elements, associatedKeyword);

			// Process
			for(int k=0; k<associatedKeyword.size(); k++)
			{
				switch(associatedKeyword[k])
				{
					case KW_LL_ADD_PATH :
						appendPath(rootParser.elements[k]);
						break;
					case KW_LL_INCLUDE_FILE :
						includeFile(rootParser.elements[k]);
						break;
					case KW_LL_SHARED_SOURCE :
						buildSharedCode(rootParser.elements[k]);
						break;
					case KW_LL_REQUIRED_FORMAT :
						buildRequiredFormat(rootParser.elements[k]);
						break;
					case KW_LL_REQUIRED_PIPELINE :
						buildRequiredPipeline(rootParser.elements[k]);
						break;
					case KW_LL_FORMAT_LAYOUT :
						buildFormat(rootParser.elements[k]);
						break;
					case KW_LL_SHADER_SOURCE :
						buildShaderSource(rootParser.elements[k]);
						break;
					case KW_LL_GEOMETRY :
						buildGeometry(rootParser.elements[k]);
						break;
					case KW_LL_FILTER_LAYOUT :
						buildFilter(rootParser.elements[k]);
						break;
					case KW_LL_PIPELINE_MAIN :
						if(!isSubLoader)
							mainPipelineName = rootParser.elements[k].name;
					case KW_LL_PIPELINE_LAYOUT :
						buildPipeline(rootParser.elements[k]);
						break;
					default :
						if(associatedKeyword[k]<LL_NumKeywords)
							throw Exception("From line " + to_string(rootParser.elements[k].startLine) + " : The keyword " + keywordsLayoutLoader[associatedKeyword[k]] + " is not allowed in a PipelineFile.", __FILE__, __LINE__);
						else
							throw Exception("From line " + to_string(rootParser.elements[k].startLine) + " : Unknown keyword : \"" + rootParser.elements[k].strKeyword + "\".", __FILE__, __LINE__);
						break;
				}
			}

			// Check Errors :
			if(mainPipelineName.empty() && !isSubLoader)
				throw Exception("No main pipeline (\"" + std::string(keywordsLayoutLoader[KW_LL_PIPELINE_MAIN]) + "\") was defined in this code.", __FILE__, __LINE__);
		}
		catch(Exception& ex)
		{
			Exception m("Exception caught while processing pipeline file : ", __FILE__, __LINE__);
			throw m + ex;
		}
	}

	/**
	\fn const std::vector<std::string>& LayoutLoader::paths(void) const
	\brief Access the list of paths.
	\return A read-only access to the list of paths.
	**/
	const std::vector<std::string>& LayoutLoader::paths(void) const
	{
		return staticPaths;
	}

	/**
	\fn void LayoutLoader::clearPaths(void)
	\brief Clear the list of paths.
	**/
	void LayoutLoader::clearPaths(void)
	{
		staticPaths.clear();
		//staticPaths.push_back("./");
	}

	/**
	\fn void LayoutLoader::addToPaths(const std::string& p)
	\brief Add a path.
	\param p The path (might be invalid, avoid input of non-canonical paths which might results in ambiguous links errors).
	**/
	void LayoutLoader::addToPaths(const std::string& p)
	{
		if(!p.empty() && p!="./")
		{
			removeFromPaths(p);
			staticPaths.push_back(p);
		}
	}

	/**
	\fn void LayoutLoader::addToPaths(const std::vector<std::string>& paths)
	\brief Add several paths.
	\param paths The list of paths to add (might be invalid, avoid input of non-canonical paths which might results in ambiguous links errors).
	**/
	void LayoutLoader::addToPaths(const std::vector<std::string>& paths)
	{
		for(std::vector<std::string>::const_iterator it = paths.begin(); it!=paths.end(); it++)
			addToPaths(*it);
	}

	/**
	\fn bool LayoutLoader::removeFromPaths(const std::string& p)
	\brief Remove an entity from the paths.
	\param p The path to remove.
	\return True if a path was remove; false otherwise.
	**/
	bool LayoutLoader::removeFromPaths(const std::string& p)
	{
		std::vector<std::string>::iterator it = std::find(staticPaths.begin(), staticPaths.end(), p);

		if(it!=staticPaths.end())
		{
			staticPaths.erase(it);
			return true;
		}
		else
			return false;
	}

	/**
	\fn __ReadOnly_PipelineLayout LayoutLoader::operator()(const std::string& source)
	\brief Loads a pipeline layout from a file (see the script language description for more information).
	\param source The source to load. It is considered as a filename if it doesn't contain '\\n'.
	\return The newly loaded layout or raise an exception if any errors occur.
	**/
	__ReadOnly_PipelineLayout LayoutLoader::operator()(const std::string& source)
	{
		clean();

		if(source.empty())
			throw Exception("LayoutLoader::operator() - The source is empty.", __FILE__, __LINE__);

		bool isAFile = false;
		std::string content;

		// Copy the path :
		dynamicPaths = staticPaths;

		// Is it a filename or the content :
		if(source.find('\n')==std::string::npos)
		{
			// Split :
			size_t section = source.find_last_of("/");
			std::string filename = source.substr(section+1);
			currentPath = source.substr(0, section+1);

			// Add to the path :
			if(!currentPath.empty())
				dynamicPaths.push_back(currentPath);

			loadFile(filename, content, currentPath);
			isAFile = true;
		}
		else
			content = source;

		try
		{
			std::string mainPipelineName;
			process(content, mainPipelineName);

			// Get the mainPipeline :
			std::map<std::string,PipelineLayout>::iterator it = pipelineList.find(mainPipelineName);

			return __ReadOnly_PipelineLayout(it->second);
		}
		catch(Exception& e)
		{
			if(isAFile && currentPath.empty())
			{
				Exception m("Exception caught while processing file \"" + source + "\" : ", __FILE__, __LINE__);
				throw m+e;
			}
			else if(isAFile)
			{
				Exception m("Exception caught while processing file \"" + source + "\" (path : \"" + currentPath + "\") : ", __FILE__, __LINE__);
				throw m+e;
			}
			else
			{
				Exception m("Exception caught while processing string : ", __FILE__, __LINE__);
				throw m+e;
			}
		}

		clean();
	}

	/**
	\fn Pipeline* LayoutLoader::operator()(const std::string& source, std::string pipelineName)
	\brief Loads a pipeline from a file (see the script language description for more information).
	\param source The source to load. It is considered as a filename if it doesn't contain '\\n'.
	\param pipelineName The name of the unique instance created (or take the type name if left empty).
	\return A pointer to the unique instance built on the newly loaded layout or raise an exception if any errors occur. You have the charge to delete the newly created object.
	**/
	Pipeline* LayoutLoader::operator()(const std::string& source, std::string pipelineName)
	{
		__ReadOnly_PipelineLayout layout = (*this)(source);

		if(pipelineName.empty())
			pipelineName = layout.getTypeName();

		Pipeline* pipeline = new Pipeline(layout, pipelineName);

		return pipeline;
	}

	/**
	\fn void LayoutLoader::addRequiredElement(const std::string& name, const __ReadOnly_HdlTextureFormat& fmt)
	\brief Add a __ReadOnly_HdlTextureFormat to do the possibly required elements, along with its name. Will raise an exception if an element with the same name already exists. All the following pipelines loaded and containing a call REQUIRED_FORMAT:name(); will use this format.
	\param name The name of the element.
	\param fmt The element to be associated.
	**/
	void LayoutLoader::addRequiredElement(const std::string& name, const __ReadOnly_HdlTextureFormat& fmt)
	{
		std::map<std::string,HdlTextureFormat>::iterator it = requiredFormatList.find(name);

		if(it!=requiredFormatList.end())
			throw Exception("LayoutLoader::addRequiredElement - An element with the name " + name + " already exists in the HdlTexture formats database.", __FILE__, __LINE__);
		else
			requiredFormatList.insert( std::pair<std::string, HdlTextureFormat>(name, fmt) );

	}

	/**
	\fn void LayoutLoader::addRequiredElement(const std::string& name, __ReadOnly_PipelineLayout& layout)
	\brief Add a __ReadOnly_PipelineLayout to do the possibly required elements, along with its name. Will raise an exception if an element with the same name already exists. All the following pipelines loaded and containing a call REQUIRED_PIPELINE:name(); will use this pipeline layout.
	\param name The name of the element.
	\param layout The element to be associated.
	**/
	void LayoutLoader::addRequiredElement(const std::string& name, __ReadOnly_PipelineLayout& layout)
	{
		std::map<std::string,PipelineLayout>::iterator it = requiredPipelineList.find(name);

		if(it!=requiredPipelineList.end())
			throw Exception("LayoutLoader::addRequiredElement - An element with the name " + name + " already exists in the pipeline layouts database.", __FILE__, __LINE__);
		else
			requiredPipelineList.insert( std::pair<std::string, PipelineLayout>(name, layout) );
	}

	/**
	\fn int LayoutLoader::clearRequiredElements(void)
	\brief Remove all the elements.
	\return The number of elements removed.
	**/
	int LayoutLoader::clearRequiredElements(void)
	{
		int numElemErased = 0;

		numElemErased += requiredFormatList.size();
		numElemErased += requiredPipelineList.size();

		requiredFormatList.clear();
		requiredPipelineList.clear();

		return numElemErased;
	}

	/**
	\fn int LayoutLoader::clearRequiredElements(const std::string& name)
	\brief Remove the elements having the given name.
	\param name The name of the targeted element.
	\return The number of elements removed.
	**/
	int LayoutLoader::clearRequiredElements(const std::string& name)
	{
		std::map<std::string,HdlTextureFormat>::iterator it1;
		std::map<std::string,PipelineLayout>::iterator it2;
		int numElemErased = 0;

		it1 = requiredFormatList.find(name);
		it2 = requiredPipelineList.find(name);

		if(it1!=requiredFormatList.end())
		{
			requiredFormatList.erase(it1);
			numElemErased++;
		}

		if(it2!=requiredPipelineList.end())
		{
			requiredPipelineList.erase(it2);
			numElemErased++;
		}
	
		return numElemErased;
	}

// LayoutWriter 
	/**
	\fn LayoutWriter::LayoutWriter(void)
	\brief LayoutWriter constructor.
	**/
	LayoutWriter::LayoutWriter(void)
	{ }

	LayoutWriter::~LayoutWriter(void)
	{
		code.clear();
	}

	VanillaParserSpace::Element LayoutWriter::write(const __ReadOnly_HdlTextureFormat& hLayout, const std::string& name)
	{
		if(name.empty())
			throw Exception("LayoutWriter - Writing " + std::string(keywordsLayoutLoader[ KW_LL_FORMAT_LAYOUT ]) + " : name cannot be empty.", __FILE__, __LINE__);
		if(hLayout.getBaseLevel()!=0)
			throw Exception("LayoutWriter - Writing " + std::string(keywordsLayoutLoader[ KW_LL_FORMAT_LAYOUT ]) + " : base level cannot be different than 0 (current : " + to_string(hLayout.getBaseLevel()) + ") .", __FILE__, __LINE__);

		VanillaParserSpace::Element e;

		e.strKeyword	= keywordsLayoutLoader[ KW_LL_FORMAT_LAYOUT ];
		e.name		= name;
		e.body.clear();
		e.noBody 	= true;

		e.arguments.push_back( to_string( hLayout.getWidth() ) );
		e.arguments.push_back( to_string( hLayout.getHeight() ) );
		e.arguments.push_back( glParamName( hLayout.getGLMode() ) );
		e.arguments.push_back( glParamName( hLayout.getGLDepth() ) );
		e.arguments.push_back( glParamName( hLayout.getMinFilter() ) );
		e.arguments.push_back( glParamName( hLayout.getMagFilter() ) );
		e.arguments.push_back( glParamName( hLayout.getSWrapping() ) );
		e.arguments.push_back( glParamName( hLayout.getTWrapping() ) );
		e.arguments.push_back( to_string( hLayout.getMaxLevel() ) );

		return e;
	}
 
	VanillaParserSpace::Element LayoutWriter::write(const ShaderSource& source, const std::string& name)
	{
		if(name.empty())
			throw Exception("LayoutWriter - Writing " + std::string(keywordsLayoutLoader[ KW_LL_SHADER_SOURCE ]) + " : name cannot be empty.", __FILE__, __LINE__);

		VanillaParserSpace::Element e;

		e.strKeyword	= keywordsLayoutLoader[ KW_LL_SHADER_SOURCE ];
		e.name		= name;
		e.body		= source.getSource();
		e.arguments.clear();
		e.noArgument	= true;

		return e;
	}

	VanillaParserSpace::Element LayoutWriter::write(const GeometryModel& mdl, const std::string& name)
	{
		if(name.empty())
			throw Exception("LayoutWriter - Writing " + std::string(keywordsLayoutLoader[ KW_LL_GEOMETRY ]) + " : name cannot be empty.", __FILE__, __LINE__);

		VanillaParserSpace::Element e;

		e.strKeyword	= keywordsLayoutLoader[ KW_LL_GEOMETRY ];
		e.name		= name;

		if(mdl.type==GeometryModel::StandardQuad)	
		{
			e.arguments.push_back( keywordsLayoutLoader[ KW_LL_STANDARD_QUAD ] );
			e.noBody	= true;
		}
		else if(mdl.type==GeometryModel::PointsGrid2D)
		{
			e.arguments.push_back( keywordsLayoutLoader[ KW_LL_GRID_2D ] );

			// Find grid size :
			GLfloat x = 0.0, y = 0.0; 
			for(int k=0; k<mdl.getNumVertices(); k++)
			{
				x = std::max(x, mdl.x(k));
				y = std::max(y, mdl.y(k));
			}

			e.arguments.push_back( to_string(x+1) );
			e.arguments.push_back( to_string(y+1) );
		}	
		else if(mdl.type==GeometryModel::PointsGrid3D)
		{
			e.arguments.push_back( keywordsLayoutLoader[ KW_LL_GRID_3D ] );

			// Find grid size : 
			GLfloat x = 0.0, y = 0.0, z = 0.0; 
			for(int k=0; k<mdl.getNumVertices(); k++)
			{
				x = std::max(x, mdl.x(k));
				y = std::max(y, mdl.y(k));
				z = std::max(z, mdl.z(k));
			}

			e.arguments.push_back( to_string(x+1) );
			e.arguments.push_back( to_string(y+1) );
			e.arguments.push_back( to_string(z+1) );
		}
		else if(mdl.type==GeometryModel::CustomModel)
		{
			e.arguments.push_back( keywordsLayoutLoader[ KW_LL_CUSTOM_MODEL ] );
			e.arguments.push_back( glParamName(mdl.primitiveGL) );

			if( mdl.hasTexCoord )
				e.arguments.push_back( "true" );

			// Write model : 
			VanillaParserSpace::Element v;
			v.strKeyword	= keywordsLayoutLoader[ KW_LL_VERTEX ];
			v.noName 	= true;
			v.noBody 	= true;
			for(int k=0; k<mdl.getNumVertices(); k++)
			{
				v.arguments.clear();

				v.arguments.push_back( to_string(mdl.x(k)) );
				v.arguments.push_back( to_string(mdl.y(k)) );

				if(mdl.dim>2)
					v.arguments.push_back( to_string(mdl.z(k)) );
				else if(mdl.dim!=2 && mdl.dim!=3)
					throw Exception("LayoutWriter::write - Geometry : internal error, unsupported number of dimensions.", __FILE__, __LINE__);

				if(mdl.hasTexCoord)
				{
					v.arguments.push_back( to_string(mdl.u(k)) );
					v.arguments.push_back( to_string(mdl.v(k)) );
				}

				// Push : 
				e.body += v.getCode() + "\n";
			}

			VanillaParserSpace::Element p;
			p.strKeyword	= keywordsLayoutLoader[ KW_LL_ELEMENT ];
			p.noName 	= true;
			p.noBody 	= true;
			e.body += "\n";
			for(int k=0; k<mdl.getNumElements(); k++)
			{
				p.arguments.clear();

				if(mdl.numVerticesPerEl>=1) p.arguments.push_back( to_string(mdl.a(k)) );
				if(mdl.numVerticesPerEl>=2) p.arguments.push_back( to_string(mdl.b(k)) );
				if(mdl.numVerticesPerEl>=3) p.arguments.push_back( to_string(mdl.c(k)) );
				if(mdl.numVerticesPerEl>=4) p.arguments.push_back( to_string(mdl.d(k)) );

				// Push : 
				e.body += p.getCode() + "\n";
			}
		}

		return e;
	}

	VanillaParserSpace::Element LayoutWriter::write(const __ReadOnly_FilterLayout& fLayout)
	{
		//if(name.empty())
		//	throw Exception("LayoutWriter - Writing " + std::string(keywordsLayoutLoader[ KW_LL_FILTER_LAYOUT ]) + " : name cannot be empty.", __FILE__, __LINE__);

		const std::string 	fmtName 	= "Format_" + fLayout.getTypeName(),
					fragName	= "Fragment_" + fLayout.getTypeName(),
					vertName	= "Vertex_" + fLayout.getTypeName(),
					mdlName		= "Model_" + fLayout.getTypeName();

		VanillaParserSpace::Element e;

		e.strKeyword	= keywordsLayoutLoader[ KW_LL_FILTER_LAYOUT ];
		e.name		= fLayout.getTypeName();
		e.body.clear();
		e.noBody	= true;
		e.arguments.clear();
		e.arguments.push_back( fmtName );
		e.arguments.push_back( fragName );

		VanillaParserSpace::Element e1 = LayoutWriter::write(fLayout, fmtName);
		code += e1.getCode() + "\n\n";
		
		VanillaParserSpace::Element e2 = LayoutWriter::write(fLayout.getFragmentSource(), fragName);
		code += e2.getCode() + "\n\n";

		if(!fLayout.isStandardVertexSource())
		{
			VanillaParserSpace::Element e3 = LayoutWriter::write(fLayout.getVertexSource(), vertName);
			code += e3.getCode() + "\n\n";

			e.arguments.push_back( vertName );
		}
		else
			e.arguments.push_back( keywordsLayoutLoader[KW_LL_DEFAULT_VERTEX_SHADER] );

		if(fLayout.isClearingEnabled())
			e.arguments.push_back( keywordsLayoutLoader[KW_LL_CLEARING_ON] );
		else
			e.arguments.push_back( keywordsLayoutLoader[KW_LL_CLEARING_OFF] );

		if(fLayout.isBlendingEnabled())
			e.arguments.push_back( keywordsLayoutLoader[KW_LL_BLENDING_ON] );
		else
			e.arguments.push_back( keywordsLayoutLoader[KW_LL_BLENDING_OFF] );

		if(!fLayout.isStandardGeometryModel())
		{
			VanillaParserSpace::Element e4 = LayoutWriter::write(fLayout.getGeometryModel(), mdlName);
			code += e4.getCode() + "\n\n";

			e.arguments.push_back( mdlName );
		}
		else
			e.arguments.push_back( keywordsLayoutLoader[KW_LL_STANDARD_QUAD] );

		return e;
	}

	VanillaParserSpace::Element LayoutWriter::write(const __ReadOnly_PipelineLayout& pLayout, bool isMain)
	{
		std::string keyword;

		if(isMain)
			keyword = keywordsLayoutLoader[ KW_LL_PIPELINE_MAIN ];
		else
			keyword = keywordsLayoutLoader[ KW_LL_PIPELINE_LAYOUT ];

		VanillaParserSpace::Element e;

		e.strKeyword	= keyword;
		e.name		= pLayout.getTypeName();
		e.arguments.clear();
		e.noArgument	= true;
		e.body.clear();

		// Declare all ports : 
		VanillaParserSpace::Element inPorts;
		inPorts.strKeyword = keywordsLayoutLoader[ KW_LL_INPUT_PORTS ];
		inPorts.name.clear();
		inPorts.noName = true;
		inPorts.body.clear();
		inPorts.noBody = true;
		
		for(int k=0; k<pLayout.getNumInputPort(); k++)
			inPorts.arguments.push_back( pLayout.getInputPortName(k) );

		e.body += inPorts.getCode() + "\n";

		VanillaParserSpace::Element outPorts;
		outPorts.strKeyword = keywordsLayoutLoader[ KW_LL_OUTPUT_PORTS ];
		outPorts.name.clear();
		outPorts.noName = true;
		outPorts.body.clear();
		outPorts.noBody = true;

		for(int k=0; k<pLayout.getNumOutputPort(); k++)
			outPorts.arguments.push_back( pLayout.getOutputPortName(k) );
		
		e.body += outPorts.getCode() + "\n";

		e.body += "\n";

		// Declare all sub-elements :
		for(int k=0; k<pLayout.getNumElements(); k++)
		{
			VanillaParserSpace::Element c, d;

			// Element to push in the body :
			d.name = pLayout.getElementName(k);
			

			switch(pLayout.getElementKind(k))
			{
				case __ReadOnly_PipelineLayout::FILTER :
					c 		= write( pLayout.filterLayout(k) );
					d.strKeyword	= keywordsLayoutLoader[ KW_LL_FILTER_INSTANCE ];
					break;
				case __ReadOnly_PipelineLayout::PIPELINE :
					c = write( pLayout.pipelineLayout(k) );
					d.strKeyword	= keywordsLayoutLoader[ KW_LL_PIPELINE_INSTANCE ];
					break;
				default :
					throw Exception("LayoutWriter::write - Internal error : unknown element type.", __FILE__, __LINE__);
			}

			// Element to push outside the body :
			code += c.getCode() + "\n\n";

			// Element to push inside the body :
			d.arguments.push_back( c.name );
			e.body += d.getCode() + "\n";
		}

		e.body += "\n";

		// Add Connections :  
		VanillaParserSpace::Element c;
		c.strKeyword	= keywordsLayoutLoader[ KW_LL_CONNECTION ];
		c.name.clear();
		c.noName = true;
		c.body.clear();
		c.noBody = true;

		for(int k=0; k<pLayout.getNumConnections(); k++)
		{
			__ReadOnly_PipelineLayout::Connection x = pLayout.getConnection(k);

			c.arguments.clear();

			if(x.idOut==__ReadOnly_PipelineLayout::THIS_PIPELINE)
			{
				c.arguments.push_back( keywordsLayoutLoader[ KW_LL_THIS_PIPELINE] );
				c.arguments.push_back( pLayout.getInputPortName(x.portOut) );
			}
			else
			{
				c.arguments.push_back( pLayout.getElementName(x.idOut) );
				c.arguments.push_back( pLayout.componentLayout(x.idOut).getOutputPortName(x.portOut) );
			}
			
			if(x.idIn==__ReadOnly_PipelineLayout::THIS_PIPELINE)
			{
				c.arguments.push_back( keywordsLayoutLoader[ KW_LL_THIS_PIPELINE] );
				c.arguments.push_back( pLayout.getOutputPortName(x.portIn) );
			}
			else
			{
				c.arguments.push_back( pLayout.getElementName(x.idIn) );
				c.arguments.push_back( pLayout.componentLayout(x.idIn).getInputPortName(x.portIn) );
			}

			e.body += c.getCode() + "\n";
		}

		return e;
	}

	/**
	\fn std::string LayoutWriter::operator()(const __ReadOnly_PipelineLayout& pipelineLayout)
	\brief Build the human-readable code for the given __ReadOnly_PipelineLayout object.
	\param pipelineLayout The pipeline layout to convert.
	\return A standard string containing the full pipeline layout description. 
	**/
	std::string LayoutWriter::operator()(const __ReadOnly_PipelineLayout& pipelineLayout)
	{
		code.clear();

		VanillaParserSpace::Element e = write(pipelineLayout, true);

		code += e.getCode() + "\n";

		return code;
	}

	/**
	\fn void LayoutWriter::writeToFile(const __ReadOnly_PipelineLayout& pipelineLayout, const std::string& filename)
	\brief Build the human-readable code for the given __ReadOnly_PipelineLayout object and write it to a file. WARNING : it will discard all previous content.
	\param pipelineLayout The pipeline layout to convert.
	\param filename The filename to write to (Warning : discard all previous content).
	**/
	void LayoutWriter::writeToFile(const __ReadOnly_PipelineLayout& pipelineLayout, const std::string& filename)
	{
		std::fstream file(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		
		if(!file.is_open())
			throw Exception("LayoutWriter::writeToFile - Unable to write pipeline layout \"" + pipelineLayout.getTypeName() + "\" to file \"" + filename + "\".", __FILE__, __LINE__);

		file << (*this)(pipelineLayout);

		file.close();
	}

