// xcc_template_images.h: interface for the Cxcc_templates class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XCC_TEMPLATE_IMAGES_H__4E596E39_F686_11D2_B601_D35C3DCCB8F1__INCLUDED_)
#define AFX_XCC_TEMPLATE_IMAGES_H__4E596E39_F686_11D2_B601_D35C3DCCB8F1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <string>
#include "fname.h"
#include "palet.h"
#include "shp_images.h"
#include "tmp_file.h"
#include "xcc_dirs.h"
#include "xcc_level.h"
#include "xcc_mixs.h"
#include "xif_key.h"

using namespace std;

enum
{
	td_flags_ridge = 1,
	td_flags_river = 2,
	td_flags_road = 4,
	td_flags_shore = 8,
	td_flags_type_specific = td_flags_ridge | td_flags_river | td_flags_road | td_flags_shore
};

struct t_template_data_entry
{
	int cx;
	int cy;
	int c_images;
	qword buildable;
	qword moveable;
	int flags;
};

class Cxcc_templates  
{
private:
	static void* bib[3];
	static byte* image_list[256];
	static t_palet palet;
	static byte shade_rp[256];
	static t_template_data_entry template_data[256];
	static byte template_list[256][64];
public:
	~Cxcc_templates();
	static int load_data();
	static int save_data();
	static int load_images(t_theater_id theater);

	static const byte* get_image(int v)
	{
		return image_list[v >> 8] + 576 * (v & 0xff);
	}

	static const t_palet& get_palet()
	{
		return palet;
	}

	static const t_template_data_entry& get_template_data(int i)
	{
		return template_data[i];
	}

	static const byte* get_shade_rp()
	{
		return shade_rp;
	}

	static int convert_bin_data(int v)
	{
		return (v & 0xff) << 8 | template_list[v & 0xff][v >> 8];
	}

	static int deconvert_bin_data(int v)
	{
		int a = v >> 8;
		int b = 0;
		while (template_list[a][b] != (v & 0xff))
			b++;
		return b << 8 | a;
	}

	static bool moveable(int v)
	{
		return template_data[v >> 8].moveable >> (v & 0xff) & 1;
	}

	static void moveable(int v, bool w)
	{
		if (moveable(v) != w)
			template_data[v >> 8].moveable ^= static_cast<qword>(1) << (v & 0xff);
	}

	static void* get_bib(int v)
	{
		return bib[v];
	}
};

#endif // !defined(AFX_XCC_TEMPLATE_IMAGES_H__4E596E39_F686_11D2_B601_D35C3DCCB8F1__INCLUDED_)
