#if !defined(AFX_XCC_LMD_FILE_WRITE_H__5C6F2862_FDAB_11D3_B605_0000B4936994__INCLUDED_)
#define AFX_XCC_LMD_FILE_WRITE_H__5C6F2862_FDAB_11D3_B605_0000B4936994__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "cc_structures.h"
#include "virtual_binary.h"

using namespace std;

class Cxcc_lmd_file_write  
{
public:
	void add_fname(const string& fname);
	Cvirtual_binary write(t_game game) const;
private:
	typedef vector<string> t_index;
	t_index m_index;
};

#endif // !defined(AFX_XCC_LMD_FILE_WRITE_H__5C6F2862_FDAB_11D3_B605_0000B4936994__INCLUDED_)
