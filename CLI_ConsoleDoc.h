
// CLI_ConsoleDoc.h : interface of the CCLI_ConsoleDoc class
//


#pragma once

#include "ScintillaDocView.h"

class CCLI_ConsoleDoc : public Scintilla::CScintillaDoc
{
public:
	CCLI_ConsoleDoc();
	DECLARE_DYNCREATE(CCLI_ConsoleDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CCLI_ConsoleDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
	virtual BOOL SaveModified();
};
