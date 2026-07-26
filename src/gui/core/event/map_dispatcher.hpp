#pragma once

#include "gui/widgets/widget.hpp"
#include "gui/auxiliary/iterator/walker.hpp"

class play_controller;

namespace gui2::event
{

class map_dispatcher : public gui2::widget {
private:
	play_controller& controller_;

public:
	map_dispatcher(play_controller& controller);
	bool is_at(const point& coordinate) const override;

	void mouse_motion(bool& handled, const point& p);
	void mouse_left_up(bool& handled, const point& p);
	void mouse_left_down(bool& handled, const point& p);
	void mouse_right_down(bool& handled, const point& p);
	void mouse_middle_up(bool& handled, const point& p);
	void mouse_middle_down(bool& handled, const point& p);
	void mouse_wheel(bool& handled, const point& p, const point& scroll);

	// TODO Necessary evil because this is a widget and not a dispatcher
	// getting rid of this requires refactor dispatcher & friends to not rely on widget
	void request_reduce_width(unsigned int) override {};
	point calculate_best_size() const override { return {0,0}; };
	bool disable_click_dismiss() const override { return true; };
	iteration::walker_ptr create_walker() override
	{
		return nullptr;
	}

};

}

