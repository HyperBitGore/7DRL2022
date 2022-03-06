#include <iostream>
#include <curses.h>
bool exitf = false;

struct Tile {
	int x;
	int y;
	int type;
	char t;
};
struct Entity {
	int x;
	int y;
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


//https://github.com/wmcbrine/PDCurses/blob/master/docs/MANUAL.md
//https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
//Add second "window" displaying all stats and other live stuff player needs to know about like enemy movements, and time of match
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
	WINDOW* win2 = newwin(10, 10, 20, 20);
	wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wborder(win2, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(win);
	wrefresh(win2);
	Entity player = { 10, 10 };
	Entity enemy = { 10, 8 };
	Tile tiles[400];
	initTiles(tiles);
	while (!exitf) {
		int key = getch();
		switch (key) {
		case KEY_LEFT:
			player.x--;
			break;
		case KEY_RIGHT:
			player.x++;
			break;
		case KEY_DOWN:
			player.y++;
			break;
		case KEY_UP:
			player.y--;
			break;
		}

		for (auto& i : tiles) {
			wmove(win, i.y, i.x);
			if (i.x == player.x && i.y == player.y) {
				waddch(win, '@');
			}
			else if(i.x == enemy.x && i.y == enemy.y){
				waddch(win, '&');
			}
			else {
				waddch(win, i.t);
			}
		}
		//presents screen changes
		wrefresh(win);
		wrefresh(win2);
	}
	endwin();
	return 0;
}