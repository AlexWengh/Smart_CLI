
// CLI_ConsoleView.cpp : implementation of the CCLI_ConsoleView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CLI_Console.h"
#endif

#include "CLI_ConsoleDoc.h"
#include "CLI_ConsoleView.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCLI_ConsoleView

IMPLEMENT_DYNCREATE(CCLI_ConsoleView, CScintillaView)

BEGIN_MESSAGE_MAP(CCLI_ConsoleView, CScintillaView)
	ON_MESSAGE(WM_CMD_READED, OnCmdReaded)
	ON_MESSAGE(WM_CMD_READERROR, OnCmdReadError)
END_MESSAGE_MAP()

// CCLI_ConsoleView construction/destruction

CCLI_ConsoleView::CCLI_ConsoleView()
{
	// TODO: add construction code here
	m_lastReadedPos = 0;
	m_lastPosBeforeTabFirstPressed = 0;
	m_lastTimeIsTabPressed = FALSE;
	m_subFileNamesIndex = -1;
}

CCLI_ConsoleView::~CCLI_ConsoleView()
{
}

BOOL CCLI_ConsoleView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CCLI_ConsoleView drawing

void CCLI_ConsoleView::OnDraw(CDC* /*pDC*/)
{
	CCLI_ConsoleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CCLI_ConsoleView diagnostics

#ifdef _DEBUG
void CCLI_ConsoleView::AssertValid() const
{
	CView::AssertValid();
}

void CCLI_ConsoleView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCLI_ConsoleDoc* CCLI_ConsoleView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCLI_ConsoleDoc)));
	return (CCLI_ConsoleDoc*)m_pDocument;
}
#endif //_DEBUG


// CCLI_ConsoleView message handlers


void CCLI_ConsoleView::OnInitialUpdate()
{
	CScintillaView::OnInitialUpdate();

	CScintillaCtrl& rCtrl = GetCtrl();
	rCtrl.StyleSetFont(STYLE_DEFAULT, _T("FixedSys"));
	rCtrl.StyleSetSize(STYLE_DEFAULT, 12);
	rCtrl.StyleSetBack(STYLE_DEFAULT, RGB(0, 0, 0));
	rCtrl.StyleSetFore(STYLE_DEFAULT, RGB(255, 255, 255));
	rCtrl.StyleClearAll();
	
	rCtrl.SetCaretFore(RGB(255, 255, 255));
	rCtrl.SetCaretStyle(CARETSTYLE_BLOCK);

	rCtrl.SetSelFore(TRUE, RGB(0, 0, 0));
	rCtrl.SetSelBack(TRUE, RGB(255, 255, 255));

	rCtrl.StyleSetVisible(STYLE_LINENUMBER, FALSE);
	rCtrl.SetCodePage(SC_CP_UTF8);
	rCtrl.SetEOLMode(SC_EOL_CRLF);

	CCLI_ConsoleApp* pApp = GetApp();
	pApp->SetViewWnd(m_hWnd);

	if (!GetApp()->StartReadingThread())
		AfxMessageBox(_T("Start the reading thread failed, please close this application."));
}

void CCLI_ConsoleView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CCLI_ConsoleApp* pApp = GetApp();
	CScintillaCtrl& rCtrl = GetCtrl();
	CString chars;

	if (pApp->IsCmdWaitingInput() == FALSE)
	{
		// discard any keyboard input when the process is not listening for input,
		// the authentic cmd window will cache these input but to implement this
		// is too complicated so just use this simple policy here.
		return;
	}

	// clear tab inputted flag
	if (nChar != VK_TAB)
	{
		m_lastTimeIsTabPressed = FALSE;
	}
	if (nChar != VK_RETURN)
	{
		// judge special characters.
		if (nChar == VK_BACK)
		{
			// delete back to last position.
			for (UINT i = 0; i < nRepCnt; ++i)
			{
				long curPos = rCtrl.GetCurrentPos();
				if (curPos > m_lastReadedPos)
				{
					rCtrl.DeleteBack();
					ResetInputtedCache();
				}
				else
					break;
			}
		}
		else if (nChar == VK_TAB)
		{
			// first, to simplify, if caret is not at the last position, this is not supported.
			long curPos = rCtrl.GetCurrentPos();
			long length = rCtrl.GetLength();
			if (curPos != length)
				return;

			// second, if previous is not tab input, mark tab has pressed, nRepCnt is ignored to be simplify,
			// and we only support one tab press at a time.
			if (m_lastTimeIsTabPressed == FALSE)
			{
				m_lastTimeIsTabPressed = TRUE;
				m_subFileNames.clear();
				m_subFileNamesIndex = -1;

				// third, try to get the current directory, and the current inputted all line chars.
				int lastLineNum = rCtrl.LineFromPosition(rCtrl.GetLength());
				CString lineContent = rCtrl.GetLine(lastLineNum);
				int promptPos = lineContent.Find(_T(">"));
				if (promptPos == -1)
				{
					// not find the prompt char, the prompt char can be changed in cmd command, but to simplify
					// we only support the ">" prompt char.
					return;
				}

				// get the actually text need to match fileName.
				int len = rCtrl.GetLength();
				m_lastPosBeforeTabFirstPressed = rCtrl.GetLength() - lineContent.GetLength() + promptPos + 1;
				CString lineInputted = lineContent.Mid(promptPos + 1);
				int i;
				BOOL find = false;
				for (i = lineInputted.GetLength() - 1; i >= 0; --i, --len)
				{
					if (lineInputted[i] == _T(' ') || lineInputted[i] == _T('\\'))
					{
						lineInputted = lineInputted.Mid(i + 1);
						m_lastPosBeforeTabFirstPressed = len;
						break;
					}
				}

				// fourth, get a list of all sub directories and sub files in this current directory
				CString curDir = lineContent.Left(promptPos);
				curDir += _T("\\*.*");
				CFileFind finder;
				BOOL hasNext = finder.FindFile(curDir);
				while (hasNext)
				{
					hasNext = finder.FindNextFile();
					CString fileName = finder.GetFileName();
					if (fileName.Find(_T(" ")) != -1)	// add " "
					{
						fileName = _T("\"") + fileName + _T("\"");
					}
					if (fileName != _T(".") && fileName != _T(".."))
					{
						int pos = fileName.Find(lineInputted);
						if (pos == 0 
							|| (pos == 1 && fileName[pos - 1] == _T('\"'))
							|| (pos == 1 && fileName[pos - 1] == _T('.'))
							|| (pos == 2 && fileName[pos - 1] == _T('.') && fileName[pos - 2] == _T('\"')))
						// find a sub directory or folder matching the inputted.
						m_subFileNames.push_back(fileName);
					}
				}

				// fifth show the first subFileName and set the index,
				// if backspace is pressed, then a tab is pressed, then need to 
				// remove currently partly displayed filename 
				if (m_subFileNames.size() > 0)
				{
					rCtrl.DeleteRange(m_lastPosBeforeTabFirstPressed, rCtrl.GetLength() - m_lastPosBeforeTabFirstPressed);

					// add the first fileName
					m_subFileNamesIndex = (m_subFileNamesIndex + 1) % m_subFileNames.size();
					rCtrl.AddText(m_subFileNames[m_subFileNamesIndex].GetLength(), m_subFileNames[m_subFileNamesIndex]);
					ResetInputtedCache();
				}
			}
			else
			{
				// sixth, last time inputted is also tab, need to remove currently displayed filename 
				// and show the next one

				// delete the currently shown one fileName
				// m_lastWritedPos saves the last position before any continues tab input happened.
				if (m_subFileNames.size() > 0)
				{
					rCtrl.DeleteRange(m_lastPosBeforeTabFirstPressed, m_subFileNames[m_subFileNamesIndex].GetLength());

					// add the next fileName
					m_subFileNamesIndex = (m_subFileNamesIndex + 1) % m_subFileNames.size();
					rCtrl.AddText(m_subFileNames[m_subFileNamesIndex].GetLength(), m_subFileNames[m_subFileNamesIndex]);
					ResetInputtedCache();
				}
			}
		}
		else    // not special characters.
		{
			for (UINT i = 0; i < nRepCnt; ++i)
				chars += (wchar_t)(nChar);

			rCtrl.AddText(chars.GetLength(), chars);
			ResetInputtedCache();

			int lastLineNum = rCtrl.LineFromPosition(rCtrl.GetLength());
			CString lineContent = rCtrl.GetLine(lastLineNum);

			TryShowHint(lineContent);
		}
	}
	else
	{
		int totalLength = rCtrl.GetLength();
		rCtrl.DeleteRange(m_lastReadedPos, totalLength - m_lastReadedPos);

		for (UINT i = 0; i < nRepCnt; ++i)
			m_inputCached += _T("\r\n");

		pApp->WritePipe(m_inputCached);
		m_inputCached = _T("");
	}
}

LRESULT CCLI_ConsoleView::OnCmdReaded(WPARAM wParam, LPARAM lParam)
{
	CCLI_ConsoleApp* pApp = GetApp();
	CString content = pApp->GetReadedContent();

	CScintillaCtrl& rCtrl = GetCtrl();

	rCtrl.AddText(content.GetLength(), content);
	rCtrl.ScrollToEnd();
	rCtrl.ScrollCaret();

	m_lastReadedPos = rCtrl.GetLength();
	m_lastPosBeforeTabFirstPressed = m_lastReadedPos;
	return 0;
}

void CCLI_ConsoleView::ResetInputtedCache()
{
	CScintillaCtrl& rCtrl = GetCtrl();
	int lastLineNum = rCtrl.LineFromPosition(rCtrl.GetLength());
	CString lineContent = rCtrl.GetLine(lastLineNum);
	m_inputCached = lineContent.Mid(m_lastReadedPos);
}

void CCLI_ConsoleView::TryShowHint(const CString& lineContent)
{

}

LRESULT CCLI_ConsoleView::OnCmdReadError(WPARAM wParam, LPARAM lParam)
{
	AfxMessageBox(_T("Error reading from the command line. Please close this application."));
	return 0;
}

// Capture WM_KEYDOWN, WM_CHAR message before Scintilla Control captured it.
BOOL CCLI_ConsoleView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_CHAR) {
		OnChar(pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		return TRUE;
	}
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam)
		{
		case 'C':
			if (GetKeyState(VK_CONTROL) < 0)
			{
				return TRUE;
			}
			else
			{
				OnChar(pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
				return TRUE;
			}
		case VK_BACK:
			OnChar(pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
			return TRUE;
		case VK_TAB:
			OnChar(pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
			return TRUE;
		default:
			break;
		}
	}

	return CScintillaView::PreTranslateMessage(pMsg);
}

CCLI_ConsoleApp* CCLI_ConsoleView::GetApp()
{
	CCLI_ConsoleApp* pApp = static_cast<CCLI_ConsoleApp*>(AfxGetApp());
	return pApp;
}
