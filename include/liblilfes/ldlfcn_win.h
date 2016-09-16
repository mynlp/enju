/*  $Id: ldlfcn_win.h,v 1.1 2011-05-02 11:36:21 matuzaki Exp $
 *
 *    Copyright (c) 2011-, Tsujii-Lab 
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */


#ifndef __ldlfcn_win_h
#define __ldlfcn_win_h

#ifdef _WIN32
#include <windows.h>
#include <errno.h>

#define dlopen(P,G)		(void*)LoadLibrary(P)
#define dlsym(D,F)		(void*)GetProcAddress((HMODULE)D, F)
#define dlclose(D)		FreeLibrary((HMODULE)D)

__inline const char* dlerror()
{
	static char sMsgBuf[256];
	
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,
				  GetLastError(),
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  sMsgBuf,
				  sizeof sMsgBuf,
				  NULL);

	return sMsgBuf;
}

#endif // _WIN32

#endif // __ldlfcn_win_h
