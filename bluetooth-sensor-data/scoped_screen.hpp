#include <ncurses.h>

// ncurses initialization / destruction functions
class Scoped_Screen
{
public:
    Scoped_Screen();
    ~Scoped_Screen();
};

Scoped_Screen::Scoped_Screen()
{
    initscr();
    cbreak();
    noecho();
    clear();
    keypad(stdscr, TRUE);
    curs_set(0);
    refresh();
}

Scoped_Screen::~Scoped_Screen()
{
    nocbreak();
    endwin();
}
