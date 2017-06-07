#pragma once
#include "stdafx.h"
#include "ItunesProtocolCommon.h"
#include <ras.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

#pragma comment(lib, "Rasapi32.lib")

class CMainDlg;
class CTaskBase
{
public:
    virtual void doTask(void *param) = 0;
};

class CTask : public CTaskBase
{

public:
    CTask(CItunesSession session) :
        m_session(session),
        m_pMainDlg(NULL)
    {

    }
    virtual ~CTask()
    {
    }
    void AppendLog(LPTSTR pszFormat, ...);
    virtual void doTask(void *pvWorkParam);
private:
    CItunesSession m_session;
    CMainDlg *m_pMainDlg;
};

class CWorker
{
public:
    typedef DWORD_PTR RequestType;
    CWorker()
    {
        m_nMyWorkerTask = 0;
    }
    virtual BOOL Initialize(void *pvWorkerParam)
    {
        CoInitialize(NULL);
        return TRUE;
    }
    virtual void Terminate(void *pvWorkerParam)
    {

    }
    void Execute(
        RequestType request,
        void *pvWorkerParam,
        OVERLAPPED *pOverlapped)
    {
        DBG_TEXT_PUT(_T("##############################################do task start#################################################"));


        //HRASCONN  hrasconn = NULL;
        //RASDIALPARAMS rdParams;
        //memset(&rdParams, 0, sizeof(rdParams));
        //rdParams.dwSize = sizeof(RASDIALPARAMS);
        //_tcscpy(rdParams.szEntryName, _T("宽带连接"));
        //_tcscpy(rdParams.szUserName, _T("02792642507"));
        //_tcscpy(rdParams.szPassword, _T("123457"));
        //int i;
        //for (i = 0; i < 3; i++)
        //{
        //    if ((RasDial(NULL, NULL, &rdParams, 0L, NULL, &hrasconn)) == ERROR_SUCCESS)//连接
        //    {
        //        break;
        //    }
        //}
        //if (i == 3)
        //    return;

        CTask * pTask = reinterpret_cast<CTask *>(request);
        pTask->doTask(pvWorkerParam);
        delete pTask;
        //RasHangUp(hrasconn);

        DBG_TEXT_PUT(_T("##############################################do task end###################################################"));
        InterlockedIncrement(&m_nTask);
        m_nMyWorkerTask++;
    }
public:
    ULONG m_nMyWorkerTask;
    static ULONG m_nTask;
};

extern ATL::CThreadPool<CWorker> g_ThreadPool;