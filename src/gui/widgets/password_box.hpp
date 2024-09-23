/*
	Copyright (C) 2009 - 2024
	by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>, Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/text_box.hpp"


/**
 * A class inherited from text_box that displays
 * its input as stars
 *
 * @todo This implementation is quite a hack that
 * needs to be rewritten cleanly
 */
namespace gui2
{
namespace implementation
{
	struct builder_password_box;
}

// ------------ WIDGET -----------{

class password_box : public text_box
{
public:
	explicit password_box(const implementation::builder_password_box& builder);

	/** Inherited from text_box_base. */
	virtual void set_value(const std::string& text) override;
	std::string get_real_value() const
	{
		return real_value_;
	}


protected:
	void insert_char(const std::string& unicode) override;
	void paste_selection() override;
	void delete_selection() override;

	// We do not override copy_selection because we
	// actually want it to copy just the stars

private:

	std::string real_value_;

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/** See @ref styled_widget::get_control_type. */
	virtual const std::string& get_control_type() const override;
};

// }---------- BUILDER -----------{

namespace implementation
{

// copy & paste from builder_text_box...
// does it make more sense to inherit from it?
struct builder_password_box : public builder_styled_widget
{
public:
	explicit builder_password_box(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

private:
	std::string history_;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
