
// CLI_ConsoleView.h : interface of the CCLI_ConsoleView class
//

#pragma once

#include "ScintillaDocView.h"

#include <vector>

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

class CCLI_ConsoleView : public Scintilla::CScintillaView
{
public:
	CCLI_ConsoleView();
	DECLARE_DYNCREATE(CCLI_ConsoleView)

// Attributes
public:
	CCLI_ConsoleDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CCLI_ConsoleView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

public:
	afx_msg LRESULT OnCmdReaded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCmdReadError(WPARAM wParam, LPARAM lParam);
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

private:
	CCLI_ConsoleApp* GetApp();
	void TryShowHint(const CString& lineContent);
	void ResetInputtedCache();

	CString m_inputCached;
	long m_lastReadedPos;

	BOOL m_lastTimeIsTabPressed;
	int m_lastPosBeforeTabFirstPressed;
	std::vector<CString> m_subFileNames;
	int m_subFileNamesIndex;
};

#ifndef _DEBUG  // debug version in CLI_ConsoleView.cpp
inline CCLI_ConsoleDoc* CCLI_ConsoleView::GetDocument() const
   { return reinterpret_cast<CCLI_ConsoleDoc*>(m_pDocument); }
#endif

