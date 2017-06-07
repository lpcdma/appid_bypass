#pragma once
#include "stdafx.h"
#include <json\json.h>
#include <json\json_link.h>
#include <tinyxml.h>
#include <tinyxml_link.h>
#include <string>
#include <HttpHeader.h>
#include <timestamp.h>
#include <Charset.h>
#include <mshtml.h>
#include <HttpHelp.h>
#include "ItunesProtocolCommon.h"

#include "ItunesFunction.h"
#include "ItunesInterface.h"
#include "common.h"

#import "c:\windows\syswow64\winhttp.dll"
#import "c:\Windows\syswow64\msscript.ocx"
#import "C:\Windows\SysWOW64\mshtml.tlb" rename("min", "mshtml_min"), rename("max", "mshtml_max"), rename("TranslateAccelerator","mshtml_TranslateAccelerator")

using namespace std;
using namespace WinHttp;
using namespace MSScriptControl;

#pragma pack(push ,1)
struct CCmdPack
{
    int magic;
    unsigned int len;
};
#pragma pack(pop)

class CItunesProtocol : public CItunesInterface
{
public:
    CItunesProtocol(CItunesSession session)
    {
        m_session = session;
    }
public:
    CItunesSession m_session;
private:
    bool getSign(const string &str_data, string &str_signed)
    {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in svr;
        svr.sin_addr.s_addr = inet_addr("192.168.0.104");
        svr.sin_port = htons(9999);
        svr.sin_family = AF_INET;

        int err = connect(s, (sockaddr *)&svr, sizeof(svr));
        if (err == SOCKET_ERROR)
        {
            closesocket(s);

            return false;
        }

        int len = str_data.length();
        int totalLen = len + sizeof(CCmdPack);

        char *buff = new char[totalLen];
        CCmdPack packHdr = {};
        packHdr.magic = 'fuck';
        packHdr.len = totalLen;
        memcpy(buff, &packHdr, sizeof(packHdr));
        memcpy(buff + sizeof(packHdr), str_data.c_str(), len);

        err = send(s, buff, totalLen, 0);
        char recvbuff[1000] = { 0 };
        err = recv(s, recvbuff, 1000, 0);
        if (err <= 0)
            return false;
        
        if (err < sizeof(CCmdPack))
            return false;

        CCmdPack *pPackHdr = (CCmdPack *)recvbuff;
        if (pPackHdr->len > err)
            return false;

        unsigned char *pin = (unsigned char *)&recvbuff[0];
        pin += sizeof(CCmdPack);

        if (!base64(pin, err - sizeof(CCmdPack), str_signed))
            return false;

        closesocket(s);

        return true;
    }
    bool getDoc(IWinHttpRequestPtr winhttp, MSHTML::IHTMLDocument3Ptr &pIDoc3)
    {
        CComPtr<IHTMLDocument2> pIDoc2;
        HRESULT hRes = pIDoc2.CoCreateInstance(CLSID_HTMLDocument);
        if (FAILED(hRes))
        {
            return false;
        }

        ByteArray bytes;
        GetSafeArrayBytes(V_ARRAY(&winhttp->ResponseBody), bytes);
        bstr_t bstrEncoding = winhttp->GetResponseHeader("Content-Encoding");
        string str_encod_type;

        if (bstrEncoding.GetBSTR())
            str_encod_type = bstrEncoding;

        if (str_encod_type.find("gzip") != string::npos)
        {
            UncompResponse(winhttp->ResponseBody, 1, bytes);
        }

        CCharset charset;
        char *p = bytes.ToString();
        bstr_t bstrRespText = charset.UTF8ToUnicode(p);
        bytes.Free(p);

        pIDoc2->clear();
        hRes = pIDoc2->put_designMode(bstr_t("on"));
        //写入html DOM
        SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, 1);
        VARIANT *pItems = NULL;
        SafeArrayAccessData(psa, (LPVOID *)&pItems);
        pItems[0].vt = VT_BSTR;
        pItems[0].bstrVal = bstrRespText.Detach();
        SafeArrayUnaccessData(psa);
        hRes = pIDoc2->write(psa);
        hRes = pIDoc2->close();

        pIDoc3 = static_cast<IUnknown *>(pIDoc2);
        if (pIDoc3 == NULL)
            return false;

        return true;
    }
    bool getAttributeByClass(MSHTML::IHTMLDocument3Ptr pIDoc3, const string &strClass, const string &strAttr, string &strAttrValue)
    {
        MSHTML::IHTMLDocument7Ptr pIDoc7 = pIDoc3;
        if (pIDoc7 == NULL)
            return false;

        MSHTML::IHTMLElementCollectionPtr pElemCollPtr = pIDoc7->getElementsByClassName(strClass.c_str());
        if (pElemCollPtr == NULL)
            return false;

        if (pElemCollPtr->length != 1)
            return false;

        MSHTML::IHTMLElementPtr pElemPtr = pElemCollPtr->item(0);
        if (pElemPtr == NULL)
            return false;

        variant_t varAttrValue = pElemPtr->getAttribute(strAttr.c_str(), 0);
        if (V_VT(&varAttrValue) != VT_BSTR)
            return false;

        bstr_t bstrAttrValue = varAttrValue;

        if (bstrAttrValue.length() == 0)
            strAttrValue = "";
        else
            strAttrValue = bstrAttrValue;

        return true;
    }
    bool getAttribute(MSHTML::IHTMLElementPtr &pElemPtr, const string &strAttr, string &strAttrValue)
    {
        variant_t varAttrValue = pElemPtr->getAttribute(strAttr.c_str(), 0);
        if (V_VT(&varAttrValue) != VT_BSTR)
            return false;

        bstr_t bstrAttrValue = varAttrValue;

        if (bstrAttrValue.length() == 0)
            strAttrValue = "";
        else
            strAttrValue = bstrAttrValue;

        return true;
    }
    bool getAttributeById(MSHTML::IHTMLDocument3Ptr pIDoc3, const string &strtID, const string &strAttr, string &strAttrValue)
    {
        MSHTML::IHTMLElementPtr pElemPtr = pIDoc3->getElementById(strtID.c_str());
        if (pElemPtr == NULL)
            return false;

        variant_t varAttrValue = pElemPtr->getAttribute(strAttr.c_str(), 0);
        if (V_VT(&varAttrValue) != VT_BSTR)
            return false;

        bstr_t bstrAttrValue = varAttrValue;

        if (bstrAttrValue.length() == 0)
            strAttrValue = "";
        else
            strAttrValue = bstrAttrValue;

        return true;
    }
    bool getSettingInfo(MSHTML::IHTMLDocument3Ptr pIDoc3, string &str_post)
    {
        CCharset charset;

        string str_pageUUID = "mzPageUUID"; string str_pageUUIDValue;
        if (!getAttributeById(pIDoc3, "pageUUID", "value", str_pageUUIDValue))
            return false;

        string str_machineGUID = string("machineGUID=") + m_session.m_guid;

        string str_xAppleActionSignature = "xAppleActionSignature"; string str_xAppleActionSignatureValue;
        string strSignData = str_pageUUIDValue; strSignData += m_session.m_guid;
        if (!getSign(strSignData, str_xAppleActionSignatureValue))
        {
            return false;
        }
        str_xAppleActionSignatureValue = charset.URLEncode(str_xAppleActionSignatureValue);

        string str_accountName, str_accountNameValue;
        if (!getAttributeById(pIDoc3, "accountNameField", "name", str_accountName))
            return false;
        if (!getAttributeById(pIDoc3, "accountNameField", "value", str_accountNameValue))
            return false;

        string str_passwordName, str_passwordValue = m_session.m_password + "g";
        if (!getAttributeById(pIDoc3, "passwordField", "name", str_passwordName))
            return false;

        string str_passwordVerificationName, str_passwordVerificationValue = m_session.m_password + "g";
        if (!getAttributeById(pIDoc3, "passwordVerificationField", "name", str_passwordVerificationName))
            return false;

        string str_questionName, str_questionValue;
        if (!getAttributeById(pIDoc3, "questionField", "name", str_questionName))
            return false;
        if (!getAttributeById(pIDoc3, "questionField", "value", str_questionValue))
            return false;

        string str_answerName, str_answerValue = "zzz";
        if (!getAttributeById(pIDoc3, "answerField", "name", str_answerName))
            return false;
        //if (!getAttributeById(pIDoc3, "answerField", "value", str_answerValue))
        //    return false;

        //年
        string str_birthYearName, str_birthYearValue = "1991";
        if (!getAttributeById(pIDoc3, "birthYear", "name", str_birthYearName))
            return false;
        //if (!getAttributeById(pIDoc3, "birthYear", "value", str_birthYearValue))
        //    return false;
        //月
        string str_birthMonthPopupName, str_birthMonthPopupValue = "1";
        if (!getAttributeById(pIDoc3, "birthMonthPopup", "name", str_birthMonthPopupName))
            return false;
        //日
        string str_birthDayPopupName, str_birthDayPopupValue = "1";
        if (!getAttributeById(pIDoc3, "birthDayPopup", "name", str_birthDayPopupName))
            return false;

        //订阅
        string str_AppleNewsCheckName, str_AppleNewsCheckValue;
        if (!getAttributeByClass(pIDoc3, "checkbox acceptsNewsletter on-off-toggle", "name", str_AppleNewsCheckName))
            return false;
        if (!getAttributeByClass(pIDoc3, "checkbox acceptsNewsletter on-off-toggle", "value", str_AppleNewsCheckValue))
            return false;

        //订阅
        string str_AppleMarketingCheckName, str_AppleMarketingCheckValue;
        if (!getAttributeByClass(pIDoc3, "checkbox acceptsMarketingEmail on-off-toggle", "name", str_AppleMarketingCheckName))
            return false;
        if (!getAttributeByClass(pIDoc3, "checkbox acceptsMarketingEmail on-off-toggle", "value", str_AppleMarketingCheckValue))
            return false;
        
        //下一步
        //hiddenBottomRightButtonId
        string str_NextStepName, str_NextStepValue;
        if (!getAttributeById(pIDoc3, "hiddenBottomRightButtonId", "name", str_NextStepName))
            return false;
        if (!getAttributeById(pIDoc3, "hiddenBottomRightButtonId", "value", str_NextStepValue))
            return false;


        str_post = str_pageUUID; str_post += "="; str_post += charset.URLEncode(str_pageUUIDValue); str_post += "&";
        str_post += str_machineGUID; str_post += "&";
        str_post += str_xAppleActionSignature; str_post += "="; str_post += str_xAppleActionSignatureValue; str_post += "&";

        //密码验证
        str_post += str_accountName; str_post += "="; str_post += charset.URLEncode(str_accountNameValue); str_post += "&";
        str_post += str_passwordName; str_post += "="; str_post += charset.URLEncode(str_passwordValue); str_post += "&";
        str_post += str_passwordVerificationName; str_post += "="; str_post += charset.URLEncode(str_passwordVerificationValue); str_post += "&";

        //密保问题
        CStringA str_temp = str_questionValue.c_str();
        CStringW str_utemp = str_temp;
        str_questionValue = charset.UnicodeToUTF8(str_utemp);
        str_post += str_questionName; str_post += "="; str_post += charset.URLEncode(str_questionValue); str_post += "&";

        str_temp = str_answerValue.c_str();
        str_utemp = str_temp;
        str_answerValue = charset.UnicodeToUTF8(str_utemp);
        str_post += str_answerName; str_post += "="; str_post += charset.URLEncode(str_answerValue); str_post += "&";

        //生日
        str_post += str_birthYearName; str_post += "="; str_post += str_birthYearValue; str_post += "&";
        str_post += str_birthMonthPopupName; str_post += "="; str_post += str_birthMonthPopupValue; str_post += "&";
        str_post += str_birthDayPopupName; str_post += "="; str_post += str_birthDayPopupValue; str_post += "&";

        //订阅
        str_post += str_AppleNewsCheckName; str_post += "="; str_post += str_AppleNewsCheckValue; str_post += "&";
        str_post += str_AppleMarketingCheckName; str_post += "="; str_post += str_AppleMarketingCheckValue; str_post += "&";

        //Step Next
        str_temp = str_NextStepValue.c_str();
        str_utemp = str_temp;
        str_NextStepValue = charset.UnicodeToUTF8(str_utemp);
        str_post += str_NextStepName; str_post += "="; str_post += charset.URLEncode(str_NextStepValue);

        return true;
    }
    bool getBypassInfo(MSHTML::IHTMLDocument3Ptr pIDoc3, string &str_post)
    {
        CCharset charset;
        
        string str_pageUUID = "mzPageUUID"; string str_pageUUIDValue;
        if (!getAttributeById(pIDoc3, "pageUUID", "value", str_pageUUIDValue))
            return false;

        string str_machineGUID = string("machineGUID=") + m_session.m_guid;
        
        string str_xAppleActionSignature = "xAppleActionSignature"; string str_xAppleActionSignatureValue;
        string strSignData = str_pageUUIDValue; strSignData += m_session.m_guid;
        if (!getSign(strSignData, str_xAppleActionSignatureValue))
        {
            return false;
        }
        str_xAppleActionSignatureValue = charset.URLEncode(str_xAppleActionSignatureValue);

        string str_cctype = "cc-type-hddn=PEAS";
        string str_sp = "sp=";
        string str_res = "res=";

        string str_email_regex = "email_regex="; str_email_regex += charset.URLEncode("^[A-Za-z0-9._%+-]+@");
        string str_phone_regex = "phone_regex="; str_phone_regex += charset.URLEncode("^[0-9-]{10,30}$");

        //电子邮件或手机号，支付方式关联的
        string str_sesame; string str_sesameValue = "";
        if (!getAttributeById(pIDoc3, "sesame-id-input", "name", str_sesame))
            return false;

        //身份证最后五位
        string str_national; string str_nationalValue = "";
        if (!getAttributeById(pIDoc3, "national-id-input", "name", str_national))
            return false;

        //信用卡
        string str_cc_number = "cc-number=";
        //安全码
        string str_ccv = "ccv=";
        //到期时间
        string str_cc_expr_month = "cc-expr-month=WONoSelectionString";
        string str_cc_expr_year = "cc-expr-year=WONoSelectionString";

        //国家代码
        string str_country_code; string str_country_codeValue = "86";
        if (!getAttributeById(pIDoc3, "country-code", "name", str_country_code))
            return false;

        //手机号
        string str_mobileNumberField; string str_mobileNumberFieldValue = "";
        if (!getAttributeById(pIDoc3, "mobileNumberField", "name", str_mobileNumberField))
            return false;

        //
        string str_card_type_id = "card-type-id=18";
        string str_getCardTypeUrl = "getCardTypeUrl="; str_getCardTypeUrl += charset.URLEncode("https://play.itunes.apple.com/WebObjects/MZPlay.woa/wa/getCardTypeSrv");

        //礼品卡
        string str_codeRedemptionField; string str_codeRedemptionFieldValue = "";
        if (!getAttributeById(pIDoc3, "codeRedemptionField", "name", str_codeRedemptionField))
            return false;

        //姓名
        string str_lastFirstName; string str_lastFirstNameValue;
        if (!getAttributeById(pIDoc3, "lastNameField", "name", str_lastFirstName))
            return false;
        if (!getAttributeById(pIDoc3, "lastNameField", "value", str_lastFirstNameValue))
            return false;

        string str_firstNameField; string str_firstNameFieldValue;
        if (!getAttributeById(pIDoc3, "firstNameField", "name", str_firstNameField))
            return false;
        if (!getAttributeById(pIDoc3, "firstNameField", "value", str_firstNameFieldValue))
            return false;

        CStringA str_temp = str_lastFirstNameValue.c_str();
        CStringW str_utemp = str_temp;
        str_lastFirstNameValue = charset.UnicodeToUTF8(str_utemp);

        str_temp = str_firstNameFieldValue.c_str();
        str_utemp = str_temp;
        str_firstNameFieldValue = charset.UnicodeToUTF8(str_utemp);

        string str_longName = "longName"; 
        string str_longNameValue = str_lastFirstNameValue; str_longNameValue += ","; str_longNameValue += str_firstNameFieldValue;
        str_longNameValue = charset.URLEncode(str_longNameValue);

        
        //街道信息
        string str_street1Field, str_street2Field, str_street3Field; 
        string str_street1FieldValue = "JIEMING", str_street2FieldValue = "", str_street3FieldValue = "";
        if (!getAttributeById(pIDoc3, "street1Field", "name", str_street1Field))
            return false;
        if (!getAttributeById(pIDoc3, "street2Field", "name", str_street2Field))
            return false;
        if (!getAttributeById(pIDoc3, "street3Field", "name", str_street3Field))
            return false;

        //邮编
        string str_postalCodeField; string str_postalCodeFieldValue = "";
        if (!getAttributeById(pIDoc3, "postalCodeField", "name", str_postalCodeField))
            return false;

        //城市
        string str_cityField; string str_cityFieldValue = "WUHAN";
        if (!getAttributeById(pIDoc3, "cityField", "name", str_cityField))
            return false;

        //省份
        string str_state = "state=1";//湖北

        //电话
        string str_phoneNumberField; string str_phoneNumberFieldValue = "7878";
        if (!getAttributeById(pIDoc3, "phoneNumberField", "name", str_phoneNumberField))
            return false;

        //ndpd-*
        string str_ndpd = "ndpd-s=&ndpd-f=&ndpd-fm=&ndpd-w=&ndpd-ipr=&ndpd-di=&ndpd-bi=&ndpd-probe=&ndpd-af=&ndpd-fv=&ndpd-fa=&ndpd-bp=&ndpd-wk=&ndpd-vk=";
        //
        string str_captchaMode = "captchaMode=VIDEO";

        string str_Submit; string str_SubmitValue = "%E4%B8%8B%E4%B8%80%E6%AD%A5";//下一步
        if (!getAttributeById(pIDoc3, "hiddenBottomRightButtonId", "name", str_Submit))
            return false;

        str_post = str_pageUUID; str_post += "="; str_post += charset.URLEncode(str_pageUUIDValue); str_post += "&";
        str_post += str_machineGUID; str_post += "&";
        str_post += str_xAppleActionSignature; str_post += "="; str_post += str_xAppleActionSignatureValue; str_post += "&";

        str_post += str_longName; str_post += "="; str_post += str_longNameValue; str_post += "&";
        str_post += str_cctype; str_post += "&";
        str_post += str_sp; str_post += "&";
        str_post += str_res; str_post += "&";

        str_post += str_email_regex; str_post += "&";
        str_post += str_phone_regex; str_post += "&";

        str_post += str_sesame; str_post += "=";  str_post += str_sesameValue; str_post += "&";
        str_post += str_national; str_post += "=";  str_post += str_nationalValue; str_post += "&";

        str_post += str_cc_number; str_post += "&";
        str_post += str_ccv; str_post += "&";
        str_post += str_cc_expr_month; str_post += "&";
        str_post += str_cc_expr_year; str_post += "&";
        
        str_post += str_country_code; str_post += "=";  str_post += str_country_codeValue; str_post += "&";
        str_post += str_mobileNumberField; str_post += "=";  str_post += str_mobileNumberFieldValue; str_post += "&";

        str_post += str_card_type_id; str_post += "&";
        str_post += str_getCardTypeUrl; str_post += "&";
        
        str_post += str_codeRedemptionField; str_post += "=";  str_post += str_codeRedemptionFieldValue; str_post += "&";
        str_post += str_lastFirstName; str_post += "=";  str_post += charset.URLEncode(str_lastFirstNameValue); str_post += "&";
        str_post += str_firstNameField; str_post += "=";  str_post += charset.URLEncode(str_firstNameFieldValue); str_post += "&";
        str_post += str_street1Field; str_post += "=";  str_post += str_street1FieldValue; str_post += "&";
        str_post += str_street2Field; str_post += "=";  str_post += str_street2FieldValue; str_post += "&";
        str_post += str_street3Field; str_post += "=";  str_post += str_street3FieldValue; str_post += "&";
        str_post += str_postalCodeField; str_post += "=";  str_post += str_postalCodeFieldValue; str_post += "&";
        str_post += str_cityField; str_post += "=";  str_post += str_cityFieldValue; str_post += "&";
        str_post += str_state; str_post += "&";
        str_post += str_phoneNumberField; str_post += "=";  str_post += str_phoneNumberFieldValue; str_post += "&";

        str_post += str_ndpd; str_post += "&";
        str_post += str_captchaMode; str_post += "&";
        str_post += str_Submit; str_post += "="; str_post += str_SubmitValue;


        return true;
    }
public:
    int send_authenticate_pack(bool bOtps, const string &strWhy)
    {
        VMPBegin("send_auth");
        USES_CONVERSION;

        DBG_TEXT_PUT(_T("auth start"));

        m_session.m_failureType = "";

        //string strKbsync;
        //if (!getKbsync_Proxy(1, m_session.m_dsid, strKbsync))
        //{
        //    DBG_TEXT_PUT(_T("send_authenticate_pack get_kbsync err"));
        //    return 0;
        //}

        vector<CAUTH_MID_OTP> otplist;
        //if (bOtps)
            //getOTPList_Proxy(otplist);

        vector<CMapItem> plist;
        plist.push_back(CMapItem("key", "appleId"));
        plist.push_back(CMapItem("string", m_session.m_appid));
        plist.push_back(CMapItem("key", "attempt"));
        plist.push_back(CMapItem("integer", "0"));
        plist.push_back(CMapItem("key", "createSession"));
        plist.push_back(CMapItem("string", "true"));
        ///otp list
        if (!otplist.empty())
        {
            plist.push_back(CMapItem("key", "auth-mid-otp"));
            plist.push_back(CMapItem("array", "fadfasd"));
        }
        ///
        plist.push_back(CMapItem("key", "guid"));
        plist.push_back(CMapItem("string", m_session.m_guid));
        //plist.push_back(CMapItem("key", "kbsync"));
        //plist.push_back(CMapItem("string", strKbsync));
        plist.push_back(CMapItem("key", "password"));
        plist.push_back(CMapItem("string", m_session.m_password));
        plist.push_back(CMapItem("key", "rmp"));
        plist.push_back(CMapItem("string", "0"));
        plist.push_back(CMapItem("key", "why"));
        plist.push_back(CMapItem("string", strWhy));

        string strAuthBody = ItunesProtocolCommon::get_request_xml(plist, otplist);

        string strActionSignature;
        if (!getActionSignature_Proxy((unsigned char *)strAuthBody.c_str(), strAuthBody.length(), strActionSignature))
        {
            DBG_TEXT_PUT(_T("auth get_ActionSignature err"));
            return 0;
        }

        CoInitialize(NULL);
        IWinHttpRequestPtr winhttp;
        string strXmlResponse;
        string strResponseHeaders;

        try
        {
            string url = string("https://p") + m_session.m_serverid + "-buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/authenticate";

            winhttp.CreateInstance(L"WinHttp.WinHttpRequest.5.1");
            winhttp->Open("POST", url.c_str(), false);
            if (!m_session.m_ProxyServer.empty()) 
                winhttp->SetProxy(2, m_session.getProxyString().c_str());
            winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            winhttp->SetRequestHeader("Content-Type", m_session.m_ContentType.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-ActionSignature", strActionSignature.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            //winhttp->SetCredentials("HX22X6K4HS648E9D", "139500294D64812A", 1);
            winhttp->Option[WinHttpRequestOption_EnableRedirects] = false;
            winhttp->Send(strAuthBody.c_str());
            //location
            if (winhttp->Status == 302 || winhttp->Status == 307)
            {
                string str_location = (LPCSTR)winhttp->GetResponseHeader("location");
                string::size_type posb = str_location.find("//p");
                string::size_type pose = str_location.find("-");
                if (posb == string::npos || pose == string::npos)
                    return -1;
                posb += 3;
                m_session.m_serverid = str_location.substr(posb, pose - posb);
                Trim(m_session.m_serverid);
                return -2;
            }
            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("auth response status : %d err"), winhttp->Status);
                return 0;
            }
            CCharset charset;
            LPCWSTR pun = (LPCWSTR)winhttp->ResponseText;
            strXmlResponse = charset.UnicodeToUTF8(pun);

            strResponseHeaders = (LPCSTR)winhttp->GetAllResponseHeaders();

        }
        catch (_com_error err)
        {
            DBG_TEXT_PUT(_T("auth except err : %s"), (LPCTSTR)err.Description());
            return 0;
        }

        TiXmlDocument xmldoc;
        xmldoc.Parse(strXmlResponse.c_str());
        if (xmldoc.Error())
        {
            DBG_TEXT_PUT(_T("auth response xml parse err %s"), _CA2T(xmldoc.ErrorDesc()));
            return 0;
        }

        TiXmlElement* root = xmldoc.RootElement();
        if (NULL == root)
        {
            DBG_TEXT_PUT(_T("auth response xml parse root err"));
            return 0;
        }

        //get cookies
        CHttpHeader httpheader;
        httpheader.Parser(strResponseHeaders);
        vector<string> arrCookies;
        arrCookies = httpheader.GetHeaders("set-cookie");
        m_session.m_cookie.clear();
        for (vector<string>::iterator iter = arrCookies.begin(); iter != arrCookies.end(); iter++)
        {
            //        DBG_TEXT_PUT(_T("cookie line %s"), _CA2T(iter->c_str()));
            string::size_type pos = iter->find(";");
            if (pos == string::npos)
                continue;
            string strCookie = iter->substr(0, pos);
            //        DBG_TEXT_PUT(_T("cookie %s"), _CA2T(strCookie.c_str()));

            pos = strCookie.find("=");
            if (pos == string::npos)
                continue;

            string str_name, str_value;
            str_name = strCookie.substr(0, pos);
            str_value = strCookie.substr(pos + 1);
            //        DBG_TEXT_PUT(_T("name %s : value %s"), _CA2T(str_name.c_str()), _CA2T(str_value.c_str()));
            m_session.m_cookie.insert(pair<string, string>(CHttpHeader::Trim(str_name), CHttpHeader::Trim(str_value)));
        }

        string strErr;
        if (ItunesProtocolCommon::getAppleXmlValueByKey(root, "failureType", strErr))
        {
            m_session.m_failureType = strErr;
            ItunesProtocolCommon::getAppleXmlValueByKey(root, "customerMessage", m_session.m_customerMessage);
            CCharset charset;
            CStringA tmp = charset.UTF8ToUnicode(m_session.m_customerMessage.c_str());
            m_session.m_customerMessage = tmp;

            DBG_TEXT_PUT(_T("auth response find failureType err"));
            return -1;
        }
        //find xtoken
        if (!ItunesProtocolCommon::getAppleXmlValueByKey(root, "passwordToken", m_session.m_xtoken))
        {
            DBG_TEXT_PUT(_T("auth response can't find passwordToken err"));
            return -1;
        }
        //find dsid
        if (!ItunesProtocolCommon::getAppleXmlValueByKey(root, "dsPersonId", m_session.m_dsid))
        {
            DBG_TEXT_PUT(_T("auth response can't find dsid err"));
            return -1;
        }

        DBG_TEXT_PUT(_T("auth success"));

        return 1;
        VMPEnd();
    }
    string get_requestId(const string &str_xp_ci)
    {
        VMPBegin("get_requestId");

        IScriptControlPtr pIScriptCtl;
        pIScriptCtl.CreateInstance(__uuidof(ScriptControl));

        pIScriptCtl->AllowUI = VARIANT_FALSE;
        pIScriptCtl->Language = "JScript";
        string str_code = "var t=\"z\",n=new Date().valueOf(),r=Math.floor(Math.random()*1e5);n = n.toString(36).toUpperCase(), r = r.toString(36).toUpperCase();nr = t + n + t + r;";
        pIScriptCtl->AddCode(str_code.c_str());
        variant_t var_exp_val = pIScriptCtl->Eval("nr");
        bstr_t bstr_exp_val = var_exp_val;

        return str_xp_ci + (LPCSTR)bstr_exp_val;

        VMPEnd();
    }
    bool send_register(map<string, string> &map_cookies)
    {
        VMPBegin("send_register");
        USES_CONVERSION;

        CoInitialize(NULL);
        IWinHttpRequestPtr winhttp;
        string strJsonResponse;

        try
        {
            string url = "https://xp.apple.com/register";

            winhttp.CreateInstance(L"WinHttp.WinHttpRequest.5.1");
            winhttp->Open("GET", url.c_str(), false);
            if (!m_session.m_ProxyServer.empty())
                winhttp->SetProxy(2, m_session.getProxyString().c_str());
            winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Partner", m_session.m_ApplePartner.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("X-Apple-Client-Versions", m_session.m_AppleClientVersion.c_str());
            winhttp->SetRequestHeader("X-Apple-Connection-Type", m_session.m_AppleConnectionType.c_str());
            winhttp->Send();

            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("send_register response err \n"));
                return false;
            }

            LPCWSTR pun = (LPCWSTR)winhttp->ResponseText;
            CCharset charset;
            strJsonResponse = charset.UnicodeToUTF8(pun);

        }
        catch (_com_error err)
        {
            DBG_TEXT_PUT(_T("send_register except err : %s\n"), (LPCTSTR)err.Description());
            return false;
        }

        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(strJsonResponse, root))
        {
            DBG_TEXT_PUT(_T("send_register response json parse err"));
            return false;
        }

        //if (root["setCookies"].type() != Json::arrayValue)
        //    return false;

        Json::Value::UInt size = root["setCookies"].size();

        for (Json::Value::UInt i = 0; i < size; i++)
        {
            Json::Value cookie_object = root["setCookies"][i];
            if (cookie_object.isNull())
                continue;

            map_cookies.insert(pair<string, string>(cookie_object["name"].asString(), cookie_object["value"].asString()));
        }

        DBG_TEXT_PUT(_T("send_register succ"));

        return true;

        VMPEnd();
    }
    bool send_appidbypass(const string &str_cookie, const string &strWhy)
    {
        VMPBegin("send_appidbypass");
        USES_CONVERSION;

        CoInitialize(NULL);
        IWinHttpRequestPtr winhttp;
        string strXmlResponse;

        /*
        eth + c:\ + product + processor + bios + computername + GetCurrentHwProfileW



HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System:SystemBiosVersion
HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0:ProcessorNameString
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion:ProductId
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\IDConfigDB\Hardware Profiles\0001:HwProfileGuid

*/
        try
        {
            CCharset charset;
            //登录后跳转地址
            string strJmpUrl = string("https://p") + m_session.m_serverid + "-buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/com.apple.jingle.app.finance.DirectAction/convertWizard?";
            string strparam;
            strparam = "rmp=0";
            strparam += "&workflowID=1238";
            strparam += "&why="; strparam += strWhy;
            strparam += "&guid="; strparam += charset.URLEncode(m_session.m_guid);
            strparam += "&createSession=true";
            strparam += "&attempt=0";
            strparam += "&guid="; strparam += charset.URLEncode(m_session.m_guid);

            strJmpUrl += strparam;
            string strActionSignature;
            if (!getActionSignature_Proxy((unsigned char *)m_session.m_guid.c_str(), m_session.m_guid.length(), strActionSignature))
            {
                DBG_TEXT_PUT(_T("send_appidbypass get_ActionSignature err"));
                return 0;
            }

            winhttp.CreateInstance(L"WinHttp.WinHttpRequest.5.1");
            winhttp->Open("GET", strJmpUrl.c_str(), false);

            if (!m_session.m_ProxyServer.empty())
                winhttp->SetProxy(2, m_session.getProxyString().c_str());
            winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-Client-Versions", m_session.m_AppleClientVersion.c_str());
            winhttp->SetRequestHeader("X-Apple-Partner", m_session.m_ApplePartner.c_str());
            winhttp->SetRequestHeader("Accept-Encoding", "gzip, deflate");
            winhttp->SetRequestHeader("Accept-Language", "zh-cn");
            winhttp->SetRequestHeader("X-Apple-Connection-Type", m_session.m_AppleConnectionType.c_str());
            winhttp->SetRequestHeader("X-Apple-ActionSignature", "AiKcQ6E6lxvA+U9W9c+SefMWEUe35SBiYIMWFZfm+AHdAAABUAMAAABaAAAAgF5JqbB+IKp7OWE1ANl78fVHTA2z4nxUAIwqZHPfKTE54owfYsPyKg+MBtE9yqMfuafORdqhwBthYmOFAIibFLdKn48roUlpr3fomwvRam5DWxXLXxX84o00caI1cT5Y8XG6pIpSZMiwtUCuJ3TJSK226WAhAJ1M+duZoXQfRmldAAAAGJSHUsvg669N2FtQRb7v/WKothO4MZhZsAAAAKMC3EfwumH5cLGaxCXjs41QWrVOcXMAAACKBQT08vAAVATg65KqQr3llho6WPI+IgAAANKc7JiKOsOQebGYr/BzplPFVJRuYSa0tG5P1CNOt3i9FT5xDo0bjuxNvoKdXgmsxErBhLqyFNYIAm5E3HZivRL+5qAvdpeTWI7wm/4fkruz013lpYaVhf5homQWgszJoVABnQNxhr6Uvgrt2cIGiH8mAAAAAAEEOEQ0AA==");//AiKcQ6E6lxvA + U9W9c + SefMWEUe35SBiYIMWFZfm + AHdAAABUAMAAABaAAAAgF5JqbB + IKp7OWE1ANl78fVHTA2z4nxUAIwqZHPfKTE54owfYsPyKg + MBtE9yqMfuafORdqhwBthYmOFAIibFLdKn48roUlpr3fomwvRam5DWxXLXxX84o00caI1cT5Y8XG6pIpSZMiwtUCuJ3TJSK226WAhAJ1M + duZoXQfRmldAAAAGJSHUsvg669N2FtQRb7v / WKothO4MZhZsAAAAKMC3EfwumH5cLGaxCXjs41QWrVOcXMAAACKBQT08vAAVATg65KqQr3llho6WPI + IgAAANKc7JiKOsOQebGYr / BzplPFVJRuYSa0tG5P1CNOt3i9FT5xDo0bjuxNvoKdXgmsxErBhLqyFNYIAm5E3HZivRL + 5qAvdpeTWI7wm / 4fkruz013lpYaVhf5homQWgszJoVABnQNxhr6Uvgrt2cIGiH8mAAAAAAEEOEQ0AA == "); //strActionSignature.c_str());
            winhttp->SetRequestHeader("Cookie", str_cookie.c_str());
            //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
            winhttp->Send();
            Sleep(2000);
            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("send_appidbypass response err \n"));
                return false;
            }
            
            MSHTML::IHTMLDocument3Ptr pIDoc3;
            if (!getDoc(winhttp, pIDoc3))
                return false;

            MSHTML::IHTMLElementPtr pElemNextPtr = pIDoc3->getElementById("hiddenBottomRightButtonId");
            if (pElemNextPtr == NULL)
                return false;

            string str_NextName;
            if (!getAttribute(pElemNextPtr, "name", str_NextName))
                return false;

            MSHTML::IHTMLElementPtr pElemNextDivPtr = pElemNextPtr->GetparentElement();
            if (pElemNextDivPtr == NULL)
                return false;

            MSHTML::IHTMLElementPtr pElemNextFormPtr = pElemNextDivPtr->GetparentElement();
            if (pElemNextFormPtr == NULL)
                return false;

            string strElemNextPost;
            if (!getAttribute(pElemNextFormPtr, "action", strElemNextPost))
                return false;

            //点下一步按钮
            string strUrlNext = string("https://p") + m_session.m_serverid + string("-buy.itunes.apple.com") + strElemNextPost;
            winhttp->Open("POST", strUrlNext.c_str(), false);
            winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-Client-Versions", m_session.m_AppleClientVersion.c_str());
            winhttp->SetRequestHeader("X-Apple-Partner", m_session.m_ApplePartner.c_str());
            winhttp->SetRequestHeader("X-Apple-Connection-Type", m_session.m_AppleConnectionType.c_str());
            winhttp->SetRequestHeader("Referer", strJmpUrl.c_str());
            winhttp->SetRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            winhttp->SetRequestHeader("Accept-Language", "zh-cn");
            winhttp->SetRequestHeader("Accept-Encoding", "gzip, deflate");
            //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
            string strSendNext;
            strSendNext += "storeFront=143465&";//CN
            strSendNext += str_NextName; strSendNext += "=%E4%B8%8B%E4%B8%80%E6%AD%A5";//下一步
            winhttp->Send(strSendNext.c_str());
            Sleep(2000);
            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("send_appidbypass response err \n"));
                return false;
            }

            if (!getDoc(winhttp, pIDoc3))
                return false;

            /*
            //选择国家
            //MSHTML::IHTMLElementPtr pElemCountrySelectPtr = pIDoc3->getElementById("country");
            //if (pElemCountrySelectPtr == NULL)
            //    return false;

            //MSHTML::IHTMLElementPtr pElemDivSelectPtr = pElemCountrySelectPtr->GetparentElement();
            //if (pElemDivSelectPtr == NULL)
            //    return false;

            //MSHTML::IHTMLElementPtr pElemFormSelectPtr = pElemDivSelectPtr->GetparentElement();
            //if (pElemFormSelectPtr == NULL)
            //    return false;

            //MSHTML::IHTMLDOMNodePtr pDomNodePtr = pElemDivSelectPtr;
            //MSHTML::IHTMLDOMNodePtr pDomNodeNextChildPtr = pDomNodePtr->nextSibling;
            //if (pDomNodeNextChildPtr == NULL)
            //    return false;

            //MSHTML::IHTMLElementPtr pElemSubmitSelectPtr = pDomNodeNextChildPtr;
            //if (pElemSubmitSelectPtr == NULL)
            //    return false;

            //variant_t varCountry = pElemCountrySelectPtr->getAttribute("name", 0);
            //bstr_t bstrCountry = varCountry;
            //string strCountry = bstrCountry;

            //variant_t varSubmitSelect = pElemSubmitSelectPtr->getAttribute("name", 0);
            //bstr_t bstrSubmitSelect = varSubmitSelect;
            //string strSubmitSelect = bstrSubmitSelect;

            //variant_t varCountrySelectPost = pElemFormSelectPtr->getAttribute("action", 0);
            //bstr_t bstrCountrySelectPost = varCountrySelectPost;
            //string strCountrySelectPost = bstrCountrySelectPost;

            ////m_session.m_AppleStoreFront 需要跟着变化
            //string strCountrySelectUrl = string("https://p") + m_session.m_serverid + string("-buy.itunes.apple.com") + strCountrySelectPost;
            //winhttp->Open("POST", strCountrySelectUrl.c_str(), false);
            //winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            //winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            //winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
            //winhttp->SetRequestHeader("Referer", strUrlGoOn.c_str());
            //winhttp->SetRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            //winhttp->SetRequestHeader("Accept-Encoding", "gzip, deflate");

            //string strCountrySelectSend;
            //strCountrySelectSend = strCountry; strCountrySelectSend += "="; strCountrySelectSend += "CN&";
            //strCountrySelectSend += strSubmitSelect; strCountrySelectSend += "=%E6%9B%B4%E6%94%B9";//utf8 更改

            //winhttp->Send(strCountrySelectSend.c_str());

            //if (winhttp->Status != 200)
            //{
            //    DBG_TEXT_PUT(_T("send_appidbypass response err \n"));
            //    return false;
            //}

            ////bstr_t bstrAppleStoreFront = winhttp->GetResponseHeader("x-set-apple-store-front");
            ////m_session.m_AppleStoreFront = (LPCSTR)bstrAppleStoreFront;

            //if (!getDoc(winhttp, pIDoc3))
            //    return false;
            */
            string strPageID, strAgreePost;
            if (!getAttributeById(pIDoc3, "pageUUID", "value", strPageID))
                return false;

            MSHTML::IHTMLElementPtr pElemSignPtr = pIDoc3->getElementById("signature");//xAppleActionSignature=
            if (pElemSignPtr == NULL)
                return false;

            MSHTML::IHTMLElementPtr pElemAgreeFormPtr = pElemSignPtr->GetparentElement();
            if (!getAttribute(pElemAgreeFormPtr, "action", strAgreePost))
                return false;

            string str_AgreeName;
            if (!getAttributeById(pIDoc3, "hiddenBottomRightButtonId", "name", str_AgreeName))
                return false;

            //同意条款
            string strAgreeUrl = string("https://p") + m_session.m_serverid + string("-buy.itunes.apple.com") + strAgreePost;
            winhttp->Open("POST", strAgreeUrl.c_str(), false);
            winhttp->SetTimeouts(600000, 600000, 600000, 600000);
            winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-Client-Versions", m_session.m_AppleClientVersion.c_str());
            winhttp->SetRequestHeader("X-Apple-Partner", m_session.m_ApplePartner.c_str());
            winhttp->SetRequestHeader("X-Apple-Connection-Type", m_session.m_AppleConnectionType.c_str());
            winhttp->SetRequestHeader("Referer", /*strCountrySelectUrl.c_str()*/strUrlNext.c_str());
            winhttp->SetRequestHeader("Accept-Language", "zh-cn");
            winhttp->SetRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            winhttp->SetRequestHeader("Accept-Encoding", "gzip, deflate");
            //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
            Sleep(2000);
            string strAgreeSend;
            strAgreeSend += "mzPageUUID="; strAgreeSend += charset.URLEncode(strPageID);
            strAgreeSend += "&machineGUID="; strAgreeSend += m_session.m_guid;
            strAgreeSend += "&"; strAgreeSend += str_AgreeName; strAgreeSend += "=%E5%90%8C%E6%84%8F";//同意
            
            string strSignData = strPageID;
            strSignData += m_session.m_guid;
            if (!getSign(strSignData, strActionSignature))
            {
                DBG_TEXT_PUT(_T("send_appidbypass get_ActionSignature err"));
                return 0;
            }
            strAgreeSend += "&xAppleActionSignature="; strAgreeSend += charset.URLEncode(strActionSignature);

            winhttp->Send(strAgreeSend.c_str());

            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("send_appidbypass response err \n"));
                return false;
            }

            if (!getDoc(winhttp, pIDoc3))
                return false;

            MSHTML::IHTMLElementPtr pElemTempPtr = pIDoc3->getElementById("pageUUID");
            if (pElemTempPtr == NULL)
                return false;

            MSHTML::IHTMLElementPtr pElemSubmitFormPtr = pElemTempPtr->GetparentElement();
            if (pElemSubmitFormPtr == NULL)
                return false;
            
            string strSubmitFormPost;
            if (!getAttribute(pElemSubmitFormPtr, "action", strSubmitFormPost))
                return false;
            
            //账户设置
            string strPostSetting;
            if (getSettingInfo(pIDoc3, strPostSetting))
            {
                string strSettingUrl = string("https://p") + m_session.m_serverid + string("-buy.itunes.apple.com") + strSubmitFormPost;
                winhttp->Open("POST", strSettingUrl.c_str(), false);
                winhttp->SetTimeouts(600000, 600000, 600000, 600000);
                winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
                winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
                winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
                winhttp->SetRequestHeader("X-Apple-Client-Versions", m_session.m_AppleClientVersion.c_str());
                winhttp->SetRequestHeader("X-Apple-Partner", m_session.m_ApplePartner.c_str());
                winhttp->SetRequestHeader("X-Apple-Connection-Type", m_session.m_AppleConnectionType.c_str());
                winhttp->SetRequestHeader("Referer", strAgreeUrl.c_str());
                winhttp->SetRequestHeader("Accept-Language", "zh-cn");
                winhttp->SetRequestHeader("Content-Type", "application/x-www-form-urlencoded");
                winhttp->SetRequestHeader("Accept-Encoding", "gzip, deflate");
                //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
                winhttp->Send(strPostSetting.c_str());

                if (winhttp->Status != 200)
                {
                    DBG_TEXT_PUT(_T("send_appidbypass setting err \n"));
                    return false;
                }

                if (!getDoc(winhttp, pIDoc3))
                    return false;

                MSHTML::IHTMLElementPtr pElemTempPtr = pIDoc3->getElementById("pageUUID");
                if (pElemTempPtr == NULL)
                    return false;

                MSHTML::IHTMLElementPtr pElemSubmitFormPtr = pElemTempPtr->GetparentElement();
                if (pElemSubmitFormPtr == NULL)
                    return false;

                if (!getAttribute(pElemSubmitFormPtr, "action", strSubmitFormPost))
                    return false;

                strAgreeUrl = strSettingUrl;
            }

            //确认提交信息过检
            string strPost;
            if (!getBypassInfo(pIDoc3, strPost))
                return false;

            string strSubmitUrl = string("https://p") + m_session.m_serverid + string("-buy.itunes.apple.com") + strSubmitFormPost;
            winhttp->Open("POST", strSubmitUrl.c_str(), false);
            winhttp->SetTimeouts(600000, 600000, 600000, 600000);
            winhttp->SetRequestHeader("User-Agent", m_session.m_UserAgent.c_str());
            winhttp->SetRequestHeader("X-Apple-Store-Front", m_session.m_AppleStoreFront.c_str());
            winhttp->SetRequestHeader("X-Apple-Tz", m_session.m_AppleTz.c_str());
            winhttp->SetRequestHeader("X-Apple-Client-Versions", m_session.m_AppleClientVersion.c_str());
            winhttp->SetRequestHeader("X-Apple-Partner", m_session.m_ApplePartner.c_str());
            winhttp->SetRequestHeader("X-Apple-Connection-Type", m_session.m_AppleConnectionType.c_str());
            winhttp->SetRequestHeader("Referer", strAgreeUrl.c_str());
            winhttp->SetRequestHeader("Accept-Language", "zh-cn");
            winhttp->SetRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            winhttp->SetRequestHeader("Accept-Encoding", "gzip, deflate");
            //winhttp->SetCredentials("HQ6N4K7EYCI0366D", "7B3DD0387D45CB46", 1);
            winhttp->Send(strPost.c_str());
            Sleep(2000);
            if (winhttp->Status != 200)
            {
                DBG_TEXT_PUT(_T("send_appidbypass response err \n"));
                return false;
            }

            if (!getDoc(winhttp, pIDoc3))
                return false;

            string strSuccText;
            if (!getAttributeById(pIDoc3, "pk-first-navigation-content", "title", strSuccText))
                return false;

            if (strSuccText != "祝贺您！")
                return false;

        }
        catch (_com_error err)
        {
            DBG_TEXT_PUT(_T("send_appidbypass except err : %s"), (LPCTSTR)err.Description());
            return false;
        }

        return true;
    }
};