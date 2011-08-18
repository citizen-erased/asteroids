#ifndef PH_PRECOMPILED_H
#define PH_PRECOMPILED_H

#if !(defined(WIN32) || defined(LINUX))
#error "Platform not supported. Define WIN32, or LINUX"
#endif

/*--------------------------------------------------------------------------*
 *                              Windows                                     *
 *--------------------------------------------------------------------------*/
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

/*--------------------------------------------------------------------------*
 *                                STL                                       *
 *--------------------------------------------------------------------------*/
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>


/*--------------------------------------------------------------------------*
 *                            GLEW/OpenGL                                   *
 *--------------------------------------------------------------------------*/
/* GLEW must be included before OpenGL. SFML will automatically include OpenGL */
#if 1
#include <glew/glew.h>
#endif


/*--------------------------------------------------------------------------*
 *                                enet                                      *
 *--------------------------------------------------------------------------*/
#include <enet/enet.h>


/*--------------------------------------------------------------------------*
 *                               Box2D                                      *
 *--------------------------------------------------------------------------*/
#include <Box2D/Box2D.h>

/*--------------------------------------------------------------------------*
 *                                SFML                                      *
 *--------------------------------------------------------------------------*/
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

/*--------------------------------------------------------------------------*
 *                               guichan                                    *
 *--------------------------------------------------------------------------*/
#include <guichan.hpp>

/*--------------------------------------------------------------------------*
 *                               Boost                                      *
 *--------------------------------------------------------------------------*/
#ifdef NDEBUG
#define BOOST_DISABLE_ASSERTS
#endif
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>


/*--------------------------------------------------------------------------*
 *                               Math                                       *
 *--------------------------------------------------------------------------*/
#include <cml/cml.h>


/*--------------------------------------------------------------------------*
 *                              project                                     *
 *--------------------------------------------------------------------------*/
#include "core/logger.h"
#include "core/error.h"


#endif /* PH_PRECOMPILED_H */

