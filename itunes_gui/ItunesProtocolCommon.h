#pragma once
#include "stdafx.h"
#include <json\json.h>
#include <json\json_link.h>
#include <tinyxml.h>
#include <tinyxml_link.h>
#include <string>
#include <map>
#include <time.h>
#include <HttpHelp.h>
#include "common.h"

#import "c:\windows\syswow64\winhttp.dll"
#import "c:\Windows\syswow64\msscript.ocx"

using namespace std;
using namespace WinHttp;
using namespace MSScriptControl;



class CItunesUserInfo
{
public:
    CItunesUserInfo() :
        m_serverid("38")
    {
    }
public:
    string m_dsid;
    string m_serverid;
    string m_appid;
    string m_password;
};

class CItunesDeviceInfo
{
public:
    CItunesDeviceInfo()
    {
        //m_machine = makeComputerName();
        //m_guid = makePhoneGUID();
    }
    static string makePhoneGUID()
    {
        LARGE_INTEGER Tick;
        QueryPerformanceCounter(&Tick);
        srand(Tick.LowPart);

        string str_guid;
        for (int j = 0; j < 40; j++)
        {
            int n = rand() % 16;
            char format[2] = { 0 };
            sprintf(format, "%x", n);
            str_guid += format;
        }
        return str_guid;
    }
    static string makeItunesGUID()
    {
        LARGE_INTEGER Tick;
        QueryPerformanceCounter(&Tick);
        srand(Tick.LowPart);
        
        string str_guid;
        for (int i = 0; i < 7; i++)
        {
            string str_item;
            for (int j = 0; j < 8; j++)
            {
                int n = rand() % 16;
                char format[2] = { 0 };
                sprintf(format, "%X", n);
                str_item += format;
            }
            str_guid += ".";
            str_guid += str_item;
        }
        
        return str_guid.substr(1);
    }
    static string makeComputerName()
    {        
        LARGE_INTEGER Tick;
        QueryPerformanceCounter(&Tick);
        srand(Tick.LowPart);

        const char str_rand[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string str_ComputerName;
        for (int i = 0; i < 10; i++)
        {
            int n = rand() % 26;
            str_ComputerName += str_rand[n];
        }

        return str_ComputerName;
    }
    static string getComputerName()
    {
        USES_CONVERSION;

        TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
        DWORD dwChrs = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName(szComputerName, &dwChrs);
        return T2A(szComputerName);
    }
    static string changeItunesGUID()
    {
        USES_CONVERSION;

        string str_processor_rand = makeRandString(10);

        CRegKey key;
        key.Open(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"));//uac
        key.SetStringValue(_T("ProcessorNameString"), _CA2T(str_processor_rand.c_str()));

        return str_processor_rand;
    }
public:
    string m_machine;
    string m_guid;
};

class CItunesHttpHeaderInfo
{
public:
    CItunesHttpHeaderInfo()
    {
        //m_AppleStoreFront = "143441-1,17 ab:vD3DtoH2";
        //m_AppleStoreFront = "143465-19,26 ab:vD3DtoH0";
        m_AppleStoreFront = "143465-19,17";
        m_AppleTz = "28800";
        m_ContentType = "application/x-apple-plist";
        m_AppleClientVersion = "GameCenter/2.0";//X-Apple-Client-Versions: GameCenter/2.0
        m_UserAgent = "AppStore/2.0 iOS/8.1.3 model/iPhone6,2 build/12B466 (6; dt:90)";//"iTunes/11.2.2 (Windows; Microsoft Windows 7 x64 Business Edition Service Pack 1 (Build 7601)) AppleWebKit/537.60.15";//
        m_ApplePartner = "origin.0";//X-Apple-Partner: origin.0
        m_AppleConnectionType = "WiFi";
        m_AppleClientApplication = "Software";
    }
public:
    string m_UserAgent;
    string m_ContentType;
    string m_AppleTz;
    string m_AppleStoreFront;
    string m_AppleClientVersion;
    string m_ApplePartner;
    string m_AppleConnectionType;
    string m_AppleClientApplication;
};

class CItunesSession : 
    public CItunesUserInfo, 
    public CItunesDeviceInfo,
    public CItunesHttpHeaderInfo
{
public:
    string getProxyString()
    {
        if (m_ProxyServer.empty())
            return "";
        return m_ProxyServer + ":" + m_ProxyPort;
    }
public:
    string m_ProxyServer;
    string m_ProxyPort;
    string m_xtoken;
    map<string, string> m_cookie;
    string m_failureType;
    string m_customerMessage;
};

class CMapItem
{
public:
    CMapItem(string key, string value) :
        m_strKey(key),
        m_strValue(value)
    {

    }
    ~CMapItem()
    {

    }
    string m_strKey;
    string m_strValue;
};

#pragma pack(push, 1)
typedef struct _OTP_LIST
{
    ULONGLONG ulldsid;
    ULONG ulmidSize;
    ULONG ulotpSize;
    UCHAR *pMidBytes;
    UCHAR *pOtpBytes;
}OTP_LIST, *POPT_LIST;
#pragma pack(pop)

class CAUTH_MID_OTP
{
public:
    CAUTH_MID_OTP(string dsid, string mid, string otp) :
        m_str_dsid(dsid),
        m_str_mid(mid),
        m_str_otp(otp)
    {

    }
    ~CAUTH_MID_OTP()
    {

    }
    string m_str_dsid;
    string m_str_mid;
    string m_str_otp;
};


class ItunesProtocolCommon
{
public:
    static bool findElementByValue(TiXmlNode *root, const string &strValue, TiXmlElement **ppElemment)
    {
        for (TiXmlNode* node = root->FirstChild(); node; node = node->NextSibling())
        {
            if (findElementByValue(node, strValue, ppElemment))
                return true;

            if (node->ToText())
            {
                TiXmlText *nodeText = node->ToText();
                if (string(nodeText->Value()) == strValue)
                {
                    if (nodeText->Parent())
                    {
                        *ppElemment = nodeText->Parent()->ToElement();
                        if (*ppElemment)
                            return true;
                    }
                }
            }
        }

        return false;
    }
    static bool getAppleXmlValueByKey(TiXmlElement *root, const string &strKey, string &strValue)
    {
        USES_CONVERSION;

        TiXmlElement* pElement = NULL;
        bool bfind = findElementByValue(root, strKey, &pElement);
        if (!bfind || pElement == NULL)
        {
            DBG_TEXT_PUT(_T("can't find %s"), _CA2T(strKey.c_str()));
            return false;
        }

        TiXmlNode* nodeValue = pElement->NextSibling();
        if (!nodeValue->ToElement())
        {
            DBG_TEXT_PUT(_T("can't find %s nodeValue"), _CA2T(strKey.c_str()));
            return false;
        }
        TiXmlElement *ElementValue = nodeValue->ToElement();
        if (ElementValue->GetText() == NULL)
        {
            strValue = "";
        }
        else
        {
            strValue = ElementValue->GetText();
        }
        

        DBG_TEXT_PUT(_T("find apple xml key %s = %s"), _CA2T(strKey.c_str()), _CA2T(strValue.c_str()));

        return true;
    }
    static string get_request_xml(vector<CMapItem> &plist)
    {
        VMPBegin("get_request_xml");
        TiXmlDocument doc;

        TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "UTF-8", "");
        doc.LinkEndChild(dec);

        TiXmlElement *root = new TiXmlElement("plist");
        root->SetAttribute("version", "1.0");
        doc.LinkEndChild(root);

        TiXmlElement *dict = new TiXmlElement("dict");
        root->LinkEndChild(dict);

        for (vector<CMapItem>::iterator iter = plist.begin(); iter != plist.end(); iter++)
        {
            TiXmlElement *name = new TiXmlElement(iter->m_strKey.c_str());
            TiXmlText *value = new TiXmlText(iter->m_strValue.c_str());
            name->LinkEndChild(value);
            dict->LinkEndChild(name);
        }

        TiXmlPrinter printer;
        doc.Accept(&printer);

        return printer.CStr();
        VMPEnd();
    }
    static bool set_otp_list(TiXmlNode *root, vector<CAUTH_MID_OTP> &otplist)
    {
        VMPBegin("set_otp_list");
        for (vector<CAUTH_MID_OTP>::iterator otpiter = otplist.begin(); otpiter != otplist.end(); otpiter++)
        {
            TiXmlNode *dict = new TiXmlElement("dict");
            root->LinkEndChild(dict);

            TiXmlElement *name = new TiXmlElement("key");
            TiXmlText *value = new TiXmlText("dsid");
            name->LinkEndChild(value);
            dict->LinkEndChild(name);

            name = new TiXmlElement("integer");
            value = new TiXmlText(otpiter->m_str_dsid.c_str());
            name->LinkEndChild(value);
            dict->LinkEndChild(name);

            name = new TiXmlElement("key");
            value = new TiXmlText("mid");
            name->LinkEndChild(value);
            dict->LinkEndChild(name);

            name = new TiXmlElement("string");
            value = new TiXmlText(otpiter->m_str_mid.c_str());
            name->LinkEndChild(value);
            dict->LinkEndChild(name);

            name = new TiXmlElement("key");
            value = new TiXmlText("otp");
            name->LinkEndChild(value);
            dict->LinkEndChild(name);

            name = new TiXmlElement("string");
            value = new TiXmlText(otpiter->m_str_otp.c_str());
            name->LinkEndChild(value);
            dict->LinkEndChild(name);

        }
        VMPEnd();
        return true;
    }
    static string get_request_xml(vector<CMapItem> &plist, vector<CAUTH_MID_OTP> &otplist)
    {
        VMPBegin("get_request_xml_2");
        TiXmlDocument doc;

        TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "UTF-8", "");
        doc.LinkEndChild(dec);

        TiXmlElement *root = new TiXmlElement("plist");
        root->SetAttribute("version", "1.0");
        doc.LinkEndChild(root);

        TiXmlElement *dict = new TiXmlElement("dict");
        root->LinkEndChild(dict);

        for (vector<CMapItem>::iterator iter = plist.begin(); iter != plist.end(); iter++)
        {
            TiXmlElement *name = new TiXmlElement(iter->m_strKey.c_str());
            TiXmlNode *value = NULL;
            if (iter->m_strKey == "array")
            {
                set_otp_list(name, otplist);
                dict->LinkEndChild(name);
                continue;
            }

            value = new TiXmlText(iter->m_strValue.c_str());

            name->LinkEndChild(value);
            dict->LinkEndChild(name);
        }

        TiXmlPrinter printer;
        doc.Accept(&printer);

        return printer.CStr();
        VMPEnd();
    }
    static int send_signSapSetup_pack(const string &str_Proxy,
        const string &str_UserAgent,
        const string &str_AppleTz,
        const string &str_AppleStoreFront,
        const string &str_cert_in_buffer,
        string &str_setup_out_buffer)
    {
        VMPBegin("send_signSapSetup_pack");

        USES_CONVERSION;

        vector<CMapItem> plist;
        plist.push_back(CMapItem("key", "sign-sap-setup-buffer"));
        plist.push_back(CMapItem("data", str_cert_in_buffer));
        string strAuthBody = get_request_xml(plist);

        CoInitialize(NULL);
        IWinHttpRequestPtr winhttp;
        string strXmlResponse;

        try
        {
            string url = "https://play.itunes.apple.com/WebObjects/MZPlay.woa/wa/signSapSetup";

            winhttp.CreateInstance(L"WinHttp.WinHttpRequest.5.1");
            winhttp->Open("POST", url.c_str(), false);
            if (!str_Proxy.empty())
                winhttp->SetProxy(2, str_Proxy.c_str());
            winhttp->SetRequestHeader("User-Agent", str_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", str_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", str_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("Content-Type", "application/x-apple-plist");
            winhttp->SetRequestHeader("Accept-Encoding", "gzip");
            //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
            winhttp->Send(strAuthBody.c_str());

            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("setup cert buffer response status : %d err"), winhttp->Status);
                return 0;
            }

            ByteArray bytesResp;
            UncompResponse(winhttp->ResponseBody, 1, bytesResp);
            char *p = bytesResp.ToString();
            strXmlResponse = p;
            free(p);

        }
        catch (_com_error err)
        {
            DBG_TEXT_PUT(_T("setup cert buffer except err : %s"), (LPCTSTR)err.Description());
            return 0;
        }

        TiXmlDocument xmldoc;
        xmldoc.Parse(strXmlResponse.c_str());
        if (xmldoc.Error())
        {
            DBG_TEXT_PUT(_T("setup cert buffer response xml parse err %s"), _CA2T(xmldoc.ErrorDesc()));
            return 0;
        }

        TiXmlElement* root = xmldoc.RootElement();
        if (NULL == root)
        {
            DBG_TEXT_PUT(_T("setup cert buffer response xml parse root err"));
            return 0;
        }
        if (!getAppleXmlValueByKey(root, "sign-sap-setup-buffer", str_setup_out_buffer))
        {
            DBG_TEXT_PUT(_T("setup cert buffer response find sign-sap-setup-buffer err"));
            return -1;
        }

        return 1;
        VMPEnd();
    }
    static int send_signSapSetupCert_pack(const string &str_Proxy,
        const string &str_UserAgent,
        const string &str_AppleTz,
        const string &str_AppleStoreFront,
        string &str_Cert)
    {
        VMPBegin("send_signSapSetupCert_pack");

        USES_CONVERSION;

        CoInitialize(NULL);
        IWinHttpRequestPtr winhttp;
        string strXmlResponse;

        try
        {
            string url = "https://init.itunes.apple.com/WebObjects/MZInit.woa/wa/signSapSetupCert";

            winhttp.CreateInstance(L"WinHttp.WinHttpRequest.5.1");
            winhttp->Open("GET", url.c_str(), false);
            if (!str_Proxy.empty())
                winhttp->SetProxy(2, str_Proxy.c_str());
            winhttp->SetRequestHeader("User-Agent", str_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", str_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", str_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("Accept-Encoding", "gzip");
            //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
            winhttp->Send();
            

            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("setup cert response status : %d err"), winhttp->Status);
                return 0;
            }
            ByteArray bytesResp;
            UncompResponse(winhttp->ResponseBody, 1, bytesResp);
            char *p = bytesResp.ToString();
            strXmlResponse = p;
            free(p);
        }
        catch (_com_error err)
        {
            DBG_TEXT_PUT(_T("setup cert except err : %s"), (LPCTSTR)err.Description());
            return 0;
        }

        TiXmlDocument xmldoc;
        xmldoc.Parse(strXmlResponse.c_str());
        if (xmldoc.Error())
        {
            DBG_TEXT_PUT(_T("setup cert response xml parse err %s"), _CA2T(xmldoc.ErrorDesc()));
            return 0;
        }

        TiXmlElement* root = xmldoc.RootElement();
        if (NULL == root)
        {
            DBG_TEXT_PUT(_T("setup cert response xml parse root err"));
            return 0;
        }
        if (!getAppleXmlValueByKey(root, "sign-sap-setup-cert", str_Cert))
        {
            DBG_TEXT_PUT(_T("setup cert response find sign-sap-setup-cert err"));
            return -1;
        }

        return 1;
        VMPEnd();
    }
};