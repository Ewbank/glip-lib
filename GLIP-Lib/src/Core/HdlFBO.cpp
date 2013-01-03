/* ***************************************************************************************************************/
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : HdlFBO.cpp                                                                                */
/*     Original Date : August 7th 2010                                                                           */
/*                                                                                                               */
/*     Description   : OpenGL FrameBuffer Object Handle                                                          */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    HdlFBO.cpp
 * \brief   OpenGL FrameBuffer Object Handle
 * \author  R. KERVICHE
 * \date    August 7th 2010
*/

#include "HdlFBO.hpp"
#include "./Exception.hpp"

using namespace Glip::CoreGL;

// Functions
	/**
	\fn    HdlFBO::HdlFBO(const __ReadOnly_HdlTextureFormat& f, int numTarget)
	\brief HdlFBO Construtor.

	\param f Format of the textures attached to the rendering point.
	\param numTarget Number of targets to be built by the constructor.
	**/
	HdlFBO::HdlFBO(const __ReadOnly_HdlTextureFormat& f, int numTarget)
	 : __ReadOnly_HdlTextureFormat(f)
	{
		NEED_EXTENSION(GL_ARB_framebuffer_object)
		FIX_MISSING_GLEW_CALL(glGenFramebuffers, glGenFramebuffersEXT)
		FIX_MISSING_GLEW_CALL(glBindFramebuffer, glBindFramebufferEXT)
		FIX_MISSING_GLEW_CALL(glDeleteFramebuffers, glDeleteFramebuffersEXT)
		FIX_MISSING_GLEW_CALL(glFramebufferTexture2D, glFramebufferTexture2DEXT)
		FIX_MISSING_GLEW_CALL(glDrawBuffers, glDrawBuffersARB)
		FIX_MISSING_GLEW_CALL(glGenerateMipmap, glGenerateMipmapEXT)

		if(isCompressed())
			throw Exception("HdlFBO::HdlFBO - Cannot render to compressed texture of format : " + glParamName(getGLMode()) + ".", __FILE__, __LINE__);

		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "HdlFBO::HdlFBO - Disabling Depth buffer." << std::endl;
		#endif

		glDisable(GL_DEPTH_TEST);

		glGenFramebuffers(1, &fboID);

		if(fboID==0)
			throw Exception("HdlFBO::HdlFBO - FBO can't be created. Last OpenGL error : " + glErrorToString(), __FILE__, __LINE__);
		else
		{
			#ifdef __GLIPLIB_TRACK_GL_ERRORS__
				OPENGL_ERROR_TRACKER("HdlFBO::HdlFBO", "glGenFramebuffers(1, &fboID)")
			#endif
		}


		for(int i=0; i<numTarget; i++) // At least one!
			addTarget();

		// check FBO status
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); //unbind
	}

	HdlFBO::~HdlFBO(void)
	{
		// Remove all textures :
		for(unsigned int i=0; i<targets.size(); i++)
			unbindTextureFromFBO(i);

		glFlush();

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::~HdlFBO", "glFlush()")
		#endif

		glDeleteFramebuffers( 1, &fboID);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::~HdlFBO", "glDeleteFramebuffers()")
		#endif

		// Delete all textures :
		for(std::vector<HdlTexture*>::iterator it=targets.begin(); it!=targets.end(); it++)
			delete (*it);
	}

	void HdlFBO::bindTextureToFBO(int i)
	{
		// Entering safe zone :
		glGetError();

		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fboID);

		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, getAttachment(i), GL_TEXTURE_2D, targets[i]->getID(), 0);

		GLenum err = glGetError();
		if(err!=GL_NO_ERROR)
			throw Exception("HdlFBO::bindTextureToFBO - Texture can't be bound to the FBO, its format might be incompatible. (OpenGL error : " + glParamName(err) + ").", __FILE__, __LINE__);
	}

	void HdlFBO::unbindTextureFromFBO(int i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fboID);

		glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, getAttachment(i), GL_TEXTURE_2D, 0, 0);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::unbindTextureFromFBO", "glFramebufferTexture2D()")
		#endif
	}

	/**
	\fn    int HdlFBO::addTarget(void)
	\brief Add a new texture to target list.
	**/
	int HdlFBO::addTarget(void)
	{
		if(targets.size()>=getMaximumColorAttachment())
			throw Exception("HdlFBO::addTarget - Can't add more target, limit reached : " + to_string(getMaximumColorAttachment()) + " textures.", __FILE__, __LINE__);
		else
		{
			int i = targets.size();
			targets.push_back(new HdlTexture(*this));
			targets[i]->fill(0);
			bindTextureToFBO(i);
			return i;
		}
	}

	/**
	\fn    int HdlFBO::getAttachmentCount(void) const
	\brief Return the number of attachment points.

	\return The number of attached textures.
	**/
	int HdlFBO::getAttachmentCount(void) const
	{
		return targets.size();
	}

	/**
	\fn    void HdlFBO::beginRendering(int usedTarget)
	\param usedTarget The number of targets to be used. It must be less or equal to getAttachmentCount().
	\brief Prepare the FBO for rendering in it
	**/
	void HdlFBO::beginRendering(int usedTarget)
	{
		static const GLenum attachmentsList[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_COLOR_ATTACHMENT4_EXT,
							GL_COLOR_ATTACHMENT5_EXT, GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT, GL_COLOR_ATTACHMENT8_EXT, GL_COLOR_ATTACHMENT9_EXT,
							GL_COLOR_ATTACHMENT10_EXT, GL_COLOR_ATTACHMENT11_EXT, GL_COLOR_ATTACHMENT12_EXT, GL_COLOR_ATTACHMENT13_EXT,GL_COLOR_ATTACHMENT14_EXT, GL_COLOR_ATTACHMENT15_EXT};


		if(usedTarget==0)
			usedTarget = targets.size();

		if(usedTarget>targets.size())
			throw Exception("HdlFBO::beginRendering - Can't render to " + to_string(usedTarget) + " textures because the current number of targets is " + to_string(targets.size()), __FILE__, __LINE__);

		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fboID);

		/*/ Set up color_tex and depth_rb for render-to-texture
		Useless : glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, assTex.getID(), 0);
		Useless : glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboID);*/

		glDrawBuffers(usedTarget, attachmentsList);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::beginRendering", "glDrawBuffers()")
		#endif

		// Save viewport configuration
		glPushAttrib(GL_VIEWPORT_BIT);

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::beginRendering", "glPushAttrib()")
		#endif

		// Create a new viewport configuration
		glViewport(0,0,getWidth(),getHeight());
	}

	/**
	\fn    void HdlFBO::endRendering(void)
	\brief End the rendering in the FBO, build the mipmaps for the target.
	**/
	void HdlFBO::endRendering(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); // unbind

		// trigger mipmaps generation explicitly
		for(std::vector<HdlTexture*>::iterator it=targets.begin(); it!=targets.end(); it++)
		{
			if((*it)->getMaxLevel()>0)
			{
				glBindTexture(GL_TEXTURE_2D, (*it)->getID());

				glGenerateMipmap(GL_TEXTURE_2D);

				#ifdef __GLIPLIB_TRACK_GL_ERRORS__
					OPENGL_ERROR_TRACKER("HdlFBO::endRendering", "glGenerateMipmap()")
				#endif
			}
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// restore viewport setting
		glPopAttrib();

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::endRendering", "glPopAttrib()")
		#endif
	}

	/**
	\fn    HdlTexture* HdlFBO::operator[](int i)
	\brief Get a target of the FBO
	\param i Index of the target.
	\return Pointer to the corresponding target.
	**/
	HdlTexture* HdlFBO::operator[](int i)
	{
		if(i<0 || i>targets.size())
			throw Exception("HdlFBO::operator[] - Invalid index : " + to_string(i) + " of " + to_string(targets.size()), __FILE__, __LINE__);
		else
			return targets[i];
	}

	/**
	\fn    void HdlFBO::bind(void)
	\brief Bind this FBO.
	**/
	void HdlFBO::bind(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fboID);
	}

	/**
	\fn     int HdlFBO::getSize(void)
	\brief  Gets the size of the FBO in bytes.
	\param  askDriver If true, it will use HdlTexture::getSizeOnGPU() to determine the real size (might be slower).
	\return The size in byte of the multiple targets.
	**/
	int HdlFBO::getSize(bool askDriver)
	{
		if(!askDriver)
			return getAttachmentCount() * __ReadOnly_HdlTextureFormat::getSize();
		else
			if(getAttachmentCount()>0)
				return getAttachmentCount() * targets.front()->getSizeOnGPU();
			else
				return 0;
	}

	/**
	\fn     int HdlFBO::getMaximumColorAttachment(void)
	\brief  Get the maximum number of attachment points.
	\return The maximum number of attachment points.
	**/
	int HdlFBO::getMaximumColorAttachment(void)
	{
		GLint maxAttachments;

		glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxAttachments );

		#ifdef __GLIPLIB_TRACK_GL_ERRORS__
			OPENGL_ERROR_TRACKER("HdlFBO::getMaximumColorAttachment", "glGetIntegerv()")
		#endif

		return maxAttachments;
	}

	/**
	\fn    GLenum HdlFBO::getAttachment(int i)
	\brief Convert an attachment ID to the corresponding OpenGL constant.
	\param i The ID of the attachment point.
	\return The corresponding OpenGL constant.
	**/
	GLenum HdlFBO::getAttachment(int i)
	{
		return GL_COLOR_ATTACHMENT0_EXT + i;
	}

	/**
	\fn    int HdlFBO::getIndexFromAttachment(GLenum attachment)
	\brief Convert an OpenGL constant to the corresponding attachment ID.
	\param attachment The OpenGL constant representing the attachment point.
	\return The corresponding ID.
	**/
	int HdlFBO::getIndexFromAttachment(GLenum attachment)
	{
		return attachment - GL_COLOR_ATTACHMENT0_EXT;
	}

	/**
	\fn    void HdlFBO::unbind(void)
	\brief Unbind the FBO which is currently bound to the OpenGL context.
	**/
	void HdlFBO::unbind(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0); //unbind
	}

