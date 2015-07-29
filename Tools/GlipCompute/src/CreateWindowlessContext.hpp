/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-COMPUTE                                                                                              */
/*     Command-Line Utility for the OpenGL Image Processing LIBrary                                              */
/*                                                                                                               */
/*     Author        : R. Kerviche                                                                               */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : glip-lib.net                                                                              */
/*                                                                                                               */
/*     File          : CreateWindowlessContext.hpp                                                               */
/*     Original Date : August 18th 2014                                                                          */
/*                                                                                                               */
/*     Description   : Create window-less GL functions (still require a display system to be available).         */
/*                                                                                                               */
/* ************************************************************************************************************* */

#ifndef __GLIPCOMPUTE_CREATEWINDOWLESSCONTEXT__
#define __GLIPCOMPUTE_CREATEWINDOWLESSCONTEXT__

	#include <iostream>
	#include "GLIPLib.hpp"
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <GL/gl.h>
	#include <GL/glx.h>

	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);

	extern void createWindowlessContext(std::string displayName="");
#endif 

