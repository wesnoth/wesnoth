#ifndef CURSOR_HPP_INCLUDED
#define CURSOR_HPP_INCLUDED

namespace cursor
{

struct manager
{
	manager();
	~manager();
};

enum CURSOR_TYPE { NORMAL, WAIT, MOVE, ATTACK, NUM_CURSORS };

void set(CURSOR_TYPE type);

struct setter
{
	setter(CURSOR_TYPE type);
	~setter();

private:
	CURSOR_TYPE old_;
};

}

#endif