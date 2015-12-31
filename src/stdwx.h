/*
   Copyright Notice in awvic.cpp
*/

// ***********************************************************************
// Please include this header file in every .cpp files.  
// For memory leak detecting in Visual C++ environment.
// Only for new() and delete(), not for malloc() and free().
// ***********************************************************************

#ifndef _STDWX_H
#define _STDWX_H

#ifdef __WXMSW__

#pragma warning(disable: 4996)

#ifdef _DEBUG
    #include <crtdbg.h>
    #define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
    #define new DEBUG_NEW
#else
    #define DEBUG_NEW new
#endif

#endif 

#endif
