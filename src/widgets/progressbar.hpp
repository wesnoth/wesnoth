#ifndef PROGRESS_BAR_HPP_INCLUDED
#define PROGRESS_BAR_HPP_INCLUDED

#include "widget.hpp"

namespace gui {

class progress_bar : public widget
{
public:
	progress_bar(CVideo& video);

	void set_progress_percent(int progress);
	void set_text(const std::string& text);

	void draw_contents();

private:
	int progress_;
	std::string text_;
};

}

#endif
