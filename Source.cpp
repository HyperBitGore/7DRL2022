#include <iostream>
#include <string>
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

bool exitf = false;

//Own body parts: Push, forward; Wrench, move back; Lift, move up; Press, move down;
enum class Actions {GRAB, PUSH, WRENCH, LIFT, RELEASE, PRESS};
enum MatPos {ONMAT, INAIR};

struct Tile {
	int x;
	int y;
	int type;
	char t;
};
struct Part {
	int fatigue;
	int power;
	int health;
	MatPos mpos;
	std::string name;
	Part* attached;
};
//0, core; 1, right leg; 2, left leg; 3, back; 4, left arm; 5, right arm; 6, neck; 7, hip; 8, right hand; 9, left hand; 10, right foot; 11 left foot;
struct Entity {
	int x;
	int y;
	int tile;
	Part bodyparts[12];
	Entity* target;
	Part* btarget;
	Part* atkpart;
	Part* activepart;
	Actions action;
	std::string recent;
	std::string name;
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
		e->bodyparts[i].power = 10;
		e->bodyparts[i].attached = NULL;
		e->bodyparts[i].health = 100;
		e->bodyparts[i].mpos = INAIR;
		//Did the e->bodyparts[i].mpos = INAIR; for all the cases in case i want to add feet
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
			e->bodyparts[i].mpos = ONMAT;
			break;
		case 11:
			e->bodyparts[i].name = "Left Foot";
			e->bodyparts[i].mpos = ONMAT;
			break;
		}
	}
}



//https://github.com/wmcbrine/PDCurses/blob/master/docs/MANUAL.md
//https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
//Add all the context checking needed for combat
//Add player body parts actions
//Add enemy AI
//Add simple generation for matches, tournaments for dungeons? Can make decisons what to do between matches for basic role if good or bad effect happens from this
//Stat increases based off matches
//Add practice throughout the week if time
//If time do fancy coloring of tiles
int main() {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	WINDOW* win = newwin(20, 20, 0, 0);
	WINDOW* win2 = newwin(30, 56, 0, 20);
	WINDOW* win3 = newwin(30, 40, 0, 68);
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
	enemy.activepart = &enemy.bodyparts[5];
	player.target = &enemy;
	player.btarget = &enemy.bodyparts[0];
	player.atkpart = &player.bodyparts[8];
	player.activepart = &player.bodyparts[5];
	initParts(&player);
	initParts(&enemy);
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
				player.recent = "You grabbed " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->attached = player.btarget;
				turnnum += 5;
				break;
			case 3:
				player.atkpart = &player.bodyparts[0];
				break;
			case 4:
				
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
				player.recent = "You pushed at " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				turnnum += 6;
				break;
			case 3:
				player.atkpart = &player.bodyparts[1];
				break;
			case 4:
				
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
					player.recent = "You can't wrench " + player.atkpart->name + " unattached";
					break;
				}
				player.action = Actions::WRENCH;
				player.recent = "You wrenched " + player.btarget->name + " back on " + player.target->name + " using " + player.atkpart->name;
				turnnum += 4;
				break;
			case 3:
				player.atkpart = &player.bodyparts[2];
				break;
			case 4:
				
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
			case 2:
				if (player.atkpart->attached == NULL) {
					player.recent = "You can't lift " + player.atkpart->name + " unattached";
					break;
				}
				player.action = Actions::LIFT;
				player.recent = "You lifted " + player.btarget->name + " up on " + player.target->name + " using " + player.atkpart->name;
				turnnum += 7;
				break;
			case 3:
				player.atkpart = &player.bodyparts[3];
				break;
			case 4:
				
				break;
			}
			break;
		case KEY_5:
			switch (actionmode) {
			case 1:
				player.btarget = &player.target->bodyparts[4];
				break;
			case 2:
				if (player.atkpart->attached == NULL) {
					player.recent = "You can't release " + player.atkpart->name + " unattached";
					break;
				}
				player.action = Actions::RELEASE;
				player.recent = "You released " + player.btarget->name + " on " + player.target->name + " using " + player.atkpart->name;
				player.atkpart->attached = NULL;
				turnnum += 1;
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
		}
		wprintw(win2, "STATS\n");
		wprintw(win2, "--------------------\n");
		for (auto& i : player.bodyparts) {
			if (i.attached != NULL) {
				if (turnchanged) {
					i.fatigue += 3;
				}
			}
			if (i.fatigue > 100) {
				i.fatigue = 100;
			}
			std::string temp = i.name + "; Fatigue:" + std::to_string(i.fatigue) + "; Power:" + std::to_string(i.power);
			wprintw(win2, temp.c_str());
			if (temp.size() < 40) {
				wprintw(win2, "\n");
			}
		}
		wprintw(win2, "--------------------\n");
		wprintw(win2, "SIGHT\n");
		std::string temp3 = player.target->recent + "\n";
		wprintw(win2, temp3.c_str());
		wprintw(win2, "--------------------\n");
		wprintw(win2, "BODY AWARENESS\n");
		std::string temp = "Targeting: " + player.btarget->name + " on " + enemy.name + "\n";
		wprintw(win2, temp.c_str());
		std::string temp4 = "Using: " + player.atkpart->name + "\n";
		wprintw(win2, temp4.c_str());
		std::string temp2 = player.recent + "\n";
		wprintw(win2, temp2.c_str());
		wprintw(win2, "--------------------\n");
		wprintw(win2, "TURN\n");
		std::string temp5 = "Turn Max: " + std::to_string(turnmax) + "\n";
		wprintw(win2, temp5.c_str());
		std::string temp6 = "Current Turn: " + std::to_string(turnnum) + "\n";
		wprintw(win2, temp6.c_str());
		if (turnchanged) {
			wprintw(win2, "Not your turn\n");
		}
		else {
			wprintw(win2, "Your turn\n");
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
			wprintw(win3, "3:Wrench\n");
			wprintw(win3, "4:Lift\n");
			wprintw(win3, "5:Release\n");
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
				std::string tem = "Can't move " + player.atkpart->name + " freely, attached to " + player.atkpart->attached->name + "\n";
				wprintw(win3, tem.c_str());
			}
			else {
				std::string tem = player.atkpart->name + " unattached can move freely\n";
				wprintw(win3, tem.c_str());
			}
			wprintw(win3, "1:Push\n");
			wprintw(win3, "2:Wrench\n");
			wprintw(win3, "3:Lift\n");
			wprintw(win3, "4:Press\n");
			wprintw(win3, "Press escape to go back\n");
			break;
		}
		wprintw(win3, "--------------------\n");
		wprintw(win3, "REACTIONS\n");
		//Hit the key for defense against enemy moves, will do stat roll to see if you succeded
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