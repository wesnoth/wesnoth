#define GETTEXT_DOMAIN "wesnoth-lib" /*@ \label{unit_attack.cpp:textdomain} @*/

#include "gui/dialogs/unit_attack.hpp"

#include "gui/widgets/image.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "unit.hpp"

namespace gui2 {

/*@ \label{unit_attack.cpp:wiki} @*/ /*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_unit_attack
 *
 * == Unit attack ==
 *
 * This shows the dialog for attacking units.
 *
 * @start_table = grid
 *
 *     attacker_portrait (image)       Shows the portrait of the attacking unit.
 *     attacker_icon (image)           Shows the icon of the attacking unit.
 *     attacker_name (control)         Shows the name of the attacking unit.
 *
 *
 *     defender_portrait (image)       Shows the portrait of the defending unit.
 *     defender_icon (image)           Shows the icon of the defending unit.
 *     defender_name (control)         Shows the name of the defending unit.
 *
 *
 *     (weapon_list) (listbox)         The list with weapons to choos from.
 *     -[attacker_weapon] (control)    The weapon for the attacker to use.
 *     -[defender_weapon] (control)    The weapon for the defender to use.
 *
 * @end_table
 */

REGISTER_WINDOW(unit_attack) /*@ \label{unit_attack.cpp:register} @*/

tunit_attack::tunit_attack(
		  const unit_map::iterator& attacker_itor
		, const unit_map::iterator& defender_itor
		, const std::vector<battle_context>& weapons
		, const int best_weapon)
	: selected_weapon_(-1)
	, attacker_itor_(attacker_itor)
	, defender_itor_(defender_itor)
	, weapons_(weapons)
	, best_weapon_(best_weapon)
{
}

template<class T>
static void set_label(
		  twindow& window
		, const std::string& id
		, const std::string& label)
{
	T* widget = find_widget<T>(&window, id, false, false);
	if(widget) {
		widget->set_label(label);
	}
}

static void set_attacker_info(twindow& w, unit& u)
{
	set_label<timage>(w, "attacker_portrait", u.absolute_image());
	set_label<timage>(w, "attacker_icon", u.absolute_image());
	set_label<tcontrol>(w, "attacker_name", u.name());
}

static void set_defender_info(twindow& w, unit& u)
{
	set_label<timage>(w, "defender_portrait", u.absolute_image());
	set_label<timage>(w, "defender_icon", u.absolute_image());
	set_label<tcontrol>(w, "defender_name", u.name());
}

static void set_weapon_info(twindow& window
		, const std::vector<battle_context>& weapons
		, const int best_weapon)
{
	tlistbox& weapon_list =
			find_widget<tlistbox>(&window, "weapon_list", false);
	window.keyboard_capture(&weapon_list);

	const config empty;
	attack_type no_weapon(empty);

	for (size_t i = 0; i < weapons.size(); ++i) {
		const battle_context::unit_stats& attacker =
				weapons[i].get_attacker_stats();

		const battle_context::unit_stats& defender =
				weapons[i].get_defender_stats();

		const attack_type& attacker_weapon = attack_type(*attacker.weapon);
		const attack_type& defender_weapon = attack_type(
				defender.weapon ? *defender.weapon : no_weapon);

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = attacker_weapon.name();
		data.insert(std::make_pair("attacker_weapon", item));

		item["label"] = defender_weapon.name();
		data.insert(std::make_pair("defender_weapon", item));

		weapon_list.add_row(data);

	}

	assert(best_weapon < static_cast<int>(weapon_list.get_item_count()));
	weapon_list.select_row(best_weapon);
}

void tunit_attack::pre_show(CVideo& /*video*/, twindow& window) /*@ \label{unit_attack.cpp:pre_show} @*/
{
	set_attacker_info(window, *attacker_itor_);
	set_defender_info(window, *defender_itor_);

	selected_weapon_ = -1;
	set_weapon_info(window, weapons_, best_weapon_);
}

void tunit_attack::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		selected_weapon_ = find_widget<tlistbox>(&window, "weapon_list", false)
				.get_selected_row();
	}
}

} // namespace gui2

