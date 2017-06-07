#pragma once
#include "stdafx.h"
#include "ItunesFunction.h"
#include <string>
#include <vector>
#include <HttpHelp.h>
#include <Base64String.h>
#include "common.h"
#include "ItunesProtocolCommon.h"
#include <AutoCriticalSection.h>

using namespace std;

class CItunesInterface
{
public:
//    CItunesInterface &getInstance()
//    {
//        static CItunesInterface * pInter = NULL;
//        if (pInter == NULL)
//            pInter = new CItunesInterface();
//        
//        return *pInter;
//    }
//private:
    CItunesInterface() :
        m_pcriti_lock(NULL),
        m_ulNum(0),
        m_ulNum2(0)
    {
    }
    ~CItunesInterface()
    {}
private:
    ULONG getNum2_Proxy()
    {
        VMPBegin("getNum2_Proxy");
        return m_ulNum2;
        VMPEnd();
    }
    ULONG getNum_Proxy()
    {
        VMPBegin("getNum_Proxy");
        return m_ulNum;
        VMPEnd();
    }
public:
    static CString getItunesInstallPath()
    {
        //USES_CONVERSION;

        //HMODULE hModule = GetModuleHandle(_T("itunes.exe"));
        //TCHAR szPath[MAX_PATH] = { 0 };
        //GetModuleFileName(hModule, szPath, MAX_PATH);
        //PathRemoveFileSpec(szPath);
        //return szPath;

        return _T("C:\\Program Files (x86)\\iTunes");
    }
    static CString getItunesDLLPath()
    {
        CString strInstallPath = getItunesInstallPath();
        PathAppend(strInstallPath.GetBuffer(MAX_PATH), _T("itunes.dll"));
        strInstallPath.ReleaseBuffer();

        return strInstallPath;
    }
    static bool CleanItunesCacheFile(bool bDelADI, bool bDelSC)
    {
        VMPBegin("CleanItunesCacheFile");

        TCHAR pathApple[MAX_PATH] = { 0 };
        SHGetSpecialFolderPath(NULL, pathApple, CSIDL_COMMON_APPDATA, FALSE);
        PathAppend(pathApple, _T("\\Apple Computer\\iTunes"));

        TCHAR pathFile[MAX_PATH] = { 0 };
        if (bDelADI)
        {
            _tcscpy(pathFile, pathApple);
            PathAppend(pathFile, _T("adi\\adi.pb"));
            DeleteFile(pathFile);
        }

        if (bDelSC)
        {
            _tcscpy(pathFile, pathApple);
            PathAppend(pathFile, _T("SC Info\\SC Info.sidb"));
            DeleteFile(pathFile);

            _tcscpy(pathFile, pathApple);
            PathAppend(pathFile, _T("SC Info\\SC Info.sidn"));
            DeleteFile(pathFile);

            _tcscpy(pathFile, pathApple);
            PathAppend(pathFile, _T("SC Info\\SC Info.sidd"));
            DeleteFile(pathFile);
        }

        return true;

        VMPEnd();
    }
    static BOOL InitItunes()
    {
        CString szPath;
        szPath = _T("%PATH%;");
        szPath += _T("C:\\Program Files (x86)\\Common Files\\Apple\\Apple Application Support;");
        szPath += _T("C:\\Program Files (x86)\\iTunes;");
        szPath += _T("C:\\Program Files (x86)\\Common Files\\Apple\\CoreFP");

        SetEnvironmentVariable(_T("PATH"), szPath);

        if (NULL == LoadLibrary(_T("CoreFoundation.dll")) ||
            NULL == LoadLibrary(_T("iTunes.dll"))
            )
        {
            return FALSE;
        }

        SetCurrentDirectory(_T("C:\\Program Files (x86)\\iTunes"));

        if (FALSE == Init())
            return FALSE;

        return TRUE;
    }
    //all itunes interface proxy stuff 
    static LPCSTR getString_Proxy(void * /*CFStringRef*/ str)
    {
        VMPBegin("getString_Proxy");
        return pfn_CFStringGetCStringPtr(str, 0x500);
        VMPEnd();
    }
    static bool getGUID_Proxy(string &strguid)
    {
        VMPBegin("getGUID_Proxy");

        void* allocator = pfn_CFGetAllocator(NULL);
        if (!allocator) return false;

        void *ref = pfn_get_guid(allocator);//dot forget call pfn_CFRelease
        if (!ref) return false;

        strguid = getString_Proxy(ref);
        pfn_CFRelease(ref);

        return true;

        VMPEnd();
    }
    void enumDict_Proxy(void *cf)
    {
        VMPBegin("enumDict_Proxy");
        USES_CONVERSION;

        int count = pfn_CFDictionaryGetCount(cf);
        PVOID *pkeys = new PVOID[count];
        if (!pkeys)
        {
            DBG_TEXT_PUT(_T("error new pkeys buffer\n"));
            return;
        }
        pfn_CFDictionaryGetKeysAndValues(cf, pkeys, 0);
        for (int i = 0; i < count; i++)
        {
            void *value_ref = (void *)pfn_CFDictionaryGetValue(cf, pkeys[i]);
            int nType = pfn_CFGetTypeID(value_ref);
            if (nType == pfn_CFStringGetTypeID())
            {
                DBG_TEXT_PUT(_T("======================key=value========================\n"));
                if (pfn_CFStringGetTypeID() == pfn_CFGetTypeID(pkeys[i]))
                    DBG_TEXT_PUT(_T("%s"), _CA2T(pfn_CFStringGetCStringPtr(pkeys[i], 0x500)));
                DBG_TEXT_PUT(_T("%s"), _CA2T(pfn_CFStringGetCStringPtr(value_ref, 0x500)));
            }
            else if (nType == pfn_CFDictionaryGetTypeID())
            {
                enumDict_Proxy(value_ref);
            }
            else
            {
                TCHAR szBuffer[100] = { 0 };
                _sntprintf_s(szBuffer, 100, _T("unknown type : %d\n"), nType);
                DBG_TEXT_PUT(szBuffer);
            }
        }
        delete[] pkeys;

        VMPEnd();
    }
    bool getKbsync_Proxy(int type, const string &strDsid, string &strKbsync)
    {
        VMPBegin("getKbsync_Proxy");

        USES_CONVERSION;

        ULONG num2 = getNum2_Proxy();
        char *pend = NULL;
        ULONGLONG ullDsid = strtoull(strDsid.c_str(), &pend, 10);
        LPBYTE pout = NULL;
        ULONG outLen = 1;
        int err = pfn_kbsync(num2, ullDsid, 0, type, &pout, &outLen);
        if (err != 0)
        {
            DBG_TEXT_PUT(_T("getKbsync_Proxy err %d \n"), err);
            return false;
        }

        int nEncodeBuffer = NeedEncodeSize(outLen);
        char *pEncodeBuffer = (char *)malloc(nEncodeBuffer);
        if (NULL == pEncodeBuffer)
        {
            DBG_TEXT_PUT(_T("getKbsync_Proxy malloc err %d"), nEncodeBuffer);
            if (pout)
                pfn_Release_Mem(pout);
            return false;
        }

        memset(pEncodeBuffer, 0, nEncodeBuffer);
        Base64Encode(pEncodeBuffer, (const unsigned char *)pout, outLen);
        strKbsync = pEncodeBuffer;
        free(pEncodeBuffer);

        if (pout)
            pfn_Release_Mem(pout);

        DBG_TEXT_PUT(_T("getKbsync_Proxy return 0"));
        DBG_TEXT_PUT(_T("getKbsync_Proxy return %s"), _CA2T(strKbsync.c_str()));

        return true;
        VMPEnd();
    }
    bool setKeybag_Proxy(const string &str_keybag)
    {
        VMPBegin("setKeybag_Proxy");

        ULONG num2 = getNum2_Proxy();

        size_t nDecodeBuffer = NeedDecodeSize(str_keybag.length());
        unsigned char *pDecodeBuffer = (unsigned char *)malloc(nDecodeBuffer);
        if (NULL == pDecodeBuffer)
        {
            DBG_TEXT_PUT(_T("setKeybag_Proxy malloc err"));
            return false;
        }
        memset(pDecodeBuffer, 0, nDecodeBuffer);
        size_t nSize = Base64Decode(pDecodeBuffer, str_keybag.c_str(), str_keybag.length());

        int err = pfn_setkeybag(num2, pDecodeBuffer, nSize);
        //fix leaked bug
        free(pDecodeBuffer);
        DBG_TEXT_PUT(_T("setKeybag_Proxy return %d"), err);

        return err == 0 ? true : false;
        VMPEnd();
    }
    bool getActionSignature_Proxy(unsigned char *pBody, size_t nBody, string &strActionSignature)
    {
        VMPBegin("getActionSignature_Proxy");

        USES_CONVERSION;

        LPBYTE pout = NULL;
        ULONG outLen = 0;
        int err = pfn_ActionSignature(getNum_Proxy(), pBody, nBody, &pout, &outLen);
        if (err != 0 || pout == NULL)
        {
            DBG_TEXT_PUT(_T("getActionSignature_Proxy err return : %d"), err);
            return false;
        }

        int nEncodeBuffer = NeedEncodeSize(outLen);
        char *pEncodeBuffer = (char *)malloc(nEncodeBuffer);
        if (!pEncodeBuffer)
        {
            DBG_TEXT_PUT(_T("getActionSignature_Proxy malloc err"));
            pfn_Release_Mem(pout);
            return false;
        }

        memset(pEncodeBuffer, 0, nEncodeBuffer);
        Base64Encode(pEncodeBuffer, pout, outLen);
        strActionSignature = pEncodeBuffer;
        free(pEncodeBuffer);

        pfn_Release_Mem(pout);

        DBG_TEXT_PUT(_T("getActionSignature_Proxy return size : %d"), outLen);
        DBG_TEXT_PUT(_T("getActionSignature_Proxy return X-Apple-ActionSignature : %s"), _CA2T(strActionSignature.c_str()));

        return true;

        VMPEnd();
    }
    bool initNum2_Proxy()
    {
        VMPBegin("initNum2_Proxy");

        ULONG pstuff0[6] = { 0 };
        ULONG pstuff[6] = { 0 };
        bool bErr = false;

        //enter
        __asm
        {
            pushad
            pushfd
            lea edi, pstuff0
            call pfn_cert_stuff_for_get_object_0
            mov bErr, al
            popfd
            popad
        }
        if (!bErr)
            return false;

        if (!pfn_cert_stuff_for_get_object(pstuff))
            return false;
        //leave

        const char scinfo_path[] = "C:\\ProgramData\\Apple Computer\\iTunes\\SC Info";
        int err = pfn_cert_stuff_for_get_num2(pstuff0, pstuff, scinfo_path, &m_ulNum2);//这里得到的num2是否存在时效性？
        if (err != 0)
            return false;

        return true;

        VMPEnd();
    }
    bool initCert_Proxy(const string &str_Proxy,
        const string &str_UserAgent,
        const string &str_AppleTz,
        const string &str_AppleStoreFront)
    {
        VMPBegin("initCert_Proxy");

        string strCert;
        int err = ItunesProtocolCommon::send_signSapSetupCert_pack(str_Proxy, str_UserAgent,
            str_AppleTz, str_AppleStoreFront, strCert);
        if (err <= 0)
            return false;

        size_t nDecodeBuffer = 0; 
        unsigned char *pDecodeBuffer = NULL;
        if (!debase64(strCert, &pDecodeBuffer, &nDecodeBuffer))
            return false;

        ULONG pstuff[6] = { 0 };
        pfn_cert_stuff_for_get_object(pstuff);
        ULONG *pobject = new ULONG[2];//mem fake
        pobject[0] = 0;
        pobject[1] = 0;
        err = pfn_cert_stuff_get_object(pstuff, pobject);//这个对象是单实例对象？这个对象里面存放num和临界区
        if (err != 0)
        {
            if (pDecodeBuffer)
                free(pDecodeBuffer);

            return false;
        }
        ULONG *plock = (ULONG *)pobject[0];
        m_pcriti_lock = (PCRITICAL_SECTION)&plock[3];
        m_ulNum = plock[0x24 / 4];

        LPBYTE pout = NULL;
        ULONG outLen = 0;
        ULONG flag = 1;
        ///
        //FILE *fpp = fopen("C:\\Users\\John\\Desktop\\cert1_new_der.cer", "rb");
        //if (fpp == NULL)
        //    return 0;
        //fseek(fpp, 0, SEEK_END);
        //long len1 = ftell(fpp);
        //fseek(fpp, 0, SEEK_SET);
        //unsigned char *p1 = (unsigned char *)malloc(len1);
        //fread(p1, 1, len1, fpp);
        //fclose(fpp);

        //fpp = fopen("C:\\Users\\John\\Desktop\\cert2_new_der.cer", "rb");
        //if (fpp == NULL)
        //    return 0;
        //fseek(fpp, 0, SEEK_END);
        //long len2 = ftell(fpp);
        //fseek(fpp, 0, SEEK_SET);
        //unsigned char *p2 = (unsigned char *)malloc(len2);
        //fread(p2, 1, len2, fpp);
        //fclose(fpp);

        //int total = len1 + len2 + 2 + 4 + 4;
        //unsigned char *p3 = (unsigned char *)malloc(total);
        //p3[0] = 1;
        //p3[1] = 2;
        //*(long *)&p3[2] = htonl(len1);
        //memcpy((long *)&p3[2 + 4], p1, len1);
        //*(long *)&p3[2 + 4 + len1] = htonl(len2);
        //memcpy((long *)&p3[2 + 4 + 4 + len1], p2, len2);
        ///

        //FILE *fpp = fopen("C:\\Users\\John\\Desktop\\apple.p7b", "rb");
        //if (fpp == NULL)
        //    return 0;
        //fseek(fpp, 0, SEEK_END);
        //long len4 = ftell(fpp);
        //fseek(fpp, 0, SEEK_SET);
        //unsigned char *p4 = (unsigned char *)malloc(len4);
        //fread(p4, 1, len4, fpp);
        //fclose(fpp);

        //FILE *fpp = fopen("C:\\Users\\John\\Desktop\\cert1_1_der.cer", "rb");
        //if (fpp == NULL)
        //    return 0;
        //fseek(fpp, 0, SEEK_END);
        //long len1 = ftell(fpp);
        //fseek(fpp, 0, SEEK_SET);
        //unsigned char *p1 = (unsigned char *)malloc(len1+2+4);
        //p1[0] = 1; p1[1] = 1; *(long *)&p1[2] = htonl(len1);
        //fread(p1+2+4, 1, len1, fpp);
        //fclose(fpp);
        ///
        //0x621AA76C - 0x61430000 = 0xD7A76C

        unsigned char pubroot[] = { 
//0xe5, 0xb7, 0x7a, 0xf4, 0x40, 0x48, 0x6a, 0x42, 0xd4, 0xa6, 0xaa, 0x68, 0x3f, 0xa8, 0xf6,
//0xa6, 0x5e, 0x9e, 0xee, 0xc6, 0x5f, 0x78, 0xb2, 0xc6, 0xfa, 0x44, 0x87, 0x60, 0x34, 0x45,
//0x40, 0xc4, 0x28, 0xc2, 0xe0, 0x76, 0x9f, 0xa2, 0xa7, 0xd3, 0xd2, 0xe3, 0x4a, 0x4e, 0xfd,
//0xfc, 0xc6, 0x57, 0x6a, 0x8e, 0xd7, 0x40, 0x29, 0x6c, 0xd7, 0xf4, 0x08, 0x3a, 0x86, 0xe8,
//0x92, 0xd9, 0x74, 0x44, 0x9a, 0x16, 0xca, 0x86, 0xd5, 0x63, 0x0f, 0x15, 0x62, 0x33, 0xee,
//0xfa, 0x59, 0x97, 0xf2, 0xf1, 0x87, 0xa4, 0x34, 0x96, 0xb4, 0xa6, 0x08, 0x3f, 0x34, 0x1b,
//0x90, 0x89, 0x35, 0x23, 0x64, 0xb8, 0xce, 0x91, 0xd5, 0x21, 0x11, 0x65, 0xb2, 0x4a, 0x14,
//0x4b, 0xdf, 0x0b, 0xa7, 0xb7, 0xbf, 0x6f, 0x83, 0xdb, 0xd1, 0xc7, 0x65, 0xcd, 0xc9, 0x0a,
//0xd6, 0x30, 0x02, 0xde, 0x4b, 0x42, 0x13, 0x4b, 0xf3, 0x4b, 0x0d, 0x2b, 0x20, 0x4b, 0x0d,
//0xb9, 0xa2, 0xd9, 0xfc, 0xc9, 0xdb, 0xd6, 0x23, 0x7b, 0xc6, 0xb1, 0x7e, 0x4d, 0xc1, 0xeb,
//0x07, 0xf7, 0x36, 0xd4, 0x14, 0x5b, 0xd2, 0xc7, 0xbc, 0xbd, 0xed, 0xed, 0xdd, 0x95, 0xc0,
//0x8d, 0x56, 0x15, 0xaf, 0xc5, 0x7d, 0x78, 0x14, 0x15, 0xbe, 0x12, 0xa4, 0x4b, 0x1e, 0x47,
//0xbc, 0xf8, 0x74, 0x5f, 0xcb, 0x58, 0xba, 0x54, 0x21, 0x20, 0x68, 0xb9, 0x52, 0xbf, 0x27,
//0x0a, 0xd6, 0x53, 0x0b, 0x86, 0x77, 0x9a, 0x6f, 0x0a, 0x6b, 0xac, 0x9f, 0x14, 0x0f, 0x61,
//0x64, 0xfb, 0xd3, 0x1e, 0x2b, 0xc8, 0xf1, 0x63, 0x1e, 0x43, 0x76, 0xbf, 0xcc, 0x0c, 0x87,
//0x50, 0x8d, 0x4c, 0x29, 0xb3, 0x24, 0x56, 0xfc, 0x1a, 0x24, 0xba, 0xa6, 0x64, 0x9e, 0x51,
//0x87, 0x42, 0x55, 0x8a, 0x6b, 0x29, 0x02, 0x23, 0xb0, 0x65, 0x7f, 0x6c, 0x43, 0x79, 0x5d,
//0xcf 
            0xfc, 0x33, 0x08, 0x4d, 0x90, 0xf8, 0xb0, 0x9b, 0x83, 0xbb, 0x06, 0xac, 0x91, 0x53,
            0x44, 0x41, 0xec, 0x5e, 0x1e, 0x49, 0x8b, 0xc0, 0x7a, 0x79, 0xcf, 0xf9, 0xa7, 0x19, 0x00,
            0xc7, 0xdc, 0xcc, 0x6f, 0x45, 0x8d, 0x81, 0xde, 0x38, 0x54, 0x67, 0xb3, 0xc0, 0x20, 0x9c,
            0xa8, 0xac, 0xe6, 0x11, 0x61, 0x9a, 0xc3, 0xa8, 0x16, 0xa1, 0x4d, 0x5b, 0x5c, 0x3a, 0x99,
            0x25, 0xde, 0xb4, 0x58, 0x04, 0xf1, 0x51, 0x47, 0x7d, 0x0b, 0x0f, 0x22, 0x17, 0xa2, 0x59,
            0x9c, 0xd4, 0xd0, 0x72, 0x13, 0x6c, 0x0a, 0xbb, 0xeb, 0x12, 0xf0, 0x7a, 0xa2, 0x01, 0x2d,
            0x5f, 0xf0, 0x47, 0xd3, 0x0f, 0x76, 0xe1, 0x20, 0xed, 0xa3, 0xe7, 0x1e, 0x30, 0xc9, 0x6b,
            0x6a, 0x78, 0x70, 0x5b, 0x92, 0xc3, 0xd0, 0x23, 0x98, 0x2d, 0x2d, 0x2c, 0x8a, 0x83, 0x51,
            0xf9, 0x0d, 0xd8, 0x9e, 0x44, 0x96, 0xe4, 0x2d, 0x1d, 0x57, 0xf9, 0x33, 0x4c, 0xc0, 0x9d,
            0xbe, 0x4e, 0x2d, 0xd3, 0x53, 0x5f, 0x1a, 0x09, 0x99, 0x8f, 0x62, 0x0e, 0xd9, 0x3a, 0x8e,
            0x80, 0x13, 0x1e, 0x0d, 0xf3, 0x11, 0x71, 0x39, 0xbc, 0x29, 0xc1, 0xa5, 0x9e, 0x7a, 0xaa,
            0x10, 0x4b, 0x74, 0xf9, 0xa1, 0x3b, 0x66, 0xa9, 0x3e, 0xf8, 0x6b, 0x40, 0x36, 0x80, 0x3b,
            0x0c, 0xef, 0x13, 0x0b, 0x54, 0xea, 0x86, 0xd5, 0x57, 0x40, 0xe2, 0xd4, 0xf3, 0x8e, 0x3d,
            0x12, 0xa7, 0xad, 0xf4, 0x57, 0x7e, 0x5a, 0xc8, 0xf8, 0x76, 0x6d, 0x0c, 0xf4, 0x0e, 0xb5,
            0x57, 0x02, 0x41, 0x67, 0x63, 0xe1, 0x36, 0xc6, 0x90, 0x2f, 0x64, 0xd1, 0x90, 0x97, 0x18,
            0x8c, 0xb5, 0x36, 0x81, 0x5e, 0x0e, 0x67, 0x71, 0x73, 0xe1, 0xea, 0x03, 0x42, 0xf8, 0x48,
            0xa4, 0x34, 0x79, 0xe2, 0x5d, 0x8b, 0xb3, 0x41, 0x3a, 0xda, 0xdc, 0x10, 0xaf, 0xd2, 0xd4,
            0xc8, 0xa7
        };

        //HMODULE hMod = GetModuleHandle(_T("CoreFP.dll"));
        //unsigned char *ppubkey = (unsigned char *)((0x621AA76C - 0x61430000) + (ULONG)hMod);
        //memcpy(ppubkey, pubroot, 0x100);
        err = pfn_cert_init(200, pstuff, m_ulNum, pDecodeBuffer, nDecodeBuffer, &pout, &outLen, &flag);
        free(pDecodeBuffer);
        if (err != 0 || flag != 1)
            return false;

        string strCertInBuffer;
        if (!base64(pout, outLen, strCertInBuffer))
            return false;

        string strCertOutBuffer;
        err = ItunesProtocolCommon::send_signSapSetup_pack(str_Proxy, str_UserAgent,
            str_AppleTz, str_AppleStoreFront, strCertInBuffer, strCertOutBuffer);
        if (err <= 0)
        {
            return false;
        }

        nDecodeBuffer = 0;
        pDecodeBuffer = NULL;
        if (!debase64(strCertOutBuffer, &pDecodeBuffer, &nDecodeBuffer))
            return false;

        pout = NULL; outLen = 0; flag = 1;
        err = pfn_cert_init(200, pstuff, m_ulNum, pDecodeBuffer, nDecodeBuffer, &pout, &outLen, &flag);
        free(pDecodeBuffer);
        if (err == 0 && flag == 0)
            return true;

        return false;

        VMPEnd();
    }
private:
    PCRITICAL_SECTION m_pcriti_lock;
    ULONG m_ulNum;
    ULONG m_ulNum2;
public:
    static CAutoCriticalSection sm_autolock;
};