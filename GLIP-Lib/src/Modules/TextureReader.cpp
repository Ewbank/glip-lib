/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : TextureReader.cpp                                                                         */
/*     Original Date : October 17th 2010                                                                         */
/*                                                                                                               */
/*     Description   : Module : Texture Reader                                                                   */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    TextureReader.cpp
 * \brief   Module : Texture Reader
 * \author  R. KERVICHE
 * \date    October 17th 2010
**/

	#include<limits>
	#include "TextureReader.hpp"
	#include "../Core/Exception.hpp"

	using namespace Glip;
	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;
	using namespace Glip::Modules;

	/**
	\fn TextureReader::TextureReader(const std::string& name, const __ReadOnly_HdlTextureFormat& format)
	\brief TextureReader constructor.
	\param name Name of the component.
	\param format Format expected.
	**/
	TextureReader::TextureReader(const std::string& name, const __ReadOnly_HdlTextureFormat& format)
	 : __ReadOnly_ComponentLayout(name), OutputDevice(name), __ReadOnly_HdlTextureFormat(format), xFlip(false), yFlip(false)
	{
		if(isCompressed())
			throw Exception("TextureReader::TextureReader - Can not read directly compressed textures with TextureReader (for " + getNameExtended() + ").", __FILE__, __LINE__);

		addInputPort("input");

		originalPointer = new char[getSize()];
		data = reinterpret_cast<void*>(originalPointer);

		// Make sure to unpack correctly ; 
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}

	TextureReader::~TextureReader(void)
	{
		delete[] originalPointer;
		data = NULL;
	}

	void TextureReader::process(void)
	{
		HdlTexture& texture = in();

		if(!isCompatibleWith(texture))
			throw Exception("TextureReader::process - Can not read texture having different layout format (for " + getNameExtended() + ").", __FILE__, __LINE__);

		texture.bind();

		GLenum alias = getGLMode();
		if(alias==GL_RGB32F) alias = GL_RGB;
		if(alias==GL_RGBA32F) alias = GL_RGBA;

		glGetTexImage(GL_TEXTURE_2D, 0, alias, getGLDepth(), reinterpret_cast<GLvoid*>(data));
		//glGetTexImage(GL_TEXTURE_2D, 0, getGLMode(), getGLDepth(), reinterpret_cast<GLvoid*>(data));

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("TextureReader::process", "glGetTexImage()")
		#endif

		HdlTexture::unbind();
	}

	/**
	\fn double TextureReader::operator()(int x, int y, int c)
	\brief Read data in the texture (after calling << on a texture object reference).
	\param x The X position.
	\param y The y position.
	\param c The channel to be read (0,1,2[,3]).
	\return A normalized intensity (in [0.0;1.0]), except for floatting point textures.
	**/
	double TextureReader::operator()(int x, int y, int c)
	{
		if(x<0 || x>=getWidth() || y<0 || y>=getHeight() || c<0 || c>=getNumChannels())
			throw Exception("TextureReader::operator() - Pixel " + to_string(x) + "x" + to_string(y) + "x" + to_string(c) + " is out of bound (" + to_string(getWidth()) + "x" + to_string(getHeight()) + "x" + to_string(getNumChannels()) + ").", __FILE__, __LINE__);

		if(xFlip) x = getWidth()-x-1;
		if(yFlip) y = getHeight()-y-1;
		int pos = (y*getWidth()+x)*getNumChannels()+c;
		double res = 0.0;

		GLenum d = getGLDepth();

		if(d==GL_BYTE)
		{
			char* s = reinterpret_cast<char*>(data);
			res = static_cast<double>(s[pos])/static_cast<double>(std::numeric_limits<char>::max());
		}

		if(d==GL_UNSIGNED_BYTE)
		{
			unsigned char* s = reinterpret_cast<unsigned char*>(data);
			res = static_cast<double>(s[pos])/static_cast<double>(std::numeric_limits<unsigned char>::max());
		}

		if(d==GL_SHORT)
		{
			short* s = reinterpret_cast<short*>(data);
			res = static_cast<double>(s[pos])/static_cast<double>(std::numeric_limits<short>::max());
		}

		if(d==GL_UNSIGNED_SHORT)
		{
			unsigned short* s = reinterpret_cast<unsigned short*>(data);
			res = static_cast<double>(s[pos])/static_cast<double>(std::numeric_limits<unsigned short>::max());
		}

		if(d==GL_INT)
		{
			int* s = reinterpret_cast<int*>(data);
			res = static_cast<double>(s[pos])/static_cast<double>(std::numeric_limits<int>::max());
		}

		if(d==GL_UNSIGNED_INT)
		{
			unsigned int* s = reinterpret_cast<unsigned int*>(data);
			res = static_cast<double>(s[pos])/static_cast<double>(std::numeric_limits<unsigned int>::max());
		}

		if(d==GL_FLOAT)
		{
			float* s = reinterpret_cast<float*>(data);
			res = static_cast<double>(s[pos]);
		}

		if(d==GL_DOUBLE)
		{
			double* s = reinterpret_cast<double*>(data);
			res = s[pos];
		}

		return res;
	}

// PBOTextureReader
	/**
	\fn PBOTextureReader::PBOTextureReader(const std::string& name, const __ReadOnly_HdlTextureFormat& format, GLenum freq)
	\brief PBOTextureReader constructor.
	\param name Name of the component.
	\param format Format expected.
	\param freq The frequency (GL_STATIC_READ_ARB, GL_DYNAMIC_READ_ARB, GL_STREAM_READ_ARB).
	**/
	PBOTextureReader::PBOTextureReader(const std::string& name, const __ReadOnly_HdlTextureFormat& format, GLenum freq)
	 : __ReadOnly_ComponentLayout(name), OutputDevice(name), __ReadOnly_HdlTextureFormat(format), HdlPBO(format.getWidth(), format.getHeight(), format.getNumChannels(), format.getChannelDepth(), GL_PIXEL_PACK_BUFFER_ARB, freq)
	{
		if(isCompressed())
			throw Exception("PBOTextureReader::PBOTextureReader - Can not read directly compressed textures with PBOTextureReader (for " + getNameExtended() + ").", __FILE__, __LINE__);
		addInputPort("input");

		// Make sure to unpack correctly ; 
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}

	PBOTextureReader::~PBOTextureReader(void)
	{
		endReadingMemory();
	}

	void PBOTextureReader::process(void)
	{
		HdlTexture& texture = in();

		if(!isCompatibleWith(texture))
			throw Exception("PBOTextureReader::process - Can not read texture having different layout format (for " + getNameExtended() + ").", __FILE__, __LINE__);

		texture.bind();

		// Bind this PBO
		bind();

		glGetTexImage(GL_TEXTURE_2D, 0, getGLMode(), getGLDepth(), 0);
	}

	/**
	\fn void* PBOTextureReader::startReadingMemory(void)
	\brief Read memory from previouly copied texture.
	\return A pointer to the CPU memory (Host memory) where the image is stored. You must check that it is not NULL before using.
	**/
	void* PBOTextureReader::startReadingMemory(void)
	{
		return map();
	}

	/**
	\fn void PBOTextureReader::endReadingMemory(void)
	\brief Stop reading memory from previouly copied texture, it releases the PBO from mapping.
	**/
	void PBOTextureReader::endReadingMemory(void)
	{
		HdlPBO::unmap(getTarget());
		HdlPBO::unbind(getTarget());
	}

	/**
	\fn CompressedTextureReader::CompressedTextureReader(const std::string& name, const __ReadOnly_HdlTextureFormat& format)
	\brief CompressedTextureReader constructor.
	\param name Name of the component.
	\param format Format expected (must be a compressed format, will raise an exception otherwise).
	**/
	CompressedTextureReader::CompressedTextureReader(const std::string& name, const __ReadOnly_HdlTextureFormat& format)
	 : __ReadOnly_ComponentLayout(name), OutputDevice(name), __ReadOnly_HdlTextureFormat(format), data(NULL), size(0)
	{
		if(!isCompressed())
			throw Exception("CompressedTextureReader::CompressedTextureReader - Can only read compressed texture format (for " + getNameExtended() + ").", __FILE__, __LINE__);

		addInputPort("input");

		// Make sure to unpack correctly ; 
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}

	CompressedTextureReader::~CompressedTextureReader(void)
	{
		delete[] data;
		data = NULL;
	}

	void CompressedTextureReader::process(void)
	{
		HdlTexture& texture = in();

		if(!isCompatibleWith(texture))
			throw Exception("CompressedTextureReader::process - Can not read texture having different layout format (for " + getNameExtended() + ").", __FILE__, __LINE__);

		// Create buffer if it is the first time this function is called :
		if(data==NULL)
		{
			size = texture.getSizeOnGPU();
			data = new char[size];
		}

		texture.bind();

		glGetCompressedTexImage(GL_TEXTURE_2D, 0, reinterpret_cast<GLvoid*>(data));

		HdlTexture::unbind();
	}

	/**
	\fn int CompressedTextureReader::getSize(void) const
	\brief Get the size, in bytes, of the texture and the output buffer. You must read a texture at least once before calling this function.
	\return Size of the buffer in bytes.
	**/
	int CompressedTextureReader::getSize(void) const
	{
		if(data==NULL)
			throw Exception("CompressedTextureReader::getSize - Read at least one texture before calling this function (for " + getNameExtended() + ").", __FILE__, __LINE__);
		else
			return size;
	}

	/**
	\fn char* CompressedTextureReader::getData(void) const
	\brief Get pointer to the buffer storing the compressed image. You must read a texture at least once before calling this function.
	\return Pointer to the buffer.
	**/
	char* CompressedTextureReader::getData(void) const
	{
		if(data==NULL)
			throw Exception("CompressedTextureReader::getData - Read at least one texture before calling this function (for " + getNameExtended() + ").", __FILE__, __LINE__);
		else
			return data;
	}

	/**
	\fn char& CompressedTextureReader::operator[](int i)
	\brief Get reference to an element of the buffer storing the compressed image. You must read a texture at least once before calling this function.
	\param i Index of the element.
	\return Reference to an element of the buffer.
	**/
	char& CompressedTextureReader::operator[](int i)
	{
		if(data==NULL)
			throw Exception("CompressedTextureReader::operator[] - Read at least one texture before calling this function (for " + getNameExtended() + ").", __FILE__, __LINE__);
		else
			return *(data+i);
	}

	/**
	\fn TextureCopier::TextureCopier(const std::string& name, const __ReadOnly_HdlTextureFormat& formatIn, const __ReadOnly_HdlTextureFormat& formatOut, bool _customTexture)
	\brief TextureCopier constructor.
	\param name Name of the component.
	\param formatIn Format expected as input (can be uncompressed or compressed format).
	\param formatOut Output format (can be uncompressed or compressed format).
	\param _customTexture If set to true, no texture will be created by this object and user must provide the correct target via TextureCopier::provideTexture() (custom mode).
	**/
	TextureCopier::TextureCopier(const std::string& name, const __ReadOnly_HdlTextureFormat& formatIn, const __ReadOnly_HdlTextureFormat& formatOut, bool _customTexture)
	 : __ReadOnly_ComponentLayout(name), OutputDevice(name), __ReadOnly_HdlTextureFormat(formatOut), targetTexture(NULL), pbo(NULL), customTexture(_customTexture)
	{
		if(!( formatIn.isCorrespondingCompressedFormat(formatOut) || formatOut.isCorrespondingCompressedFormat(formatIn) || formatIn.isCompatibleWith(formatOut)))
			throw Exception("TextureCopier::TextureCopier - Can not read texture having different layout format (uncompressed/compressed format accepted though) in " + getNameExtended() + ".", __FILE__, __LINE__);

		if(!formatIn.isCompressed())
			pbo = new HdlPBO(formatIn, GL_PIXEL_PACK_BUFFER_ARB, GL_STREAM_COPY_ARB);

		if(!customTexture)
			targetTexture = new HdlTexture(formatOut);

		addInputPort("input");

		// Make sure to unpack correctly ; 
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}

	TextureCopier::~TextureCopier(void)
	{
		if(!customTexture)
			delete targetTexture;
		delete pbo;
	}

	void TextureCopier::process(void)
	{
		HdlTexture& texture = in();

		if(!( texture.isCorrespondingCompressedFormat(*this) || isCorrespondingCompressedFormat(texture) || isCompatibleWith(texture) ))
			throw Exception("TextureCopier::process - Can not read texture having different layout format (for copier " + getNameExtended() + ").", __FILE__, __LINE__);

		if(targetTexture==NULL)
			if(!customTexture)
				throw Exception("TextureCopier::process - Internal error : texture wasn't properly created for " + getNameExtended() + ".", __FILE__, __LINE__);
			else
				throw Exception("TextureCopier::process - A custom texture was declared bu not given for " + getNameExtended() + ".", __FILE__, __LINE__);

		int tsize = texture.getSizeOnGPU();
		GLint inputMode = texture.getInternalMode();

		if(pbo==NULL)
			pbo = new HdlPBO(texture.getWidth(), texture.getHeight(), texture.getNumChannels(), texture.getChannelDepth(), GL_PIXEL_PACK_BUFFER_ARB, GL_STREAM_COPY_ARB, tsize);

		texture.bind();

		// Bind this PBO
		pbo->bindAsPack();

		if(texture.isCompressed())
			glGetCompressedTexImage(GL_TEXTURE_2D, 0, 0);
		else
			glGetTexImage(GL_TEXTURE_2D, 0, texture.getGLMode(), texture.getGLDepth(), 0);

		HdlPBO::unbind(GL_PIXEL_PACK_BUFFER);

		pbo->bindAsUnpack();
		targetTexture->bind();

		if(texture.isCompressed())
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, inputMode, getWidth(), getHeight(), 0, tsize, 0);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, targetTexture->getGLMode(), texture.getWidth(), texture.getHeight(), 0, texture.getGLMode(), texture.getGLDepth(), 0);

		HdlTexture::unbind();

		HdlPBO::unbind(GL_PIXEL_UNPACK_BUFFER);
	}

	/**
	\fn bool TextureCopier::isUsingCustomTargetTexture(void) const
	\brief Text if the mode is custom mode or not. In custom mode, the user provide the target and thus can swap targets easily for this copier.
	\return True if the mode is custom mode.
	**/
	bool TextureCopier::isUsingCustomTargetTexture(void) const
	{
		return customTexture;
	}

	/**
	\fn void TextureCopier::provideTexture(HdlTexture* texture)
	\brief Give the target texture to be used. Will change the mode to custom texture mode. If the texture is not of the required format it will raise an exception.
	\param texture A pointer to a valid HdlTexture object (passing NULL will raise an exception).
	**/
	void TextureCopier::provideTexture(HdlTexture* texture)
	{
		if(!targetTexture->isCompatibleWith(*this))
			throw Exception("TextureCopier::process - The texture given to " + getNameExtended() + " as an incompatible format.", __FILE__, __LINE__);

		if(!customTexture)
			delete targetTexture;

		customTexture = true;

		targetTexture = texture;
	}

	/**
	\fn HdlTexture& TextureCopier::texture(void)
	\brief Access the output texture.
	\return Reference to the output texture.
	**/
	HdlTexture& TextureCopier::texture(void)
	{
		return *targetTexture;
	}

