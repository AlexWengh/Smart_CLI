
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"

#include "CLI_Console.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_MOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~FWS_ADDTOTITLE;    // no "untitled-" in title

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers



int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// hide menu bar
	SetMenu(NULL);

	return 0;
}


void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CScintillaView* pView = GetScintillaView();
	if (pView != NULL)
	{
		const MSG* pMsg = GetCurrentMessage();
		pView->GetCtrl().SendMessage(WM_SETTINGCHANGE, pMsg->wParam, pMsg->lParam);
	}

	CFrameWnd::OnSettingChange(uFlags, lpszSection);
}


void CMainFrame::OnSysColorChange()
{
	CScintillaView* pView = GetScintillaView();
	if (pView != NULL)
	{
		const MSG* pMsg = GetCurrentMessage();
		pView->GetCtrl().SendMessage(WM_SYSCOLORCHANGE, pMsg->wParam, pMsg->lParam);
	}

	CFrameWnd::OnSysColorChange();
}


void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
	CScintillaView* pView = GetScintillaView();
	if (pView != NULL)
	{
		const MSG* pMsg = GetCurrentMessage();
		pView->GetCtrl().SendMessage(WM_PALETTECHANGED, pMsg->wParam, pMsg->lParam);
	}

	CFrameWnd::OnPaletteChanged(pFocusWnd);
}


BOOL CMainFrame::OnQueryNewPalette()
{
	CScintillaView* pView = GetScintillaView();
	if (pView != NULL)
	{
		const MSG* pMsg = GetCurrentMessage();
		pView->GetCtrl().SendMessage(WM_QUERYNEWPALETTE, pMsg->wParam, pMsg->lParam);
	}

	return CFrameWnd::OnQueryNewPalette();
}

CScintillaView* CMainFrame::GetScintillaView()
{
	CScintillaView* pView = static_cast<CScintillaView*>(GetActiveView());
	if (pView != NULL)
	{
		if (pView->IsKindOf(RUNTIME_CLASS(CScintillaView)))
		{
			return pView;
		}
	}
	return NULL;
}


void CMainFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);

	CScintillaView* pView = GetScintillaView();
	if (pView != NULL)
	{
		CScintillaCtrl& rCtrl = pView->GetCtrl();

		// Cancel any outstanding call tip
		if (rCtrl.CallTipActive())
			rCtrl.CallTipCancel();
	}
}


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	CScintillaView* pView = GetScintillaView();
	if (pView != NULL)
	{
		CScintillaCtrl& rCtrl = pView->GetCtrl();

		// Cancel any outstanding call tip
		if (rCtrl.CallTipActive())
			rCtrl.CallTipCancel();
	}
}
