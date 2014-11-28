/* ************************************************************************************************************* */
/*                                                                                                               */
/*     LIBRAW INTERFACE                                                                                          */
/*     Interface to the LibRaw library for the OpenGL Image Processing LIBrary                                   */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : LibRawInterface.pp                                                                       */
/*     Original Date : August 18th 2014                                                                          */
/*                                                                                                               */
/*     Description   : LibRaw interface for image input/output.                                                  */
/*                                                                                                               */
/* ************************************************************************************************************* */

#ifndef __GLIPLIB_LIBRAWINTERFACE__
#define __GLIPLIB_LIBRAWINTERFACE__

	#include <libraw/libraw.h>
	#include "GLIPLib.hpp"

	extern Glip::Modules::ImageBuffer* libRawLoadImage(const std::string& filename);

#endif

