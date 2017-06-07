#include "stdafx.h"
#include "SendWork.h"
#include "MainDlg.h"
#include "ItunesProtocol.h"

ULONG CWorker::m_nTask = 0;
ATL::CThreadPool<CWorker> g_ThreadPool;

void CTask::AppendLog(LPTSTR pszFormat, ...)
{
    va_list argList;
    va_start(argList, pszFormat);
    TCHAR szDest[1024] = { 0 };
    _vsntprintf_s(szDest, 1024, pszFormat, argList);
    va_end(argList);
    
    m_pMainDlg->m_edit_log.AppendText(szDest);
}

void CTask::doTask(void *pvWorkParam)
{
    VMPBegin("doTask");
    USES_CONVERSION;

    m_pMainDlg = reinterpret_cast<CMainDlg *>(pvWorkParam);
    CItunesProtocol itunes_protocol(m_session);
    
    if (!itunes_protocol.initNum2_Proxy())
    {
        AppendLog(_T("初始化1失败。 %s|%s|%s\r\n"), 
            _CA2T(itunes_protocol.m_session.m_appid.c_str()), 
            _CA2T(itunes_protocol.m_session.m_guid.c_str()),
            _CA2T(itunes_protocol.m_session.getProxyString().c_str()));
        return;
    }
   
    bool binitcert = itunes_protocol.initCert_Proxy(
        itunes_protocol.m_session.getProxyString(),
        itunes_protocol.m_session.m_UserAgent,
        itunes_protocol.m_session.m_AppleTz,
        itunes_protocol.m_session.m_AppleStoreFront);
    if (!binitcert)
    {
        AppendLog(_T("初始化2失败。 %s|%s|%s\r\n"),
            _CA2T(itunes_protocol.m_session.m_appid.c_str()),
            _CA2T(itunes_protocol.m_session.m_guid.c_str()),
            _CA2T(itunes_protocol.m_session.getProxyString().c_str()));
        return;
    }

    AppendLog(_T("初始化1和2成功。 %s|%s|%s\r\n"),
        _CA2T(itunes_protocol.m_session.m_appid.c_str()),
        _CA2T(itunes_protocol.m_session.m_guid.c_str()),
        _CA2T(itunes_protocol.m_session.getProxyString().c_str()));

    int err = itunes_protocol.send_authenticate_pack(false, "signIn");
    if (err == -2)
    {
        err = itunes_protocol.send_authenticate_pack(false, "signIn");
    }
    if (err == 1)
    {
        AppendLog(_T("登录成功。 %s|%s|%s\r\n"),
            _CA2T(itunes_protocol.m_session.m_appid.c_str()),
            _CA2T(itunes_protocol.m_session.m_guid.c_str()),
            _CA2T(itunes_protocol.m_session.getProxyString().c_str()));

        return;
    }
    if (err <= 0 && itunes_protocol.m_session.m_failureType != "5001")
    {
        AppendLog(_T("登录失败。 %s|%s|%s|%s\r\n"),
            _CA2T(itunes_protocol.m_session.m_appid.c_str()),
            _CA2T(itunes_protocol.m_session.m_guid.c_str()),
            _CA2T(itunes_protocol.m_session.getProxyString().c_str()),
            _CA2T(itunes_protocol.m_session.m_customerMessage.c_str()));

        return;
    }
    AppendLog(_T("需要过检。 %s|%s|%s\r\n"),
        _CA2T(itunes_protocol.m_session.m_appid.c_str()),
        _CA2T(itunes_protocol.m_session.m_guid.c_str()),
        _CA2T(itunes_protocol.m_session.getProxyString().c_str()));

    string strCookies;
    string str_name_temp = "wosid";
    if (itunes_protocol.m_session.m_cookie.end() != itunes_protocol.m_session.m_cookie.find(str_name_temp))
    {
        strCookies += str_name_temp;
        strCookies += "=";
        strCookies += itunes_protocol.m_session.m_cookie.find(str_name_temp)->second;
        strCookies += ";";
    }
    str_name_temp = "ns-mzf-inst";
    if (itunes_protocol.m_session.m_cookie.end() != itunes_protocol.m_session.m_cookie.find(str_name_temp))
    {
        strCookies += str_name_temp;
        strCookies += "=";
        strCookies += itunes_protocol.m_session.m_cookie.find(str_name_temp)->second;
        strCookies += ";";
    }
    strCookies += "itspod="; strCookies += itunes_protocol.m_session.m_serverid;

    //Cookie:wosid=efPFc8jdT0GwYW9OKTV6Tg; ns-mzf-inst=39-89-443-124-74-8170-302404-30-nk11;
    if (!itunes_protocol.send_appidbypass(strCookies, "signIn"))
    {
        AppendLog(_T("过检失败。 %s|%s|%s\r\n"),
            _CA2T(itunes_protocol.m_session.m_appid.c_str()),
            _CA2T(itunes_protocol.m_session.m_guid.c_str()),
            _CA2T(itunes_protocol.m_session.getProxyString().c_str()));

        return;
    }

    AppendLog(_T("过检成功。 %s|%s|%s\r\n"),
        _CA2T(itunes_protocol.m_session.m_appid.c_str()),
        _CA2T(itunes_protocol.m_session.m_guid.c_str()),
        _CA2T(itunes_protocol.m_session.getProxyString().c_str()));

    return ;

    VMPEnd();
}