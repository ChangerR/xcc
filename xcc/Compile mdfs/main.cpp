#include "stdafx.h"
#include <fstream>
#include <map>
#include <iostream>
#include <string>
#include "fname.h"
#include "file32.h"
#include "mix_file.h"
#include "multi_line.h"
#include "string_conversion.h"
#include "vartypes.h"
#include "windows.h"
#include "xcc_dirs.h"

using namespace std;

struct t_idinfo
{
	string name;
	string description;
};

typedef map<dword, t_idinfo, less<dword> > t_idlist;

void handle_error(const string &s)
{
	cerr << s << endl;
	exit(1);
}

bool is_valid_line(const string& line, char seperator, int c_seperators)
{
	const char* s = line.c_str();
	long tabcount = 0;
	char c;
	while (c = *(s++))
	{
		if (c == seperator)
			tabcount++;
		else if (c < 0x20)
			return false;
	}
	return tabcount == c_seperators;	
}

string g(dword v)
{
	return n(static_cast<unsigned int>(v));
}

string gh(long l, dword v)
{
	return nh(l, v);
}
void tolower(string &s)
{
	for (long i = 0; i < s.length(); i++)
		s[i] = tolower(s[i]);
}

void parse_line_mm(string &line, t_idinfo &idinfo)
{
	tolower(line);
	Cmulti_line mline = line;
	idinfo.name = mline.get_next_line(',');
	mline.get_next_line(',');
	idinfo.description = mline.get_next_line(',');
}

void parse_line_rm(string &line, t_idinfo &idinfo)
{
	tolower(line);
	Cmulti_line mline = line;
	mline.get_next_line(',');
	mline.get_next_line(',');
	idinfo.name = mline.get_next_line(',');
	idinfo.description = mline.get_next_line(',');
}

void parse_line_normal(string &line, t_idinfo &idinfo)
{
	tolower(line);
	Cmulti_line mline = line;
	idinfo.name = mline.get_next_line('\t');
	idinfo.description = mline.get_next_line('\t');
}

void add_file(t_game game, const string &fname, t_idlist& id_list)
{
	cout << "Adding " + fname << endl;
	Cfile32 inf;
	if (inf.open(fname, GENERIC_READ))
	{
		cerr << "Error opening file" << endl;
		return;
	}
	long lineindex = 0;
	string line;
	t_idinfo idinfo;
	while (!inf.eof())
	{
		lineindex++;
		inf.read_line(line);
		if (!is_valid_line(line.c_str(), '\t', 1))
		{
			cerr << "Error in line " + g(lineindex) << endl;
			continue;
		}
		parse_line_normal(line, idinfo);
		if (idinfo.name != "")
			id_list[Cmix_file::get_id(game, idinfo.name)] = idinfo;
	}
	inf.close();
}

void add_rm(const string& fname, t_idlist& id_list)
{
	cout << "Adding " + fname << endl;
	Cfile32 inf;
	if (inf.open(fname, GENERIC_READ))
	{
		cerr << "Error opening file " << fname << endl;
		return;
	}
	long lineindex = 0;
	string line;
	t_idinfo idinfo;
	while (!inf.eof())
	{
		lineindex++;
		inf.read_line(line);
		if (!is_valid_line(line.c_str(), ',', 3))
		{
			cerr << "Error in line " + g(lineindex) << endl;
			continue;
		}
		parse_line_rm(line, idinfo);
		if (idinfo.name != "")
			id_list[Cmix_file::get_id(game_ra, idinfo.name)] = idinfo;
	}
	inf.close();
}

int get_id(const string& s)
{
	int r = 0;
	for (int i = 0; i < 8; i++)
	{
		int c = s.c_str()[i];
		c -= c < 'a' ? '0' : 'a' - 0x0a;
		r = r << 4 | c;
	}
	return r;
}

void write_list(t_idlist& id_list, Cfile32& f1, ofstream& f2, t_game game)
{
	t_idinfo idinfo;
	const char* v_name[] = {"TD", "RA", "TS", "D2", "D2K", "RA2"};
	int size = id_list.size();
	f1.write(&size, 4);
	for (t_idlist::iterator i = id_list.begin(); i != id_list.end(); i++)
	{
		idinfo = i->second;
		f1.write(idinfo.name.c_str(), idinfo.name.length() + 1); 
		f1.write(idinfo.description.c_str(), idinfo.description.length() + 1); 
		f2 << gh(8, Cmix_file::get_id(game, idinfo.name)) + '\t' + v_name[game] + '\t' + idinfo.name + '\t' + idinfo.description << endl;
	}
}

void write_database(const string &ifname, t_idlist& td_list, t_idlist& ra_list, t_idlist& ts_list, t_idlist& ra2_list)
{
	Cfile32 f1;
	Cfname fname = ifname;
	ofstream f2;
	fname.set_ext(".dat");
	if (f1.open(fname.get_all().c_str(), GENERIC_WRITE))
		handle_error("error opening binary output file");
	fname.set_ext(".txt");
	f2.open(fname.get_all().c_str());
	if (f2.fail())
		handle_error("error opening text output file");
	write_list(td_list, f1, f2, game_td);
	write_list(ra_list, f1, f2, game_ra);
	write_list(ts_list, f1, f2, game_ts);
	write_list(ra2_list, f1, f2, game_ra2);
	f2.close();
	f1.close();
}

int main()
{
	xcc_dirs::load_from_registry();
	t_idlist td_list, ra_list, ts_list, ra2_list;
	const string indir = xcc_dirs::get_data_dir();
	const string filenameend = " description.txt";
	if (0)
		add_file(game_ra, indir + "ra movies description.txt", ra_list);
	else
	{
		// add_mm(indir + "mm list.txt", td_list);
		add_rm(indir + "rm list.txt", ra_list);
		WIN32_FIND_DATA finddata;
		HANDLE findhandle = FindFirstFile((indir + "*").c_str(), &finddata);
		if (findhandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (~finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					string filename = finddata.cFileName;
					if (filename.find(filenameend, 0) != filename.npos)
					{
						if (filename.substr(0, 3) == "ra2")
							add_file(game_ra2, indir + filename, ra2_list);
						else if (filename.substr(0, 2) == "ra")
							add_file(game_ra, indir + filename, ra_list);
						else if (filename.substr(0, 2) == "ts")
							add_file(game_ts, indir + filename, ts_list);
						else
							add_file(game_td, indir + filename, td_list);
					}
				}
			}
			while (FindNextFile(findhandle, &finddata));
			FindClose(findhandle);
		}
	}
	write_database(indir + "global mix database", td_list, ra_list, ts_list, ra2_list);
	return 0;
}