// SplitterDoc.h : interface of the CSplitterDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLITTERDOC_H__0EE870AA_0CF6_11D5_866A_E8FF0541A007__INCLUDED_)
#define AFX_SPLITTERDOC_H__0EE870AA_0CF6_11D5_866A_E8FF0541A007__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSplitterDoc : public CDocument
{
protected: // create from serialization only
	CSplitterDoc();
	DECLARE_DYNCREATE(CSplitterDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitterDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSplitterDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSplitterDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLITTERDOC_H__0EE870AA_0CF6_11D5_866A_E8FF0541A007__INCLUDED_)
