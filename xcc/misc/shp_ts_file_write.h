// shp_ts_file_write.h: interface for the Cshp_ts_file_write class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHP_TS_FILE_WRITE_H__08424461_01C6_11D4_B605_0000B4936994__INCLUDED_)
#define AFX_SHP_TS_FILE_WRITE_H__08424461_01C6_11D4_B605_0000B4936994__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "virtual_file.h"
#include "virtual_image.h"

int shp_ts_file_write(const byte* s, byte* d, int cx, int cy, int c_images, bool enable_compression = true);
void shp_ts_file_write(const Cvirtual_image& image, Cvirtual_file& f, int c_images, bool enable_compression = true);

#endif // !defined(AFX_SHP_TS_FILE_WRITE_H__08424461_01C6_11D4_B605_0000B4936994__INCLUDED_)