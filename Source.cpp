#include <iostream>
#include <string>
#include <time.h>
#include <random>
#include <vector>
#include <algorithm>
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
int matcheswon = 1;
int tournmax = 0;

//Own body parts: Push, forward; Wrench, move back; Lift, move up; Press, move down;
enum class Actions {GRAB, PUSH, WRENCH, LIFT, RELEASE, PRESS, TWIST, RESIST, REMOVE, HOLD, AWAY, BEHIND, TOP};
//Add a position above inair 
enum class MatPos {ONMAT, INAIR, UNDEFINED};
enum class SpacePos {FORWARD, BACK, UNDEFINED};
//Up meaning the front of the body not facing the mat and down facing the mat
enum class Facing {UP, DOWN, UNDEFINED};
//Enemy ai personalities, maybe just use function pointers to update them
enum class Personal {DEFENSIVE, AGGRESIVE, PATIENT, SCARED};
//Enemy states
enum class State {DEFEND, ATTACK, REST, RUN};

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
	int n;
	int importance;
	bool moveable;
	bool held;
};
struct ActionContext {
	Actions action;
	std::vector<MatPos> mpos;
	std::vector<SpacePos> spos;
	std::vector<Facing> face;
	std::vector<int> pos;
};
struct Mcont {
	Part* part;
	int n;
	std::vector<ActionContext> acs;
};
//Used to tell entities what parts are connected to them from another entity
struct PartConnector {
	Part* mpart;
	Part* attpart;
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
	Mcont mconts[12];
	Personal person;
	State state;
	std::vector<PartConnector> pcs;
	int type;
	int upgradepoints;
};
struct {
	bool operator()(Part* a, Part* b) const { return a->importance > b->importance; }
} partSortGreater;
struct {
	bool operator()(Part* a, Part* b) const { return a->power < b->power; }
} partSortPower;

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
	int points = matcheswon * 10;
	for (int i = 0; i < 12; i++) {
		e->bodyparts[i].fatigue = 0;
		if (e->type == 1) {
			if (points == 0) {
				points = 1;
			}
			e->bodyparts[i].power = rand() % points + 1;
			points -= e->bodyparts[i].power;
			if (points == 0) {
				points = matcheswon * 5;
			}
		}
		else {
			e->bodyparts[i].power = 1;
		}
		e->bodyparts[i].attached = NULL;
		e->bodyparts[i].health = 100;
		e->bodyparts[i].mpos = MatPos::INAIR;
		e->bodyparts[i].spos = SpacePos::BACK;
		e->bodyparts[i].face = Facing::UP;
		e->bodyparts[i].n = i;
		e->bodyparts[i].moveable = true;
		e->bodyparts[i].held = false;
		switch (i) {
		case 0:
			e->bodyparts[i].name = "Core";
			e->bodyparts[i].importance = 85;
			break;
		case 1:
			e->bodyparts[i].name = "Right Leg";
			e->bodyparts[i].importance = 50;
			break;
		case 2:
			e->bodyparts[i].name = "Left Leg";
			e->bodyparts[i].importance = 50;
			break;
		case 3:
			e->bodyparts[i].name = "Back";
			e->bodyparts[i].importance = 90;
			break;
		case 4:
			e->bodyparts[i].name = "Left Arm";
			e->bodyparts[i].importance = 40;
			break;
		case 5:
			e->bodyparts[i].name = "Right Arm";
			e->bodyparts[i].importance = 40;
			break;
		case 6:
			e->bodyparts[i].name = "Neck";
			e->bodyparts[i].importance = 80;
			break;
		case 7:
			e->bodyparts[i].name = "Hips";
			e->bodyparts[i].importance = 75;
			break;
		case 8:
			e->bodyparts[i].name = "Right Hand";
			e->bodyparts[i].importance = 30;
			break;
		case 9:
			e->bodyparts[i].name = "Left Hand";
			e->bodyparts[i].importance = 30;
			break;
		case 10:
			e->bodyparts[i].name = "Right Foot";
			e->bodyparts[i].mpos = MatPos::ONMAT;
			e->bodyparts[i].spos = SpacePos::FORWARD;
			e->bodyparts[i].importance = 70;
			break;
		case 11:
			e->bodyparts[i].name = "Left Foot";
			e->bodyparts[i].mpos = MatPos::ONMAT;
			e->bodyparts[i].spos = SpacePos::FORWARD;
			e->bodyparts[i].importance = 70;
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
std::string MatPosToString(MatPos m) {
	switch (m) {
	case MatPos::INAIR:
		return "In Air";
		break;
	case MatPos::ONMAT:
		return "On Mat";
		break;
	case MatPos::UNDEFINED:
		return "UNDEFINED";
		break;
	}
	return "UNDEFINED";
}
std::string SpacePosToString(SpacePos m) { 
	switch (m) {
	case SpacePos::BACK:
		return "Back";
		break;
	case SpacePos::FORWARD:
		return "Forward";
		break;
	case SpacePos::UNDEFINED:
		return "UNDEFINED";
		break;
	}
	return "UNDEFINED";
}
std::string FacePosToString(Facing m) {
	switch (m) {
	case Facing::DOWN:
		return "Down";
		break;
	case Facing::UP:
		return "Up";
		break;
	case Facing::UNDEFINED:
		return "UNDEFINED";
		break;
	}
	return "UNDEFINED";
}
Facing reverseFace(Facing face) {
	switch (face) {
		case Facing::DOWN:
			return Facing::UP;
		break;
		case Facing::UP:
			return Facing::DOWN;
			break;
	}
	return Facing::UNDEFINED;
}
MatPos reverseMatPos(MatPos mat) {
	switch (mat) {
	case MatPos::ONMAT:
		return MatPos::INAIR;
		break;
	case MatPos::INAIR:
		return MatPos::ONMAT;
		break;
	}
	return MatPos::UNDEFINED;
}
SpacePos reverseSpacePos(SpacePos space) {
	switch (space) {
	case SpacePos::BACK:
		return SpacePos::FORWARD;
		break;
	case SpacePos::FORWARD:
		return SpacePos::BACK;
		break;
	}
	return SpacePos::UNDEFINED;
}


int rollLimb(Part* atkpart, Part* epart) {
	//Take fatigue as a modifer
	//Take power as a modifier
	//Take into account enemies fatigue and power
	int movemod = 0;
	if (!epart->moveable) {
		movemod = 15;
		atkpart->fatigue += 1;
	}
	if (atkpart->held) {
		movemod += 20;
	}

	int roll = rand() % 100;
	roll = roll - atkpart->fatigue + atkpart->power - epart->power + epart->fatigue - movemod;
	return roll;
}
int rollBasic() {
	int roll = rand() % 100;
	return roll;
}


bool checkTileRange(Entity* p, Entity* e) {
	if (p->tile == e->tile || p->tile == e->tile - 20 || p->tile == e->tile + 20 || p->tile == e->tile-1 || p->tile == e->tile+1 || p->tile == e->tile-21 || p->tile == e->tile-19 || p->tile == e->tile + 21 || p->tile == e->tile + 19) {
		return true;
	}

	return false;
}
void findAttachedLimbs(std::vector<Part*>& temps, Part parts[12]) {
	for (int i = 0; i < 12; i++) {
		if (parts[i].attached != NULL) {
			temps.push_back(&parts[i]);
		}
	}
}
void findLimbs(std::vector<Part*>& ps, Part parts[12]) {
	for (int i = 0; i < 12; i++) {
		ps.push_back(&parts[i]);
	}
}

Part* findAttachedPart(Part* p, Entity* e) {
	for (auto& i : e->bodyparts) {
		if (i.attached != NULL) {
			if (i.attached->n == p->n) {
				return &i;
			}
		}
	}
	return NULL;
}


//Implement context checking for shots(so can't grab leg unless leg ONMAT), do a seperate vector of contexts for limbs to check for a movement
void updateBodyPartPos(Entity *e, Part* part) {
	for (auto& k : e->conts[part->n].pos) {
		e->bodyparts[k].mpos = e->bodyparts[part->n].mpos;
		e->bodyparts[k].spos = e->bodyparts[part->n].spos;
		e->bodyparts[k].face = e->bodyparts[part->n].face;
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
			initContext(&e->conts[0], { 0, 1, 2, 3, 7, 10, 11}, "Core", 0);
			break;
		case 1:
			initContext(&e->conts[1], { 1, 10 }, "Right Leg", 0);
			break;
		case 2:
			initContext(&e->conts[2], { 2, 11 }, "Left Leg", 0);
			break;
		case 3:
			initContext(&e->conts[3], { 0, 3, 7}, "Back", 0);
			break;
		case 4:
			initContext(&e->conts[4], { 4, 9 }, "Left Arm", 0);
			break;
		case 5:
			initContext(&e->conts[5], { 8 }, "Right Arm", 0);
			break;
		case 6:
			initContext(&e->conts[6], {0, 3, 6 }, "Neck", 0);
			break;
		case 7:
			initContext(&e->conts[7], { 0, 1, 2, 3, 7, 10, 11}, "Hips", 0);
			break;
		case 8:
			initContext(&e->conts[8], { 5, 8 }, "Right Hand", 0);
			break;
		case 9:
			initContext(&e->conts[9], { 4, 9 }, "Left Hand", 0);
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
//Rewrite this entire thing to be based around moving parts if they aren't in the right position so another part can move
void initActionCont(std::vector<ActionContext>& acs, int n){
	ActionContext ac;
		switch (n) {
		case 0:
			ac.action = Actions::PUSH;
			ac.pos = { 10, 11 };
			ac.mpos = { MatPos::INAIR, MatPos::INAIR };
			ac.spos = { SpacePos::UNDEFINED, SpacePos::UNDEFINED };
			ac.face = { Facing::UNDEFINED, Facing::UNDEFINED };
			acs.push_back(ac);
			ac.action = Actions::WRENCH;
			ac.pos = { 0, 3, 7 };
			ac.mpos = { MatPos::UNDEFINED, MatPos::UNDEFINED, MatPos::UNDEFINED };
			ac.spos = { SpacePos::BACK, SpacePos::BACK, SpacePos::BACK };
			ac.face = { Facing::UNDEFINED, Facing::UNDEFINED, Facing::UNDEFINED };
			acs.push_back(ac);
			ac.action = Actions::LIFT;
			ac.pos = { 0, 7, 10, 11 };
			ac.mpos = { MatPos::INAIR, MatPos::UNDEFINED, MatPos::INAIR, MatPos::INAIR };
			ac.spos = { SpacePos::UNDEFINED, SpacePos::UNDEFINED, SpacePos::UNDEFINED, SpacePos::UNDEFINED };
			ac.face = { Facing::UNDEFINED, Facing::UP, Facing::UNDEFINED, Facing::UNDEFINED };
			acs.push_back(ac);
			ac.action = Actions::PRESS;
			ac.pos = { 0, 7 };
			ac.mpos = { MatPos::ONMAT, MatPos::ONMAT };
			ac.spos = { SpacePos::UNDEFINED, SpacePos::BACK };
			ac.face = { Facing::DOWN, Facing::UNDEFINED };
			acs.push_back(ac);
			break;
		case 1:
			ac.action = Actions::GRAB;
			ac.pos = {0, 7};
			ac.mpos = {MatPos::UNDEFINED, MatPos::UNDEFINED};
			ac.spos = {SpacePos::BACK, SpacePos::BACK};
			ac.face = {Facing::UNDEFINED, Facing::UNDEFINED};
			acs.push_back(ac);
			break;
		case 2:
			ac.action = Actions::GRAB;
			ac.pos = { 0, 7 };
			ac.mpos = { MatPos::UNDEFINED, MatPos::UNDEFINED };
			ac.spos = { SpacePos::BACK, SpacePos::BACK };
			ac.face = { Facing::UNDEFINED, Facing::UNDEFINED };
			acs.push_back(ac);
			break;
		case 3:
			ac.action = Actions::LIFT;
			ac.pos = {7};
			ac.mpos = {MatPos::INAIR};
			ac.spos = {SpacePos::UNDEFINED};
			ac.face = {Facing::UNDEFINED};
			acs.push_back(ac);
			ac.action = Actions::PRESS;
			ac.pos = {7};
			ac.mpos = { MatPos::INAIR };
			ac.spos = { SpacePos::UNDEFINED };
			ac.face = { Facing::UNDEFINED };
			acs.push_back(ac);
			break;
		case 4:
			ac.action = Actions::GRAB;
			ac.pos = {2, 6};
			ac.mpos = {MatPos::UNDEFINED, MatPos::UNDEFINED};
			ac.spos = {SpacePos::BACK, SpacePos::BACK};
			ac.face = {Facing::UNDEFINED, Facing::UNDEFINED};
			acs.push_back(ac);
			break;
		case 5:
			ac.action = Actions::GRAB;
			ac.pos = { 1, 6 };
			ac.mpos = { MatPos::UNDEFINED, MatPos::UNDEFINED };
			ac.spos = { SpacePos::BACK, SpacePos::BACK };
			ac.face = { Facing::UNDEFINED, Facing::UNDEFINED };
			acs.push_back(ac);
			break;
		case 6:
			ac.action = Actions::GRAB;
			ac.pos = {6, 7};
			ac.mpos = {MatPos::UNDEFINED, MatPos::UNDEFINED};
			ac.spos = {SpacePos::BACK, SpacePos::BACK};
			ac.face = {Facing::UNDEFINED, Facing::UNDEFINED};
			acs.push_back(ac);
			break;
		case 7:
			ac.action = Actions::LIFT;
			ac.pos = {10, 11};
			ac.mpos = {MatPos::INAIR, MatPos::INAIR};
			ac.spos = {SpacePos::UNDEFINED, SpacePos::UNDEFINED};
			ac.face = {Facing::UNDEFINED, Facing::UNDEFINED};
			acs.push_back(ac);
			ac.action = Actions::PRESS;
			ac.pos = {1, 2};
			ac.mpos = {MatPos::ONMAT, MatPos::ONMAT};
			ac.spos = {SpacePos::UNDEFINED, SpacePos::UNDEFINED};
			ac.face = {Facing::UNDEFINED, Facing::UNDEFINED};
			acs.push_back(ac);
			break;
		case 8:
			ac.action = Actions::GRAB;
			ac.pos = {5};
			ac.mpos = {MatPos::UNDEFINED};
			ac.spos = {SpacePos::BACK};
			ac.face = {Facing::UNDEFINED};
			acs.push_back(ac);
			break;
		case 9:
			ac.action = Actions::GRAB;
			ac.pos = { 4 };
			ac.mpos = { MatPos::UNDEFINED };
			ac.spos = { SpacePos::BACK };
			ac.face = { Facing::UNDEFINED };
			acs.push_back(ac);
			break;
		case 10:
			ac.action = Actions::GRAB;
			ac.pos = {1};
			ac.mpos = {MatPos::UNDEFINED};
			ac.spos = {SpacePos::BACK};
			ac.face = {Facing::UNDEFINED};
			acs.push_back(ac);
			break;
		case 11:
			ac.action = Actions::GRAB;
			ac.pos = { 2 };
			ac.mpos = { MatPos::UNDEFINED };
			ac.spos = { SpacePos::BACK };
			ac.face = { Facing::UNDEFINED };
			acs.push_back(ac);
			break;
		}
}
bool checkMoveCont(Entity* e, Part* part) {
	for (auto& i : e->mconts[part->n].acs) {
		if (i.action == e->action) {
			//e->recent = "action found";
			for (int j = 0; j < i.pos.size(); j++) {
				for (auto& k : e->bodyparts) {
					if (k.n == i.pos[j]) {
						if (k.mpos == i.mpos[j]) {
							if (e->type == 1) {
								k.mpos = reverseMatPos(k.mpos);
							}
							else {
								e->recent = "Can't do action " + k.name + " is " + MatPosToString(k.mpos);
								return false;
							}
						}
						else if (k.spos == i.spos[j]) {
							if (e->type == 1) {
								k.spos = reverseSpacePos(k.spos);
							}
							else {
								e->recent = "Can't do action " + k.name + " is " + SpacePosToString(k.spos);
								return false;
							}
						}
						else if (k.face == i.face[j]) {
							if (e->type == 1) {
								k.face = reverseFace(k.face);
							}
							else {
								e->recent = "Can't do action " + k.name + " is facing" + FacePosToString(k.face);
								return false;
							}
						}
					}
				}
			}
			//e->recent = "No out of place limbs found";
			return true;
		}
	}
	e->recent = "Action not found";
	return true;
}
void initMConts(Entity* e){
	for (int i = 0; i < 12; i++) {
		e->mconts[i].n = i;
		e->mconts[i].part = &e->bodyparts[i];
		initActionCont(e->mconts[i].acs, i);
	}
}

void generateEnemy(Entity* e, Entity *p) {
	e->x = 10;
	e->y = 11;
	e->tile = 230;
	e->name = "Enemy";
	e->target = p;
	e->btarget = &p->bodyparts[0];
	e->atkpart = &e->bodyparts[8];
	e->action = Actions::PUSH;
	e->type = 1;
	initParts(e);
	initEntityContext(e);
	initMConts(e);
	//Put AI generation here
	int roll = rand() % 3;
	e->state = State::DEFEND;
	switch (roll) {
	case 0:
		e->person = Personal::DEFENSIVE;
		break;
	case 1:
		e->person = Personal::AGGRESIVE;
		break;
	case 2:
		e->person = Personal::PATIENT;
		break;
	case 3:
		e->person = Personal::SCARED;
		break;
	}
}



//https://github.com/wmcbrine/PDCurses/blob/master/docs/MANUAL.md
//https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
int main() {
	srand(time(NULL));
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	WINDOW* win = newwin(20, 20, 0, 0);
	WINDOW* win2 = newwin(30, 56, 0, 20);
	WINDOW* win3 = newwin(30, 49, 0, 71);
	wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wborder(win2, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wborder(win3, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(win);
	wrefresh(win2);
	wrefresh(win3);
	Entity player = { 10, 12, 250};
	player.action = Actions::PUSH;
	player.name = "Player";
	player.type = 0;
	
	Entity enemy = { 10, 11, 230};
	enemy.name = "Enemy";
	enemy.target = &player;
	enemy.btarget = &player.bodyparts[0];
	enemy.atkpart = &enemy.bodyparts[8];
	enemy.action = Actions::PUSH;

	player.target = &enemy;
	player.btarget = &enemy.bodyparts[0];
	player.atkpart = &player.bodyparts[8];
	player.upgradepoints = 0;
	initParts(&player);
	initEntityContext(&player);
	initMConts(&player);

	generateEnemy(&enemy, &player);

	Tile tiles[400];
	initTiles(tiles);
	int turnnum = 0;
	int turnmax = 10;
	bool playerturn = true;
	bool turnchanged = false;
	int actionmode = 0;
	int ebturns = 0;
	int pbturns = 0;
	int displayemode = 1;
	matcheswon = 0;
	while (!exitf) {
		wclear(win2);
		wclear(win3);
		int key = getch();
		if(displayemode == 0){
		std::string keycode = std::to_string(key);
		Part* t = NULL;
		bool cont;
		switch (key) {
		case KEY_LEFT:
			if (turnchanged) {
				player.recent.clear();
				player.recent = "Not your turn";
				break;
			}
			for (int i = 0; i < 10; i++) {
				if (player.bodyparts[i].mpos == MatPos::ONMAT) {
					player.recent.clear();
					player.recent = "You can't move over " + player.bodyparts[i].name + " is on the mat";
					break;
				}
			}
			cont = true;
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 12; j++) {
					if (&player.bodyparts[i] == enemy.bodyparts[j].attached) {
						cont = false;
					}
				}
			}
			if (!cont) {
				if(rollBasic() > 30){
					player.recent.clear();
					player.recent = "Removed limbs and moved";
				}
				else {
					player.recent.clear();
					player.recent = "Couldn't remove an attached limb, so you're stuck";
					break;
				}
			}
			if (tiles[player.tile - 1].type == 0) {
				player.tile--;
				player.x--;
				player.recent.clear();
				player.recent = "You moved left";
				turnnum += 10;
			}
			break;
		case KEY_RIGHT:
			if (turnchanged) {
				player.recent.clear();
				player.recent = "Not your turn";
				break;
			}
			for (int i = 0; i < 10; i++) {
				if (player.bodyparts[i].mpos == MatPos::ONMAT) {
					player.recent.clear();
					player.recent = "You can't move over " + player.bodyparts[i].name + " is on the mat";
					break;
				}
			}
			cont = true;
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 12; j++) {
					if (&player.bodyparts[i] == enemy.bodyparts[j].attached) {
						cont = false;
					}
				}
			}
			if (!cont) {
				if (rollBasic() > 30) {
					player.recent.clear();
					player.recent = "Removed limbs and moved";
				}
				else {
					player.recent.clear();
					player.recent = "Couldn't remove an attached limb, so you're stuck";
					break;
				}
			}
			if (tiles[player.tile + 1].type == 0) {
				player.tile++;
				player.x++;
				player.recent.clear();
				player.recent = "You moved right";
				turnnum += 10;
			}
			break;
		case KEY_DOWN:
			if (turnchanged) {
				player.recent.clear();
				player.recent = "Not your turn";
				break;
			}
			for (int i = 0; i < 10; i++) {
				if (player.bodyparts[i].mpos == MatPos::ONMAT) {
					player.recent.clear();
					player.recent = "You can't move over " + player.bodyparts[i].name + " is on the mat";
					break;
				}
			}
			cont = true;
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 12; j++) {
					if (&player.bodyparts[i] == enemy.bodyparts[j].attached) {
						cont = false;
					}
				}
			}
			if (!cont) {
				if (rollBasic() > 30) {
					player.recent.clear();
					player.recent = "Removed limbs and moved";
				}
				else {
					player.recent.clear();
					player.recent = "Couldn't remove an attached limb, so you're stuck";
					break;
				}
			}
			if (tiles[player.tile + 20].type == 0) {
				player.tile += 20;
				player.y++;
				player.recent.clear();
				player.recent = "You moved back";
				turnnum += 10;
			}
			break;
		case KEY_UP:
			if (turnchanged) {
				player.recent.clear();
				player.recent = "Not your turn";
				break;
			}
			for (int i = 0; i < 10; i++) {
				if (player.bodyparts[i].mpos == MatPos::ONMAT) {
					player.recent.clear();
					player.recent = "You can't move over " + player.bodyparts[i].name + " is on the mat";
					break;
				}
			}
			cont = true;
			for (int i = 0; i < 12; i++) {
				for (int j = 0; j < 12; j++) {
					if (&player.bodyparts[i] == enemy.bodyparts[j].attached) {
						cont = false;
					}
				}
			}
			if (!cont) {
				if (rollBasic() > 30) {
					player.recent.clear();
					player.recent = "Removed limbs and moved";
				}
				else {
					player.recent.clear();
					player.recent = "Couldn't remove an attached limb, so you're stuck";
					break;
				}
			}
			if (tiles[player.tile - 20].type == 0) {
				player.tile -= 20;
				player.y--;
				player.recent.clear();
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
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				if (!checkTileRange(&player, player.target)) {
					player.recent.clear();
					player.recent = "Not in range of target!";
					break;
				}
				if (player.atkpart->name == "Core" || player.atkpart->name == "Back" || player.atkpart->name == "Hips") {
					player.recent.clear();
					player.recent = "You can't grab with " + player.atkpart->name;
					break;
				}
				if (player.atkpart->attached != NULL) {
					player.recent.clear();
					player.recent = "You can't grab " + player.atkpart->name + " attached to " + player.atkpart->attached->name;
					break;
				}
				player.action = Actions::GRAB;
				if (rollLimb(player.atkpart, player.btarget) > 80) {
					player.recent.clear();
					player.recent = "Failed to grab " + player.btarget->name;
					break;
				}
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				updateBodyPartPos(&player, player.atkpart);
				player.recent.clear();
				player.recent = "You grabbed " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->attached = player.btarget;
				player.atkpart->fatigue += 1;
				turnnum += 2;
				break;
			case 3:
				player.atkpart = &player.bodyparts[0];
				break;
			}
			break;
		case KEY_2:
			switch (actionmode) {
			case 0:
				actionmode = 3;
				break;
			case 1:
				player.btarget = &player.target->bodyparts[1];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				if (!checkTileRange(&player, player.target)) {
					player.recent.clear();
					player.recent = "Not in range of target!";
					break;
				}
				if (player.atkpart->attached != NULL) {
					player.recent.clear();
					player.recent = "You can't push " + player.atkpart->name + " attached to " + player.atkpart->attached->name;
					break;
				}
				player.action = Actions::PUSH;
				if (rollLimb(player.atkpart, player.btarget) > 60) {
					player.recent.clear();
					player.recent = "Failed to push " + player.btarget->name;
					break;
				}
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				player.recent.clear();
				player.recent = "You pushed at " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->spos = SpacePos::FORWARD;
				player.btarget->spos = SpacePos::BACK;
				player.atkpart->face = Facing::UP;
				player.btarget->health -= player.atkpart->power;
				player.btarget->fatigue += 3;
				turnnum += 6;
				updateBodyPartPos(&player, player.atkpart);
				updateBodyPartPos(&enemy, player.btarget);
				break;
			case 3:
				player.atkpart = &player.bodyparts[1];
				break;
			}
			break;
		case KEY_3:
			switch (actionmode) {
			case 0:
				actionmode = 2;
				break;
			case 1:
				player.btarget = &player.target->bodyparts[2];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				if (!checkTileRange(&player, player.target)) {
					player.recent.clear();
					player.recent = "Not in range of target!";
					break;
				}
				if (player.atkpart->attached == NULL) {
					player.recent.clear();
					player.recent = "You can't release " + player.atkpart->name + " unattached";
					break;
				}
				player.action = Actions::RELEASE;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				player.recent.clear();
				player.recent = "You released " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->attached = NULL;
				turnnum += 1;
				updateBodyPartPos(&player, player.atkpart);
				break;
			case 3:
				player.atkpart = &player.bodyparts[2];
				break;
			}
			break;
		case KEY_4:
			switch (actionmode) {
			case 0:

				break;
			case 1:
				player.btarget = &player.target->bodyparts[3];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::PUSH;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 6;
				if (player.atkpart->attached != NULL) {
					if (rollLimb(player.atkpart, player.btarget) > 50) {
						player.recent.clear();
						player.recent = "Failed to push attached " + player.atkpart->attached->name;
						break;
					}
					player.recent.clear();
					player.recent = "You pushed " + player.atkpart->attached->name + " using " + player.atkpart->name;
					player.atkpart->attached->spos = SpacePos::FORWARD;
					player.atkpart->spos = SpacePos::FORWARD;
					player.atkpart->face = Facing::UP;
					player.atkpart->attached->face = Facing::UP;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 1;
					updateBodyPartPos(&enemy, player.atkpart->attached);
				}
				else {
					player.recent.clear();
					player.recent = "You pushed your " + player.atkpart->name + " forward";
					player.atkpart->spos = SpacePos::FORWARD;
					player.atkpart->face = Facing::UP;
				}
				updateBodyPartPos(&player, player.atkpart);
				break;
			case 3:
				player.atkpart = &player.bodyparts[3];
				break;
			}
			break;
		case KEY_5:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[4];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::WRENCH;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 4;
				if (player.atkpart->attached != NULL) {
					if (rollLimb(player.atkpart, player.btarget) > 50) {
						player.recent.clear();
						player.recent = "Failed to wrench attached " + player.atkpart->attached->name;
						break;
					}
					player.recent.clear();
					player.recent = "You wrenched " + player.atkpart->attached->name + " back using " + player.atkpart->name;
					player.atkpart->attached->spos = SpacePos::BACK;
					player.atkpart->spos = SpacePos::BACK;
					player.atkpart->face = Facing::DOWN;
					player.atkpart->attached->face = Facing::DOWN;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 2;
					updateBodyPartPos(&enemy, player.atkpart->attached);
				}
				else {
					player.recent.clear();
					player.recent = "You wrenched your " + player.atkpart->name + " back";
					player.atkpart->spos = SpacePos::BACK;
					player.atkpart->face = Facing::DOWN;
				}
				updateBodyPartPos(&player, player.atkpart);
				break;
			case 3:
				player.atkpart = &player.bodyparts[4];
				break;
			}
			break;
		case KEY_6:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[5];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::LIFT;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 4;
				if (player.atkpart->attached != NULL) {
					if (rollLimb(player.atkpart, player.btarget) > 45) {
						player.recent.clear();
						player.recent = "Failed to lift attached " + player.atkpart->attached->name;
						break;
					}
					player.recent.clear();
					player.recent = "You lifted " + player.atkpart->attached->name + " using " + player.atkpart->name;
					player.atkpart->attached->mpos = MatPos::INAIR;
					player.atkpart->mpos = MatPos::INAIR;
					player.atkpart->face = Facing::UP;
					player.atkpart->attached->face = Facing::UP;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 4;
					turnnum += 3;
					updateBodyPartPos(&enemy, player.atkpart->attached);
				}
				else {
					player.recent.clear();
					player.recent = "You lifted " + player.atkpart->name + " up";
					player.atkpart->mpos = MatPos::INAIR;
					player.atkpart->face = Facing::UP;
				}
				updateBodyPartPos(&player, player.atkpart);
				break;
			case 3:
				player.atkpart = &player.bodyparts[5];
				break;
			}
			break;
		case KEY_7:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[6];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::PRESS;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				turnnum += 4;
				if (player.atkpart->attached != NULL) {
					if (rollLimb(player.atkpart, player.btarget) > 40) {
						player.recent.clear();
						player.recent = "Failed to press down attached " + player.atkpart->attached->name;
						break;
					}
					player.recent.clear();
					player.recent = "You pressed down " + player.atkpart->attached->name + " using " + player.atkpart->name;
					player.atkpart->attached->mpos = MatPos::ONMAT;
					player.atkpart->mpos = MatPos::ONMAT;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 3;
					updateBodyPartPos(&enemy, player.atkpart->attached);
				}
				else {
					player.recent.clear();
					player.recent = "You pressed down " + player.atkpart->name;
					player.atkpart->mpos = MatPos::ONMAT;
					player.atkpart->face = Facing::DOWN;
				}
				updateBodyPartPos(&player, player.atkpart);
				break;
			case 3:
				player.atkpart = &player.bodyparts[6];
				break;
			}
			break;
		case KEY_8:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[7];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::TWIST;
				turnnum += 3;
				if (!checkMoveCont(&player, player.atkpart)) {
					break;
				}
				if (player.atkpart->attached != NULL) {
					if (rollLimb(player.atkpart, player.btarget) > 30) {
						player.recent.clear();
						player.recent = "Failed to twist attached " + player.atkpart->attached->name;
						break;
					}
					player.atkpart->attached->face = reverseFace(player.atkpart->attached->face);
					player.atkpart->face = reverseFace(player.atkpart->face);
					player.recent.clear();
					player.recent = "You twisted " + player.atkpart->attached->name + " " + FacePosToString(player.atkpart->attached->face) + " using " + player.atkpart->name;
					player.atkpart->attached->health -= player.atkpart->power;
					player.atkpart->attached->fatigue += 5;
					updateBodyPartPos(&enemy, player.atkpart->attached);
				}
				else {
					player.atkpart->face = reverseFace(player.atkpart->face);
					player.recent.clear();
					player.recent = "You twisted " + player.atkpart->name + " " + FacePosToString(player.atkpart->face);
				}
				updateBodyPartPos(&player, player.atkpart);
				break;
			case 3:
				player.atkpart = &player.bodyparts[7];
				break;
			}
			break;
		case KEY_9:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[8];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::RESIST;
				turnnum += 3;
				t = findAttachedPart(player.atkpart, &enemy);
				if (t != NULL) {
					if (rollLimb(player.atkpart, t) > 55) {
						player.atkpart->moveable = false;
						player.recent.clear();
						player.recent = "Resisting at " + player.atkpart->name;
						player.atkpart->health -= 2;
						player.atkpart->fatigue += 3;
					}
					else {
						player.recent.clear();
						player.recent = "Failed to resist at " + player.atkpart->name;
					}

				}
				else {
					player.recent.clear();
					player.recent = "Can't resist on at limb because it's not being grabbed by anything!";
				}
				break;
			case 3:
				player.atkpart = &player.bodyparts[8];
				break;
			}
			break;
		case KEY_0:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[9];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::REMOVE;
				t = findAttachedPart(player.atkpart, &enemy);
				turnnum += 2;
				if (t != NULL) {
					if (rollLimb(player.atkpart, t) > 55) {
						t->attached = NULL;
						player.recent.clear();
						player.recent = "You removed " + t->name + " from your " + player.atkpart->name;
					}
					else {
						player.recent.clear();
						player.recent = "You failed to remove " + t->name + " from your " + player.atkpart->name;
					}
				}
				else { 
					player.recent.clear();
					player.recent = "Nothing to remove not being grabbed by anything at that limb";
				}
				break;
			case 3:
				player.atkpart = &player.bodyparts[9];
				break;
			}
			break;
		case KEY_MINUS:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[10];
				break;
			case 2:
				if (turnchanged) {
					player.recent.clear();
					player.recent = "Not your turn";
					break;
				}
				player.action = Actions::HOLD;
				turnnum += 5;
				if (player.atkpart->attached != NULL) {
					if (rollLimb(player.atkpart, player.atkpart->attached) > 50) {
						player.atkpart->attached->held = true;
						player.atkpart->moveable = false;
						player.atkpart->attached->health -= 2;
						player.atkpart->attached->fatigue += 3;
						player.atkpart->fatigue += 2;
						player.recent.clear();
						player.recent = "You are holding down " + player.atkpart->attached->name + " with " + player.atkpart->name;
					}
					else {
						player.recent.clear();
						player.recent = "Failed to hold down " + player.atkpart->attached->name;
					}
				}
				else {
					player.recent.clear();
					player.recent = "Nothing to hold down not grabbing anything";
				}
				break;
			case 3:
				player.atkpart = &player.bodyparts[10];
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
			}
			break;
		}
		bool removeattach = false;
		if (!checkTileRange(&player, player.target)) {
			removeattach = true;
		}
		wprintw(win2, "STATS\n");
		wprintw(win2, "--------------------\n");
		for (auto& i : player.bodyparts) {
			if (removeattach) {
				i.attached = NULL;
			}
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
			std::string temp = i.name + " " + PosToString(i.mpos, i.spos) + " " + FacePosToString(i.face) + ";Fatigue:" + std::to_string(i.fatigue) + ";Power:" + std::to_string(i.power) + "\n";
			wprintw(win2, temp.c_str());
		}
		wprintw(win2, "--------------------\n");
		wprintw(win2, "SIGHT\n");
		std::string temp3 = enemy.recent + "\n";
		wprintw(win2, temp3.c_str());
		for (auto& i : enemy.bodyparts) {
			if (removeattach) {
				i.attached = NULL;
			}
			std::string temp = i.name + " " + PosToString(i.mpos, i.spos) + " " + FacePosToString(i.face) + ";Fatigue:" + std::to_string(i.fatigue) + ";Power:" + std::to_string(i.power) + "\n";
			wprintw(win2, temp.c_str());
		}
		wprintw(win2, "--------------------\n");
		wprintw(win3, "ACTIONS\n");
		int num = 1;
		switch (actionmode) {
		case 0:
			wprintw(win3, "1:Choose Body Target\n");
			wprintw(win3, "2:Choose your Body Part to Use\n");
			wprintw(win3, "3:Choose Action\n");
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
			wprintw(win3, "Attack Target Part\n");
			wprintw(win3, "1:Grab\n");
			wprintw(win3, "2:Push\n");
			wprintw(win3, "3:Release\n");
			wprintw(win3, "Move Your Own Parts\n");
			wprintw(win3, "4:Push Forward\n");
			wprintw(win3, "5:Wrench Backwards\n");
			wprintw(win3, "6:Lift Up\n");
			wprintw(win3, "7:Press Down\n");
			wprintw(win3, "8:Twist\n");
			wprintw(win3, "9:Resist\n");
			wprintw(win3, "0:Remove Attached Limb\n");
			wprintw(win3, "-:Hold Down Limb\n");
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
		}
		wprintw(win3, "--------------------\n");
		wprintw(win3, "BODY AWARENESS\n");
		std::string temp = "Targeting: " + player.btarget->name + " on " + enemy.name + "\n";
		wprintw(win3, temp.c_str());
		std::string temp4 = "Using: " + player.atkpart->name + "\n";
		wprintw(win3, temp4.c_str());
		if (player.atkpart->attached != NULL) {
			std::string tem = "Selected part " + player.atkpart->name + " attached to " + player.atkpart->attached->name + "\n";
			wprintw(win3, tem.c_str());
		}
		else {
			std::string tem = "Selected part " + player.atkpart->name + " unattached\n";
			wprintw(win3, tem.c_str());
		}
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
		if (turnnum >= turnmax) {
			turnchanged = true;
			playerturn = false;
		}
		if (turnchanged) {
			if (player.target->bodyparts[3].mpos == MatPos::ONMAT && player.target->bodyparts[3].face == Facing::DOWN) {
				ebturns++;
			}
			if (player.bodyparts[3].mpos == MatPos::ONMAT && player.bodyparts[3].face == Facing::DOWN) {
				pbturns++;
			}
			if (ebturns >= 3) {
				player.recent.clear();
				player.recent = "You pinned your enemy!";
				ebturns = 0;
				matcheswon++;
				tournmax++;
				player.upgradepoints += 10;
				for (auto& i : player.bodyparts) {
					i.attached = NULL;
					i.moveable = true;
					i.held = false;
					if (i.n != 10 && i.n != 11) {
						i.mpos = MatPos::INAIR;
					}
					else {
						i.mpos = MatPos::ONMAT;
					}
					i.fatigue = 0;
					i.health = 100;

				}
				if (tournmax > 10) {
					displayemode = 7;
					tournmax = 0;
				}
				else {
					displayemode = 2;
				}
			}
			if (pbturns >= 3) {
				player.recent.clear();
				player.recent = "You got pinned!?";
				pbturns = 0;
				player.x = 10;
				player.y = 12;
				player.tile = 250;
				player.action = Actions::PUSH;
				player.name = "Player";
				player.type = 0;
				player.upgradepoints = 0;
				enemy.x = 10;
				enemy.y = 11;
				enemy.tile = 230;
				enemy.name = "Enemy";
				enemy.target = &player;
				enemy.btarget = &player.bodyparts[0];
				enemy.atkpart = &enemy.bodyparts[8];
				enemy.action = Actions::PUSH;
				player.target = &enemy;
				player.btarget = &enemy.bodyparts[0];
				player.atkpart = &player.bodyparts[8];
				turnnum = 0;
				turnmax = 10;
				playerturn = true;
				turnchanged = false;
				actionmode = 0;
				ebturns = 0;
				pbturns = 0;
				matcheswon = 0;
				initParts(&player);
				initEntityContext(&player);
				initMConts(&player);
				generateEnemy(&enemy, &player);
				displayemode = 3;
			}
			//Put enemy AI here
			std::vector<Part*> temps;
			findAttachedLimbs(temps, enemy.bodyparts);
			std::sort(temps.begin(), temps.end(), partSortGreater);
			std::vector<Part*> powerLimbs;
			findLimbs(powerLimbs, enemy.bodyparts);
			std::sort(powerLimbs.begin(), powerLimbs.end(), partSortPower);

			enemy.pcs.clear();
			for (auto& i : player.bodyparts) {
				if (i.attached != NULL) {
					for (auto j : enemy.bodyparts) {
						if (i.attached->n == j.n) {
							PartConnector p = { &j, &i };
							enemy.pcs.push_back(p);
						}
					}
				}
			}
			int totalf = 0;
			for (auto& i : enemy.bodyparts) {
				totalf += i.fatigue;
				i.moveable = true;
			}
			for (auto& i : player.bodyparts) {
				i.held = false;
			}
			if (rollBasic() > 70) {
				if (totalf > 200) {
					enemy.state = State::REST;
				}
			}
			if (enemy.bodyparts[3].face == Facing::DOWN) {
				if (enemy.bodyparts[3].attached != NULL) {
					if (rollLimb(&enemy.bodyparts[3], enemy.bodyparts[3].attached) > 25) {
						enemy.recent.clear();
						enemy.recent = "Enemy failed to twist attached " + temps[0]->attached->name;
						
					}
					else {
						enemy.bodyparts[3].attached->face = reverseFace(enemy.bodyparts[3].attached->face);
						enemy.bodyparts[3].face = reverseFace(enemy.bodyparts[3].face);
						enemy.recent.clear();
						enemy.recent = "Enemy twisted his back up";
						enemy.bodyparts[3].attached->health -= enemy.bodyparts[3].power;
						enemy.bodyparts[3].attached->fatigue += 5;
						updateBodyPartPos(&enemy, &enemy.bodyparts[3]);
						updateBodyPartPos(&player, enemy.bodyparts[3].attached);
						enemy.state = State::DEFEND;
					}
				}
				else {
					if (rollBasic() > 50) {
						enemy.action = Actions::TWIST;
						enemy.bodyparts[3].face = reverseFace(player.atkpart->face);
						enemy.recent.clear();
						enemy.recent = "Enemy twisted his back up";
						updateBodyPartPos(&enemy, &enemy.bodyparts[3]);
						enemy.state = State::DEFEND;
					}
				}
			}
			if (rollBasic() > 55) {
				Part* ph = NULL;
				for (int i = temps.size() - 1; i >= 0; i--) {
					if (!temps[i]->attached->moveable) {
						ph = temps[i];
					}
				}
				if (ph != NULL) {
					if (rollLimb(ph, ph->attached) > 50) {
						enemy.action = Actions::HOLD;
						ph->attached->held = true;
						ph->moveable = false;
						ph->attached->health -= 2;
						ph->attached->fatigue += 3;
						ph->fatigue += 2;
						enemy.recent.clear();
						enemy.recent = "Enemy holding down " + ph->attached->name + " with " + ph->name;
					}
				}
				else {
					if (rollBasic() > 35) {
						//Pick random limb to hold down
						std::vector<Part*> out;
						if (temps.size() > 0) {
							std::sample(temps.begin(), temps.end(), std::back_inserter(out), 1, std::mt19937{ std::random_device{}() });
							Part* p = out[0];
							enemy.action = Actions::HOLD;
							p->attached->held = true;
							p->moveable = false;
							p->attached->health -= 2;
							p->attached->fatigue += 3;
							p->fatigue += 2;
							enemy.recent.clear();
							enemy.recent = "Enemy holding down " + p->attached->name + " with " + p->name;
						}
					}
				}
			}
			if (rollBasic() > 40) {
				int bpchk[12] = { 6, 0, 7, 3, 1, 2, 10, 11, 4, 5, 8, 9 };
				for (int i = 0; i < 12; i++) {
					if (enemy.bodyparts[bpchk[i]].mpos == MatPos::ONMAT) {
						enemy.action = Actions::LIFT;
						if (enemy.bodyparts[bpchk[i]].attached == NULL) {
							if (!enemy.bodyparts[bpchk[i]].held) {
								if (rollLimb(&enemy.bodyparts[bpchk[i]], player.atkpart) > 55) {
									enemy.recent.clear();
									enemy.recent = "Lifted " + enemy.bodyparts[bpchk[i]].name;
									enemy.bodyparts[bpchk[i]].mpos = MatPos::INAIR;
									enemy.bodyparts[bpchk[i]].face = Facing::UP;
									//updateBodyPartPos(&enemy, &enemy.bodyparts[bpchk[i]]);
								}
								else {
									enemy.recent.clear();
									enemy.recent = "Failed to lift " + enemy.bodyparts[bpchk[i]].name;
								}
							}
						}
					}
				}
			}
			int r;
			int roll = rollBasic();
			//Need to add the three new Actions
			switch (enemy.state) {
			case State::DEFEND:
				if (!checkTileRange(&enemy, &player)) {
					if (player.x < enemy.x) {
						int dif = player.tile - enemy.tile;
						if (tiles[enemy.tile - 1].type != 1) {
							enemy.x--;
							enemy.tile--;
						}
						}
						else {
							if (tiles[enemy.tile + 1].type != 1) {
								enemy.tile++;
								enemy.x++;
							}
						}
						if (enemy.y < player.y) {
							if (tiles[enemy.tile + 20].type != 1) {
								enemy.tile += 20;
								enemy.y++;
							}
						}
						else {
							if (tiles[enemy.tile - 20].type != 1) {
								enemy.tile -= 20;
								enemy.y--;
							}
						}
					
				}
				else {
					if (player.tile == enemy.tile) {
						if (tiles[enemy.tile + 20].type != 1) {
							enemy.tile += 20;
							enemy.y++;
						}
						else if (tiles[enemy.tile - 20].type != 1) {
							enemy.tile -= 20;
							enemy.y--;
						}
						else if (tiles[enemy.tile + 1].type != 1) {
							enemy.tile++;
							enemy.x++;
						}
						else if (tiles[enemy.tile - 1].type != 1) {
							enemy.tile--;
							enemy.x--;
						}
					}
				}
				if (roll > 70) {
					//Will try to grab onto random limb, will then activate an indicator, so it's next turn it will try and wrench you down
					//Figure out how to do random grab of parts from a set list, instead of in a range
					std::vector<Part*> rando;
					std::sample(powerLimbs.begin(), powerLimbs.end(), std::back_inserter(rando), 1, std::mt19937{ std::random_device{}() });
					enemy.atkpart = powerLimbs[0];
					if (enemy.atkpart->n == 3 || enemy.atkpart->n == 0 || enemy.atkpart->n == 7) {
						enemy.atkpart = &enemy.bodyparts[8];
					}
					enemy.action = Actions::GRAB;
					int bpchk[12] = { 6, 0, 7, 3, 1, 2, 10, 11, 4, 5, 8, 9 };
					MatPos mp[12] = { MatPos::ONMAT, MatPos::INAIR, MatPos::INAIR, MatPos::INAIR, MatPos::INAIR, MatPos::ONMAT, MatPos::ONMAT, MatPos::INAIR, MatPos::INAIR, MatPos::INAIR, MatPos::INAIR, MatPos::INAIR};
					for (int i = 0; i < 12; i++) {
						if (player.bodyparts[bpchk[i]].mpos == mp[i]) {
							enemy.btarget = &player.bodyparts[bpchk[i]];
							break;
						}
					}
					if (!checkTileRange(&enemy, &player)) {
						enemy.recent.clear();
						enemy.recent = "Enemy tried to grab you but was out of range";
						break;
					}
					if (!checkMoveCont(&enemy, enemy.atkpart)) {
						break;
					}
					if (rollLimb(player.atkpart, player.btarget) > 75) {
						enemy.recent.clear();
						enemy.recent = "Failed to grab " + enemy.btarget->name;
						break;
					}
					enemy.atkpart->attached = enemy.btarget;
					updateBodyPartPos(&enemy, enemy.atkpart);
					enemy.recent.clear();
					enemy.recent = "Enemy grabbed " + enemy.btarget->name + " using " + enemy.atkpart->name;
					enemy.atkpart->fatigue += 1;
					enemy.state = State::ATTACK;
				}
				else if (roll > 30) {
					//Will just push on random limb
					enemy.action = Actions::PUSH;
					if (!checkTileRange(&enemy, &player)) {
						enemy.recent.clear();
						enemy.recent = "Enemy tried to push you but was out of range";
						break;
					}
					r = rand() % 11;
					enemy.atkpart = &enemy.bodyparts[r];
					r = rand() % 11;
					enemy.btarget = &player.bodyparts[r];
					if (rollLimb(enemy.atkpart, enemy.btarget) > 60) {
						enemy.recent.clear();
						enemy.recent = "Failed to push " + enemy.btarget->name;
						break;
					}
					if (!checkMoveCont(&enemy, enemy.atkpart)) {
						break;
					}
					enemy.recent.clear();
					enemy.recent = "Enemy pushed at " + enemy.btarget->name + " using " + enemy.atkpart->name;
					enemy.atkpart->spos = SpacePos::FORWARD;
					enemy.btarget->spos = SpacePos::BACK;
					enemy.atkpart->face = Facing::UP;
					enemy.btarget->health -= enemy.atkpart->power;
					enemy.btarget->fatigue += 3;
					enemy.atkpart->fatigue++;
					updateBodyPartPos(&player, enemy.atkpart);
					updateBodyPartPos(&enemy, enemy.btarget);
				}
				else {
					enemy.recent.clear();
					enemy.recent = "Enemy hesitated too much";
				}
				//Tries to remove any limbs you have attached to it
				if (enemy.pcs.size() > 0) {
					std::vector<PartConnector> out;
					//PartConnector ot = enemy.pcs[0];
					std::sample(enemy.pcs.begin(), enemy.pcs.end(), std::back_inserter(out), 1, std::mt19937{ std::random_device{}() });
					if (rollLimb(out[0].mpart, out[0].attpart) > 70) {
						out[0].attpart->attached = NULL;
						//enemy.recent.clear();
						//enemy.recent = "Target removed " + out[0].attpart->name + " from their " + out[0].mpart->name;
					}
					else {
						//enemy.recent.clear();
						//enemy.recent = "Target tried to remove " + out[0].attpart->name + " from their " + out[0].mpart->name;
						if (rollLimb(out[0].mpart, out[0].attpart) > 55) {
							//enemy.recent.clear();
							//enemy.recent = "Enemy resisting at " + out[0].mpart->name;
							enemy.action = Actions::RESIST;
							out[0].mpart->moveable = false;
						}
					}
				}
				break;
			case State::ATTACK:
				if (!checkTileRange(&enemy, &player)) {
					if (player.x < enemy.x) {
						int dif = player.tile - enemy.tile;
						if (tiles[enemy.tile - 1].type != 1) {
							enemy.x--;
							enemy.tile--;
						}
					}
					else {
						if (tiles[enemy.tile + 1].type != 1) {
							enemy.tile++;
							enemy.x++;
						}
					}
					if (enemy.y < player.y) {
						if (tiles[enemy.tile + 20].type != 1) {
							enemy.tile += 20;
							enemy.y++;
						}
					}
					else {
						if (tiles[enemy.tile - 20].type != 1) {
							enemy.tile -= 20;
							enemy.y--;
						}
					}

				}
				//In this state will just ignore limbs you have attached and just try to wrench any limbs it has attached to you down
				if (roll > 30) {
					//Try to press all limbs down, and if the limb selected is on the mat already try to twist it over, maybe add limb selection so it targets limbs that will help it pin you
					if (temps.size() > 0) {
						if (temps[0]->attached->mpos == MatPos::ONMAT) {
							
							enemy.action = Actions::TWIST;
							if (!checkMoveCont(&enemy, temps[0])) {
								break;
							}
							if (rollLimb(temps[0], temps[0]->attached) > 40) {
								enemy.recent.clear();
								enemy.recent = "Enemy failed to twist attached " + temps[0]->attached->name;
								break;
							}
							temps[0]->attached->face = reverseFace(temps[0]->attached->face);
							temps[0]->face = reverseFace(player.atkpart->face);
							enemy.recent.clear();
							enemy.recent = "Enemy twisted " + temps[0]->attached->name + " " + FacePosToString(temps[0]->attached->face) + " using " + temps[0]->name;
							temps[0]->attached->health -= temps[0]->power;
							temps[0]->attached->fatigue += 5;
							updateBodyPartPos(&enemy, temps[0]);
							updateBodyPartPos(&player, temps[0]->attached);
							enemy.state = State::DEFEND;
						}
						else {
							enemy.action = Actions::PRESS;
							if (!checkMoveCont(&enemy, enemy.atkpart)) {
								break;
							}
							if (rollLimb(temps[0], temps[0]->attached) > 60) {
								enemy.recent.clear();
								enemy.recent = "Failed to press down attached " + temps[0]->attached->name;
								break;
							}
							enemy.recent.clear();
							enemy.recent = "Enemy pressed down " + temps[0]->attached->name + " using " + temps[0]->name;
							temps[0]->attached->mpos = MatPos::ONMAT;
							temps[0]->mpos = MatPos::ONMAT;
							temps[0]->fatigue += 2;
							temps[0]->attached->health -= player.atkpart->power;
							temps[0]->attached->fatigue += 3;
							updateBodyPartPos(&enemy, temps[0]);
							updateBodyPartPos(&player, temps[0]->attached);
						}
					}
					else {
						enemy.state = State::DEFEND;
					}
				}
				else {
					enemy.recent.clear();
					enemy.recent = "Enemy hesitated too much";
				}
				break;
			case State::REST:
				enemy.recent.clear();
				enemy.recent = "Enemy resting!";
				//Will just stayback and do nothing besides from to time removing limbs you attach, will try to move away from you, but will stay in action distance
				if (!checkTileRange(&enemy, &player)) {
					if (player.x < enemy.x) {
						int dif = player.tile - enemy.tile;
						if (tiles[enemy.tile - 1].type != 1) {
							enemy.x--;
							enemy.tile--;
						}
					}
					else {
						if (tiles[enemy.tile + 1].type != 1) {
							enemy.tile++;
							enemy.x++;
						}
					}
					if (enemy.y < player.y) {
						if (tiles[enemy.tile + 20].type != 1) {
							enemy.tile += 20;
							enemy.y++;
						}
					}
					else {
						if (tiles[enemy.tile - 20].type != 1) {
							enemy.tile -= 20;
							enemy.y--;
						}
					}

				}
				else {
					if (player.tile == enemy.tile) {
						if (tiles[enemy.tile + 20].type != 1) {
							enemy.tile += 20;
							enemy.y++;
						}
						else if (tiles[enemy.tile - 20].type != 1) {
							enemy.tile -= 20;
							enemy.y--;
						}
						else if (tiles[enemy.tile + 1].type != 1) {
							enemy.tile++;
							enemy.x++;
						}
						else if (tiles[enemy.tile - 1].type != 1) {
							enemy.tile--;
							enemy.x--;
						}
					}
				}
				if (roll > 60) {
					if (totalf <= 150) {
						enemy.state = State::DEFEND;
					}
				}
				else {
					enemy.recent.clear();
					enemy.recent = "Enemy hesitated too much";
				}
				if (enemy.pcs.size() > 0) {
					std::vector<PartConnector> out;
					std::sample(enemy.pcs.begin(), enemy.pcs.end(), std::back_inserter(out), 1, std::mt19937{ std::random_device{}() });
					if (rollLimb(out[0].mpart, out[0].attpart) > 65) {
						out[0].attpart->attached = NULL;
						enemy.recent.clear();
						enemy.recent = "Target removed " + out[0].attpart->name + " from their " + out[0].mpart->name;
					}
					else {
						enemy.recent.clear();
						enemy.recent = "Target tried to remove " + out[0].attpart->name + " from their " + out[0].mpart->name;
						if (rollLimb(out[0].mpart, out[0].attpart) > 55) {
							enemy.recent.clear();
							enemy.recent = "Enemy resisting at " + out[0].mpart->name;
							enemy.action = Actions::RESIST;
							out[0].mpart->moveable = false;
						}
					}
				}
				break;
			case State::RUN:
				//Will try to move out of action distance, make this super short so as not be frustrating to player
				enemy.recent.clear();
				enemy.recent = "Enemy running!";
				if (!checkTileRange(&enemy, &player)) {
					if (player.x < enemy.x) {
						int dif = player.tile - enemy.tile;
						if (tiles[enemy.tile + 1].type != 1) {
							enemy.tile++;
							enemy.x++;
						}
					}
					else {
						if (tiles[enemy.tile - 1].type != 1) {
							enemy.x--;
							enemy.tile--;
						}
					}
					if (enemy.y < player.y) {
						if (tiles[enemy.tile - 20].type != 1) {
							enemy.tile -= 20;
							enemy.y--;
						}
						
					}
					else {
						if (tiles[enemy.tile + 20].type != 1) {
							enemy.tile += 20;
							enemy.y++;
						}
					}

				}
				else {
					if (player.tile == enemy.tile) {
						if (tiles[enemy.tile + 20].type != 1) {
							enemy.tile += 20;
							enemy.y++;
						}
						else if (tiles[enemy.tile - 20].type != 1) {
							enemy.tile -= 20;
							enemy.y--;
						}
						else if (tiles[enemy.tile + 1].type != 1) {
							enemy.tile++;
							enemy.x++;
						}
						else if (tiles[enemy.tile - 1].type != 1) {
							enemy.tile--;
							enemy.x--;
						}
					}
				}
				if (roll > 50) {
					enemy.state = State::DEFEND;
				}

				break;
			}

			for (auto& i : player.bodyparts) {
				i.moveable = true;
			}
			for (auto& i : enemy.bodyparts) {
				i.held = false;
			}

			turnchanged = false;
			turnnum = 0;
		}
		//presents screen changes
		wrefresh(win);
		wrefresh(win2);
		wrefresh(win3);
	}
	else if (displayemode == 1) {
		wclear(win);
		wclear(win2);
		wclear(win3);
		switch (key) {
		case KEY_1:
			displayemode = 0;
			break;
		case KEY_2:
			exitf = true;
			break;
		}
		wprintw(win2, "SELECT WHAT TO DO\n");
		wprintw(win2, "1:Play\n");
		wprintw(win2, "2:Exit\n");
		wrefresh(win);
		wrefresh(win2);
		wrefresh(win3);
	}
	else if (displayemode == 2) {
	wclear(win);
	wclear(win2);
	wclear(win3);
	switch (key) {
	case KEY_1:
		displayemode = 4;
		break;
	case KEY_2:
		displayemode = 5;
		break;
	}
	wprintw(win2, "YOU WON, 10 POWER POINTS GAINED, SELECT WHAT TO DO NEXT\n");
	wprintw(win2, "1:Select limb to focus on to increase muscle growth overnight\n");
	wprintw(win2, "2:Go to bed immediately and not use your current points\n");
	wrefresh(win);
	wrefresh(win2);
	wrefresh(win3);
	}
	else if (displayemode == 3) {
	wclear(win);
	wclear(win2);
	wclear(win3);
	switch (key) {
	case KEY_1:
		displayemode = 1;
		break;
	}
	wprintw(win2, "YOU LOST\n");
	wprintw(win2, "Press 1 to return to menu");
		wrefresh(win);
		wrefresh(win2);
		wrefresh(win3);
	}
	else if (displayemode == 4) {
	wclear(win);
	wclear(win2);
	wclear(win3);
	switch (key) {
	case KEY_1:
		if (player.upgradepoints > 0) {
			player.bodyparts[0].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_2:
		if (player.upgradepoints > 0) {
			player.bodyparts[1].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_3:
		if (player.upgradepoints > 0) {
			player.bodyparts[2].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_4:
		if (player.upgradepoints > 0) {
			player.bodyparts[3].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_5:
		if (player.upgradepoints > 0) {
			player.bodyparts[4].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_6:
		if (player.upgradepoints > 0) {
			player.bodyparts[5].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_7:
		if (player.upgradepoints > 0) {
			player.bodyparts[6].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_8:
		if (player.upgradepoints > 0) {
			player.bodyparts[7].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_9:
		if (player.upgradepoints > 0) {
			player.bodyparts[8].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_0:
		if (player.upgradepoints > 0) {
			player.bodyparts[9].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_MINUS:
		if (player.upgradepoints > 0) {
			player.bodyparts[10].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_EQUAL:
		if (player.upgradepoints > 0) {
			player.bodyparts[11].power++;
			player.upgradepoints--;
		}
		break;
	case KEY_Q:
		if (player.upgradepoints > 1) {
			turnmax++;
			player.upgradepoints -= 2;
		}
		break;
	case 27:
		displayemode = 5;
		break;
	}
	wprintw(win2, "LIMBS TO UPGRADE\n");
	std::string te = "Upgrade Points: " + std::to_string(player.upgradepoints) + "\n";
	wprintw(win2, te.c_str());
	for (int i = 0; i < 12; i++) {
		std::string temp = std::to_string(i + 1) + ":" + player.bodyparts[i].name + " Power: " + std::to_string(player.bodyparts[i].power) + "\n";
		switch (i) {
		case 9:
			temp = "0:" + player.bodyparts[i].name + " Power: " + std::to_string(player.bodyparts[i].power) + "\n";
			break;
		case 10:
			temp = "-:" + player.bodyparts[i].name + " Power: " + std::to_string(player.bodyparts[i].power) + "\n";
			break;
		case 11:
			temp = "=:" + player.bodyparts[i].name + " Power: " + std::to_string(player.bodyparts[i].power) + "\n";
			break;
		}
		wprintw(win2, temp.c_str());
	}
	wprintw(win2, "Press Q to increase conditiong\n");
	wprintw(win2, "Press escape to go to sleep\n");
	wrefresh(win);
	wrefresh(win2);
	wrefresh(win3);
	}
	else if (displayemode == 5) {
	wclear(win);
	wclear(win2);
	wclear(win3);
	switch (key) {
	case KEY_1:
		displayemode = 0;
		pbturns = 0;
		player.x = 10;
		player.y = 12;
		player.tile = 250;
		player.action = Actions::PUSH;
		enemy.x = 10;
		enemy.y = 11;
		enemy.tile = 230;
		enemy.name = "Enemy";
		enemy.target = &player;
		enemy.btarget = &player.bodyparts[0];
		enemy.atkpart = &enemy.bodyparts[8];
		enemy.action = Actions::PUSH;
		player.target = &enemy;
		player.btarget = &enemy.bodyparts[0];
		player.atkpart = &player.bodyparts[8];
		turnnum = 0;
		playerturn = true;
		turnchanged = false;
		actionmode = 0;
		ebturns = 0;
		pbturns = 0;
		generateEnemy(&enemy, &player);
		break;
	}
	wprintw(win2, "You slept through the night, back to the tournament\n");
	wprintw(win2, "Press 1 to wrestle your next match\n");
	wrefresh(win);
	wrefresh(win2);
	wrefresh(win3);
	}
	else if (displayemode == 7) {
	wclear(win);
	wclear(win2);
	wclear(win3);
	switch (key) {
	case KEY_1:
		displayemode = 2;
		break;
	case KEY_2:
		displayemode = 1;
		break;
	}

	wprintw(win2, "You won the tournament!\n");
	wprintw(win2, "Press 1 to continue wrestling\n");
	wprintw(win2, "Press 2 to return to menu\n");
	wrefresh(win);
	wrefresh(win2);
	wrefresh(win3);
	}


	}
	endwin();
	return 0;
}