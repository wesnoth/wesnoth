/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_UNIT_PREVIEW_PANE_HPP_INCLUDED
#define GUI_WIDGETS_UNIT_PREVIEW_PANE_HPP_INCLUDED

#include "gui/widgets/container.hpp"

#include <string>

class attack_type;
class unit;
class unit_type;

namespace gui2
{

// ------------ WIDGET -----------{

class tbutton;
class timage;
class tlabel;

namespace implementation
{
	struct tbuilder_unit_preview_pane;
}

class tunit_preview_pane : public tcontainer_
{
	friend struct implementation::tbuilder_unit_preview_pane;

public:
	tunit_preview_pane()
		: tcontainer_(1)
		, current_type_()
		, icon_type_(nullptr)
		, icon_race_(nullptr)
		, icon_alignment_(nullptr)
		, label_name_(nullptr)
		, label_level_(nullptr)
		, label_details_(nullptr)
		, label_details_minimal_(nullptr)
		, button_profile_(nullptr)
		, image_mods_()
	{
	}

	/** Displays the stats of a specified unit type */
	void set_displayed_type(const unit_type* type);

	/** Displays the stats of a specific unit */
	void set_displayed_unit(const unit* unit);

	/** Sets the facing of the unit image */
	void set_image_mods(const std::string& mods);

	/** Callback for the profile button */
	void profile_button_callback();

	/** See @ref tcontrol::set_active. */
	virtual void set_active(const bool active) override;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const override;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const override;

protected:
	/**
	 * Initializes the interneral sub-widget pointers.
	 * Should be called when building the window, so the pointers
	 * are initilized when set_displayed_type() is called.
	 */
	void finalize_setup();

private:
	std::string current_type_;

	timage* icon_type_;
	timage* icon_race_;
	timage* icon_alignment_;

	tlabel* label_name_;
	tlabel* label_level_;

	tcontrol* label_details_;
	tcontrol* label_details_minimal_;

	tbutton* button_profile_;

	std::string image_mods_;

	void print_attack_details(const std::vector<attack_type>& attacks, std::stringstream& str);

	enum tstate {
		ENABLED
	};

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const override;

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) override;

};

// }---------- DEFINITION ---------{

struct tunit_preview_pane_definition : public tcontrol_definition
{

	explicit tunit_preview_pane_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		tbuilder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_unit_preview_pane : public tbuilder_control
{
public:
	explicit tbuilder_unit_preview_pane(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
	const std::string image_mods_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
