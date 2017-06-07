// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "stdafx.h"
#include "resource.h"
#include "ItunesInterface.h"
#include "SendWork.h"
#include <atlcrack.h>
#include <atlddx.h>

class CUserInfo
{
public:
    string m_strappid;
    string m_strpassword;
    string m_strguid;
};

class CMainDlg : 
    public CDialogImpl<CMainDlg>, 
    public CWinDataExchange<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_COMMAND(OnCommand)
        MSG_WM_CLOSE(OnClose)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CMainDlg)
        DDX_TEXT(IDC_EDIT_APPID, m_strAPPIDFile)
        DDX_TEXT(IDC_EDIT_PROXY, m_strProxyFile)
        DDX_TEXT(IDC_COMBO_AGENT, m_strUserAgent)
    END_DDX_MAP()

    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);
        AttachAllControl();

        SetWindowText(MAIN_CAPTION);
        if (!CItunesInterface::InitItunes())
        {
            MessageBox(_T("初始化失败"), MAIN_CAPTION);
            ExitProcess(0);
        }
        m_edit_log.AppendText(_T("初始化成功...\r\n"));

        g_ThreadPool.Initialize(this, 50);

        m_strUserAgent = _T("AppStore/2.0 iOS/8.1.3 model/iPhone6,2 build/12B466 (6; dt:90)");
        m_comb_agent.AddString(_T("AppStore/2.0 iOS/8.1.3 model/iPhone6,2 build/12B466 (6; dt:90)"));

        m_btn_start.EnableWindow(TRUE);
        m_btn_stop.EnableWindow(FALSE);

        DoDataExchange(DDX_LOAD);

		return TRUE;
	}
    void OnClose()
    {
        EndDialog(0);
    }
    void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
    {
        if (nID == ID_APP_ABOUT)
        {
            CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
            dlg.DoModal();
            
            //CItunesInterface obj;
            //CItunesHttpHeaderInfo info;

            //obj.initCert_Proxy("", info.m_UserAgent, info.m_AppleTz, info.m_AppleStoreFront);
        }
        else if(nID == IDOK)
        {
            m_btn_start.EnableWindow(FALSE);
            m_btn_stop.EnableWindow(TRUE);
            DoDataExchange(DDX_SAVE);
            DisableAllControl();
            
            OnStart();
        }
        else if (nID == IDCANCEL)
        {
            m_btn_stop.EnableWindow(FALSE);
            m_btn_start.EnableWindow(TRUE);
            EnableAllControl();

            OnStop();
        }
        else if (nID == IDC_BUTTON_APPID)
        {
            CFileDialog fileDlg(TRUE);
            if (IDOK == fileDlg.DoModal(m_hWnd))
            {
                m_strAPPIDFile = fileDlg.m_szFileName;
                DoDataExchange(DDX_LOAD);
            }
        }
        else if (nID == IDC_BUTTON_PROXY)
        {
            CFileDialog fileDlg(TRUE);
            if (IDOK == fileDlg.DoModal(m_hWnd))
            {
                m_strProxyFile = fileDlg.m_szFileName;
                DoDataExchange(DDX_LOAD);
            }
        }

	}
    void AttachAllControl()
    {
        m_edit_log = GetDlgItem(IDC_EDIT_LOG);
        m_comb_agent = GetDlgItem(IDC_COMBO_AGENT);
        m_edit_proxy_file = GetDlgItem(IDC_EDIT_PROXY);
        m_edit_appid_file = GetDlgItem(IDC_EDIT_APPID);
        m_btn_proxy_file = GetDlgItem(IDC_BUTTON_PROXY);
        m_btn_appid_file = GetDlgItem(IDC_BUTTON_APPID);
        m_btn_start = GetDlgItem(IDOK);
        m_btn_stop = GetDlgItem(IDCANCEL);
    }
    void DisableAllControl()
    {
        m_edit_appid_file.EnableWindow(FALSE);
        m_edit_proxy_file.EnableWindow(FALSE);
        m_btn_appid_file.EnableWindow(FALSE);
        m_btn_proxy_file.EnableWindow(FALSE);
        m_comb_agent.EnableWindow(FALSE);
    }
    void EnableAllControl()
    {
        m_edit_appid_file.EnableWindow(TRUE);
        m_edit_proxy_file.EnableWindow(TRUE);
        m_btn_appid_file.EnableWindow(TRUE);
        m_btn_proxy_file.EnableWindow(TRUE);
        m_comb_agent.EnableWindow(TRUE);
    }
    void OnStart()
    {
        USES_CONVERSION;

        m_proxys.clear();
        getips(CT2A(m_strProxyFile.GetString()), m_proxys);
        m_appids.clear();
        getappids(CT2A(m_strAPPIDFile.GetString()), m_appids);
        saveappids("bak.txt", m_appids);

        int n = m_proxys.size();
        if (n == 0)
            return;

        int i = 0;
        for (vector<CUserInfo>::iterator iter = m_appids.begin(); iter != m_appids.end(); iter++)
        {
            string str_port, str_ip;
            if (i >= n)
            {
                i = 0;
            }

            string::size_type pos = m_proxys[i].find(':');
            str_ip = m_proxys[i].substr(0, pos);
            str_port = m_proxys[i].substr(pos + 1);
            i++;

            CItunesSession Session;
            Session.m_appid = iter->m_strappid;
            Session.m_password = iter->m_strpassword;
            Session.m_ProxyServer = str_ip;
            Session.m_ProxyPort = str_port;
            Session.m_guid = iter->m_strguid;
            Session.m_machine = CItunesDeviceInfo::makeComputerName();

            Session.m_UserAgent = CT2A(m_strUserAgent.GetString());

            CTask *pTask = new CTask(Session);
            g_ThreadPool.QueueRequest(reinterpret_cast<CWorker::RequestType>(pTask));
        }
    }
    void OnStop()
    {

    }
    void saveappids(const char *szFile, vector<CUserInfo> &map_appids)
    {
        FILE *fp = fopen(szFile, "a+");
        if (fp == NULL)
            return;
        for (vector<CUserInfo>::iterator iter = map_appids.begin(); iter != map_appids.end(); iter++)
        {
            string str_line = iter->m_strappid + "\t" + iter->m_strpassword + "\t" + iter->m_strguid + "\r\n";
            fwrite(str_line.c_str(), 1, str_line.length(), fp);
        }

        fclose(fp);
        return;
    }
    void getappids(const char *szFile, vector<CUserInfo> &map_appids)
    {
        FILE *fp = fopen(szFile, "r");
        if (fp == NULL)
            return;
        char line[1000] = { 0 };
        while (line == fgets(line, 1000, fp))
        {
            int n = strlen(line);
            if (n >= 1 && line[n - 1] == '\n')
                line[--n] = '\0';
            if (n >= 1 && line[n - 1] == '\r')
                line[--n] = '\0';
            string str_ip, str_port, str_guid;
            vector<string> str_items;
            Split(line, "\t", str_items);
            if (str_items.size() != 2 && str_items.size() != 3)
                continue;
            CUserInfo userItem;
            if (str_items.size() == 2)
            {
                userItem.m_strappid = Trim(str_items[0]);
                userItem.m_strpassword = Trim(str_items[1]);
                userItem.m_strguid = CItunesDeviceInfo::makePhoneGUID();
            }
            else
            {
                userItem.m_strappid = Trim(str_items[0]);
                userItem.m_strpassword = Trim(str_items[1]);
                userItem.m_strguid = Trim(str_items[2]);
            }
            map_appids.push_back(userItem);
        }
        fclose(fp);
    }
    void getips(const char *szFile, vector<string> &map_ips)
    {
        FILE *fp = fopen(szFile, "r");
        if (fp == NULL)
            return;
        char line[1000] = { 0 };
        while (line == fgets(line, 1000, fp))
        {
            int n = strlen(line);
            if (n >= 1 && line[n - 1] == '\n')
                line[--n] = '\0';
            if (n >= 1 && line[n - 1] == '\r')
                line[--n] = '\0';
            string str_ip, str_port, str_line;
            str_line = line;
            string::size_type pos = str_line.find(':');
            if (pos == string::npos)
            {
                pos = str_line.find('\t');
                if (pos == string::npos)
                {
                    str_port = "8080";
                    str_ip = str_line;
                }
                else
                {
                    str_ip = str_line.substr(0, pos);
                    str_port = str_line.substr(pos + 1);
                }

            }
            else
            {
                str_ip = str_line.substr(0, pos);
                str_port = str_line.substr(pos + 1);
            }

            u_long ip = inet_addr(str_ip.c_str());
            if (INADDR_NONE == ip || 0 == ip)
                continue;
            USES_CONVERSION;
            u_short port = _ttoi(_CA2T(str_port.c_str()));
            if (port == 0 || port == 0xffff)
                continue;
            map_ips.push_back(str_ip + ":" + str_port);
        }
        fclose(fp);
    }

public:
    CEdit m_edit_log;
private:

    CString m_strAPPIDFile;
    CString m_strProxyFile;
    CString m_strUserAgent;

    CEdit m_edit_appid_file;
    CEdit m_edit_proxy_file;
    CComboBox m_comb_agent;
    CButton m_btn_appid_file;
    CButton m_btn_proxy_file;
    CButton m_btn_start;
    CButton m_btn_stop;

    vector<CUserInfo> m_appids;
    vector<string> m_proxys;
};
