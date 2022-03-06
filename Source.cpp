#include <iostream>
#include <string>
#include <curses.h>
bool exitf = false;

struct Tile {
	int x;
	int y;
	int type;
	char t;
};
struct Part {
	int fatigue;
	int power;
	std::string name;
};
//0, core; 1, right leg; 2, left leg; 3, back; 4, left arm; 5, right arm; 6, neck;
struct Entity {
	int x;
	int y;
	int tile;
	Part bodyparts[7];
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
	for (int i = 0; i < 7; i++) {
		e->bodyparts[i].fatigue = 0;
		e->bodyparts[i].power = 10;
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
		}
	}
}



//https://github.com/wmcbrine/PDCurses/blob/master/docs/MANUAL.md
//https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
//Add basic wrestling combat
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
	WINDOW* win2 = newwin(30, 30, 0, 20);
	wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wborder(win2, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(win);
	wrefresh(win2);
	Entity player = { 10, 10, 250};
	Entity enemy = { 20, 14, 230};
	initParts(&player);
	initParts(&enemy);
	Tile tiles[400];
	initTiles(tiles);
	while (!exitf) {
		wclear(win2);
		int key = getch();
		switch (key) {
		case KEY_LEFT:
			if (tiles[player.tile - 1].t == ' ') {
				player.tile--;
			}
			break;
		case KEY_RIGHT:
			if (tiles[player.tile + 1].t == ' ') {
				player.tile++;
			}
			break;
		case KEY_DOWN:
			if (tiles[player.tile + 20].t == ' ') {
				player.tile += 20;
			}
			break;
		case KEY_UP:
			if (tiles[player.tile - 20].t == ' ') {
				player.tile -= 20;
			}
			break;
		}
		wprintw(win2, "STATS\n");
		wprintw(win2, "--------------------\n");
		for (auto& i : player.bodyparts) {
			std::string temp = i.name + "; Fatigue:" + std::to_string(i.fatigue) + "; Power:" + std::to_string(i.power);
			wprintw(win2, temp.c_str());
			if (temp.size() < 30) {
				wprintw(win2, "\n");
			}
		}
		wprintw(win2, "--------------------\n");
		wprintw(win2, "SIGHT\n");
		//Put most action of enemy here
		wprintw(win2, "--------------------\n");
		wprintw(win2, "ACTIONS\n");
		//Hit the key corresponing to context action
		wprintw(win2, "--------------------\n");
		wprintw(win2, "REACTIONS\n");
		//Hit the key for defense against enemy moves, will do stat roll to see if you succeded


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
	}
	endwin();
	return 0;
}