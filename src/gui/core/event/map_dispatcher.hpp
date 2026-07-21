#pragma once

#include "gui/widgets/widget.hpp"
#include "gui/auxiliary/iterator/walker.hpp"

namespace gui2
{

namespace event
{

class map_dispatcher : public gui2::widget {
public:
	map_dispatcher();
	~map_dispatcher();
	bool is_at(const point& coordinate) const override;

	void request_reduce_width(unsigned int) override {};
	point calculate_best_size() const override { return {0,0}; };
	bool disable_click_dismiss() const override { return true; };
	iteration::walker_ptr create_walker() override
	{
		return nullptr;
	}

};

}

}
