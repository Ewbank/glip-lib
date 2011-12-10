/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : GPLv3                                                                                     */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : HdlFBO.hpp                                                                                */
/*     Original Date : August 7th 2010                                                                           */
/*                                                                                                               */
/*     Description   : OpenGL FrameBuffer Object Handle                                                          */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    HdlFBO.hpp
 * \brief   OpenGL FrameBuffer Object Handle
 * \author  R. KERVICHE
 * \version 0.6
 * \date    August 7th 2010
*/

#ifndef __HDLFBO_INCLUDE__
#define __HDLFBO_INCLUDE__

    // Includes
        #include <vector>
        #include "./HdlTexture.hpp"
        #include "OglInclude.hpp"

namespace Glip
{
    namespace CoreGL
    {

        // Constants
            #define NO_ATTACHMENT (-1)

        // FBO Handle
            /**
             \class HdlFBO
             \brief Object handle for OpenGL Frame Buffer Objects
            **/
        class HdlFBO : public __ReadOnly_HdlTextureFormat
        {
            private :
                // Data
                    std::vector<HdlTexture*> targets;
                    GLuint                   fboID;
                // Tools
                    void bindTextureToFBO(int i);
                    void unbindTextureFromFBO(int i);
                    /*void clearList(void);
                    void addTolist(GLenum attach);
                    void removeFromList(GLenum attach);*/
            public :
                // Tools
                    HdlFBO(const __ReadOnly_HdlTextureFormat& f, int numTarget = 1);
                    ~HdlFBO(void);

                    int         addTarget(void);
                    int         getAttachmentCount(void) const;

                    void        beginRendering(void);
                    void        endRendering(void);
                    void        bind(void);
                    HdlTexture* operator[](int i);

                // Static tools :
                    static int    getMaximumColorAttachment(void);
                    static GLenum getAttachment(int i);
                    static int    getIndexFromAttachment(GLenum attachment);
                    static void   unbind(void);
        };
    }
}

#endif

