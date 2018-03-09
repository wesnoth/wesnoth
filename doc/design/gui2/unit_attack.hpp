#ifndef GUI_DIALOGS_UNIT_ATTACK_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_ATTACK_HPP_INCLUDED

#include "actions.hpp"
#include "gui/dialogs/dialog.hpp"
#include "unit_map.hpp"

namespace gui2 {

class tunit_attack /*@ \label{unit_attack.hpp:class} @*/
	: public tdialog
{
public:
	tunit_attack( /*@ \label{unit_attack.hpp:constructor} @*/
			  const unit_map::iterator& attacker_itor
			, const unit_map::iterator& defender_itor
			, const std::vector<battle_context>& weapons
			, const int best_weapon);

	/***** ***** ***** setters / getters for members ***** ****** *****/ /*@ \label{unit_attack.hpp:settersgetters} @*/

	int get_selected_weapon() const { return selected_weapon_; }

private:

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */ /*@ \label{unit_attack.hpp:window_id} @*/
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */ /*@ \label{unit_attack.hpp:pre_show} @*/
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */ /*@ \label{unit_attack.hpp:post_show} @*/
	void post_show(twindow& window);
/*@ \label{unit_attack.hpp:members} @*/
	/** The index of the selected weapon. */
	int selected_weapon_;

	/** Iterator pointing to the attacker. */
	unit_map::iterator attacker_itor_;

	/** Iterator pointing to the defender. */
	unit_map::iterator defender_itor_;

	/** List of all battle contexts used for getting the weapons. */
	std::vector<battle_context> weapons_;

	/** The best weapon, aka the one high-lighted. */
	int best_weapon_;
};

} // namespace gui2

#endif
