/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. Kerviche                                                                               */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : glip-lib.net                                                                              */
/*                                                                                                               */
/*     File          : LayoutLoaderModules.cpp                                                                   */
/*     Original Date : October 14th 2013                                                                         */
/*                                                                                                               */
/*     Description   : Layout Loader Modules.                                                                    */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    LayoutLoaderModules.cpp
 * \brief   Layout Loader Modules.
 * \author  R. KERVICHE
 * \date    October 14th 2013
**/

	// Includes : 
	#include <cmath>
	#include "Modules/LayoutLoaderModules.hpp"
	#include "Core/Exception.hpp"
	#include "Modules/LayoutLoader.hpp"
	#include "Modules/FFT.hpp"
	#include "Modules/GeometryLoader.hpp"

	// Namespaces :
	using namespace Glip;
	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;
	using namespace Glip::Modules;
	using namespace Glip::Modules::VanillaParserSpace;

	// Modules tools : 
		/**
		\fn LayoutLoaderModule::LayoutLoaderModule( const std::string& _name, const std::string& _manual, const int& _minNumArguments, const int& _maxNumArguments, const char& _bodyPresence)
		\brief LayoutLoaderModule constructor. For simple modules you can just use the macro LAYOUT_LOADER_MODULE_APPLY.
		\param _name Name of the module.
		\param _manual Manual of the module, see extended description for formanting.
		\param _minNumArguments Minimum number of arguments of the module.
		\param _maxNumArguments Maximum number of arguments of the module (-1 for no limitation).
		\param _bodyPresence Requirement on the body (-1 for no body, 0 for indifferent, 1 for needed).

		The Manual string can either be a simple string containin the description or a string containing the following elements (all optional) :
		\code
			const std::string manual = 	"DESCRIPTION{Description of the module.}"
							"ARGUMENT:argument0Name{Argument 0 description.}"
							"ARGUMENT:argument1Name{Argument 1 description.}"
							"BODY_DESCRIPTION{Body description}";
		\endcode
		With this specific formating the documentation will be automatically generated in various formats (HTML, Command Line Help, etc.) with a coherent layout.
		**/
		LayoutLoaderModule::LayoutLoaderModule( const std::string& _name, const std::string& _manual, const int& _minNumArguments, const int& _maxNumArguments, const char& _bodyPresence)
		 : 	name(_name), 
			minNumArguments(_minNumArguments), 
			maxNumArguments(_maxNumArguments), 
			bodyPresence(_bodyPresence)
		{
			initManual(_manual);
		}

		LayoutLoaderModule::LayoutLoaderModule(const LayoutLoaderModule& m)
		 : 	name(m.name), 
			description(m.description), 
			bodyDescription(m.bodyDescription),
			argumentsDescriptions(m.argumentsDescriptions),
			minNumArguments(m.minNumArguments), 
			maxNumArguments(m.maxNumArguments), 
			bodyPresence(m.bodyPresence) 
		{ }

		LayoutLoaderModule::~LayoutLoaderModule(void)
		{ }

		void LayoutLoaderModule::initManual(const std::string& manual)
		{
			const std::string 	descriptionKeyword = "DESCRIPTION",
						bodyDescriptionKeyword = "BODY_DESCRIPTION",
						argumentKeyword = "ARGUMENT";
			description.clear();
			bodyDescription.clear();
			argumentsDescriptions.clear();
			try
			{
				VanillaParserSpace::VanillaParser parser(manual);

				for(std::vector<VanillaParserSpace::Element>::const_iterator it=parser.elements.begin(); it!=parser.elements.end(); it++)
				{
					if((*it).strKeyword==descriptionKeyword && description.empty())
						description = (*it).body;
					else if((*it).strKeyword==bodyDescriptionKeyword && bodyDescription.empty())
						bodyDescription = (*it).body;
					else if((*it).strKeyword==argumentKeyword)
						argumentsDescriptions.push_back( std::pair<std::string,std::string>((*it).name, (*it).body));
					else
						throw Exception("Unknown description keyword : " + (*it).strKeyword);
				}
			}
			catch(Exception& e)
			{
				#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
					std::cerr << "From LayoutLoaderModule::initManual, caught exception while processing manual of module " << name << " : " << std::endl;
					std::cerr << e.what() << std::endl;
				#endif 

				description = manual;
				bodyDescription.clear();
				argumentsDescriptions.clear();
			}
		}

		/**
		\fn const std::string& LayoutLoaderModule::getName(void) const
		\brief Get the name of the module.
		\return A standard string containing the name of the module.
		**/
		const std::string& LayoutLoaderModule::getName(void) const
		{
			return name;
		}

		/**
		\fn const int& LayoutLoaderModule::getMinNumArguments(void) const
		\brief Get the minimum number of arguments of the module.
		\return An integer being the  minimum number of arguments of the module.
		**/
		const int& LayoutLoaderModule::getMinNumArguments(void) const
		{
			return minNumArguments;
		}

		/**
		\fn const int& LayoutLoaderModule::getMaxNumArguments(void) const
		\brief Get the maximum number of arguments of the module.
		\return An integer being the  maximum number of arguments of the module.
		**/
		const int& LayoutLoaderModule::getMaxNumArguments(void) const
		{
			return maxNumArguments;
		}

		/**
		\fn const char& LayoutLoaderModule::bodyPresenceTest(void) const
		\brief Get the requirement on the body.
		\return A byte containing the requirement as -1 for no body, 0 for indifferent and 1 for needed.
		**/
		const char& LayoutLoaderModule::bodyPresenceTest(void) const
		{
			return bodyPresence;
		}	

		/**
		\fn const std::string& LayoutLoaderModule::getDescription(void) const
		\brief Get the description of the module.
		\return A string containing the description of the module.
		**/
		const std::string& LayoutLoaderModule::getDescription(void) const
		{
			return description;
		}

		/**
		\fn const std::string& LayoutLoaderModule::getBodyDescription(void) const
		\brief Get the description of the body of the module.
		\return A string containing the description of the body of the module. Possibly empty.
		**/
		const std::string& LayoutLoaderModule::getBodyDescription(void) const
		{
			return bodyDescription;
		}

		/**
		\fn const std::vector<std::pair<std::string,std::string> >& LayoutLoaderModule::getArgumentsDescriptions(void) const
		\brief Get the description of all the arguments of the module.
		\return A vector of pairs containing the description (second) of each arguments (first).
		**/
		const std::vector<std::pair<std::string,std::string> >& LayoutLoaderModule::getArgumentsDescriptions(void) const
		{
			return argumentsDescriptions;
		}

		/**
		\fn std::string LayoutLoaderModule::getManual(void) const
		\brief Get the manual of the module.
		\return A standard string containing the manual of the module.
		**/
		std::string LayoutLoaderModule::getManual(void) const
		{
			std::string manual;
			manual += "MODULE : " + name + "\n";
			if(!description.empty())
				manual += "    " + description + "\n";
		
			if(!argumentsDescriptions.empty())
				manual += "  Arguments :\n";	

			size_t maxSize = 0;
			for(std::vector<std::pair<std::string,std::string> >::const_iterator it=argumentsDescriptions.begin(); it!=argumentsDescriptions.end(); it++)
				maxSize = std::max(maxSize, it->first.size());

			for(std::vector<std::pair<std::string,std::string> >::const_iterator it=argumentsDescriptions.begin(); it!=argumentsDescriptions.end(); it++)
				manual += "  * " + it->first + std::string(maxSize-it->first.size(),' ') + " : " + it->second + "\n";
			
			if(!bodyDescription.empty())
				manual += "  Body : " + bodyDescription + "\n";
		
			return manual;
		}

		/**
		\fn void LayoutLoaderModule::beginLoadLayout(void)
		\brief Function called at the beginning of a loading session.
		**/
		void LayoutLoaderModule::beginLoadLayout(void)
		{ }

		/**
		\fn void LayoutLoaderModule::endLoadLayout(void)
		\brief Function called at the end of a loading session.
		**/
		void LayoutLoaderModule::endLoadLayout(void)
		{ }

		/**
		\fn void LayoutLoaderModule::addBasicModules(LayoutLoader& loader)
		\brief Add the basic modules to a LayoutLoader.
		\param loader A LayoutLoader object.
		**/
		void LayoutLoaderModule::addBasicModules(LayoutLoader& loader)
		{		
			std::vector<LayoutLoaderModule*> modules = LayoutLoaderModule::getBasicModulesList();
			for(std::vector<LayoutLoaderModule*>::iterator it=modules.begin(); it!=modules.end(); it++)
				loader.addModule(*it);
		}

		/**
		\fn std::vector<LayoutLoaderModule*> LayoutLoaderModule::getBasicModulesList(void)
		\brief Retrieve the list of standard modules.
		\return A vector containing pointers to the basic modules. The client code has the responsability of deleting these modules.
		**/
		std::vector<LayoutLoaderModule*> LayoutLoaderModule::getBasicModulesList(void)
		{
			std::vector<LayoutLoaderModule*> result;

			result.push_back( new IF_MODULE_AVAILABLE );
			result.push_back( new IF_FORMAT_DEFINED );
			result.push_back( new IF_SOURCE_DEFINED );
			result.push_back( new IF_GEOMETRY_DEFINED );
			result.push_back( new IF_FILTERLAYOUT_DEFINED );
			result.push_back( new IF_PIPELINELAYOUT_DEFINED );
			result.push_back( new IF_REQUIREDFORMAT_DEFINED );
			result.push_back( new IF_REQUIREDGEOMETRY_DEFINED );
			result.push_back( new IF_REQUIREDPIPELINE_DEFINED );
			result.push_back( new FORMAT_CHANGE_SIZE );
			result.push_back( new FORMAT_INCREASE_SIZE );
			result.push_back( new FORMAT_DECREASE_SIZE );
			result.push_back( new FORMAT_SCALE_SIZE );
			result.push_back( new FORMAT_INVSCALE_SIZE );
			result.push_back( new FORMAT_CHANGE_CHANNELS );
			result.push_back( new FORMAT_CHANGE_DEPTH );
			result.push_back( new FORMAT_CHANGE_FILTERING );
			result.push_back( new FORMAT_CHANGE_WRAPPING );
			result.push_back( new FORMAT_CHANGE_MIPMAP );
			result.push_back( new FORMAT_MINIMUM_WIDTH );
			result.push_back( new FORMAT_MAXIMUM_WIDTH );
			result.push_back( new FORMAT_MINIMUM_HEIGHT );
			result.push_back( new FORMAT_MAXIMUM_HEIGHT );
			result.push_back( new FORMAT_MINIMUM_PIXELS );
			result.push_back( new FORMAT_MAXIMUM_PIXELS );
			result.push_back( new FORMAT_MINIMUM_ELEMENTS );
			result.push_back( new FORMAT_MAXIMUM_ELEMENTS );
			result.push_back( new FORMAT_SMALLER_POWER_OF_TWO );
			result.push_back( new FORMAT_LARGER_POWER_OF_TWO );
			result.push_back( new FORMAT_SWAP_DIMENSIONS );
			result.push_back( new IF_FORMAT_SETTING_MATCH );
			result.push_back( new IF_FORMAT_SETTING_LARGERTHAN );
			result.push_back( new OVERRIDE_REQUIRED_FORMAT );
			result.push_back( new GENERATE_SAME_SIZE_2D_GRID );
			result.push_back( new GENERATE_SAME_SIZE_3D_GRID );
			result.push_back( new CHAIN_PIPELINES );
			result.push_back( new FORMAT_TO_CONSTANT );
			result.push_back( new SINGLE_FILTER_PIPELINE );
			result.push_back( new IF_GLSL_VERSION_MATCH );
			result.push_back( new ABORT_ERROR );
			result.push_back( new GenerateFFT1DPipeline );
			result.push_back( new GenerateFFT2DPipeline );
			result.push_back( new OBJLoader );
			result.push_back( new STLLoader );

			return result;
		}

		/**
		\fn bool LayoutLoaderModule::getBoolean(const std::string& arg, const std::string& sourceName, int line)
		\brief Convert a keyword to a boolean.
		\param arg The keyword (expected to be either TRUE or FALSE).
		\param sourceName Name of the source.
		\param line Corresponding line number in the source.
		\return True or false depending on the content of arg or raise an exception if the value is not any of these two symbols.
		**/
		bool LayoutLoaderModule::getBoolean(const std::string& arg, const std::string& sourceName, int line)
		{
			if(arg==LayoutLoader::getKeyword(KW_LL_FALSE))
				return false;
			else if(arg==LayoutLoader::getKeyword(KW_LL_TRUE))
				return true;
			else
				throw Exception("Unknown boolean keyword \"" + arg + "\". Expected " + LayoutLoader::getKeyword(KW_LL_TRUE) + " or " + LayoutLoader::getKeyword(KW_LL_FALSE) + ".", sourceName, line, Exception::ClientScriptException);
		}

		/**
		\fn void LayoutLoaderModule::getCases(const std::string& body, std::string& trueCase, int& trueCaseStartLine, std::string& falseCase, int& falseCaseStartLine, const std::string& sourceName, int bodyLine)
		\brief Get true and false cases out of a body.
	
		In a if-statement you can write : 
		\code
		IF(statement)
		{
			TRUE
			{
				...
			}
			FALSE
			{
				...
			}
		}
		\endcode

		\param body Body to extract the data from.
		\param trueCase Body of the true statement (not modified if not found).
		\param trueCaseStartLine Line number where the body of the true case is starting (not modified if not found).
		\param falseCase Body of the false statement (not modified if not found).
		\param falseCaseStartLine Line number where the body of the false case is starting (not modified if not found).
		\param sourceName Name of the source.
		\param bodyLine Line counter start index.
		**/
		void LayoutLoaderModule::getCases(const std::string& body, std::string& trueCase, int& trueCaseStartLine, std::string& falseCase, int& falseCaseStartLine, const std::string& sourceName, int bodyLine)
		{
			VanillaParser parser(body, sourceName, bodyLine);
			bool 	trueCaseAlreadySet = false,
				falseCaseAlreadySet = false;

			for(std::vector<Element>::iterator it=parser.elements.begin(); it!=parser.elements.end(); it++)
			{
				if(it->strKeyword==LayoutLoader::getKeyword(KW_LL_TRUE) && !it->noBody)
				{
					if(trueCaseAlreadySet)
						throw Exception("True case already set.", it->sourceName, it->startLine, Exception::ClientScriptException);
					
					trueCase = it->getCleanBody();
					trueCaseStartLine = it->bodyLine;
					trueCaseAlreadySet = true;
				}
				else if(it->strKeyword==LayoutLoader::getKeyword(KW_LL_FALSE)  && !it->noBody)
				{
					if(falseCaseAlreadySet)
						throw Exception("False case already set.", it->sourceName, it->startLine, Exception::ClientScriptException);

					falseCase = it->getCleanBody();
					falseCaseStartLine = it->bodyLine;
					falseCaseAlreadySet = true;
				}				
				else
					throw Exception("Unknown case keyword \"" + it->strKeyword + "\". Expected " + LayoutLoader::getKeyword(KW_LL_TRUE) + " or " + LayoutLoader::getKeyword(KW_LL_FALSE) + ".", it->sourceName, it->startLine, Exception::ClientScriptException);
			}
		}

		/**
		\fn std::vector<std::string> LayoutLoaderModule::findFile(const std::string& filename, const std::vector<std::string>& dynamicPaths)
		\brief Find in which path a file can be found.
		\param filename File name to be searched.
		\param dynamicPaths List of paths in which to search.
		\return A list of paths in which the file was found (possibly empty).
		**/
		std::vector<std::string> LayoutLoaderModule::findFile(const std::string& filename, const std::vector<std::string>& dynamicPaths)
		{
			// Check all path :
			std::vector<std::string> possiblePaths;

			// Blank :
			{
				std::ifstream file;
				file.open(filename.c_str());
				if(file.is_open() && file.good() && !file.fail())
					possiblePaths.push_back("");
				file.close();
			}

			// From dynamic path (which already include static path) :
			for(std::vector<std::string>::const_iterator it=dynamicPaths.begin(); it!=dynamicPaths.end(); it++)
			{
				const std::string currentFilename = (*it) + filename;
				std::ifstream file;
				file.open(currentFilename.c_str());
				if(file.is_open() && file.good() && !file.fail())
					possiblePaths.push_back(*it);
				file.close();
			}

			for(std::vector<std::string>::iterator it=possiblePaths.begin(); it!=possiblePaths.end(); it++)
				(*it) += filename;
			return possiblePaths;
		}

	// Simple modules : 
			LAYOUT_LOADER_MODULE_APPLY( IF_MODULE_AVAILABLE, 1, 1, 1,	"DESCRIPTION{Check if the MODULE is available and can be used.}"
											"ARGUMENT:moduleName{Name of the module to be used.}"
											"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;	
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_MODULE(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=moduleList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}
			}
 
			LAYOUT_LOADER_MODULE_APPLY( IF_FORMAT_DEFINED, 1, 2, 1,	"DESCRIPTION{Check if the FORMAT was defined.}"
										"ARGUMENT:formatName{Name of the format to test.}"
										"ARGUMENT:includeRequiredFormats{Optional, include the required formats. Either TRUE (default) or FALSE.}"
										"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;	
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);

				const bool includeRequiredFormats = (arguments.size()<=1) ? true : getBoolean(arguments[1], sourceName, startLine);
				CONST_ITERATOR_TO_FORMAT(it1, arguments[0])
				CONST_ITERATOR_TO_REQUIREDFORMAT(it2, arguments[0])

				executionSourceName = sourceName;
				if(VALID_ITERATOR_TO_FORMAT(it1) || (includeRequiredFormats && VALID_ITERATOR_TO_REQUIREDFORMAT(it2)))
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_SOURCE_DEFINED, 1, 1, 1, "DESCRIPTION{Check if the SOURCE was defined.}"
										"ARGUMENT:sourceName{Name of the source to test.}"
										"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_SOURCE(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=sourceList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}				
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_GEOMETRY_DEFINED, 1, 1, 1, 	"DESCRIPTION{Check if the GEOMETRY was defined.}"
											"ARGUMENT:geometryName{Name of the geometry to test.}"
											"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_GEOMETRY(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=geometryList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}			
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_FILTERLAYOUT_DEFINED, 1, 1, 1,	"DESCRIPTION{Check if the FILTER_LAYOUT was defined.}"
											"ARGUMENT:filterLayoutName{Name of the filter layout to test.}"	
											"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_FILTER(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=filterList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}				
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_PIPELINELAYOUT_DEFINED, 1, 1, 1,	"DESCRIPTION{Check if the PIPELINE_LAYOUT was defined.}"
											"ARGUMENT:pipelineLayoutName{Name of the pipeline layout to test.}"
											"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_PIPELINE(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=pipelineList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}			
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_REQUIREDFORMAT_DEFINED, 1, 1, 1,	"DESCRIPTION{Check if the REQUIRED_FORMAT was defined.}"
											"ARGUMENT:requiredFormatName{Name of the required format to test.}"	
											"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_REQUIREDFORMAT(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=requiredFormatList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}			
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_REQUIREDSOURCE_DEFINED, 1, 1, 1,	"DESCRIPTION{Check if the REQUIRED_SOURCE was defined.}"
											"ARGUMENT:requiredSourceName{Name of the required source to test.}"	
											"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_REQUIREDSOURCE(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=requiredSourceList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_REQUIREDGEOMETRY_DEFINED, 1, 1, 1,	"DESCRIPTION{Check if the REQUIRED_GEOMETRY was defined.}"
												"ARGUMENT:requiredGeometryName{Name of the required geometry to test.}"
												"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_REQUIREDGEOMETRY(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=requiredGeometryList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}			
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_REQUIREDPIPELINE_DEFINED, 1, 1, 1,	"DESCRIPTION{Check if the REQUIRED_PIPELINE was defined.}"
												"ARGUMENT:requiredPipelineName{Name of the required pipeline to test.}"
												"BODY_DESCRIPTION{Cases corresponding to the test : TRUE{...} FALSE{...}.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)

				std::string 	trueCase, 
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				
				CONST_ITERATOR_TO_REQUIREDPIPELINE(it, arguments[0])

				executionSourceName = sourceName;
				if(it!=requiredPipelineList.end())
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_CHANGE_SIZE, 4, 4, -1,	"DESCRIPTION{Change the size of a format, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format}"
											"ARGUMENT:widthNew{New width.}"
											"ARGUMENT:heightNew{New height.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;

				CAST_ARGUMENT( 2, double, w)
				CAST_ARGUMENT( 3, double, h)

				newFmt.setWidth(w);
				newFmt.setHeight(h);

				if(newFmt.getWidth()<=0 || newFmt.getHeight()<=0)
					throw Exception("The new format is not valid (size : " + toString(newFmt.getWidth()) + "x" + toString(newFmt.getHeight()) + ").", sourceName, startLine, Exception::ClientScriptException);

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_INCREASE_SIZE, 3, 4, -1,	"DESCRIPTION{Increase a format size by a scalar (or two), save as a new format. "
											"Will prevent to reach a 0x0 texture by ensuring that the size is "
											"at least 1 pixel in each dimension.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:increaseDelta{Increase to be applied, can be splitted into X and Y. It can also be the name of an existing format.}")
		 	{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;

				CONST_ITERATOR_TO_FORMAT( itScale, arguments[2] )
			
				if(itScale!=formatList.end())
				{
					if(arguments.size()==4)
						throw Exception("Fourth argument not tolerated when the third is a format name.", sourceName, startLine, Exception::ClientScriptException);
					
					newFmt.setWidth( std::max(newFmt.getWidth() + itScale->second.getWidth(), 1) );
					newFmt.setHeight( std::max(newFmt.getHeight() + itScale->second.getHeight(), 1) );
				}
				else if(arguments.size()==3)
				{
					CAST_ARGUMENT( 2, double, s) 	
					newFmt.setWidth( std::max(newFmt.getWidth() + s, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() + s, 1.0) );
				}
				else
				{
					CAST_ARGUMENT( 2, double, sx)
					CAST_ARGUMENT( 3, double, sy)
					newFmt.setWidth( std::max(newFmt.getWidth() + sx, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() + sy, 1.0) );
				}
				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_DECREASE_SIZE, 3, 4, -1,	"DESCRIPTION{Decrease a format size by a scalar (or two), save as a new format. "
											"Will prevent to reach a 0x0 texture by ensuring that the size is "
											"at least 1 pixel in each dimension.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:increaseDelta{Decrease delta to be applied, can be splitted into X and Y. It can also be the name of an existing format.}")
		 	{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;

				CONST_ITERATOR_TO_FORMAT( itScale, arguments[2] )
			
				if(itScale!=formatList.end())
				{
					if(arguments.size()==4)
						throw Exception("Fourth argument not tolerated when the third is a format name.", sourceName, startLine, Exception::ClientScriptException);
					
					newFmt.setWidth( std::max(newFmt.getWidth() * itScale->second.getWidth(), 1) );
					newFmt.setHeight( std::max(newFmt.getHeight() * itScale->second.getHeight(), 1) );
				}
				else if(arguments.size()==3)
				{
					CAST_ARGUMENT( 2, double, s) 		
					newFmt.setWidth( std::max(newFmt.getWidth() - s, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() - s, 1.0) );
				}
				else
				{
					CAST_ARGUMENT( 2, double, sx)
					CAST_ARGUMENT( 3, double, sy)	
					newFmt.setWidth( std::max(newFmt.getWidth() - sx, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() - sy, 1.0) );
				}
				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_SCALE_SIZE, 3, 4, -1,	"DESCRIPTION{Scale a format by a scalar (or two), save as a new format. "
											"Will prevent to reach a 0x0 texture by ensuring that the size is "
											"at least 1 pixel in each dimension.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:scaleFactor{Scaling to be applied, can be splitted into X and Y. It can also be the name of an existing format.}")
		 	{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;

				CONST_ITERATOR_TO_FORMAT( itScale, arguments[2] )
			
				if(itScale!=formatList.end())
				{
					if(arguments.size()==4)
						throw Exception("Fourth argument not tolerated when the third is a format name.", sourceName, startLine, Exception::ClientScriptException);
					
					newFmt.setWidth( std::max(newFmt.getWidth() * itScale->second.getWidth(), 1) );
					newFmt.setHeight( std::max(newFmt.getHeight() * itScale->second.getHeight(), 1) );
				}
				else if(arguments.size()==3)
				{
					CAST_ARGUMENT( 2, double, s) 	
					if(s<=0.0)
						throw Exception("The scale cannot be negative or equal to zero (s = " + toString(s) + ").", sourceName, startLine, Exception::ClientScriptException);
		
					newFmt.setWidth( std::max(newFmt.getWidth() * s, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() * s, 1.0) );
				}
				else
				{
					CAST_ARGUMENT( 2, double, sx)
					CAST_ARGUMENT( 3, double, sy)
					if(sx<=0.0)
						throw Exception("The scale cannot be negative or equal to zero (sx = " + toString(sx) + ").", sourceName, startLine, Exception::ClientScriptException);
					if(sy<=0.0)
						throw Exception("The scale cannot be negative or equal to zero (sy = " + toString(sy) + ").", sourceName, startLine, Exception::ClientScriptException);
		
					newFmt.setWidth( std::max(newFmt.getWidth() * sx, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() * sy, 1.0) );
				}

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_INVSCALE_SIZE, 3, 4, -1,	"DESCRIPTION{Scale a format by the inverse of a scalar (or two), save as a new format. "
											"Will prevent to reach a 0x0 texture by ensuring that the size is "
											"at least 1 pixel in each dimension. The results is rounded to the floor.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:scaleFactor{Inverse scaling to be applied, can be splitted into X and Y. It can also be the name of an existing format.}")
		 	{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;

				CONST_ITERATOR_TO_FORMAT( itScale, arguments[2] )
			
				if(itScale!=formatList.end())
				{
					if(arguments.size()==4)
						throw Exception("Fourth argument not tolerated when the third is a format name.", sourceName, startLine, Exception::ClientScriptException);
					
					newFmt.setWidth( std::max(newFmt.getWidth() / itScale->second.getWidth(), 1) );
					newFmt.setHeight( std::max(newFmt.getHeight() / itScale->second.getHeight(), 1) );
				}
				else if(arguments.size()==3)
				{
					CAST_ARGUMENT( 2, double, s) 	
					if(s<=0.0)
						throw Exception("The scale cannot be negative or equal to zero (s = " + toString(s) + ").", sourceName, startLine, Exception::ClientScriptException);
		
					newFmt.setWidth( std::max(newFmt.getWidth() / s, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() / s, 1.0) );
				}
				else
				{
					CAST_ARGUMENT( 2, double, sx)
					CAST_ARGUMENT( 3, double, sy)
					if(sx<=0.0)
						throw Exception("The scale cannot be negative or equal to zero (sx = " + toString(sx) + ").", sourceName, startLine, Exception::ClientScriptException);
					if(sy<=0.0)
						throw Exception("The scale cannot be negative or equal to zero (sy = " + toString(sy) + ").", sourceName, startLine, Exception::ClientScriptException);
		
					newFmt.setWidth( std::max(newFmt.getWidth() / sx, 1.0) );
					newFmt.setHeight( std::max(newFmt.getHeight() / sy, 1.0) );
				}

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_CHANGE_CHANNELS, 3, 3, -1,	"DESCRIPTION{Change the channels of a format, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:channelNew{New channel mode.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;
				newFmt.setGLMode( getGLEnum(arguments[2]) );

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_CHANGE_DEPTH, 3, 3, -1,	"DESCRIPTION{Change the depth of a format, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:depthNew{New depth.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;
				newFmt.setGLDepth( getGLEnum(arguments[2]) );

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_CHANGE_FILTERING, 4, 4, -1,	"DESCRIPTION{Change the filtering of a format, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:minNew{New minification filter.}"
											"ARGUMENT:magNew{New magnification filter.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;
				newFmt.setMinFilter( getGLEnum(arguments[2]) );
				newFmt.setMagFilter( getGLEnum(arguments[3]) );

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_CHANGE_WRAPPING, 4, 4, -1,	"DESCRIPTION{Change the wrapping of a format, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:sNew{New S wrapping parameter.}"
											"ARGUMENT:tNew{New T wrapping parameter.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)
	
				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;
				newFmt.setSWrapping( getGLEnum(arguments[2]) );
				newFmt.setTWrapping( getGLEnum(arguments[3]) );

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_CHANGE_MIPMAP, 4, 4, -1,	"DESCRIPTION{Change the mipmap level of a format, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameOriginal{Name of the original format.}"
											"ARGUMENT:mNew{New maximum mipmap parameter.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				// Build the new format : 
				HdlTextureFormat newFmt = it->second;
				CAST_ARGUMENT( 2, int, m)
				newFmt.setMaxLevel(m);

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MINIMUM_WIDTH, 3, -1, -1,	"DESCRIPTION{Find the format having the smallest width, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"	
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				int 	kBest 		= 1,
					widthBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getWidth() <= widthBest || k==0 )
					{
						kBest 		= k;
						widthBest 	= it->second.getWidth();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MAXIMUM_WIDTH, 3, -1, -1,	"DESCRIPTION{Find the format having the largest width, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the first format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				int 	kBest 		= 1,
					widthBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getWidth() >= widthBest || k==0 )
					{
						kBest 		= k;
						widthBest 	= it->second.getWidth();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MINIMUM_HEIGHT, 3, -1, -1,	"DESCRIPTION{Find the format having the smallest height, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				int 	kBest 		= 1,
					heightBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getHeight() <= heightBest || k==0 )
					{
						kBest 		= k;
						heightBest 	= it->second.getHeight();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MAXIMUM_HEIGHT, 3, -1, -1,	"DESCRIPTION{Find the format having the largest height, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				int 	kBest 		= 1,
					heightBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getHeight() >= heightBest || k==0 )
					{
						kBest 		= k;
						heightBest 	= it->second.getHeight();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MINIMUM_PIXELS, 3, -1, -1,	"DESCRIPTION{Find the format having the smallest number of pixels, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")

			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				int 	kBest 		= 1,
					sizeBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getNumPixels() <= sizeBest || k==0 )
					{
						kBest 		= k;
						sizeBest 	= it->second.getNumPixels();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MAXIMUM_PIXELS, 3, -1, -1,	"DESCRIPTION{Find the format having the largest number of pixels, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				int 	kBest 		= 1,
					sizeBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getNumPixels() >= sizeBest || k==0 )
					{
						kBest 		= k;
						sizeBest 	= it->second.getNumPixels();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MINIMUM_ELEMENTS, 3, -1, -1,	"DESCRIPTION{Find the format having the smallest number of elements (pixels times channels count), save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)				

				int 	kBest 		= 1,
					sizeBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getNumElements() <= sizeBest || k==0 )
					{
						kBest 		= k;
						sizeBest 	= it->second.getNumElements();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_MAXIMUM_ELEMENTS, 3, -1, -1,	"DESCRIPTION{Find the format having the largest number of elements (pixels times channels count), save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat1{Name of the fist format.}"
											"ARGUMENT:nameFormat2{Name of the second format.}"
											"ARGUMENT:nameFormat3...{Other formats.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)				

				int 	kBest 		= 1,
					sizeBest	= 0;

				for(unsigned int k=1; k<arguments.size(); k++)
				{
					FORMAT_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_FORMAT( it, arguments[k] )

					if( it->second.getNumElements() >= sizeBest || k==0 )
					{
						kBest 		= k;
						sizeBest 	= it->second.getNumElements();
					}
				}

				FORMAT_MUST_NOT_EXIST( arguments[0] );

				// New format : 
				CONST_ITERATOR_TO_FORMAT( it, arguments[kBest] )
				HdlTextureFormat newFmt = it->second;

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_SMALLER_POWER_OF_TWO, 2, 3, -1,	"DESCRIPTION{Generate a new format clamped to the closest smaller power of 2.}"
												"ARGUMENT:nameNew{Name of the new format.}"
												"ARGUMENT:nameFormat{Name of the original format.}"
												"ARGUMENT:strict{Either TRUE or FALSE (default).}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] );
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				bool strict = false;
				if(arguments.size()==3)
					strict = getBoolean(arguments[2]);

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )
				
				HdlTextureFormat newFmt = it->second;
	
				if(!strict)
				{
					newFmt.setWidth( std::floor(std::log(newFmt.getWidth())/std::log(2.0)) );
					newFmt.setWidth( std::floor(std::log(newFmt.getHeight())/std::log(2.0)) );
				}
				else
				{
					newFmt.setWidth( std::floor(std::log(newFmt.getWidth()-1.0)/std::log(2.0)) );
					newFmt.setWidth( std::floor(std::log(newFmt.getHeight()-1.0)/std::log(2.0)) );
				}

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_LARGER_POWER_OF_TWO, 2, 3, -1,	"DESCRIPTION{Generate a new format clamped to the closest larger power of 2.}"
												"ARGUMENT:nameNew{Name of the new format.}"
												"ARGUMENT:nameFormat{Name of the original format.}"
												"ARGUMENT:strict{Either TRUE or FALSE (default).}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] );
				FORMAT_MUST_NOT_EXIST( arguments[0] );

				bool strict = false;
				if(arguments.size()==3)
					strict = getBoolean(arguments[2]);

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )
				
				HdlTextureFormat newFmt = it->second;
	
				if(!strict)
				{
					newFmt.setWidth( std::ceil(std::log(newFmt.getWidth())/std::log(2.0)) );
					newFmt.setWidth( std::ceil(std::log(newFmt.getHeight())/std::log(2.0)) );
				}
				else
				{
					newFmt.setWidth( std::ceil(std::log(newFmt.getWidth()+1.0)/std::log(2.0)) );
					newFmt.setWidth( std::ceil(std::log(newFmt.getHeight()+1.0)/std::log(2.0)) );
				}

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_SWAP_DIMENSIONS, 2, 2, -1,	"DESCRIPTION{Swap the width and height values, save as a new format.}"
											"ARGUMENT:nameNew{Name of the new format.}"
											"ARGUMENT:nameFormat{Name of the original format.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] );
				FORMAT_MUST_NOT_EXIST( arguments[0] );
			
				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				HdlTextureFormat newFmt = it->second;
				newFmt.setWidth( it->second.getHeight() );
				newFmt.setHeight( it->second.getWidth() );

				APPEND_NEW_FORMAT( arguments[0], newFmt )
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_FORMAT_SETTING_MATCH, 3, 3, 1,	"DESCRIPTION{Match if a format setting is equal to a value (unsigned integer or GL keyword).}"
											"ARGUMENT:nameFormat{Name of the targeted format.}"
											"ARGUMENT:nameSetting{Name of the setting. See the documentation of HdlAbstractTextureFormat.}"
											"ARGUMENT:value{The unsigned integer value or GLenum name to test against.}"
											"BODY_DESCRIPTION{Contains any or all of the blocks TRUE{...} and FALSE{...}. The right block will be exectuted following this test.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
			
				FORMAT_MUST_EXIST( arguments[0] )
				CONST_ITERATOR_TO_FORMAT( it, arguments[0] )

				// Get the value : 
				GLenum setting = getGLEnum( arguments[1] );

				// Special cast : 
				unsigned int value;

				if(!fromString(arguments[2], value))
					value = getGLEnum( arguments[2] );

				// Get the cases : 
				std::string 	trueCase, 
						falseCase;
				int 	trueCaseStartLine=1,
					falseCaseStartLine=1;

				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);

				executionSourceName = sourceName;
				if(it->second.getSetting(setting)==value)
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}
			}

			LAYOUT_LOADER_MODULE_APPLY( IF_FORMAT_SETTING_LARGERTHAN, 3, 3, 1,	"DESCRIPTION{Match if a format setting is larger than a value (unsigned integer or GL keyword).}"
												"ARGUMENT:nameFormat{Name of the targeted format.}"
												"ARGUMENT:nameSetting{Name of the setting. See the documentation of HdlAbstractTextureFormat.}"
												"ARGUMENT:value{The unsigned integer value or GLenum name to test against.}"	
												"BODY_DESCRIPTION{Contains any or all of the blocks TRUE{...} and FALSE{...}. The right block will be exectuted following this test.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)

				FORMAT_MUST_EXIST( arguments[0] )

				CONST_ITERATOR_TO_FORMAT( it, arguments[0] )

				// Get the value : 
				GLenum setting = getGLEnum( arguments[1] );

				// Special cast : 
				unsigned int value;

				if(!fromString(arguments[2], value))
					value = getGLEnum( arguments[2] );

				// Get the cases : 
				std::string 	trueCase, 
						falseCase;
				int 	trueCaseStartLine=1,
					falseCaseStartLine=1;

				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);

				executionSourceName = sourceName;
				if(it->second.getSetting(setting)>value)
				{
					executionSource = trueCase;
					executionStartLine = trueCaseStartLine;
				}
				else
				{
					executionSource = falseCase;
					executionStartLine = falseCaseStartLine;
				}
			}

			LAYOUT_LOADER_MODULE_APPLY( OVERRIDE_REQUIRED_FORMAT, 3, 12, -1,	"DESCRIPTION{Override the selection of a required format if another format with higher priority is found.}"
												"ARGUMENT:name{Name of the format to be created.}"
												"ARGUMENT:overrideFormat{Name of the format which will be used first, if it exists.}"
												"ARGUMENT:defaultFormat{Name of the default format to use}"
												"ARGUMENT:remainingArguments{See REQUIRED_FORMAT arguments.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				const std::string& overrideName = arguments[1],
						   defaultName = arguments[2];
				CONST_ITERATOR_TO_REQUIREDFORMAT( overrideRequiredIt, overrideName )
				CONST_ITERATOR_TO_FORMAT( overrideIt, overrideName )
				CONST_ITERATOR_TO_REQUIREDFORMAT( defaultRequiredIt, defaultName )
				CONST_ITERATOR_TO_FORMAT( defaultIt, defaultName )

				std::string formatTargetName;
				if(overrideRequiredIt!=requiredFormatList.end() || overrideIt!=formatList.end())
					formatTargetName = overrideName;
				else if(defaultRequiredIt!=requiredFormatList.end() || defaultIt!=formatList.end())
					formatTargetName = defaultName;
				else
					throw Exception("Neither formats " + overrideName + " or " + defaultName + " could be found.", sourceName, startLine, Exception::ClientScriptException);

				executionSource = LayoutLoader::getKeyword(KW_LL_REQUIRED_FORMAT);
				executionSource += ":" + arguments[0] + "(" + formatTargetName;
				for(std::vector<std::string>::const_iterator it=arguments.begin()+3; it!=arguments.end(); it++)
					executionSource += ", " + (*it);
				executionSource += ')';
				executionSourceName = sourceName;
				executionStartLine = startLine;
			}

			LAYOUT_LOADER_MODULE_APPLY( GENERATE_SAME_SIZE_2D_GRID, 2, 4, -1,	"DESCRIPTION{Create a 2D grid geometry of the same size as the format in argument (width and height).}"
												"ARGUMENT:nameNewGeometry{Name of the new geometry.}"
												"ARGUMENT:nameFormat{Name of the original format.}"
												"ARGUMENT:normalized{Either TRUE or FALSE. If enabled, the vertices coordinates will be in the range [0,1].}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				GEOMETRY_MUST_NOT_EXIST( arguments[0] )

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				const bool normalized = (arguments.size()>=3) ? getBoolean(arguments[2], sourceName, startLine) : false;;

				// Build the new geometry : 
				APPEND_NEW_GEOMETRY( arguments[0], std::list<GeometryModel>(1, GeometryPrimitives::PointsGrid2D(it->second.getWidth(), it->second.getHeight(), normalized)) )
			}

			LAYOUT_LOADER_MODULE_APPLY( GENERATE_SAME_SIZE_3D_GRID, 2, 3, -1,	"DESCRIPTION{Create a 3D grid geometry of the same size as the format in argument (width, height and number of channels).}"
												"ARGUMENT:nameNewGeometry{Name of the new geometry.}"
												"ARGUMENT:nameFormat{Name of the original format.}"
												"ARGUMENT:normalized{Either TRUE or FALSE. If enabled, the vertices coordinates will be in the range [0,1].}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				FORMAT_MUST_EXIST( arguments[1] )
				GEOMETRY_MUST_NOT_EXIST( arguments[0] )

				CONST_ITERATOR_TO_FORMAT( it, arguments[1] )

				const bool normalized = (arguments.size()>=3) ? getBoolean(arguments[2], sourceName, startLine) : false;

				// Build the new geometry : 
				APPEND_NEW_GEOMETRY( arguments[0], std::list<GeometryModel>(1, GeometryPrimitives::PointsGrid3D(it->second.getWidth(), it->second.getHeight(), it->second.getNumChannels(), normalized)) )
			}

			LAYOUT_LOADER_MODULE_APPLY( CHAIN_PIPELINES, 4, -1, -1,	"DESCRIPTION{Create a pipeline by connecting the pipelines passed in arguments.}\n"
										"ARGUMENT:nameNewPipelineLayout{Name of the new pipeline.}"
										"ARGUMENT:isStrict{Either TRUE or FALSE. If enabled, the pipelines connection are enforced strictly (if outputs of the first pipeline are not equal to the number of input of the second pipeline, then the module will report an error.}"
										"ARGUMENT:namePipelineLayout1{Name of the first pipeline in the chain.}"
										"ARGUMENT:namePipelineLayout2{Name of the second pipeline in the chain.}"
										"ARGUMENT:namePipelineLayout3...{Other pipelines.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				const unsigned int startPipelines = 2;
				std::string requiredElements;
				VanillaParserSpace::Element 	result;

				PIPELINE_MUST_NOT_EXIST( arguments[0] );

				// Get the arguments : 
				const bool isStrict = getBoolean(arguments[1], sourceName, startLine);

				result.strKeyword 	= LayoutLoader::getKeyword(KW_LL_PIPELINE_LAYOUT);
				result.name		= arguments[0];
				result.noName		= false;
				result.noArgument	= true;
				result.noBody		= false;
				
				result.body.clear();

				// Get all the pipelines : 
				std::string lastInstance;
				std::vector<std::string> outputPortNames;
				for(unsigned int k=startPipelines; k<arguments.size(); k++)
				{
					// Access the pipeline : 
					PIPELINE_MUST_EXIST( arguments[k] );

					CONST_ITERATOR_TO_PIPELINE(itCurrentPipeline, arguments[k]);

					// Add the requirement : 
					VanillaParserSpace::Element requirement;
				
					requirement.strKeyword	= LayoutLoader::getKeyword(KW_LL_REQUIRED_PIPELINE);
					requirement.name	= arguments[k] + "_required" + toString(k-startPipelines);
					requirement.noName	= false;
					requirement.arguments.push_back(arguments[k]);
					requirement.noArgument	= false;

					requiredElements += requirement.getCode() + "\n";

					// Add the instance : 
					VanillaParserSpace::Element instance;
				
					instance.strKeyword 	= LayoutLoader::getKeyword(KW_LL_PIPELINE_INSTANCE);
					instance.name		= arguments[k] + "_instance" + toString(k-startPipelines);
					instance.noName		= false;
					instance.arguments.push_back(requirement.name);
					instance.noArgument	= false;

					result.body += instance.getCode() + "\n";

					if(k==startPipelines)
					{
						// Append the inputs : 
						VanillaParserSpace::Element inputs;

						inputs.strKeyword = LayoutLoader::getKeyword(KW_LL_INPUT_PORTS);

						for(int i=0; i<itCurrentPipeline->second.getNumInputPort(); i++)
							inputs.arguments.push_back(itCurrentPipeline->second.getInputPortName(i));
						inputs.noArgument = false;

						result.body += inputs.getCode() + "\n";

						// Make the connections : 
						for(int i=0; i<itCurrentPipeline->second.getNumInputPort(); i++)
						{
							VanillaParserSpace::Element connection;

							connection.strKeyword = LayoutLoader::getKeyword(KW_LL_CONNECTION);
							connection.noArgument = false;
							connection.arguments.push_back( LayoutLoader::getKeyword(KW_LL_THIS_PIPELINE) );
							connection.arguments.push_back( itCurrentPipeline->second.getInputPortName(i) );
							connection.arguments.push_back( instance.name );
							connection.arguments.push_back( itCurrentPipeline->second.getInputPortName(i) );

							result.body += connection.getCode() + "\n";
						}
					}
					else
					{
						// Standard check : 
						if(itCurrentPipeline->second.getNumInputPort()>static_cast<int>(outputPortNames.size()))
							throw Exception("The pipeline " + instance.name + " has " + toString(itCurrentPipeline->second.getNumInputPort()) + " input ports while the previous element in the chain (" + lastInstance + ") has only " + toString(outputPortNames.size()) + " output ports.", sourceName, startLine, Exception::ClientScriptException);

						// Check the previous number of outputs : 
						if(isStrict && itCurrentPipeline->second.getNumInputPort()!=static_cast<int>(outputPortNames.size()))
							throw Exception("The pipeline " + instance.name + " has " + toString(itCurrentPipeline->second.getNumInputPort()) + " input ports while the previous element in the chain (" + lastInstance + ") has " + toString(outputPortNames.size()) + " output ports and the connections are specified as STRICT.", sourceName, startLine, Exception::ClientScriptException);

						// Make the connections : 
						for(int k=0; k<itCurrentPipeline->second.getNumInputPort(); k++)
						{
							VanillaParserSpace::Element connection;

							connection.strKeyword = LayoutLoader::getKeyword(KW_LL_CONNECTION);
							connection.noArgument = false;
							connection.arguments.push_back( lastInstance );
							connection.arguments.push_back( outputPortNames[k] );
							connection.arguments.push_back( instance.name );
							connection.arguments.push_back( itCurrentPipeline->second.getInputPortName(k) );

							result.body += connection.getCode() + "\n";
						}
					}

					// If last : 
					if(k==arguments.size()-2)
					{
						// Outputs : 
						VanillaParserSpace::Element outputs;

						outputs.strKeyword = LayoutLoader::getKeyword(KW_LL_OUTPUT_PORTS);

						for(int o=0; o<itCurrentPipeline->second.getNumOutputPort(); o++)
							outputs.arguments.push_back(itCurrentPipeline->second.getOutputPortName(o));
						outputs.noArgument = false;

						result.body += outputs.getCode() + "\n";

						// Make the connections : 
						for(int o=0; o<itCurrentPipeline->second.getNumOutputPort(); o++)
						{
							VanillaParserSpace::Element connection;

							connection.strKeyword = LayoutLoader::getKeyword(KW_LL_CONNECTION);
							connection.noArgument = false;
							connection.arguments.push_back( instance.name );
							connection.arguments.push_back( itCurrentPipeline->second.getOutputPortName(o) );
							connection.arguments.push_back( LayoutLoader::getKeyword(KW_LL_THIS_PIPELINE) );
							connection.arguments.push_back( itCurrentPipeline->second.getOutputPortName(o) );
							
							result.body += connection.getCode() + "\n";
						}
					}
					else
					{
						lastInstance = instance.name;

						// Update the outputs list :
						outputPortNames.clear();
						for(int o=0; o<itCurrentPipeline->second.getNumOutputPort(); o++)
							outputPortNames.push_back(itCurrentPipeline->second.getOutputPortName(o));

					}
				}

				// Return : 
				executionSource = requiredElements + "\n" + result.getCode();
			}

			LAYOUT_LOADER_MODULE_APPLY( FORMAT_TO_CONSTANT, 1, 2, -1,	"DESCRIPTION{Create a SOURCE containing a const ivec2 declaration describing the size of the texture passed in argument. For instance, can be used in a shader with : INSERT(name).}"
											"ARGUMENT:formatName{Name of the texture format to be used.}"
											"ARGUMENT:sourceName{Name of the source to be created. If not set, the name of the source will be the same as the name of the format.}")
			{
				UNUSED_PARAMETER(body)
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				const std::string generatedSourceName = (arguments.size()>=2) ? arguments[1] : arguments[0];

				FORMAT_MUST_EXIST( arguments[0] )
				SOURCE_MUST_NOT_EXIST( generatedSourceName )
	
				CONST_ITERATOR_TO_FORMAT( it, arguments[0] )

				std::string str = "const ivec2 " + generatedSourceName + " = ivec2(" + toString(it->second.getWidth()) + ", " + toString(it->second.getHeight()) + ");\n";
				// The newline is mandatory here, to avoid ShaderSource mistaking this for a filename.

				APPEND_NEW_SOURCE(generatedSourceName, ShaderSource(str, sourceName, startLine));
			}

			LAYOUT_LOADER_MODULE_APPLY(SINGLE_FILTER_PIPELINE, 2, 3, 1,	"DESCRIPTION{Create a pipeline with a single filter.}"
											"ARGUMENT:pipelineName{Name of the new pipeline.}"
											"ARGUMENT:outputTextureFormat{Name of the format to render to.}"
											"BODY_DESCRIPTION{Source of the fragment shader to be implemented.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)

				const std::string	innerFormatSuffix = "_innerFormat",
							sourceSuffix = "_source",
							filterSuffix = "_filter",
							pipelineSuffix = "_pipeline";

				PIPELINE_MUST_NOT_EXIST( arguments[0] )
				FORMAT_MUST_EXIST( arguments[1] )
				CONST_ITERATOR_TO_FORMAT( formatIterator, arguments[1] )

				VanillaParserSpace::Element 	elementRequiredFormat,
								elementSource,
								elementFilter,
								elementInputPorts,
								elementOutputPorts,
								elementFilterInstance,
								elementPipeline;

				const bool asMainPipeline = (arguments.size()>=3 && getBoolean(arguments[2], sourceName, startLine));
				const ShaderSource fragmentSource(body);

				// Required format : 
				elementRequiredFormat.sourceName= sourceName;
				elementRequiredFormat.strKeyword= LayoutLoader::getKeyword(KW_LL_REQUIRED_FORMAT);
				elementRequiredFormat.name	= arguments[1] + innerFormatSuffix;
				elementRequiredFormat.noName	= false;
				elementRequiredFormat.noArgument= false;
				elementRequiredFormat.noBody	= true;
				elementRequiredFormat.startLine	= startLine;
				elementRequiredFormat.bodyLine	= bodyLine;
				elementRequiredFormat.arguments.push_back(arguments[1]);

				// Source : 
				elementSource.sourceName	= sourceName;
				elementSource.strKeyword	= LayoutLoader::getKeyword(KW_LL_SOURCE);
				elementSource.name		= arguments[0] + sourceSuffix;
				elementSource.body		= body;
				elementSource.noName		= false;
				elementSource.noArgument	= true;
				elementSource.noBody		= false;
				elementSource.startLine		= startLine;
				elementSource.bodyLine		= bodyLine;

				// Filter : 
				elementFilter.sourceName	= sourceName;
				elementFilter.strKeyword	= LayoutLoader::getKeyword(KW_LL_FILTER_LAYOUT);
				elementFilter.name		= arguments[0] + filterSuffix;
				elementFilter.noName		= false;
				elementFilter.noArgument	= false;
				elementFilter.noBody		= true;
				elementFilter.startLine		= startLine;
				elementFilter.bodyLine		= bodyLine;
				elementFilter.arguments.push_back(arguments[1] + innerFormatSuffix);
				elementFilter.arguments.push_back(arguments[0] + sourceSuffix);

				// Pipeline body ; 
				elementInputPorts.sourceName	= sourceName;
				elementInputPorts.strKeyword	= LayoutLoader::getKeyword(KW_LL_INPUT_PORTS);
				elementInputPorts.noName	= true;
				elementInputPorts.noArgument	= false;
				elementInputPorts.noBody	= true;
				elementInputPorts.startLine	= startLine;
				elementInputPorts.bodyLine	= bodyLine;
				elementInputPorts.arguments	= fragmentSource.getInputVars();

				elementOutputPorts.sourceName	= sourceName;
				elementOutputPorts.strKeyword	= LayoutLoader::getKeyword(KW_LL_OUTPUT_PORTS);
				elementOutputPorts.noName	= true;
				elementOutputPorts.noArgument	= false;
				elementOutputPorts.noBody	= true;
				elementOutputPorts.startLine	= startLine;
				elementOutputPorts.bodyLine	= bodyLine;
				elementOutputPorts.arguments	= fragmentSource.getOutputVars();
				
				elementFilterInstance.sourceName= sourceName;
				elementFilterInstance.strKeyword= LayoutLoader::getKeyword(KW_LL_FILTER_INSTANCE);
				elementFilterInstance.name	= arguments[0] + filterSuffix;
				elementFilterInstance.noName	= false;
				elementFilterInstance.noArgument= true;
				elementFilterInstance.noBody	= true;
				elementFilterInstance.startLine	= startLine;
				elementFilterInstance.bodyLine	= bodyLine;

				// Pipeline : 
				elementPipeline.sourceName	= sourceName;
				elementPipeline.strKeyword	= asMainPipeline ? LayoutLoader::getKeyword(KW_LL_PIPELINE_MAIN) : LayoutLoader::getKeyword(KW_LL_PIPELINE_LAYOUT);
				elementPipeline.name		= arguments[0];
				elementPipeline.body		= (elementInputPorts.arguments.empty() ? "" : (elementInputPorts.getCode() + "\n")) + elementOutputPorts.getCode() + "\n" + elementFilterInstance.getCode();
				elementPipeline.noName		= false;
				elementPipeline.noArgument	= true;
				elementPipeline.noBody		= false;
				elementPipeline.startLine	= startLine;
				elementPipeline.bodyLine	= bodyLine;
	
				if(asMainPipeline)
					mainPipelineName = arguments[0];

				executionSource = elementRequiredFormat.getCode() + "\n" + elementSource.getCode() + "\n" + elementFilter.getCode() + "\n" + elementPipeline.getCode();
				executionSourceName = getName(); // Insert the name of the module to avoid confusion.
				executionStartLine = startLine;
			}	

			LAYOUT_LOADER_MODULE_APPLY( IF_GLSL_VERSION_MATCH, 1, 1, 1,	"DESCRIPTION{Test the GLSL available available during compilation.}"
											"ARGUMENT:version{The GLSL version name to be tested : 1.30, 3.30, etc.}"
											"BODY_DESCRIPTION{Contains any or all of the blocks TRUE{...} and FALSE{...}. The right block will be exectuted following this test.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(startLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)
			
				std::string	trueCase,
						falseCase;
				int	trueCaseStartLine=1,
					falseCaseStartLine=1;
				getCases(body, trueCase, trueCaseStartLine, falseCase, falseCaseStartLine, sourceName, bodyLine);
				const std::string glslVersion = HandleOpenGL::getGLSLVersion();
				const bool test = (glslVersion.find(arguments[0])!=std::string::npos);
				executionSource = test ? trueCase : falseCase;
				executionStartLine = test ? trueCaseStartLine : falseCaseStartLine;
				executionSourceName = sourceName;
			} 

			LAYOUT_LOADER_MODULE_APPLY( ABORT_ERROR, 1, 1, 0,	"DESCRIPTION{Abort the Layout loading operation with a user defined error.}"
										"ARGUMENT:error{Error description.}"
										"BODY_DESCRIPTION{Optional, More complete description of the error.}")
			{
				UNUSED_PARAMETER(currentPath)
				UNUSED_PARAMETER(dynamicPaths)
				UNUSED_PARAMETER(formatList)
				UNUSED_PARAMETER(sourceList)
				UNUSED_PARAMETER(geometryList)
				UNUSED_PARAMETER(filterList)
				UNUSED_PARAMETER(pipelineList)
				UNUSED_PARAMETER(mainPipelineName)
				UNUSED_PARAMETER(staticPaths)
				UNUSED_PARAMETER(requiredFormatList)
				UNUSED_PARAMETER(requiredSourceList)
				UNUSED_PARAMETER(requiredGeometryList)
				UNUSED_PARAMETER(requiredPipelineList)
				UNUSED_PARAMETER(moduleList)
				UNUSED_PARAMETER(bodyLine)
				UNUSED_PARAMETER(executionSource)
				UNUSED_PARAMETER(executionSourceName)
				UNUSED_PARAMETER(executionStartLine)

				Exception m("Error : " + arguments.front(), sourceName, startLine, Exception::ClientScriptException);

				if(body.empty())
					throw m;
				else
				{
					Exception e(body, sourceName, startLine, Exception::ClientScriptException);
					m << e;
					throw m;
				}
			}

