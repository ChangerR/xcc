// xcl_database.cpp: implementation of the Cxcl_database class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xcl_database.h"

#include <cmath>

int Cxcl_database::pid(const string& name)
{
	Csql_query q(*this);
	q.write("select pid from xcl_players where name = %s");
	q.pe(name);
	Csql_result result = q.execute();
	Csql_row row;
	if (row = result.fetch_row())
		return row.f_int(0);
	q.write("insert into xcl_players (name) values (lcase(%s))");
	q.pe(name);
	q.execute();
	return mysql_insert_id(&handle());
}

int Cxcl_database::update_player(int pid, int cmp, const Cxcl_player& a, const Cxcl_player& b)
{
	int points_win = 64 * (1 - 1 / (powf(10, static_cast<float>(b.points - a.points) / 400) + 1));
	int points_loss = min(64 - points_win, a.points / 10);
	Csql_query q(*this);
	switch (cmp)
	{
	case 0x100:
		q.write("update xcl_players set win_count = win_count + 1, points = points + %s where pid = %s");
		q.p(points_win);
		q.p(pid);
		q.execute();
		return points_win;
	case 0x200:
	case 0x210:
		q.write("update xcl_players set loss_count = loss_count + 1, points = points - %s where pid = %s");
		q.p(points_loss);
		q.p(pid);
		q.execute();
		return -points_loss;
	}
	return 0;
}

void Cxcl_database::insert_game(const Cgame_result& gr)
{
	int pids[4];
	Cxcl_player players[4];
	int pc[4];
	int i;
	pids[0] = pid(gr.get_string("nam0"));
	pids[1] = pid(gr.get_string("nam1"));
	for (i = 0; i < 2; i++)
		players[i] = player(pids[i]);
	for (i = 0; i < 2; i++)
		pc[i] = update_player(pids[i], gr.get_int("cmp", i), players[i], players[1 - i]);
	Csql_query q(*this);
	q.write("insert into xcl_games (afps, dura, gsku, oosy, scen, trny, a_pid, a_cmp, a_col, a_cty, a_pc, b_pid, b_cmp, b_col, b_cty, b_pc, ws_gid) values (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)");
	q.p(gr.get_int("afps"));
	q.p(gr.get_int("dura"));
	q.p(gr.get_int("gsku"));
	q.p(gr.get_int("oosy"));
	q.pe(gr.get_string("scen"));
	q.p(gr.get_int("trny"));
	for (i = 0; i < 2; i++)
	{
		q.p(pids[i]);
		q.p(gr.get_int("cmp", i));
		q.p(gr.get_int("col", i));
		q.p(gr.get_int("cty", i));
		q.p(pc[i]);
	}
	q.p(gr.get_int("idno"));
	q.execute();
}

Cxcl_player Cxcl_database::player(int pid)
{
	Csql_query q(*this);
	q.write("select * from xcl_players where pid = %s");
	q.p(pid);
	return Cxcl_player(q.execute().fetch_row());
}

void Cxcl_database::create_tables()
{
	query("create table if not exists xcl_games(gid int auto_increment primary key, afps int not null, dura int not null, gsku int not null, oosy int not null, scen varchar(32) not null, trny int not null, a_pid int not null, a_cmp int not null, a_col int not null, a_cty int not null, a_pc int not null, b_pid int not null, b_cmp int not null, b_col int not null, b_cty int not null, b_pc int not null, ws_gid int not null, mtime timestamp, key(a_pid), key(b_pid))");
	query("create table if not exists xcl_input (iid int auto_increment primary key, d blob not null, ipa varchar(15) not null, mtime timestamp)");
	query("create table if not exists xcl_maps (mid int auto_increment primary key, fname varchar(32) not null, name varchar(32) not null, unique (fname))");
	query("create table if not exists xcl_players(pid int auto_increment primary key, name varchar(9) not null, rank int not null, win_count int not null, loss_count int not null, points int not null, mtime timestamp, unique(name), key(rank))");
}

void Cxcl_database::drop_tables()
{
	query("drop table if exists xcl_games");
	query("drop table if exists xcl_input");
	query("drop table if exists xcl_maps");
	query("drop table if exists xcl_players");
}

void Cxcl_database::insert_maps()
{
	struct t_key
	{
		const char* name;
		const char* value;
	};
	t_key map_names[] = 
	{
		"mp01du.map", "South Pacific (2-4)",
		"mp01t4.map", "South Pacific (2-4)",
		"mp02t2.map", "Isle of War (2)",
		"mp03t4.map", "Anytown, Amerika (2-4)",
		"mp05du.map", "Heartland (2-4)",
		"mp05mw.map", "Heartland (2-4)",
		"mp05t4.map", "Heartland (2-4)",
		"mp06mw.map", "Urban Rush (2)",
		"mp06t2.map", "Urban Rush (2)",
		"mp08mw.map", "Little Big Lake (2)",
		"mp08t2.map", "Little Big Lake (2)",
		"mp09du.map", "Canyon Fodder (3)",
		"mp09t3.map", "Canyon Fodder (3)",
		"mp10s4.map", "Depth Charge (2-4)",
		"mp11t2.map", "Pinch Point (2)",
		"mp12s4.map", "Lake Blitzen (2-4)",
		"mp13du.map", "Montana DMZ (2-4)",
		"mp13mw.map", "Montana DMZ (2-4)",
		"mp13s4.map", "Montana DMZ (2-4)",
		"mp14mw.map", "El Dorado (2)",
		"mp14t2.map", "El Dorado (2)",
		"mp15du.map", "Snow Valley (2-4)",
		"mp15mw.map", "Snow Valley (2-4)",
		"mp15s4.map", "Snow Valley (2-4)",
		"mp16mw.map", "Snowball's Chance (2-4)",
		"mp16s4.map", "Snowball's Chance (2-4)",
		"mp17du.map", "Malibu Cliffs (2-6)",
		"mp17mw.map", "Malibu Cliffs (2-6)",
		"mp17t6.map", "Malibu Cliffs (2-6)",
		"mp18du.map", "Cold War (2-4)",
		"mp18s3.map", "Cold War (2-3)",
		"mp19t4.map", "Golden State Fwy (2-4)",
		"mp20t6.map", "Wild Animal Park (2-6)",
		"mp21s2.map", "Alaskan Oil Spill (2)",
		"mp22mw.map", "Arctic Circle (2-8)",
		"mp22s8.map", "Arctic Circle (2-8)",
		"mp23mw.map", "Hammer and Sickle (2-4)",
		"mp23t4.map", "Hammer and Sickle (2-4)",
		"mp24du.map", "Bonanza (2-4)",
		"mp25du.map", "DEFCON 6 (2-6)",
		"mp25mw.map", "DEFCON 6 (2-6)",
		"mp25t6.map", "DEFCON 6 (2-6)",
		"mp26s6.map", "Bering Strait (2-6)",
		"mp27du.map", "A Path Beyond II (2-8)",
		"mp27mw.map", "A Path Beyond II (2-8)",
		"mp27t8.map", "A Path Beyond II (2-8)",
		"mp29mw.map", "The Alamo (2)",
		"mp29u2.map", "The Alamo (2)",
		"mp30mw.map", "Siberian Wastes (2-6)",
		"mp30s6.map", "Siberian Wastes (2-6)",
		"mp31s2.map", "May Day (2)",
		"mp32du.map", "Heck Freezes Over (2-8)",
		"mp32mw.map", "Heck Freezes Over (2-8)",
		"mp32s8.map", "Heck Freezes Over (2-8)",
		"mp33u4.map", "Paris Revisited (2-4)",
		"mp34u4.map", "DC Uprising (2-4)",
		"tn01mw.map", "Official Tournament Map A (2)",
		"tn01t2.map", "Official Tournament Map A (2)",
		"tn02mw.map", "Official Tournament Map (4)",
		"tn02s4.map", "Official Tournament Map (4)",
		"tn04mw.map", "Official Tournament Map B (2)",
		"tn04t2.map", "Official Tournament Map B (2)",

		"2peaks.map", "Twin Peaks (2)",
		"arena33forever.map", "Arena 33 Forever (2-6)",
		"austintx.map", "Austin, TX (2-4)",
		"bldfeud.map", "Blood Feud (2)",
		"bridgegap.map", "Bridging the Gap (2-6)",
		"c1a01md.map", "Campaign 1 Map 1",
		"c1a02md.map", "Campaign 1 Map 2",
		"c1a03md.map", "Campaign 1 Map 3",
		"c2s01md.map", "Campaign 2 Map 1",
		"c2s02md.map", "Campaign 2 Map 2",
		"c2s03md.map", "Campaign 2 Map 3",
		"c3y01md.map", "Campaign 3 Map 1",
		"c3y02md.map", "Campaign 3 Map 2",
		"c3y03md.map", "Campaign 3 Map 3",
		"c4w01md.map", "Campaign 4 Map 1",
		"deathvalleygirl.map", "Death Valley Girl (2-4)",
		"deathvalleygirlmw.map", "Death Valley Girl (2-4)",
		"doubletrouble.map", "Double Trouble (2-4)",
		"downtown.map", "Downtown, Cityville (2-4)",
		"dryheat.map", "Dry Heat (2-4)",
		"dryheatmw.map", "Dry Heat (2-4)",
		"dunepatr.map", "Dune Patrol (2)",
		"eastvsbest.map", "East vs Best (2-6)",
		"facedown.map", "Face Down (2-4)",
		"fight.map", "Let There Be Fight (2)",
		"fourcorners.map", "Four Corners (2-4)",
		"fourcornersmw.map", "Four Corners (2-4)",
		"frstbite.map", "Frostbite (2-4)",
		"groundze.map", "Ground Zero (2-4)",
		"groundzemw.map", "Ground Zero (2-4)",
		"hidvally.map", "Hidden Valley (2)",
		"hillbtwn.map", "A Hill Between (2)",
		"isleofoades.map", "Isle Of Oades (2-4)",
		"manhatta.map", "Manhattan Mayhem (2-4)",
		"monumentvalley.map", "Monument Valley (2-6)",
		"monumentvalleymw.map", "Monument Valley (2-6)",
		"mountmoras.map", "Mount Rush More (2-5)",
		"nearoref.map", "Near Ore Far (2-8)",
		"nowimps.map", "No Wimps (2-4)",
		"offensedefense.map", "Offense Defense (2-4)",
		"ottersrevenge.map", "Otters Revenge (2-4)",
		"pcofdune.map", "Little Piece of Dune (2)",
		"rushhr.map", "Rush Hour (2-4)",
		"saharami.map", "Sahara Mirage (2)",
		"saharamimw.map", "Sahara Mirage (2)",
		"sedonapass.map", "Sedona Pass (2-6)",
		"sedonapassmw.map", "Sedona Pass (2-6)",
		"topothehill.map", "Top o'the Hill (2-3)",
		"tourofegypt.map", "Tour Of Egypt (2-6)",
		"trailerpark.map", "Trailer Park (2-8)",
		"triplecrossed.map", "Triple Crossed (3-6)",
		"turfwar.map", "Turfwar (2-4)",
		"xamazon01.map", "Stormy Weather (2-4)",
		"xarena.map", "Arena (2-4)",
		"xbarrel.map", "Loaded Barrel (2)",
		"xbayopigs.map", "Bay of Pigs (2-6)",
		"xbermuda.map", "Bermuda Triangle (2-6)",
		"xbreak.map", "Breakaway (2-3)",
		"xcarville.map", "Carville's Convoy (2-4)",
		"xdeadman.map", "Deadman's Ridge (2)",
		"xdeath.map", "Death trap (2-8)",
		"xdisaster.map", "Brink of Disaster (2-4)",
		"xdustbowl.map", "Dustbowl (2)",
		"xdustbowlmw.map", "Dustbowl (2)",
		"xeb1.map", "City Under Siege (2-4)",
		"xeb1mw.map", "City Under Siege (2-4)",
		"xeb2.map", "Sinkhole (2-4)",
		"xeb3.map", "Sovereign Land (2)",
		"xeb4.map", "Country Swing (2-4)",
		"xeb5.map", "Mount Olympus (2-4)",
		"xgoldst.map", "Streets of Gold (2-6)",
		"xgrinder.map", "Meat Grinder (2)",
		"xhailmary.map", "Hail Mary (2)",
		"xhills.map", "Head for the Hills (2-4)",
		"xinvasion.map", "Invasion Confirmed (2-4)",
		"xkaliforn.map", "Kalifornia (2-6)",
		"xkiller.map", "Killer Instinct (2-3)",
		"xlostlake.map", "Lost Lake (2-4)",
		"xmp01du.map", "South Pacific (2-4)",
		"xmp01t4.map", "South Pacific (2-4)",
		"xmp02t2.map", "Isle of War (2)",
		"xmp03t4.map", "Anytown, Amerika (2-4)",
		"xmp05du.map", "Heartland (2-4)",
		"xmp05mw.map", "Heartland (2-4)",
		"xmp05t4.map", "Heartland (2-4)",
		"xmp06mw.map", "Urban Rush (2)",
		"xmp06t2.map", "Urban Rush (2)",
		"xmp08mw.map", "Little Big Lake (2)",
		"xmp08t2.map", "Little Big Lake (2)",
		"xmp09du.map", "Canyon Fodder (2-3)",
		"xmp09t3.map", "Canyon Fodder (2-3)",
		"xmp10s4.map", "Depth Charge (2-4)",
		"xmp11t2.map", "Pinch Point (2)",
		"xmp12s4.map", "Lake Blitzen (2-4)",
		"xmp13du.map", "Montana DMZ (2-4)",
		"xmp13mw.map", "Montana DMZ (2-4)",
		"xmp13s4.map", "Montana DMZ (2-4)",
		"xmp14mw.map", "El Dorado (2)",
		"xmp14t2.map", "El Dorado (2)",
		"xmp15du.map", "Snow Valley (2-4)",
		"xmp15mw.map", "Snow Valley (2-4)",
		"xmp15s4.map", "Snow Valley (2-4)",
		"xmp16mw.map", "Snowball's Chance (2-4)",
		"xmp16s4.map", "Snowball's Chance (2-4)",
		"xmp17du.map", "Malibu Cliffs (2-6)",
		"xmp17mw.map", "Malibu Cliffs (2-6)",
		"xmp17t6.map", "Malibu Cliffs (2-6)",
		"xmp18du.map", "Cold War (2-4)",
		"xmp18s3.map", "Cold War (2-3)",
		"xmp19t4.map", "Golden State Fwy (2-4)",
		"xmp20t6.map", "Wild Animal Park (2-6)",
		"xmp21s2.map", "Alaskan Oil Spill (2)",
		"xmp22mw.map", "Arctic Circle (2-8)",
		"xmp22s8.map", "Arctic Circle (2-8)",
		"xmp23mw.map", "Hammer and Sickle (2-4)",
		"xmp23t4.map", "Hammer and Sickle (2-4)",
		"xmp24du.map", "Bonanza (2-4)",
		"xmp25du.map", "DEFCON 6 (2-6)",
		"xmp25mw.map", "DEFCON 6 (2-6)",
		"xmp25t6.map", "DEFCON 6 (2-6)",
		"xmp26s6.map", "Bering Strait (2-6)",
		"xmp27du.map", "A Path Beyond II (2-8)",
		"xmp27mw.map", "A Path Beyond II (2-8)",
		"xmp27t8.map", "A Path Beyond II (2-8)",
		"xmp29mw.map", "The Alamo (2)",
		"xmp29u2.map", "The Alamo (2)",
		"xmp30mw.map", "Siberian Wastes (2-6)",
		"xmp30s6.map", "Siberian Wastes (2-6)",
		"xmp31s2.map", "May Day (2)",
		"xmp32du.map", "Heck Freezes Over (2-8)",
		"xmp32mw.map", "Heck Freezes Over (2-8)",
		"xmp32s8.map", "Heck Freezes Over (2-8)",
		"xmp33u4.map", "Paris Revisited (2-4)",
		"xmp34u4.map", "DC Uprising (2-4)",
		"xnewhghts.map", "New Heights (2)",
		"xnorest.map", "No Rest for the Wicked (2-4)",
		"xoceansid.map", "Oceanside (2-4)",
		"xpacific.map", "Pacific Heights (2-4)",
		"xpacificmw.map", "Pacific Heights (2-4)",
		"xpotomac.map", "Army of the Potomac (2-6)",
		"xpowdrkeg.map", "Powder Keg (2-8)",
		"xrockets.map", "Rockets Red Glare (2-4)",
		"xroulette.map", "Russian Roulette (2-8)",
		"xround.map", "Roundhouse Kick (2-4)",
		"xseaofiso.map", "Sea of Isolation (2-4)",
		"xshrapnel.map", "Shrapnel Mountain (2-4)",
		"xtanyas.map", "Tanya's Training Grounds (2-4)",
		"xterritor.map", "Territorial Imperative (2-8)",
		"xtn01mw.map", "Official Tournament Map A (2)",
		"xtn01t2.map", "Official Tournament Map A (2)",
		"xtn02mw.map", "Official Tournament Map (4)",
		"xtn02s4.map", "Official Tournament Map (4)",
		"xtn04mw.map", "Official Tournament Map B (2)",
		"xtn04t2.map", "Official Tournament Map B (2)",
		"xtower.map", "Tower of Power (2-4)",
		"xtowermw.map", "Tower of Power (2-4)",
		"xtsunami.map", "Tsunami (2-4)",
		"xvalley.map", "Valley of Gems (2-4)",
		"xvalleymw.map", "Valley of Gems (2-4)",
		"xxmas.map", "Happy Trails (2)",
		"xyuriplot.map", "Yuri's Plot (2-3)",
		NULL
	};
	t_key* i = map_names;
	while (i->name)
	{
		Csql_query q(*this);
		q.write("insert into xcl_maps (fname, name) values (%s, %s)");
		q.pe(i->name);
		q.pe(i->value);
		q.execute();
		i++;
	}
}
