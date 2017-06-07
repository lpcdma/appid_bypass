// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0500

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#define _CRTDBG_MAP_ALLOC
#define _ATL_DISABLE_NOTHROW_NEW
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlapp.h>
#define _WTL_NO_CSTRING
#include <atlmisc.h>
#include <atlstr.h>
#include <atlutil.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <ShlObj.h>
#include <stdio.h>
#include <WinSock2.h>

#define _MY_DEBUG
#define TRACE_LOG_FILE _T("D:\\trace.log")
#include <mytrace.h>
#include <ByteArray.h>

#define _CA2T(X) ((LPCTSTR)CA2T(X))
//#define DBG_TEXT_PUT(format, ...) TRACE_LOG_INFO(format, __VA_ARGS__)
#define DBG_TEXT_PUT(format, ...) CTraceHelper(_T(__FILE__), _T(__FUNCTION__), __LINE__, TRACE_LOG_FILE, CTraceHelper::FORMATS_TIME_PROCESS)(CTraceHelper::DBG_OUT_LOGFILE, format, __VA_ARGS__)

#ifndef _DEBUG
#include <VMProtectSDK.h>
#define VMPBegin(s) VMProtectBegin(s)
#define VMPEnd() VMProtectEnd()
#else
#define VMPBegin(s) 
#define VMPEnd() 
#endif

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

extern CAppModule _Module;

#define MAIN_CAPTION _T("appid auto bypass，write by debugee，qq519306795")