/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. Kerviche                                                                               */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : glip-lib.net                                                                              */
/*                                                                                                               */
/*     File          : HdlGeBO.cpp                                                                               */
/*     Original Date : August 7th 2010                                                                           */
/*                                                                                                               */
/*     Description   : OpenGL Pixel Buffer Object Handle                                                         */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    HdlGeBO.cpp
 * \brief   OpenGL Generic handle for Buffer Objects (VBO, PBO, VAO, ...)
 * \author  R. KERVICHE
 * \date    February 6th 2011
*/

#include "Core/Exception.hpp"
#include "Core/HdlGeBO.hpp"

using namespace Glip::CoreGL;

// HdlGeBO :
	/**
	\fn HdlGeBO::HdlGeBO(const GLsizeiptr& _size, const GLenum& _target, const GLenum& _usage)
	\brief HdlGeBO Construtor.
	\param _size Size of the buffer, in bytes.
	\param infoTarget Target kind, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER.
	\param infoUsage Usage kind among GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY, GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY.
	**/
	HdlGeBO::HdlGeBO(const GLsizeiptr& _size, const GLenum& _target, const GLenum& _usage)
	 :	bufferId(0),
		target(_target),
		usage(_usage),
		size(_size)
	{
		#ifdef GLIP_USE_GL
		NEED_EXTENSION(GLEW_VERSION_1_5)
		NEED_EXTENSION(GLEW_ARB_pixel_buffer_object)
		NEED_EXTENSION(GLEW_ARB_vertex_buffer_object)
		FIX_MISSING_GLEW_CALL(glGenBuffers, glGenBuffersARB)
		FIX_MISSING_GLEW_CALL(glBindBuffer, glBindBufferARB)
		FIX_MISSING_GLEW_CALL(glBufferData, glBufferDataARB)
		FIX_MISSING_GLEW_CALL(glBufferSubData, glBufferSubDataARB)
		FIX_MISSING_GLEW_CALL(glDeleteBuffers, glDeleteBuffersARB)
		FIX_MISSING_GLEW_CALL(glMapBuffer, glMapBufferARB)
		FIX_MISSING_GLEW_CALL(glUnmapBuffer, glUnmapBufferARB)
		#endif

		// Generate the buffer :
		glGenBuffers(1, &bufferId);
		if(bufferId==0)
		{
			GLenum err = glGetError();
			throw Exception("HdlGeBO::HdlGeBO - Buffer Object can't be created. OpenGL error " + getGLEnumNameSafe(err) + " : " + getGLErrorDescription(err), __FILE__, __LINE__, Exception::GLException);
		}

		// Bind it and allocate some space :
		bind();
		glBufferData(target, size, NULL, usage);
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::HdlGeBO", "glBufferData()")
		#endif

		// Release :
		unbind();
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlGeBO::HdlGeBO - New GeBO : " << getGLErrorDescription(glGetError()) << std::endl;
		#endif
	}

	/**
	\fn HdlGeBO::HdlGeBO(const GLuint& id, const GLsizeiptr& _size, const GLenum& _target, const GLenum& _usage)
	\brief HdlGeBO Construtor.
	\param id GL id of the Buffer Object to be mapped in.
	\param _size Size of the buffer, in bytes.
	\param _target Target kind, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER.
	\param _usage  Usage kind among GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY, GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY.
	**/
	HdlGeBO::HdlGeBO(const GLuint& id, const GLsizeiptr& _size, const GLenum& _target, const GLenum& _usage)
	 :	bufferId(id),
		target(_target),
		usage(_usage),
		size(_size)
	{ }

	HdlGeBO::~HdlGeBO(void)
	{
		// Delete the object
		glDeleteBuffers(1, &bufferId);
		bufferId = 0;
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::~HdlGeBO", "glDeleteBuffers()")
		#endif
	}

	/**
	\fn GLsizeiptr HdlGeBO::getSize(void) const
	\brief Get the size of the Buffer Object.

	\return Size of the BO in bytes.
	**/
	GLsizeiptr HdlGeBO::getSize(void) const
	{
		return size;
	}

	/**
	\fn GLuint HdlGeBO::getID(void) const
	\brief Get the ID of the Buffer Object.
	\return ID of the Buffer Object
	**/
	GLuint HdlGeBO::getID(void) const
	{
		return bufferId;
	}

	/**
	\fn GLenum HdlGeBO::getTarget(void) const
	\brief Get the target of the Buffer Object.
	\return Target of the Buffer Object, among : GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER.
	**/
	GLenum HdlGeBO::getTarget(void) const
	{
		return target;
	}

	/**
	\fn GLenum HdlGeBO::getUsage(void) const
	\brief Get the usage of the Buffer Object.
	\return ID of the usage, among : GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY, GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY.
	**/
	GLenum HdlGeBO::getUsage(void) const
	{
		return usage;
	}

	/**
	\fn void HdlGeBO::bind(GLenum tgt)
	\brief Bind the Buffer Object to target.
	\param tgt The target (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER), default is the target specified for this object.
	**/
	void HdlGeBO::bind(GLenum tgt)
	{
		if(tgt==GL_NONE)
			tgt = getTarget();
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlGeBO::bind - Binding " << bufferId << " to " << getGLEnumNameSafe(tgt) << "." << std::endl;
		#endif
		glBindBuffer(tgt, bufferId);
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::bind", "glBindBuffer()")
		#endif
	}

	/**
	\fn void HdlGeBO::unbind(GLenum tgt)
	\brief Unbind the Buffer Object from target.
	\param target The target (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER), default is the target specified for this object.
	**/
	void HdlGeBO::unbind(GLenum tgt)
	{
		if(tgt==GL_NONE)
			tgt = getTarget();
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlGeBO::bind - Unbinding " << bufferId << " from " << getGLEnumNameSafe(tgt) << "." << std::endl;
		#endif
		glBindBuffer(tgt, 0);
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::unbind", "glBindBuffer()")
		#endif
	}

	/**
	\fn void* HdlGeBO::map(GLenum access, GLenum tgt)
	\brief Map the Buffer Object into the host memory.
	\param access Kind of access, among GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE, default can be used is target is GL_PIXEL_UNPACK_BUFFER or GL_PIXEL_PACK_BUFFER (will use respectively GL_WRITE_ONLY or GL_READ_ONLY). Will raise an exception otherwise.
	\param tgt Target mapping point, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER, default is the target specified for this object.
	\return A pointer in host memory.
	**/
	void* HdlGeBO::map(GLenum access, GLenum tgt)
	{
		if(tgt==GL_NONE)
			tgt = getTarget();
		if(access==GL_NONE)
		{
			switch(tgt)
			{
				case GL_PIXEL_UNPACK_BUFFER :
					access = GL_WRITE_ONLY;
					break;
				case GL_PIXEL_PACK_BUFFER :
					access = GL_READ_ONLY;
					break;
				default :
					throw Exception("HdlGeBO::map - You must provide an acces type (R/W) for target " + getGLEnumNameSafe(tgt), __FILE__, __LINE__, Exception::GLException);
			}
		}

		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlGeBO::map - Infos : " << std::endl;
			std::cout << "    glDebug : " << std::endl;
			debugGL();
		#endif
		if(isMapped(tgt))
		{
			glUnmapBuffer(tgt);
			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlGeBO::map", "glUnmapBuffer()")
			#endif
		}	

		bind(tgt);
		#ifdef GLIP_USE_GL
			void* ptr = glMapBuffer(tgt, access);
		#else
			void* ptr = glMapBufferRange(tgt, 0, 0, access);
		#endif

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::map", "glMapBuffer()")
		#endif
		return ptr;
	}

	/**
	\fn void HdlGeBO::unmap(GLenum tgt=GL_NONE)
	\brief Map the Buffer Object into the host memory.
	\param tgt Target mapping point, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_PACK_BUFFER, default is the target specified for this object.
	**/
	void HdlGeBO::unmap(GLenum tgt)
	{
		if(tgt==GL_NONE)
			tgt = getTarget();
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlGeBO::unmap : Unmapping buffer " << getID() << " from " << getGLEnumNameSafe(tgt) << std::endl;
		#endif
		glUnmapBuffer(tgt);
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::unmap", "glUnmapBuffer()")
		#endif
	}

	/**
	\fn void HdlGeBO::write(const void* data)
	\brief Write data to a Buffer Object with classical glBufferData method.
	\param data The data to write (must be the same size than the GeBO).
	**/
	void HdlGeBO::write(const void* data)
	{
		bind();
		glBufferData(getTarget(), static_cast<GLsizeiptr>(size), reinterpret_cast<const GLvoid *>(data), getUsage());
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::write", "glBufferData()")
		#endif
	}

	/**
	\fn void HdlGeBO::subWrite(const void* data, GLsizeiptr size, GLintptr offset)
	\brief Write data to a Buffer Object with classical glBufferSubData method.
	\param data The data to write.
	\param size Size, in bytes, of the subset.
	\param offset Offset to apply, in bytes, before writting.
	**/
	void HdlGeBO::subWrite(const void* data, GLsizeiptr size, GLintptr offset)
	{
		bind();
		glBufferSubData(getTarget(), offset, size,  reinterpret_cast<const GLvoid *>(data));
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::subWrite", "glBufferSubData()")
		#endif
	}


// Static tools :
	/**
	\fn GLenum HdlGeBO::getBindingEnum(const GLenum& tgt)
	\brief Get the binding enum corresponding to a given target enum.
	\param tgt The target enum, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER.
	\return The corresponding binding enum.
	\throw Throw an Exception if the enum is not a valid enum.
	**/
	GLenum HdlGeBO::getBindingEnum(const GLenum& tgt)
	{
		switch(tgt)
		{
			case GL_ARRAY_BUFFER :		return GL_ARRAY_BUFFER_BINDING;
			case GL_ELEMENT_ARRAY_BUFFER :	return GL_ELEMENT_ARRAY_BUFFER_BINDING;
			case GL_PIXEL_PACK_BUFFER :	return GL_PIXEL_PACK_BUFFER_BINDING;
			case GL_PIXEL_UNPACK_BUFFER :	return GL_PIXEL_UNPACK_BUFFER_BINDING;
			default :
				throw Exception("HdlGeBO::getBindingEnum - No binding enum corresponding to " + getGLEnumNameSafe(tgt) + ".", __FILE__, __LINE__, Exception::GLException);
		}
	}

	/**
	\fn bool HdlGeBO::isBound(const GLenum& tgt)
	\brief Test if a target is bound.
	\param tgt The target enum, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER.
	\return True if the target is bound, false otherwise.
	**/
	bool HdlGeBO::isBound(const GLenum& tgt)
	{
		GLint paramInt = GL_FALSE;
		glGetIntegerv(getBindingEnum(tgt), &paramInt);
		return paramInt!=0;
	}

	/**
	\fn bool HdlGeBO::isMapped(const GLenum& tgt)
	\brief Test if a target is mapped.
	\param tgt The target enum, among GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER.
	\return True if the target is mapped, false otherwise.
	**/
	bool HdlGeBO::isMapped(const GLenum& tgt)
	{
		GLint paramInt = GL_FALSE;
		glGetIntegerv(getBindingEnum(tgt), &paramInt);
		if(paramInt!=0)
		{
			glGetBufferParameteriv(tgt, GL_BUFFER_MAPPED, &paramInt);
			return paramInt!=GL_FALSE;
		}
		return false;
	}

	/**
	void HdlGeBO::unbindAll(void)
	\brief Unbind all the buffers.
	**/
	void HdlGeBO::unbindAll(void)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::unbindAll", "glBindBuffer()")
		#endif
	}

	/**
	\fn void HdlGeBO::unmapAll(void)
	\brief Unmap all the buffers.
	**/
	void HdlGeBO::unmapAll(void)
	{
		// We must test here to avoid GL errors :
		if(isMapped(GL_ARRAY_BUFFER))
			glUnmapBuffer(GL_ARRAY_BUFFER);
		if(isMapped(GL_ELEMENT_ARRAY_BUFFER))
			glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		if(isMapped(GL_PIXEL_UNPACK_BUFFER))
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		if(isMapped(GL_PIXEL_PACK_BUFFER))
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlGeBO::unmapAll", "glUnmapBuffer()")
		#endif
	}

