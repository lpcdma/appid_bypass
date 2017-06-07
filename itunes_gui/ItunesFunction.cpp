#include "stdafx.h"
#include "ItunesFunction.h"

PFN_CFGetAllocator pfn_CFGetAllocator = NULL;
PFN_CFRelease pfn_CFRelease = NULL;
PFN_CFStringGetCStringPtr pfn_CFStringGetCStringPtr = NULL;
PFN_CFStringGetTypeID pfn_CFStringGetTypeID = NULL;
PFN_CFGetTypeID pfn_CFGetTypeID = NULL;

PFN_CFDictionaryGetTypeID pfn_CFDictionaryGetTypeID = NULL;
PFN_CFDictionaryGetKeysAndValues pfn_CFDictionaryGetKeysAndValues = NULL;
PFN_CFDictionaryGetCount pfn_CFDictionaryGetCount = NULL;
PFN_CFDictionaryGetValue pfn_CFDictionaryGetValue = NULL;

PFN_cert_stuff_for_get_object_0 pfn_cert_stuff_for_get_object_0 = NULL;
PFN_cert_stuff_for_get_num2 pfn_cert_stuff_for_get_num2 = NULL;
PFN_cert_stuff_for_get_object pfn_cert_stuff_for_get_object = NULL;
PFN_cert_stuff_get_object pfn_cert_stuff_get_object = NULL;
PFN_cert_init pfn_cert_init = NULL;
PFN_get_guid pfn_get_guid = NULL;
PFN_ActionSignature pfn_ActionSignature = NULL;
PFN_Release_Mem pfn_Release_Mem = NULL;
PFN_kbsync pfn_kbsync = NULL;
PFN_setkeybag pfn_setkeybag = NULL;


BOOL Init()
{
    VMPBegin("Init");

    HMODULE hModCF = GetModuleHandle(_T("CoreFoundation.dll"));
    if (!hModCF) return FALSE;

    pfn_CFStringGetCStringPtr = (PFN_CFStringGetCStringPtr)GetProcAddress(hModCF, "CFStringGetCStringPtr");
    if (!pfn_CFStringGetCStringPtr) return FALSE;

    pfn_CFGetAllocator = (PFN_CFGetAllocator)GetProcAddress(hModCF, "CFGetAllocator");
    if (!pfn_CFGetAllocator) return FALSE;

    pfn_CFRelease = (PFN_CFRelease)GetProcAddress(hModCF, "CFRelease");
    if (!pfn_CFRelease) return FALSE;

    pfn_CFStringGetTypeID = (PFN_CFStringGetTypeID)GetProcAddress(hModCF, "CFStringGetTypeID");
    if (!pfn_CFStringGetTypeID) return FALSE;

    pfn_CFGetTypeID = (PFN_CFGetTypeID)GetProcAddress(hModCF, "CFGetTypeID");
    if (!pfn_CFGetTypeID) return FALSE;

    pfn_CFDictionaryGetTypeID = (PFN_CFDictionaryGetTypeID)GetProcAddress(hModCF, "CFDictionaryGetTypeID");
    if (!pfn_CFDictionaryGetTypeID) return FALSE;

    pfn_CFDictionaryGetKeysAndValues = (PFN_CFDictionaryGetKeysAndValues)GetProcAddress(hModCF, "CFDictionaryGetKeysAndValues");
    if (!pfn_CFDictionaryGetKeysAndValues) return FALSE;

    pfn_CFDictionaryGetCount = (PFN_CFDictionaryGetCount)GetProcAddress(hModCF, "CFDictionaryGetCount");
    if (!pfn_CFDictionaryGetCount) return FALSE;

    pfn_CFDictionaryGetValue = (PFN_CFDictionaryGetValue)GetProcAddress(hModCF, "CFDictionaryGetValue");
    if (!pfn_CFDictionaryGetValue) return FALSE;

//    HMODULE hModit = GetModuleHandle(CItunesInterface::getItunesDLLPath());
    HMODULE hModit = GetModuleHandle(_T("Itunes.dll"));
    if (!hModit) return FALSE;

    ULONG_PTR ulFuncAddr = (ULONG_PTR)hModit + (0x6F9DD170 - 0x6F7B0000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_cert_stuff_for_get_object_0 = (PFN_cert_stuff_for_get_object_0)ulFuncAddr;
    if (!pfn_cert_stuff_for_get_object_0) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + (0x707763D0 - 0x6F7B0000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_cert_stuff_for_get_num2 = (PFN_cert_stuff_for_get_num2)ulFuncAddr;
    if (!pfn_cert_stuff_for_get_num2) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + 0x22D290;
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_cert_stuff_for_get_object = (PFN_cert_stuff_for_get_object)ulFuncAddr;
    if (!pfn_cert_stuff_for_get_object) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + 0x6B05F0;
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_cert_stuff_get_object = (PFN_cert_stuff_get_object)ulFuncAddr;
    if (!pfn_cert_stuff_get_object) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + 0xFC0C70;
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_cert_init = (PFN_cert_init)ulFuncAddr;
    if (!pfn_cert_init) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + (0x6DFED4F0 - 0x6DDC0000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_get_guid = (PFN_get_guid)ulFuncAddr;
    if (!pfn_get_guid) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + (0x71166080 - 0x70190000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_ActionSignature = (PFN_ActionSignature)ulFuncAddr;
    if (!pfn_ActionSignature) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + (0x7114FD90 - 0x70190000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_Release_Mem = (PFN_Release_Mem)ulFuncAddr;
    if (!pfn_Release_Mem) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + (0x6EAA9EF0 - 0x6DAD0000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_kbsync = (PFN_kbsync)ulFuncAddr;
    if (!pfn_kbsync) return FALSE;

    ulFuncAddr = (ULONG_PTR)hModit + (0x6FE07800 - 0x6EE40000);
    ATLASSERT(IsBadReadPtr((void *)ulFuncAddr, 1) == 0);
    pfn_setkeybag = (PFN_setkeybag)ulFuncAddr;
    if (!pfn_setkeybag) return FALSE;

    return TRUE;

    VMPEnd();
}