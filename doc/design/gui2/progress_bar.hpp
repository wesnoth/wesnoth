#ifndef GUI_WIDGETS_PROGRESS_BAR_HPP_INCLUDED
#define GUI_WIDGETS_PROGRESS_BAR_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2 {

class tprogress_bar /*@ \label{progress_bar.hpp:class} @*/
	: public tcontrol
{
public:

	tprogress_bar()  /*@ \label{progress_bar.hpp:constructor} @*/
		: tcontrol(COUNT)
		, percentage_(-1)
	{
		// Force canvas update
		set_percentage(0);
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/ /*@ \label{progress_bar.hpp:inherited} @*/

	/** Inherited from tcontrol. */
	void set_active(const bool /*active*/) {}

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return ENABLED; }

	/** Inherited from tcontrol. */
	bool disable_click_dismiss() const { return false; }


	/***** ***** ***** setters / getters for members ***** ****** *****/ /*@ \label{progress_bar.hpp:settersgetters} @*/

	void set_percentage(const unsigned percentage);
	unsigned get_percentage() const { return percentage_; }

private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, COUNT }; /*@ \label{progress_bar.hpp:state} @*/

	/** The percentage done. */
	unsigned percentage_;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;
};

} // namespace gui2

#endif

