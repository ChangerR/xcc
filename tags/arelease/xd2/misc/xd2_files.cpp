// xd2_files.cpp: implementation of the Cxd2_files class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xd2_files.h"

#include "cps_file.h"
#include "mix_file.h"
#include "shp_decode.h"
#include "shp_dune2_file.h"
#include "voc_file.h"
#include "wsa_dune2_file.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cxd2_files::Cxd2_files()
{
}

enum
{
	vi_animations,
	vi_audio,
	vi_data,
	vi_images,
	vi_shapes,
};

int Cxd2_files::load(const Cxif_key_r& key)
{
	m_animation_map.load(key.get_key(vi_animations));
	m_audio_map.load(key.get_key(vi_audio));
	m_data_map.load(key.get_key(vi_data));
	m_image_map.load(key.get_key(vi_images));
	m_shape_map.load(key.get_key(vi_shapes));
	return 0;
}

Cxif_key Cxd2_files::save() const
{
	Cxif_key key;
	key.set_key(vi_animations) = m_animation_map.save();
	key.set_key(vi_audio) = m_audio_map.save();
	key.set_key(vi_data) = m_data_map.save();
	key.set_key(vi_images) = m_image_map.save();
	key.set_key(vi_shapes) = m_shape_map.save();
	return key;
}

int Cxd2_files::load(const string& dir)
{
	return load_audio_pak(dir + "atre.pak")
		|| load_audio_pak(dir + "hark.pak")
		|| load_audio_pak(dir + "introvoc.pak")
		|| load_audio_pak(dir + "merc.pak")
		|| load_audio_pak(dir + "ordos.pak")
		|| load_audio_pak(dir + "voc.pak")
		|| load_pak(dir + "dune.pak")
		|| load_pak(dir + "english.pak")
		|| load_pak(dir + "finale.pak")
		|| load_pak(dir + "intro.pak")
		|| load_pak(dir + "mentat.pak")
		|| load_pak(dir + "scenario.pak");
	// sound.pak
}

int Cxd2_files::load_audio_pak(const string& name)
{
	Cmix_file pak_f;
	if (pak_f.open(name))
		return 1;
	for (int i = 0; i < pak_f.get_c_files(); i++)
	{
		int id = pak_f.get_id(i);
		string name = pak_f.get_name(id);
		if (name == "filler.voc")
			continue;
		Cvoc_file f;
		f.open(id, pak_f);
		assert(f.is_valid());
		assert(!m_audio_map.has(name));
		m_audio_map.set(name, Cxd2_audio(Cvirtual_binary(f.get_sound_data(), f.get_c_samples()), f.get_samplerate()));
		f.close();
	}
	pak_f.close();
	return 0;
}

static Cxd2_animation load_animation(const Cwsa_dune2_file& f)
{
	const int cf = f.get_c_frames();
	const int cx = f.get_cx();
	const int cy = f.get_cy();
	Cvirtual_binary image;
	Cvirtual_binary s;
	byte* w = image.write_start(cf * cx * cy);
	for (int i = 0; i < cf; i++)
	{
		if (i)
			memcpy(w, w - cx * cy, cx * cy);
		else
			memset(w, 0, cx * cy);
		if (f.get_offset(i))
		{
			decode80(f.get_frame(i), s.write_start(1 << 16));
			decode40(s.data(), w);
		}
		w += cx * cy;
	}
	return Cxd2_animation(image, cf, cx, cy);
}

static Cxd2_shape load_shape(const Cshp_dune2_file& f)
{
	Cxd2_shape d;
	d.resize(f.get_c_images());
	const int c_images = f.get_c_images();
	for (int i = 0; i < c_images; i++)
	{
		const int cx = f.get_cx(i);
		const int cy = f.get_cy(i);
		Cvirtual_binary image(NULL, cx * cy);
		if (f.is_compressed(i))
		{
			Cvirtual_binary d(NULL, f.get_image_header(i)->size_out);
			decode2(d.data_edit(), image.data_edit(), decode80(f.get_image(i), d.data_edit()), f.get_reference_palet(i));
		}
		else
			decode2(f.get_image(i), image.data_edit(), f.get_image_header(i)->size_out, f.get_reference_palet(i));
		d.set(i, Cxd2_image(image, cx, cy));
	}
	return d;
}

int Cxd2_files::load_pak(const string& name)
{
	Cmix_file pak_f;
	if (pak_f.open(name))
		return 1;
	for (int i = 0; i < pak_f.get_c_files(); i++)
	{
		int id = pak_f.get_id(i);
		string name = pak_f.get_name(id);
		switch (pak_f.get_type(id))
		{
		case ft_cps:
			{
				Ccps_file f;
				f.open(id, pak_f);
				assert(f.is_valid());
				// assert(!f.has_palet());
				Cvirtual_binary d;
				decode80(f.get_image(), d.write_start(320 * 200));
				m_image_map.set(name, Cxd2_image(d, 320, 200));
				f.close();
				break;
			}
		case ft_shp_dune2:
			{
				Cshp_dune2_file f;
				f.open(id, pak_f);
				assert(f.is_valid());
				m_shape_map.set(name, load_shape(f));
				f.close();
				break;
			}
		case ft_wsa_dune2:
			{
				Cwsa_dune2_file f;
				f.open(id, pak_f);
				assert(f.is_valid());
				m_animation_map.set(name, load_animation(f));
				f.close();
				break;
			}
		default:
			{
				Ccc_file f(true);
				f.open(id, pak_f);
				m_data_map.set(name, f.get_vdata());
				f.close();
				break;
			}
		}
	}
	pak_f.close();
	return 0;
}