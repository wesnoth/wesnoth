#include "suppose_dead.hpp"

#include "visitor.hpp"
#include "manager.hpp"
#include "side_actions.hpp"
#include "utility.hpp"

#include "arrow.hpp"
#include "config.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "mouse_events.hpp"
#include "play_controller.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_display.hpp"
#include "unit_map.hpp"

namespace wb
{

	std::ostream& operator<<(std::ostream &s, suppose_dead_ptr sup_d)
	{
		assert(sup_d);
		return sup_d->print(s);
	}

	std::ostream& operator<<(std::ostream &s, suppose_dead_const_ptr sup_d)
	{
		assert(sup_d);
		return sup_d->print(s);
	}

	std::ostream& suppose_dead::print(std::ostream &s) const
	{
		s << "Suppose-dead for unit " << get_unit()->name() << " [" << get_unit()->id() << "] "
				<< "at (" << loc_ << ")";
		return s;
	}

	suppose_dead::suppose_dead(size_t team_index, unit& curr_unit)
	: action(team_index)
	, unit_(&curr_unit)
	, unit_id_(curr_unit.id())
	, loc_(curr_unit.get_location())
	, valid_(true)
	{
		assert(unit_);
	}

	suppose_dead::~suppose_dead()
	{
	}

	void suppose_dead::accept(visitor& v)
	{
		v.visit_suppose_dead(shared_from_this());
	}

	bool suppose_dead::execute()
	{
		valid_=false;
		return false;
	}

	void suppose_dead::apply_temp_modifier(unit_map& unit_map)
	{
		// Remove the unit
		unit const* removed_unit = unit_map.extract(get_source_hex());
		DBG_WB << "Suppose dead: Temporarily removing unit " << removed_unit->name() << " [" << removed_unit->id()
				<< "] from (" << get_source_hex() << ")\n";

		// Just check to make sure we removed the unit we expected to remove
		assert(get_unit() == removed_unit);
	}

	void suppose_dead::remove_temp_modifier(unit_map& unit_map)
	{
		// Just check to make sure the hex is empty
		unit_map::iterator unit_it = resources::units->find(get_source_hex());
		assert(unit_it == resources::units->end());

		// Restore the unit
		unit_map.insert(unit_);
	}

	void suppose_dead::draw_hex(const map_location& hex)
	{
		if(hex == loc_) //add symbol to hex
		{
			//@todo: Possibly use a different layer
			const display::tdrawing_layer layer = display::LAYER_ARROWS;

			int xpos = resources::screen->get_location_x(loc_);
			int ypos = resources::screen->get_location_y(loc_);

			resources::screen->drawing_buffer_add(layer, loc_, xpos, ypos,
					image::get_image("whiteboard/suppose_dead.png", image::SCALED_TO_HEX));
		}
	}

	bool suppose_dead::is_numbering_hex(const map_location& hex) const
	{
		return hex == get_source_hex();
	}

	void suppose_dead::set_valid(bool valid)
	{
		valid_ = valid;
	}

} // end namespace wb
