// cc_file.cpp: implementation of the Ccc_file class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <assert.h>
#include "art_ts_ini_reader.h"
#include "aud_file.h"
#include "avi_file.h"
#include "bin_file.h"
#include "bink_file.h"
#include "cc_file.h"
#include "cps_file.h"
#include "fnt_file.h"
#include "hva_file.h"
#include "map_td_ini_reader.h"
#include "map_ra_ini_reader.h"
#include "map_ts_ini_reader.h"
#include "mix_file.h"
#include "mp3_file.h"
#include "null_ini_reader.h"
#include "pak_file.h"
#include "pal_file.h"
#include "pcx_file.h"
#include "png_file.h"
#include "rules_ts_ini_reader.h"
#include "shp_dune2_file.h"
#include "shp_file.h"
#include "shp_ts_file.h"
#include "sound_ts_ini_reader.h"
#include "st_file.h"
#include "text_file.h"
#include "theme_ts_ini_reader.h"
#include "tmp_file.h"
#include "tmp_ra_file.h"
#include "tmp_ts_file.h"
#include "virtual_tfile.h"
#include "voc_file.h"
#include "vqa_file.h"
#include "vqp_file.h"
#include "vxl_file.h"
#include "wav_file.h"
#include "wsa_dune2_file.h"
#include "wsa_file.h"
#include "xcc_dirs.h"
#include "xcc_file.h"
#include "xcc_lmd_file.h"
#include "xif_file.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const char* ft_name[] = {"ai ini (ts)", "ai ini (ra2)", "art ini (ts)", "art ini (ra2)", "aud", "avi", "bin", "bink", "bmp", "cps", "csv", "dir", "drive", "fnt", "html", "hva", 
	"ini", "map (dune2)", "map (td)", "map (ra)", "map (ts)", "map (ts) preview", "map (ra2)", 
	"mix", "mng", "mp3", "mrf", "pak", "pal", "pal (jasc)", "pcx", "png", "riff", "rules ini (ts)", "rules ini (ra2)", "shp (dune2)", "shp", 
	"shp (ts)", "sound ini (ts)", "sound ini (ra2)", "string table", "text", "theme ini (ts)", "theme ini (ra2)", 
	"tmp", "tmp (ra)", "tmp (ts)", "voc", "vqa", "vqp", "vxl", "wav", "wsa (dune2)", "wsa", "xcc lmd", "xcc unknown", "xif", "zip", "unknown"};

Ccc_file::Ccc_file(bool read_on_open):
    m_read_on_open(read_on_open)
{
    m_data = NULL;
    m_data_loaded = m_is_open = false;
}

Ccc_file::~Ccc_file()
{
    assert(!is_open() || m_data_loaded);
}

#define test_fail(res) { int v = res; if (v) { clean_up(); return v; }}

#ifndef NO_MIX_SUPPORT
int Ccc_file::open(unsigned int id, Cmix_file& mix_f)
{
	assert(!is_open());
	if (mix_f.get_index(id) == -1)
		return -1;
    m_mix_f = &mix_f;
	m_offset = m_mix_f->get_offset(id);
	m_size = m_mix_f->get_size(id);
	m_p = 0;
    m_is_open = true;
    if (m_read_on_open)
    { 
        m_data = new byte[m_size];
        test_fail(read(m_data, m_size));
    }
    test_fail(post_open())
    return 0;
}

int Ccc_file::open(const string& name, Cmix_file& mix_f)
{
    return open(Cmix_file::get_id(mix_f.get_game(), name), mix_f);
}
#endif

int Ccc_file::open(const string& name)
{
    assert(!is_open());
#ifdef NO_MIX_SUPPORT
	test_fail(m_f.open_read(name));
#else
	test_fail(m_f.open_read(xcc_dirs::find_file(name)));
#endif
	m_mix_f = NULL;
	m_size = m_f.get_size();
	m_p = 0;
    m_is_open = true;
    if (m_read_on_open)
    { 
        m_data = new byte[m_size];
        test_fail(read(m_data, m_size));
        m_f.close();
    }
    test_fail(post_open())
    return 0;
}

void Ccc_file::load(const byte* data, int size)
{
	m_data = const_cast<byte*>(data);
	m_data_loaded = true;
	m_mix_f = NULL;
	m_is_open = true;
	m_p = 0;
	m_size = size;
	post_open();
}

#ifndef NO_MIX_SUPPORT
#ifndef NO_FT_SUPPORT
t_file_type Ccc_file::get_file_type()
{
	byte* data;
	int size;
	if (m_read_on_open)
	{
		data = m_data;
		size = m_size;
	}
	else
	{
		size = min(m_size, 64 << 10);
		data = new byte[size];
		if (read(data, size))
		{
			delete[] data;
			return ft_unknown;
		}
		seek(0);
	}
	t_file_type ft = ft_unknown;
	Caud_file aud_f;
	Cbin_file bin_f;
	Cbink_file bink_f;
	Ccps_file cps_f;
	Cfnt_file fnt_f;
	Chva_file hva_f;
	Cmix_file mix_f;
	Cmp3_file mp3_f;
	Cpak_file pak_f;
	Cpal_file pal_f;
	Cpcx_file pcx_f;
	Cpng_file png_f;
	Criff_file riff_f;
	Cshp_dune2_file shp_dune2_f;
	Cshp_file shp_f;
	Cshp_ts_file shp_ts_f;
	Cst_file st_f;
	Ctext_file text_f;
	Ctmp_file tmp_f;
	Ctmp_ra_file tmp_ra_f;
	Ctmp_ts_file tmp_ts_f;
	Cvoc_file voc_f;
	Cvqa_file vqa_f;
	Cvqp_file vqp_f;
	Cvxl_file vxl_f;
	Cwsa_dune2_file wsa_dune2_f;
	Cwsa_file wsa_f;
	Cxcc_file xcc_f;
	Cxif_file xif_f;
	aud_f.load(data, m_size);
	bin_f.load(data, m_size);
	bink_f.load(data, m_size);
	cps_f.load(data, m_size);
	fnt_f.load(data, m_size);
	hva_f.load(data, m_size);
	mix_f.load(data, m_size);
	mp3_f.load(data, m_size);
	pak_f.load(data, m_size);
	pal_f.load(data, m_size);
	pcx_f.load(data, m_size);
	png_f.load(data, m_size);
	riff_f.load(data, m_size);
	shp_dune2_f.load(data, m_size);
	shp_f.load(data, m_size);
	shp_ts_f.load(data, m_size);
	st_f.load(data, m_size);
	text_f.load(data, m_size);
	tmp_f.load(data, m_size);
	tmp_ra_f.load(data, m_size);
	tmp_ts_f.load(data, m_size);
	voc_f.load(data, m_size);
	vqa_f.load(data, m_size);
	vqp_f.load(data, m_size);
	vxl_f.load(data, m_size);
	wsa_dune2_f.load(data, m_size);
	wsa_f.load(data, m_size);
	xcc_f.load(data, m_size);
	xif_f.load(data, m_size);
	if (aud_f.is_valid())
		ft = ft_aud;
	else if (bin_f.is_valid())
		ft = ft_bin;
	else if (bink_f.is_valid())
		ft = ft_bink;
	else if (cps_f.is_valid())
		ft = ft_cps;
	else if (fnt_f.is_valid())
		ft = ft_fnt;
	else if (hva_f.is_valid())
		ft = ft_hva;
	else if (mix_f.is_valid())
		ft = ft_mix;
	else if (mp3_f.is_valid())
		ft = ft_mp3;
	else if (pak_f.is_valid())
		ft = ft_pak;
	else if (pal_f.is_valid())
		ft = ft_pal;
	else if (pcx_f.is_valid())
		ft = ft_pcx;
	else if (png_f.is_valid())
		ft = ft_png;
	else if (riff_f.is_valid())
	{
		Cavi_file avi_f;
		Cwav_file wav_f;
		avi_f.load(data, m_size);
		wav_f.load(data, m_size);
		if (avi_f.is_valid())
			ft = ft_avi;
		else if (wav_f.is_valid())
			ft = ft_wav;
		else
			ft = ft_riff;
	}
	else if (shp_dune2_f.is_valid())
		ft = ft_shp_dune2;
	else if (shp_f.is_valid())
		ft = ft_shp;
	else if (shp_ts_f.is_valid())
		ft = ft_shp_ts;
	else if (st_f.is_valid())
		ft = ft_st;
	else if (text_f.is_valid())
	{
		/*
		Cvirtual_tfile tf;
		tf.load_data(data, size);
		Cnull_ini_reader ir;
		int error = 0;
		while (!error && !tf.eof())
		{
			error = ir.process_line(tf.read_line());
			if (tf.eof())
				error = 0;
		}
		if (!error && ir.is_valid())
		{
			if (!m_read_on_open && m_size != size)
			{
				delete[] data;
				size = m_size;
				data = new byte[size];
				if (read(data, size))
				{
					delete[] data;
					return ft_unknown;
				}
				seek(0);
			}
			Cart_ts_ini_reader ir;
			ir.fast(true);
			if (!ir.process(data, size) && ir.is_valid())
				ft = ft_art_ini_ts;
			else
			{
				Cmap_td_ini_reader ir;
				if (!ir.process(data, size) && ir.is_valid())
					ft = ft_map_td;
				else
				{
					Cmap_ra_ini_reader ir;
					if (!ir.process(data, size) && ir.is_valid())
						ft = ft_map_ra;
					else
					{
						Cmap_ts_ini_reader ir;
						ir.fast(true);
						if (!ir.process(data, size) && ir.is_valid())
							ft = ft_map_ts;
						else
						{
							Crules_ts_ini_reader ir;
							ir.fast(true);
							if (!ir.process(data, size) && ir.is_valid())
								ft = ft_rules_ini_ts;
							else
							{
								Csound_ts_ini_reader ir;
								ir.fast(true);
								if (!ir.process(data, size) && ir.is_valid())
									ft = ft_sound_ini_ts;
								else
								{
									Ctheme_ts_ini_reader ir;
									if (!ir.process(data, size) && ir.is_valid())
										ft = ft_theme_ini_ts;
									else
										ft = ft_ini;
								}
							}
						}
					}
				}
			}
		}
		else
			ft = ft_text;
		*/
		ft = ft_text;
	}
	else if (tmp_f.is_valid())
		ft = ft_tmp;
	else if (tmp_ra_f.is_valid())
		ft = ft_tmp_ra;
	else if (tmp_ts_f.is_valid())
		ft = ft_tmp_ts;
	else if (voc_f.is_valid())
		ft = ft_voc;
	else if (vqa_f.is_valid())
		ft = ft_vqa;
	else if (vqp_f.is_valid())
		ft = ft_vqp;
	else if (vxl_f.is_valid())
		ft = ft_vxl;
	else if (wsa_dune2_f.is_valid())
		ft = ft_wsa_dune2;
	else if (wsa_f.is_valid())
		ft = ft_wsa;
	else if (xcc_f.is_valid())
	{
		switch (xcc_f.get_ft())
		{
		case xcc_ft_lmd:
			ft = ft_xcc_lmd;
			break;
		default:
			ft = ft_xcc_unknown;
		}
	}
	else if (xif_f.is_valid())
		ft = ft_xif;
	if (!m_read_on_open)
	{
		delete[] data;
		data = NULL;
	}
	return ft;
}
#endif
#endif

int Ccc_file::read()
{
    assert(is_open() && !m_data);
    m_data = new byte[m_size];
	seek(0);
	int error = read(m_data, m_size);
	m_data_loaded = true;
	return error;
}

int Ccc_file::read(void* data, int size)
{
	if (m_data_loaded)
	{
		memcpy(data, m_data + m_p, size);
		m_p += size;
		return 0;
	}
	assert(is_open());
    int res = 0;
	if (m_mix_f)
    {
        m_mix_f->seek(m_offset + m_p);
        res = m_mix_f->read(data, size);
    }
    else
    {
        m_f.seek(m_p);
        res = m_f.read(data, size);
    }
    if (!res)
        m_p += size;
    return res;
}

int Ccc_file::extract(const string& name)
{
	assert(is_open());
	int error = 0;
	Cfile32 f;
	error = f.open_write(name);
	if (!error)
	{
		if (get_data())
			error = f.write(get_data(), get_size());
		else
		{
			int cb_max_write = 1024 * 1024;
			int size = get_size();
			if (cb_max_write > size)
				cb_max_write = size;
			byte* data = new byte[cb_max_write];
			while (!error && size)
			{
				int cb_write = size > cb_max_write ? cb_max_write : size;
				error = read(data, cb_write);
				if (!error)
				{
					error = f.write(data, cb_write);
					size -= cb_write;
				}
			}
		}
		f.close();
	}
	return error;
}

void Ccc_file::close()
{
    assert(is_open());
    if (!m_data_loaded)
	{
        delete[] m_data;
		m_data = NULL;
	}
    if (!m_mix_f && m_f.is_open())
        m_f.close();
    m_is_open = false;
}

void Ccc_file::clean_up()
{
    delete[] m_data;
    m_data = NULL;
    if (is_open())
        close();
}