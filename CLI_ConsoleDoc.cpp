
// CLI_ConsoleDoc.cpp : implementation of the CCLI_ConsoleDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CLI_Console.h"
#endif

#include "CLI_ConsoleDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCLI_ConsoleDoc

IMPLEMENT_DYNCREATE(CCLI_ConsoleDoc, CScintillaDoc)

BEGIN_MESSAGE_MAP(CCLI_ConsoleDoc, CScintillaDoc)
END_MESSAGE_MAP()


// CCLI_ConsoleDoc construction/destruction

CCLI_ConsoleDoc::CCLI_ConsoleDoc()
{
	// TODO: add one-time construction code here

}

CCLI_ConsoleDoc::~CCLI_ConsoleDoc()
{
}

BOOL CCLI_ConsoleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CCLI_ConsoleDoc serialization

void CCLI_ConsoleDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CCLI_ConsoleDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CCLI_ConsoleDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CCLI_ConsoleDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCLI_ConsoleDoc diagnostics

#ifdef _DEBUG
void CCLI_ConsoleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCLI_ConsoleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCLI_ConsoleDoc commands


BOOL CCLI_ConsoleDoc::SaveModified()
{
	return true;

	// never try to pop up the "save changes to Untitled" dialog
	
	// return CScintillaDoc::SaveModified();
}
