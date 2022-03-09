#include <iostream>
#include <string>
#include <time.h>
#include <random>
#include <curses.h>
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_4 52
#define KEY_5 53
#define KEY_6 54
#define KEY_7 55
#define KEY_8 56
#define KEY_9 57
#define KEY_0 48
#define KEY_MINUS 45
#define KEY_EQUAL 61
#define KEY_Q 113
#define KEY_W 119
#define KEY_E 101
#define KEY_R 114
#define KEY_A 97

bool exitf = false;

//Own body parts: Push, forward; Wrench, move back; Lift, move up; Press, move down;
enum class Actions {GRAB, PUSH, WRENCH, LIFT, RELEASE, PRESS, AWAY, BEHIND, TOP};
enum class MatPos {ONMAT, INAIR, UNDEFINED};
enum class SpacePos {FORWARD, BACK, UNDEFINED};
//Up meaning the front of the body not facing the mat and down facing the mat
enum class Facing {UP, DOWN, UNDEFINED};



struct Tile {
	int x;
	int y;
	int type;
	char t;
};
struct Context {
	int mod;
	std::vector<int> pos;
	std::string name;
};
struct Part {
	int fatigue;
	int power;
	int health;
	MatPos mpos;
	SpacePos spos;
	Facing face;
	std::string name;
	Part* attached;
};
struct ActionPos {
	Actions action;
	std::vector<MatPos> mpos;
	std::vector<SpacePos> spos;
	std::vector<Facing> face;
	std::vector<int> pos;
};

struct MoveContext {
	Part* mpart;
	std::vector<ActionPos> act;
};
struct Entity {
	int x;
	int y;
	int tile;
	Part bodyparts[12];
	Entity* target;
	Part* btarget;
	Part* atkpart;
	Actions action;
	std::string recent;
	std::string name;
	Context conts[17];
	MoveContext movconts[12];
};


void initTiles(Tile tiles[]) {
	int sx = 0;
	int sy = 0;
	for (int i = 0; i < 400; i++) {
		tiles[i].x = sx;
		tiles[i].y = sy;
		if (sx == 0 || sy == 0 || sx == 19 || sy == 19) {
			tiles[i].type = 1;
			tiles[i].t = '#';
		}
		else {
			tiles[i].type = 0;
			tiles[i].t = ' ';
		}
		sx++;
		if (sx == 20) {
			sy++;
			sx = 0;
		}
	}
}
void initParts(Entity* e) {
	for (int i = 0; i < 12; i++) {
		e->bodyparts[i].fatigue = 0;
		e->bodyparts[i].power = 1;
		e->bodyparts[i].attached = NULL;
		e->bodyparts[i].health = 100;
		e->bodyparts[i].mpos = MatPos::INAIR;
		e->bodyparts[i].spos = SpacePos::BACK;
		e->bodyparts[i].face = Facing::UP;
		switch (i) {
		case 0:
			e->bodyparts[i].name = "Core";
			break;
		case 1:
			e->bodyparts[i].name = "Right Leg";
			break;
		case 2:
			e->bodyparts[i].name = "Left Leg";
			break;
		case 3:
			e->bodyparts[i].name = "Back";
			break;
		case 4:
			e->bodyparts[i].name = "Left Arm";
			break;
		case 5:
			e->bodyparts[i].name = "Right Arm";
			break;
		case 6:
			e->bodyparts[i].name = "Neck";
			break;
		case 7:
			e->bodyparts[i].name = "Hips";
			break;
		case 8:
			e->bodyparts[i].name = "Right Hand";
			break;
		case 9:
			e->bodyparts[i].name = "Left Hand";
			break;
		case 10:
			e->bodyparts[i].name = "Right Foot";
			e->bodyparts[i].mpos = MatPos::ONMAT;
			e->bodyparts[i].spos = SpacePos::FORWARD;
			break;
		case 11:
			e->bodyparts[i].name = "Left Foot";
			e->bodyparts[i].mpos = MatPos::ONMAT;
			e->bodyparts[i].spos = SpacePos::FORWARD;
			break;
		}
	}
}
std::string PosToString(MatPos mpos, SpacePos spos) {
	std::string temp;
	switch (mpos) {
	case MatPos::INAIR:
		temp = "In Air ";
		break;
	case MatPos::ONMAT:
		temp = "On Mat ";
		break;
	}
	std::string temp2;
	switch (spos) {
	case SpacePos::BACK:
		temp2 = "Back";
		break;
	case SpacePos::FORWARD:
		temp2 = "Forwards";
		break;
	}
	return temp + temp2;
}
bool rollLimb(Part* atkpart, Part* epart, int atkpos, int epartpos) {
	//Take fatigue as a modifer
	//Take power as a modifier
	//Take into account enemies fatigue and power
	int roll = rand() % 100 - atkpart->fatigue + atkpart->power - atkpart->attached->power + atkpart->attached->fatigue;
	if (roll > 50) {
		return true;
	}
	return false;
}



//Implement context checking for shots(so can't grab leg unless leg ONMAT), do a seperate vector of contexts for limbs to check for a movement
void updateBodyPartPos(Entity *e) {
	for (int i = 0; i < 12; i++) {
		for (auto& k : e->conts[i].pos) {
			e->bodyparts[k].mpos = e->bodyparts[i].mpos;
			e->bodyparts[k].spos = e->bodyparts[i].spos;
			e->bodyparts[k].face = e->bodyparts[i].face;
		}
	}
	//Need to put rolls here too
	int contpos = 12;
	MatPos mp = MatPos::INAIR;
	SpacePos sp = SpacePos::BACK;
	Facing fac = Facing::UP;
	bool laction = false;
	switch (e->action) {
	case Actions::AWAY:
		contpos = 12;
		mp = MatPos::INAIR;
		sp = SpacePos::BACK;
		fac = Facing::UP;
		laction = true;
		e->recent = "You moved away from " + e->target->name;
		break;
	case Actions::BEHIND:
		contpos = 13;
		mp = MatPos::INAIR;
		sp = SpacePos::FORWARD;
		fac = Facing::UP;
		laction = true;
		e->recent = "You moved behind " + e->target->name;
		break;
	case Actions::TOP:
		contpos = 14;
		mp = MatPos::ONMAT;
		sp = SpacePos::FORWARD;
		fac = Facing::DOWN;
		laction = true;
		e->recent = "You moved on top of " + e->target->name;
		break;
	}
	if (laction) {
		for (auto& i : e->conts[contpos].pos) {
			e->bodyparts[i].mpos = mp;
			e->bodyparts[i].spos = sp;
			e->bodyparts[i].face = fac;
		}
		e->action = Actions::PUSH;
	}
}
void editContext(std::vector<Context>& conts, std::string name, int edit){
	for (auto& i : conts){
		if (i.name == name) {
			i.mod = edit;
		}
	}
}

void initContext(Context *c, std::vector<int> bps, std::string name, int mod) {
	for (auto& i : bps) { 
		(*c).pos.push_back(i);
	}
	c->name = name;
	c->mod = mod;
}
//Bodypart numbers correspond to which bodyparts will be moved by moving this body part
void initEntityContext(Entity* e) {
	for (int i = 0; i < 15; i++) {
		switch (i) {
		case 0:
			initContext(&e->conts[0], { 0, 1, 2, 7, 8, 9, 10, 11 }, "Core", 0);
			break;
		case 1:
			initContext(&e->conts[1], { 0, 3, 7, 10 }, "Right Leg", 0);
			break;
		case 2:
			initContext(&e->conts[2], { 0, 3, 7, 11 }, "Left Leg", 0);
			break;
		case 3:
			initContext(&e->conts[3], { 0, 3, 7}, "Back", 0);
			break;
		case 4:
			initContext(&e->conts[4], { 3, 4, 6, 9 }, "Left Arm", 0);
			break;
		case 5:
			initContext(&e->conts[5], { 3, 5, 6, 8 }, "Right Arm", 0);
			break;
		case 6:
			initContext(&e->conts[6], { 0, 3}, "Neck", 0);
			break;
		case 7:
			initContext(&e->conts[7], { 0, 1, 2, 3, 10, 11}, "Hips", 0);
			break;
		case 8:
			initContext(&e->conts[8], { 5 }, "Right Hand", 0);
			break;
		case 9:
			initContext(&e->conts[9], { 4 }, "Left Hand", 0);
			break;
		case 10:
			initContext(&e->conts[10], { 1 }, "Right Foot", 0);
			break;
		case 11:
			initContext(&e->conts[11], { 2 }, "Left Foot", 0);
			break;
		case 12:
			initContext(&e->conts[12], { 0, 1, 2, 3, 4, 5, 6, 7, 10, 11 }, "Move Away", 0);
			break;
		case 13:
			initContext(&e->conts[13], { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }, "Move Behind", 0);
			break;
		case 14:
			initContext(&e->conts[14], { 0, 1, 2, 7, 10, 11 }, "Move On Top", 0);
			break;
		}
	}
}
//Keep all the vectors even
void initActionCont(std::vector<ActionPos>& acts, int num){
	ActionPos ac;
	switch (num) {
	case 0:
		ac.action = Actions::PUSH;
		ac.pos = {10, 11};
		ac.mpos = {MatPos::INAIR, MatPos::INAIR};
		ac.spos = {SpacePos::UNDEFINED, SpacePos::UNDEFINED};
		ac.face = {Facing::UNDEFINED, Facing::UNDEFINED};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 1:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 2:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 3:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 4:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 5:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 6:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 7:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 8:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 9:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 10:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	case 11:
		ac.action = Actions::GRAB;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PUSH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::WRENCH;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::LIFT;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::PRESS;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		ac.action = Actions::RELEASE;
		ac.pos = {};
		ac.mpos = {};
		ac.spos = {};
		ac.face = {};
		acts.push_back(ac);
		break;
	}
}

void initMoveConts(Entity* e){
	for (int i = 0; i < 12; i++) {
		MoveContext m;
		m.mpart = &e->bodyparts[i];
		initActionCont(m.act, i);
	}
}
//Put this when a move is done by player to catch if movement is allowed
bool checkMoveCont(Entity* e, Part* part) {
	for (auto& i : e->movconts) {
		if (i.mpart == part) {
			for (auto& j : i.act) {
				if (j.action == e->action) {
					int m = 0;
					for (auto& k : j.pos) {
						if (e->bodyparts[k].mpos == j.mpos[m]) {
							//Put recent response here to tell player why you couldn't move that way
							if (j.mpos[m] == MatPos::INAIR) {
								e->recent = "You can't move " + part->name + " because " + e->bodyparts[k].name + " is in the air";
								//e->bodyparts[k].mpos = MatPos::ONMAT;
							}
							else {
								e->recent = "You can't move " + part->name + " because " + e->bodyparts[k].name + " on the mat";
								//e->bodyparts[k].mpos = MatPos::INAIR;
							}
							return false;
						}
						else if (e->bodyparts[k].spos == j.spos[m]) {
							if (j.spos[m] == SpacePos::BACK) {
								e->recent = "You can't move " + part->name + " because " + e->bodyparts[k].name + " is back";
								//e->bodyparts[k].spos = SpacePos::FORWARD;
							}
							else {
								e->recent = "You can't move " + part->name + " because " + e->bodyparts[k].name + " is forward";
								//e->bodyparts[k].spos = SpacePos::BACK;
							}
							return false;
						}
						else if (e->bodyparts[k].face == j.face[m]) {
							if (j.face[m] == Facing::DOWN) {
								e->recent = "You can't move " + part->name + " because " + e->bodyparts[k].name + " is facing down";
								//e->bodyparts[k].face = Facing::UP;
							}
							else {
								e->recent = "You can't move " + part->name + " because " + e->bodyparts[k].name + " is facing up";
								//e->bodyparts[k].face = Facing::DOWN;
							}
							return false;
						}
						m++;
						/*
						if (e->bodyparts[k].attached != NULL) {
							return false;
						}*/
					}
				}
			}
		}
	}
	return true;
}

//https://github.com/wmcbrine/PDCurses/blob/master/docs/MANUAL.md
//https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
//Add context checking for when you want a movement, if your bodypart isn't in right position can't do that move
// -Add all the rest of this
//Add stat rolls for all the body movements, and attacking of enemy
//This system should allow for actual moves, but lets see if it works
//Add enemy AI
//Add simple generation for matches, tournaments for dungeons? Can make decisons what to do between matches for basic roll if good or bad effect happens from this
//Stat increases based off matches
//Add practice throughout the week if time
//If time do fancy coloring of tiles
int main() {
	srand(time(NULL));
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	WINDOW* win = newwin(20, 20, 0, 0);
	WINDOW* win2 = newwin(30, 56, 0, 20);
	WINDOW* win3 = newwin(30, 50, 0, 68);
	wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wborder(win2, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wborder(win3, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(win);
	wrefresh(win2);
	wrefresh(win3);
	Entity player = { 10, 10, 250};
	player.action = Actions::PUSH;
	player.name = "Player";
	
	Entity enemy = { 20, 14, 230};
	enemy.name = "Enemy";
	enemy.target = &player;
	enemy.btarget = &player.bodyparts[0];
	enemy.atkpart = &enemy.bodyparts[8];
	enemy.action = Actions::PUSH;

	player.target = &enemy;
	player.btarget = &enemy.bodyparts[0];
	player.atkpart = &player.bodyparts[8];
	initParts(&player);
	initParts(&enemy);
	initEntityContext(&player);
	initEntityContext(&enemy);
	initMoveConts(&player);
	initMoveConts(&player);

	Tile tiles[400];
	initTiles(tiles);
	int turnnum = 0;
	int turnmax = 10;
	bool playerturn = true;
	bool turnchanged = false;
	int actionmode = 0;
	while (!exitf) {
		wclear(win2);
		wclear(win3);
		if (turnnum >= turnmax) {
			turnchanged = true;
			playerturn = false;
		}
		int key = getch();
		std::string keycode = std::to_string(key);
		switch (key) {
		case KEY_LEFT:
			if (tiles[player.tile - 1].t == ' ') {
				player.tile--;
				player.recent = "You moved left";
				turnnum += 10;
			}
			break;
		case KEY_RIGHT:
			if (tiles[player.tile + 1].t == ' ') {
				player.tile++;
				player.recent = "You moved right";
				turnnum += 10;
			}
			break;
		case KEY_DOWN:
			if (tiles[player.tile + 20].t == ' ') {
				player.tile += 20;
				player.recent = "You moved back";
				turnnum += 10;
			}
			break;
		case KEY_UP:
			if (tiles[player.tile - 20].t == ' ') {
				player.tile -= 20;
				player.recent = "You moved forwards";
				turnnum += 10;
			}
			break;
		case 27:
			actionmode = 0;
			break;
		case KEY_1:
			//0 select action mode, 1 select btarget, 2 select action, 3 select target, 4 player body parts actions, 
			switch (actionmode) {
			case 0:
				actionmode = 1;
				break;
			case 1:
				player.btarget = &player.target->bodyparts[0];
				break;
			case 2:
				if (player.atkpart->name == "Core" || player.atkpart->name == "Back" || player.atkpart->name == "Hips") {
					player.recent = "You can't grab with " + player.atkpart->name;
					break;
				}
				if (player.atkpart->attached != NULL) {
					player.recent = "You can't grab " + player.atkpart->name + " attached to " + player.atkpart->attached->name;
					break;
				}
				player.action = Actions::GRAB;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				player.recent = "You grabbed " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->attached = player.btarget;
				player.atkpart->fatigue += 1;
				turnnum += 2;
				break;
			case 3:
				player.atkpart = &player.bodyparts[0];
				break;
			case 4:
				player.action = Actions::PUSH;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 6;
				if (player.atkpart->attached != NULL) {
					player.recent = "You pushed " + player.atkpart->attached->name + " using " + player.atkpart->name;
					player.atkpart->attached->spos = SpacePos::FORWARD;
					player.atkpart->spos = SpacePos::FORWARD;
					player.atkpart->face = Facing::UP;
					player.atkpart->attached->face = Facing::UP;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 1;
				}
				else {
					player.recent = "You pushed your " + player.atkpart->name + " forward";
					player.atkpart->spos = SpacePos::FORWARD;
					player.atkpart->face = Facing::UP;
				}
				break;
			}
			break;
		case KEY_2:
			switch (actionmode) {
			case 0:
				actionmode = 2;
				break;
			case 1:
				player.btarget = &player.target->bodyparts[1];
				break;
			case 2:
				if (player.atkpart->attached != NULL) {
					player.recent = "You can't push " + player.atkpart->name + " attached to " + player.atkpart->attached->name;
					break;
				}
				player.action = Actions::PUSH;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				player.recent = "You pushed at " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->spos = SpacePos::FORWARD;
				player.btarget->spos = SpacePos::BACK;
				player.atkpart->face = Facing::UP;
				player.btarget->health -= player.atkpart->power;
				player.btarget->fatigue += 1;
				turnnum += 6;
				break;
			case 3:
				player.atkpart = &player.bodyparts[1];
				break;
			case 4:
				player.action = Actions::WRENCH;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 4;
				if (player.atkpart->attached != NULL) {
					player.recent = "You wrenched " + player.atkpart->attached->name + " back using " + player.atkpart->name;
					player.atkpart->attached->spos = SpacePos::BACK;
					player.atkpart->spos = SpacePos::BACK;
					player.atkpart->face = Facing::DOWN;
					player.atkpart->attached->face = Facing::DOWN;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 2;
				}
				else {
					player.recent = "You wrenched your " + player.atkpart->name + " back";
					player.atkpart->spos = SpacePos::BACK;
					player.atkpart->face = Facing::DOWN;
				}
				break;
			}
			break;
		case KEY_3:
			switch (actionmode) {
			case 0:
				actionmode = 3;
				break;
			case 1:
				player.btarget = &player.target->bodyparts[2];
				break;
			case 2:
				if (player.atkpart->attached == NULL) {
					player.recent = "You can't release " + player.atkpart->name + " unattached";
					break;
				}
				player.action = Actions::RELEASE;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				player.recent = "You released " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->attached = NULL;
				turnnum += 1;
				break;
			case 3:
				player.atkpart = &player.bodyparts[2];
				break;
			case 4:
				player.action = Actions::LIFT;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 4;
				if (player.atkpart->attached != NULL) {
					player.recent = "You lifted " + player.atkpart->attached->name + " using " + player.atkpart->name;
					player.atkpart->attached->mpos = MatPos::INAIR;
					player.atkpart->mpos = MatPos::INAIR;
					player.atkpart->face = Facing::UP;
					player.atkpart->attached->face = Facing::UP;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 4;
					turnnum += 3;
				}
				else {
					player.recent = "You lifted " + player.atkpart->name + " up";
					player.atkpart->mpos = MatPos::INAIR;
					player.atkpart->face = Facing::UP;
				}
				break;
			}
			break;
		case KEY_4:
			switch (actionmode) {
			case 0:
				actionmode = 4;
				break;
			case 1:
				player.btarget = &player.target->bodyparts[3];
				break;
			case 3:
				player.atkpart = &player.bodyparts[3];
				break;
			case 4:
				player.action = Actions::PRESS;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 2;
				if (player.atkpart->attached != NULL) {
					player.recent = "You pressed down " + player.atkpart->attached->name + " using " + player.atkpart->name;
					player.atkpart->attached->mpos = MatPos::ONMAT;
					player.atkpart->mpos = MatPos::ONMAT;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 3;
				}
				else {
					player.recent = "You pressed down " + player.atkpart->name;
					player.atkpart->mpos = MatPos::ONMAT;
					player.atkpart->face = Facing::DOWN;
				}
				break;
			}
			break;
		case KEY_5:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[4];
				break;
			case 3:
				player.atkpart = &player.bodyparts[4];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_6:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[5];
				break;
			case 3:
				player.atkpart = &player.bodyparts[5];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_7:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[6];
				break;
			case 3:
				player.atkpart = &player.bodyparts[6];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_8:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[7];
				break;
			case 3:
				player.atkpart = &player.bodyparts[7];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_9:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[8];
				break;
			case 3:
				player.atkpart = &player.bodyparts[8];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_0:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[9];
				break;
			case 3:
				player.atkpart = &player.bodyparts[9];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_MINUS:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[10];
				break;
			case 3:
				player.atkpart = &player.bodyparts[10];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_EQUAL:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[11];
				break;
			case 3:
				player.atkpart = &player.bodyparts[11];
				break;
			case 4:

				break;
			}
			break;
		case KEY_Q:
			//Move away
			//Roll to see if you can get away from enemy, this will release grabs enemy has on you
			player.action = Actions::AWAY;
			turnnum += 10;
			break;
		case KEY_W:
			//Move Behind
			//Roll to see if you can get by enemy
			player.action = Actions::BEHIND;
			turnnum += 10;
			break;
		case KEY_E:
			//Move on Top
			//Check if enemy is on mat
			player.action = Actions::TOP;
			turnnum += 10;
			break;
		}
		updateBodyPartPos(&player);
		updateBodyPartPos(&enemy);
		wprintw(win2, "STATS\n");
		wprintw(win2, "--------------------\n");
		for (auto& i : player.bodyparts) {
			if (i.attached != NULL) {
				if (turnchanged) {
					i.fatigue += 3;
				}
			}
			else {
				if (turnchanged) {
					i.fatigue -= 2;
				}
			}
			if (i.fatigue > 100) {
				i.fatigue = 100;
			}
			else if (i.fatigue < 0) {
				i.fatigue = 0;
			}
			std::string temp = i.name + " " + PosToString(i.mpos, i.spos) + "; Fatigue:" + std::to_string(i.fatigue) + "; Power:" + std::to_string(i.power) + "\n ";
			wprintw(win2, temp.c_str());
		}
		wprintw(win2, "--------------------\n");
		wprintw(win2, "SIGHT\n");
		std::string temp3 = player.target->recent + "\n";
		wprintw(win2, temp3.c_str());
		for (auto& i : enemy.bodyparts) {
			std::string temp = i.name + " " + PosToString(i.mpos, i.spos) + "; Fatigue:" + std::to_string(i.fatigue) + "; Power:" + std::to_string(i.power) + "\n ";
			wprintw(win2, temp.c_str());
		}
		wprintw(win2, "--------------------\n");
		wprintw(win3, "ACTIONS\n");
		int num = 1;
		switch (actionmode) {
		case 0:
			wprintw(win3, "1:Choose Body Target\n");
			wprintw(win3, "2:Choose Action\n");
			wprintw(win3, "3:Choose your Body Part to Use\n");
			wprintw(win3, "4:Action for your Own Body\n");
			break;
		case 1:
			for (auto& i : player.target->bodyparts) {
				std::string temp;
				temp = std::to_string(num) + ":" + i.name + "\n";
				switch (num) {
				case 10:
					num = 0;
					temp = std::to_string(num) + ":" + i.name + "\n";
					num = 10;
					break;
				case 11:
					temp = "-:" + i.name + "\n";
					break;
				case 12:
					temp = "=:" + i.name + "\n";
					break;
				}
				wprintw(win3, temp.c_str());
				num++;
			}
			wprintw(win3, "Press escape to go back\n");
			break;
		case 2:
			if(player.atkpart->attached != NULL){
				std::string tem = "Selected part " + player.atkpart->name + " attached to " + player.atkpart->attached->name + "\n";
				wprintw(win3, tem.c_str());
			}
			else {
				std::string tem = "Selected part " + player.atkpart->name + " unattached\n";
				wprintw(win3, tem.c_str());
			}
			wprintw(win3, "1:Grab\n");
			wprintw(win3, "2:Push\n");
			wprintw(win3, "3:Release\n");
			wprintw(win3, "Press escape to go back\n");
			break;
		case 3:
			for (auto& i : player.bodyparts) {
				std::string temp;
				temp = std::to_string(num) + ":" + i.name + "\n";
				switch (num) {
				case 10:
					num = 0;
					temp = std::to_string(num) + ":" + i.name + "\n";
					num = 10;
					break;
				case 11:
					temp = "-:" + i.name + "\n";
					break;
				case 12:
					temp = "=:" + i.name + "\n";
					break;
				}
				wprintw(win3, temp.c_str());
				num++;
			}
			wprintw(win3, "Press escape to go back\n");
			break;
		case 4:
			if (player.atkpart->attached != NULL) {
				std::string tem = "Can't move " + player.atkpart->name + " freely, attached to target " + player.atkpart->attached->name + "\n";
				wprintw(win3, tem.c_str());
			}
			else {
				std::string tem = player.atkpart->name + " unattached can move freely\n";
				wprintw(win3, tem.c_str());
			}
			wprintw(win3, "1:Push Forward\n");
			wprintw(win3, "2:Wrench Backwards\n");
			wprintw(win3, "3:Lift Up\n");
			wprintw(win3, "4:Press Down\n");
			wprintw(win3, "Press escape to go back\n");
			break;
		}
		wprintw(win3, "--------------------\n");
		wprintw(win3, "COMPLEX MOVEMENTS\n");
		wprintw(win3, "Q:Move Away\n");
		wprintw(win3, "W:Move Behind\n");
		wprintw(win3, "E:Move On Top\n");
		wprintw(win3, "--------------------\n");
		wprintw(win3, "BODY AWARENESS\n");
		std::string temp = "Targeting: " + player.btarget->name + " on " + enemy.name + "\n";
		wprintw(win3, temp.c_str());
		std::string temp4 = "Using: " + player.atkpart->name + "\n";
		wprintw(win3, temp4.c_str());
		std::string temp2 = player.recent + "\n";
		wprintw(win3, temp2.c_str());
		wprintw(win3, "--------------------\n");
		wprintw(win3, "TURN\n");
		std::string temp5 = "Turn Max: " + std::to_string(turnmax) + "\n";
		wprintw(win3, temp5.c_str());
		std::string temp6 = "Current Turn: " + std::to_string(turnnum) + "\n";
		wprintw(win3, temp6.c_str());
		if (turnchanged) {
			wprintw(win3, "Not your turn\n");
		}
		else {
			wprintw(win3, "Your turn\n");
		}
		wprintw(win3, "--------------------\n");
		wprintw(win2, keycode.c_str());

		int j = 0;
		for (auto& i : tiles) {
			wmove(win, i.y, i.x);
			if (j == player.tile) {
				waddch(win, '@');
			}
			else if(j == enemy.tile){
				waddch(win, '&');
			}
			else {
				waddch(win, i.t);
			}
			j++;
		}
		//presents screen changes
		wrefresh(win);
		wrefresh(win2);
		wrefresh(win3);
		if (turnchanged) {
			//Put enemy AI here
			turnchanged = false;
			turnnum = 0;
		}
	}
	endwin();
	return 0;
}