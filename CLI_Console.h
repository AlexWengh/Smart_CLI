
// CLI_Console.h : main header file for the CLI_Console application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <string>

// CCLI_ConsoleApp:
// See CLI_Console.cpp for the implementation of this class
//

#define WM_CMD_READED					WM_USER + 999
#define WM_CMD_READERROR				WM_USER + 1000

class CCLI_ConsoleApp : public CWinApp
{
public:
	CCLI_ConsoleApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()


public:
	void ReadPipe();
	void WritePipe(const CString& str);

	CString GetReadedContent();

	BOOL IsCmdWaitingInput();

	BOOL StartReadingThread();
	void SetViewWnd(HWND hWnd);

private:
	HMODULE LoadScintillaLibrary(const LPCTSTR lpDir);
	BOOL CreateCmdReadPipe();
	BOOL CreateCmdWritePipe();
	BOOL CreateCmdProcess();
	BOOL CreateReadingThread();

	std::string GetMultibuteString(const CString& str);
	CString GetWideCharString(const std::string& str);

	HANDLE m_hReadPipeRead;
	HANDLE m_hReadPipeWrite;
	HANDLE m_hWritePipeRead;
	HANDLE m_hWritePipeWrite;

	PROCESS_INFORMATION m_pi;

	HINSTANCE m_hSciDLL;

	char m_readBuf[4108];

	CString m_readedContent;

	volatile BOOL m_needExit;

	CWinThread* m_pThread;

	volatile HWND m_hViewWnd;
};

extern CCLI_ConsoleApp theApp;
