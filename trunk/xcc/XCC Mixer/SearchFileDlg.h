#if !defined(AFX_SEARCHFILEDLG_H__0D799940_7EAA_11D4_B606_0000B4936994__INCLUDED_)
#define AFX_SEARCHFILEDLG_H__0D799940_7EAA_11D4_B606_0000B4936994__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// SearchFileDlg.h : header file
//

#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CSearchFileDlg dialog

class CSearchFileDlg : public ETSLayoutDialog
{
// Construction
public:
	void open_mix(int id);
	void add(string name, string fname, int mix_id, int file_id);
	void find(Cmix_file& f, string file_name, string mix_name, int mix_id);
	void set(CMainFrame* main_frame);
	CSearchFileDlg(CWnd* pParent = NULL);   // standard constructor

	string get_filename() const
	{
		return m_filename;
	}

// Dialog Data
	//{{AFX_DATA(CSearchFileDlg)
	enum { IDD = IDD_SEARCH_FILE };
	CListCtrl	m_list;
	CString	m_filename;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSearchFileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFind();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	struct t_map_entry
	{
		// string name;
		int id;
		int parent;
	};

	typedef map<int, t_map_entry> t_map;

	CMainFrame* m_main_frame;
	t_map m_map;
	CString m_reg_key;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHFILEDLG_H__0D799940_7EAA_11D4_B606_0000B4936994__INCLUDED_)
