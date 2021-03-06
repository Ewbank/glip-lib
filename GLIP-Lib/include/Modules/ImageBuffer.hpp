/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. Kerviche                                                                               */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : glip-lib.net                                                                              */
/*                                                                                                               */
/*     File          : TextureReader.hpp                                                                         */
/*     Original Date : October 17th 2010                                                                         */
/*                                                                                                               */
/*     Description   : Module : Image Buffer 	                                                                 */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    ImageBuffer.hpp
 * \brief   Module : Image Buffer
 * \author  R. KERVICHE
 * \date    November 23rd 2013
**/

#ifndef __IMAGE_BUFFER_INCLUDE__
#define __IMAGE_BUFFER_INCLUDE__

	// Includes
	#include <limits>
	#include "Core/LibTools.hpp"
	#include "Core/OglInclude.hpp"
	#include "Core/HdlDynamicData.hpp"
	#include "Core/HdlTexture.hpp"

namespace Glip
{
	// Prototypes
	using namespace Glip::CoreGL;

	namespace Modules
	{
		// Structures :
/**
\class ImageBuffer
\brief Host-side image buffer.
\related PixelIterator
**/
		class GLIP_API ImageBuffer : public HdlAbstractTextureFormat
		{
			private : 
				static const unsigned int headerNumBytes;
				static const unsigned int maxCommentLength;
				static const std::string headerSignature;

				const HdlTextureFormatDescriptor&	descriptor;
				HdlDynamicTable*			table;
		
			public : 
				ImageBuffer(const HdlAbstractTextureFormat& format, int _alignment=1);
				ImageBuffer(void* buffer, const HdlAbstractTextureFormat& format, int _alignment=1);
				ImageBuffer(HdlTexture& texture, int _alignment=1);
				ImageBuffer(const ImageBuffer& image);
				~ImageBuffer(void);

				const HdlTextureFormatDescriptor& getDescriptor(void) const;
				void* getPtr(void);
				const void* getPtr(void) const;
				void* getRowPtr(int i);
				const void* getRowPtr(int i) const;
				HdlDynamicTable& getTable(void);
				const HdlDynamicTable& getTable(void) const;

				void setMinFilter(GLenum mf);
				void setMagFilter(GLenum mf);
				void setSWrapping(GLenum m);
				void setTWrapping(GLenum m);

				const ImageBuffer& operator<<(HdlTexture& texture);
				const ImageBuffer& operator<<(const ImageBuffer& image);
				const ImageBuffer& operator<<(const void* bytes);
				const ImageBuffer& operator>>(HdlTexture& texture) const;
				const ImageBuffer& operator>>(ImageBuffer& image) const;
				const ImageBuffer& operator>>(void* bytes) const;

				bool isInside(const int& x, const int& y) const;
				bool isInside(const int& x, const int& y, const GLenum& channel) const;
				int getIndex(const int& x, const int& y, const GLenum& channel) const;
				long long get(const int& x, const int& y, const GLenum& channel) const;
				void set(const long long& value, const int& x, const int& y, const GLenum& channel);
				float getNormalized(const int& x, const int& y, const GLenum& channel) const;
				void setNormalized(const float& value, const int& x, const int& y, const GLenum& channel);

				void blit(const ImageBuffer& src, const int& xSrc=0, const int& ySrc=0, const int& xDst=0, const int& yDst=0, int _width=0, int _height=0, const bool xFlip=false, const bool yFlip=false);

				static ImageBuffer* load(const std::string& filename, std::string* comment=NULL);
				void write(const std::string& filename, const std::string& comment="") const;
		};
	}
}

#endif

