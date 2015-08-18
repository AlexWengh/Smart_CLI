
// CLI_Console.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "CLI_Console.h"
#include "MainFrm.h"

#include "CLI_ConsoleDoc.h"
#include "CLI_ConsoleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCLI_ConsoleApp

BEGIN_MESSAGE_MAP(CCLI_ConsoleApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CCLI_ConsoleApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CCLI_ConsoleApp construction

CCLI_ConsoleApp::CCLI_ConsoleApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Author.Marvin.CLI_Console.1.0.0"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_hReadPipeRead = NULL;
	m_hReadPipeWrite = NULL;
	m_hWritePipeRead = NULL;
	m_hWritePipeWrite = NULL;
	m_hSciDLL = NULL;
	m_pThread = NULL;
	memset(&m_pi, 0, sizeof(m_pi));
}

// The one and only CCLI_ConsoleApp object

CCLI_ConsoleApp theApp;


// CCLI_ConsoleApp initialization

BOOL CCLI_ConsoleApp::InitInstance()
{
	// Load the scintilla dll
	// Note: "..\\lib" is used for debug, when release out need to change this to ".\\lib"
	m_hSciDLL = LoadScintillaLibrary(_T("..\\lib"));
	if (m_hSciDLL == NULL)
	{
		AfxMessageBox(_T("Scintilla DLL is not installed, Please download the SciTE editor and copy the SciLexer.dll into this application's directory"));
		return FALSE;
	}

	// Create the pipe to read/write from the cmd window
	if (CreateCmdReadPipe() == FALSE)
	{
		AfxMessageBox(_T("Create read pipe failed"));
		return FALSE;
	}
	if (CreateCmdWritePipe() == FALSE)
	{
		AfxMessageBox(_T("Create write pipe failed"));
		return FALSE;
	}

	// Create the cmd window and make it non-visible
	if (CreateCmdProcess() == FALSE)
	{
		AfxMessageBox(_T("Create command line process failed"));
		return FALSE;
	}

	// Create the reading thread
	if (CreateReadingThread() == FALSE)
	{
		AfxMessageBox(_T("Create command line reading thread failed"));
		return FALSE;
	}


	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("CLI_Console_Application_Marvin"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CCLI_ConsoleDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CCLI_ConsoleView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

HMODULE CCLI_ConsoleApp::LoadScintillaLibrary(const LPCTSTR lpDir)
{
	// Get the Application directory
	TCHAR szFullPath[_MAX_PATH];
	szFullPath[0] = _T('\0');
	if (GetModuleFileName(NULL, szFullPath, _countof(szFullPath)) == 0)
		return NULL;

	// Form the new path
	TCHAR szDrive[_MAX_DRIVE];
	szDrive[0] = _T('\0');
	TCHAR szDir[_MAX_DIR];
	szDir[0] = _T('\0');
	_tsplitpath_s(szFullPath, szDrive, sizeof(szDrive) / sizeof(TCHAR), szDir, sizeof(szDir) / sizeof(TCHAR), NULL, 0, NULL, 0);
	lstrcat(szDir, lpDir);
	lstrcat(szDir, _T("\\"));
	TCHAR szFname[_MAX_FNAME];
	szFname[0] = _T('\0');
	TCHAR szExt[_MAX_EXT];
	szExt[0] = _T('\0');
	_tsplitpath_s(_T("SciLexer.dll"), NULL, 0, NULL, 0, szFname, sizeof(szFname) / sizeof(TCHAR), szExt, sizeof(szExt) / sizeof(TCHAR));
	_tmakepath_s(szFullPath, sizeof(szFullPath) / sizeof(TCHAR), szDrive, szDir, szFname, szExt);

	// Delegate to LoadLibrary
	return LoadLibrary(szFullPath);
}

int CCLI_ConsoleApp::ExitInstance()
{
	// End the reading thread
	m_needExit = TRUE;

	// Free up the Scintilla DLL
	if (m_hSciDLL)
		FreeLibrary(m_hSciDLL);

	if (m_hReadPipeRead)
		CloseHandle(m_hReadPipeRead);
	if (m_hReadPipeWrite)
		CloseHandle(m_hReadPipeWrite);
	if (m_hWritePipeRead)
		CloseHandle(m_hWritePipeRead);

	if (m_pi.hProcess)
	{
		WritePipe(CString(_T("exit\r\n")));

		if (m_hWritePipeWrite)
			CloseHandle(m_hWritePipeWrite);

		if (WaitForSingleObject(m_pi.hProcess, 5000) != WAIT_OBJECT_0)
			TerminateProcess(m_pi.hProcess, -1);

		CloseHandle(m_pi.hProcess);
	}
	else
	{
		if (m_hWritePipeWrite)
			CloseHandle(m_hWritePipeWrite);
	}

	if (m_pThread)
		WaitForSingleObject(m_pThread->m_hThread, 5000);

	return CWinApp::ExitInstance();
}


BOOL CCLI_ConsoleApp::CreateCmdReadPipe()
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&m_hReadPipeRead, &m_hReadPipeWrite, &sa, 0))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CCLI_ConsoleApp::CreateCmdWritePipe()
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&m_hWritePipeRead, &m_hWritePipeWrite, &sa, 0))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CCLI_ConsoleApp::CreateCmdProcess()
{
	STARTUPINFO si = { sizeof(si) };
	si.hStdInput = m_hWritePipeRead;
	si.hStdError = m_hReadPipeWrite;
	si.hStdOutput = m_hReadPipeWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	
	TCHAR cmdline[64];
	ZeroMemory(cmdline, sizeof(cmdline));
	lstrcpy(cmdline, _T("cmd"));

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreateProcess(NULL, cmdline, &sa, NULL, TRUE, NULL, NULL, NULL, &si, &m_pi))
	{
		return FALSE;
	}

	return TRUE;
}

// Thread Procedure
UINT ReadCmdOutputThreadFunc(LPVOID pParam)
{
	CCLI_ConsoleApp* pApp = (CCLI_ConsoleApp*)pParam;
	pApp->ReadPipe();
	return 0;
}

BOOL CCLI_ConsoleApp::CreateReadingThread()
{
	m_pThread = AfxBeginThread(ReadCmdOutputThreadFunc, this, 0, 0, CREATE_SUSPENDED, NULL);
	if (m_pThread == NULL)
		return FALSE;

	return TRUE;
}

BOOL CCLI_ConsoleApp::StartReadingThread()
{
	return (m_pThread->ResumeThread() != -1 ? TRUE : FALSE);
}

void CCLI_ConsoleApp::SetViewWnd(HWND hWnd)
{
	m_hViewWnd = hWnd;
}

// executed in thread.
void CCLI_ConsoleApp::ReadPipe()
{
	DWORD dwLen;
	while (TRUE)
	{
		if (m_needExit)
			return;

		ZeroMemory(m_readBuf, sizeof(m_readBuf));
		// blocking read.
		BOOL ret = ReadFile(m_hReadPipeRead, m_readBuf, 4096, &dwLen, NULL);

		if (m_needExit)
			return;

		if (!ret)
		{
			if (!m_needExit)
				SendMessage(m_hViewWnd, WM_CMD_READERROR, NULL, NULL);
			return;
		}

		if (m_needExit)
			return;
		
		if (dwLen > 0) 
		{
			std::string str = m_readBuf;
			m_readedContent = GetWideCharString(str);
			SendMessage(m_hViewWnd, WM_CMD_READED, NULL, NULL);
		}
	}
}

void CCLI_ConsoleApp::WritePipe(const CString& str)
{
	DWORD writedBytes = 0;
	std::string sstr = GetMultibuteString(str);
	int len = sstr.size();

	while (writedBytes != len)
	{
		DWORD dwLen;
		BOOL ret = WriteFile(m_hWritePipeWrite, sstr.c_str(), len, &dwLen, NULL);
		if (!ret)
		{
			AfxMessageBox(_T("Error writting to the command line. Please close this application."));
			break;
		}

		writedBytes += dwLen;
		if (dwLen < sstr.size())
			sstr = &sstr[dwLen + 1];
	}
}

CString CCLI_ConsoleApp::GetReadedContent()
{
	return m_readedContent;
}

BOOL CCLI_ConsoleApp::IsCmdWaitingInput()
{
	DWORD ret = WaitForSingleObject(m_hWritePipeRead, 0);
	if (ret == WAIT_TIMEOUT)
		return TRUE;

	return FALSE;
}

std::string CCLI_ConsoleApp::GetMultibuteString(const CString& str)
{
	int ansiLen = WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)str, -1, NULL, 0, NULL, NULL);
	char* pAnsiText = new char[ansiLen + 1];
	memset(pAnsiText, 0, sizeof(char) * (ansiLen + 1));
	WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)str, -1, pAnsiText, ansiLen, NULL, NULL);

	std::string ret = pAnsiText;
	delete[] pAnsiText;
	return ret;
}

CString CCLI_ConsoleApp::GetWideCharString(const std::string& str)
{
	int  unicodeLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t*  pUnicode = new  wchar_t[unicodeLen + 2];
	memset(pUnicode, 0, sizeof(wchar_t) * (unicodeLen + 2));
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);

	CString  ret = pUnicode;
	delete[]  pUnicode;
	return  ret;
}


// CCLI_ConsoleApp message handlers









// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// App command to run the dialog
void CCLI_ConsoleApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCLI_ConsoleApp message handlers





void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
