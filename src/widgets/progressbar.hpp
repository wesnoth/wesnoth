#ifndef PROGRESS_BAR_HPP_INCLUDED
#define PROGRESS_BAR_HPP_INCLUDED

#include "widget.hpp"

namespace gui {

class progress_bar : public widget
{
public:
	progress_bar(display& disp);

	void set_progress_percent(int progress);

	void draw();

private:
	int progress_;
};

}

#endif