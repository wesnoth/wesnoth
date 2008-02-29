#include "actions.hpp"
#include "callable_objects.hpp"
#include "formula.hpp"
#include "formula_ai.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"
#include "pathutils.hpp"

namespace {
using namespace game_logic;

class position_callable : public formula_callable {
	unit_map units_;
	int chance_;
	variant get_value(const std::string& key) const {
		if(key == "chance") {
			return variant(chance_);
		} else {
			return variant();
		}
	}

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("chance", game_logic::FORMULA_READ_ONLY));
	}
public:
	position_callable(unit_map* units, int chance) : chance_(chance)
	{
		units->swap(units_);
	}

	void swap_position(formula_ai& ai) {
		ai.get_info().units.swap(units_);
	}

	struct swapper {
		formula_ai& ai;
		unit_map& a;
		unit_map& b;
		formula_ai::move_map_backup backup;
		void swap() {
			a.swap(b);
			ai.swap_move_map(backup);
		}
		swapper(formula_ai& ai, position_callable& pos)
		  : ai(ai), a(ai.get_info().units), b(pos.units_) { 
		  swap();
		}

		~swapper() {
			swap();
		}
	};
};

class distance_between_function : public function_expression {
public:
	explicit distance_between_function(const args_list& args)
	  : function_expression("distance_between", args, 2, 2)
	{}

private:
	variant execute(const formula_callable& variables) const {
		const location_callable* loc1 = args()[0]->evaluate(variables).convert_to<location_callable>();
		const location_callable* loc2 = args()[1]->evaluate(variables).convert_to<location_callable>();
		return variant(distance_between(loc1->loc(), loc2->loc()));
	}
};

class distance_to_nearest_unowned_village_function : public function_expression {
public:
	distance_to_nearest_unowned_village_function(const args_list& args, const formula_ai& ai)
	  : function_expression("distance_to_nearest_unowned_village", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		const gamemap::location& loc = args()[0]->evaluate(variables).convert_to<location_callable>()->loc();
		int best = 1000000;
		const std::vector<gamemap::location>& villages = ai_.get_info().map.villages();
		const std::set<gamemap::location>& my_villages = ai_.current_team().villages();
		for(std::vector<gamemap::location>::const_iterator i = villages.begin(); i != villages.end(); ++i) {
			int distance = distance_between(loc, *i);
			if(distance < best) {
				if(my_villages.count(*i) == 0) {
					best = distance;
				}
			}
		}

		return variant(best);
	}
	
	const formula_ai& ai_;
};

class outcomes_function : public function_expression {
public:
	outcomes_function(const args_list& args, const formula_ai& ai)
	  : function_expression("outcomes", args, 1, 1), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		variant attack = args()[0]->evaluate(variables);
		ai::attack_analysis* analysis = attack.convert_to<ai::attack_analysis>();
		unit_map units_with_moves(ai_.get_info().units);
		for(int n = 0; n != analysis->movements.size(); ++n) {
			std::pair<gamemap::location,unit>* pair = units_with_moves.extract(analysis->movements[n].first);
			pair->first = analysis->movements[n].second;
			units_with_moves.add(pair);
		}
		
		std::vector<variant> vars;
		if(analysis->chance_to_kill > 0.0) {
			unit_map units(units_with_moves);
			units.erase(analysis->target);
			vars.push_back(variant(new position_callable(&units, analysis->chance_to_kill*100)));

		}

		if(analysis->chance_to_kill < 1.0) {
			unit_map units(units_with_moves);
			vars.push_back(variant(new position_callable(&units, 100 - analysis->chance_to_kill*100)));
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};

class evaluate_for_position_function : public function_expression {
public:
	evaluate_for_position_function(const args_list& args, formula_ai& ai)
	  : function_expression("evaluate_for_position", args, 2, 2), ai_(ai) {
	}

private:
	variant execute(const formula_callable& variables) const {
		variant position = args()[0]->evaluate(variables);
		position_callable* pos = position.convert_to<position_callable>();
		position_callable::swapper swapper(ai_, *pos);
		return args()[1]->evaluate(variables);
	}

	formula_ai& ai_;
};

class recruit_callable : public formula_callable {
	gamemap::location loc_;
	std::string type_;
	variant get_value(const std::string& key) const { return variant(); }
public:
	recruit_callable(const gamemap::location& loc, const std::string& type)
	  : loc_(loc), type_(type)
	{}

	const gamemap::location& loc() const { return loc_; }
	const std::string& type() const { return type_; }
};

class recruit_function : public function_expression {
public:
	explicit recruit_function(const args_list& args)
	  : function_expression("recruit", args, 1, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const std::string type = args()[0]->evaluate(variables).as_string();
		gamemap::location loc;
		if(args().size() >= 2) {
			loc = args()[1]->evaluate(variables).convert_to<location_callable>()->loc();
		}

		return variant(new recruit_callable(loc, type));
	}
};

class move_function : public function_expression {
public:
	explicit move_function(const args_list& args)
	  : function_expression("move", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap::location& src = args()[0]->evaluate(variables).convert_to<location_callable>()->loc();
		const gamemap::location& dst = args()[1]->evaluate(variables).convert_to<location_callable>()->loc();
		return variant(new move_callable(src, dst));
	}
};

class set_var_callable : public formula_callable {
	std::string key_;
	variant value_;
	variant get_value(const std::string& key) const { return variant(); }
public:
	set_var_callable(const std::string& key, const variant& value)
	  : key_(key), value_(value)
	{}

	const std::string& key() const { return key_; }
	variant value() const { return value_; }
};

class set_var_function : public function_expression {
public:
	explicit set_var_function(const args_list& args)
	  : function_expression("set_var", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(new set_var_callable(args()[0]->evaluate(variables).as_string(), args()[1]->evaluate(variables)));
	}
};

class fallback_callable : public formula_callable {
	std::string key_;
	variant get_value(const std::string& key) const { return variant(); }
public:
	explicit fallback_callable(const std::string& key) : key_(key) {
	}

	const std::string& key() const { return key_; }
};

class fallback_function : public function_expression {
public:
	explicit fallback_function(const args_list& args)
	  : function_expression("fallback", args, 1, 1)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(new fallback_callable(args()[0]->evaluate(variables).as_string()));
	}
};

class attack_callable : public formula_callable {
	gamemap::location move_from_, src_, dst_;
	battle_context bc_;
	variant get_value(const std::string& key) const {
		if(key == "attacker") {
			return variant(new location_callable(src_));
		} else if(key == "defender") {
			return variant(new location_callable(dst_));
		} else if(key == "move_from") {
			return variant(new location_callable(move_from_));
		} else {
			return variant();
		}
	}

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("attacker", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("defender", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("move_from", game_logic::FORMULA_READ_ONLY));
	}
public:
	attack_callable(const formula_ai& ai,
					const gamemap::location& move_from,
					const gamemap::location& src, const gamemap::location& dst,
	                int weapon)
	  : move_from_(move_from), src_(src), dst_(dst),
		bc_(ai.get_info().map, ai.get_info().teams, ai.get_info().units,
			ai.get_info().state, ai.get_info().gameinfo,
			src, dst, weapon, -1, 1.0, NULL, &ai.get_info().units.find(move_from)->second)
	{
	}

	const gamemap::location& move_from() const { return move_from_; }
	const gamemap::location& src() const { return src_; }
	const gamemap::location& dst() const { return dst_; }
	int weapon() const { return bc_.get_attacker_stats().attack_num; }
	int defender_weapon() const { return bc_.get_defender_stats().attack_num; }
};

class attack_function : public function_expression {
public:
	explicit attack_function(const args_list& args, const formula_ai& ai)
	  : function_expression("attack", args, 3, 4),
		ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap::location& move_from = args()[0]->evaluate(variables).convert_to<location_callable>()->loc();
		const gamemap::location& src = args()[1]->evaluate(variables).convert_to<location_callable>()->loc();
		const gamemap::location& dst = args()[2]->evaluate(variables).convert_to<location_callable>()->loc();
		const int weapon = args().size() == 4 ? args()[3]->evaluate(variables).as_int() : -1;
		if(ai_.get_info().units.count(move_from) == 0 || ai_.get_info().units.count(dst) == 0) {
			std::cerr << "AI ERROR: Formula produced illegal attack: " << move_from << " -> " << src << " -> " << dst << "\n";
			return variant();
		}
		return variant(new attack_callable(ai_, move_from, src, dst, weapon));
	}

	const formula_ai& ai_;
};

class loc_function : public function_expression {
public:
	explicit loc_function(const args_list& args)
	  : function_expression("loc", args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(new location_callable(gamemap::location(args()[0]->evaluate(variables).as_int()-1, args()[1]->evaluate(variables).as_int()-1)));
	}
};

class is_village_function : public function_expression {
public:
	explicit is_village_function(const args_list& args)
	  : function_expression("is_village", args, 2, 3)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap& m = args()[0]->evaluate(variables).convert_to<gamemap_callable>()->get_gamemap();

		gamemap::location loc;
		if(args().size() == 2) {
			loc = args()[1]->evaluate(variables).convert_to<location_callable>()->loc();
		} else {
			loc = gamemap::location(args()[1]->evaluate(variables).as_int(),
			                        args()[2]->evaluate(variables).as_int());
		}
		return variant(m.is_village(loc));
	}
};

class unit_at_function : public function_expression {
public:
	unit_at_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("unit_at", args, 1, 1), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const location_callable* loc = args()[0]->evaluate(variables).convert_to<location_callable>();
		const unit_map::const_iterator i = ai_.get_info().units.find(loc->loc());
		if(i != ai_.get_info().units.end()) {
			return variant(new unit_callable(*i, ai_.current_team(), ai_.get_info().team_num));
		} else {
			return variant();
		}
	}

	const formula_ai& ai_;
};

class unit_moves_function : public function_expression {
public:
	unit_moves_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("unit_moves", args, 1, 1), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant res = args()[0]->evaluate(variables);
		std::vector<variant> vars;
		if(res.is_null()) {
			return variant(&vars);
		}

		const gamemap::location& loc = res.convert_to<location_callable>()->loc();
		const formula_ai::move_map& srcdst = ai_.srcdst();
		typedef formula_ai::move_map::const_iterator Itor;
		std::pair<Itor,Itor> range = srcdst.equal_range(loc);

		for(Itor i = range.first; i != range.second; ++i) {
			vars.push_back(variant(new location_callable(i->second)));
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};

class units_can_reach_function : public function_expression {
public:
	units_can_reach_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("units_can_reach", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		std::vector<variant> vars;
		variant dstsrc_var = args()[0]->evaluate(variables);
		const ai::move_map& dstsrc = dstsrc_var.convert_to<move_map_callable>()->dstsrc();
		std::pair<ai::move_map::const_iterator,ai::move_map::const_iterator> range =
		    dstsrc.equal_range(args()[1]->evaluate(variables).convert_to<location_callable>()->loc());
		while(range.first != range.second) {
			unit_map::const_iterator un = ai_.get_info().units.find(range.first->second);
			assert(un != ai_.get_info().units.end());
			const int side = un->second.side();
			vars.push_back(variant(new unit_callable(*un, ai_.get_info().teams[side-1], side)));
			++range.first;
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};

class defense_on_function : public function_expression {
public:
	defense_on_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("defense_on", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant u = args()[0]->evaluate(variables);
		variant loc_var = args()[1]->evaluate(variables);
		if(u.is_null() || loc_var.is_null()) {
			return variant();
		}

		const unit& un = u.convert_to<unit_callable>()->get_unit();
		const gamemap::location& loc = loc_var.convert_to<location_callable>()->loc();
		if(!ai_.get_info().map.on_board(loc)) {
			return variant();
		}

		return variant(un.defense_modifier(ai_.get_info().map[loc]));
	}

	const formula_ai& ai_;
};

class max_possible_damage_function : public function_expression {
public:
	max_possible_damage_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression("max_possible_damage", args, 2, 2), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		variant u1 = args()[0]->evaluate(variables);
		variant u2 = args()[1]->evaluate(variables);
		if(u1.is_null() || u2.is_null()) {
			return variant();
		}

		const unit& attacker = u1.convert_to<unit_callable>()->get_unit();
		const unit& defender = u2.convert_to<unit_callable>()->get_unit();
		const std::vector<attack_type>& attacks = attacker.attacks();

		int best = 0;
		for(std::vector<attack_type>::const_iterator i = attacks.begin(); i != attacks.end(); ++i) {
			const int dmg = ((defender.damage_from(*i, false, gamemap::location()) * i->damage())/100) * i->num_attacks();
			if(dmg > best) {
				best = dmg;
			}
		}

		return variant(best);
	}

	const formula_ai& ai_;
};

class ai_function_symbol_table : public function_symbol_table {
	formula_ai& ai_;

	expression_ptr create_function(const std::string& fn,
	                               const std::vector<expression_ptr>& args) const {
		if(fn == "outcomes") {
			return expression_ptr(new outcomes_function(args, ai_));
		} else if(fn == "evaluate_for_position") {
			return expression_ptr(new evaluate_for_position_function(args, ai_));
		} else if(fn == "move") {
			return expression_ptr(new move_function(args));
		} else if(fn == "attack") {
			return expression_ptr(new attack_function(args, ai_));
		} else if(fn == "recruit") {
			return expression_ptr(new recruit_function(args));
		} else if(fn == "is_village") {
			return expression_ptr(new is_village_function(args));
		} else if(fn == "loc") {
			return expression_ptr(new loc_function(args));
		} else if(fn == "unit_at") {
			return expression_ptr(new unit_at_function(args, ai_));
		} else if(fn == "unit_moves") {
			return expression_ptr(new unit_moves_function(args, ai_));
		} else if(fn == "set_var") {
			return expression_ptr(new set_var_function(args));
		} else if(fn == "fallback") {
			return expression_ptr(new fallback_function(args));
		} else if(fn == "units_can_reach") {
			return expression_ptr(new units_can_reach_function(args, ai_));
		} else if(fn == "defense_on") {
			return expression_ptr(new defense_on_function(args, ai_));
		} else if(fn == "max_possible_damage") {
			return expression_ptr(new max_possible_damage_function(args, ai_));
		} else if(fn == "distance_to_nearest_unowned_village") {
			return expression_ptr(new distance_to_nearest_unowned_village_function(args, ai_));
		} else if(fn == "distance_between") {
			return expression_ptr(new distance_between_function(args));
		} else {
			return function_symbol_table::create_function(fn, args);
		}
	}

public:
	explicit ai_function_symbol_table(formula_ai& ai) : ai_(ai)
	{}
};
}

formula_ai::formula_ai(info& i) : ai(i), move_maps_valid_(false)
{
	//make sure we don't run out of refcount
	add_ref();
	vars_.add_ref();
}

void formula_ai::play_turn()
{
	ai_function_symbol_table function_table(*this);

	const config& ai_param = current_team().ai_parameters();
	config::const_child_itors functions = ai_param.child_range("function");
	for(config::const_child_iterator i = functions.first; i != functions.second; ++i) {
		const t_string& name = (**i)["name"];
		const t_string& inputs = (**i)["inputs"];
		const t_string& formula_str = (**i)["formula"];

		std::vector<std::string> args = utils::split(inputs);
		function_table.add_formula_function(name, game_logic::const_formula_ptr(new game_logic::formula(formula_str, &function_table)), game_logic::formula::create_optional_formula((**i)["precondition"], &function_table), args);
	}

	recruit_formula_ = game_logic::formula::create_optional_formula(current_team().ai_parameters()["recruit"], &function_table);
	move_formula_ = game_logic::formula::create_optional_formula(current_team().ai_parameters()["move"], &function_table);

	while(make_move()) {
	}
}

std::string formula_ai::evaluate(const std::string& formula_str)
{
	ai_function_symbol_table function_table(*this);
	const config& ai_param = current_team().ai_parameters();
	config::const_child_itors functions = ai_param.child_range("function");
	for(config::const_child_iterator i = functions.first; i != functions.second; ++i) {
		const t_string& name = (**i)["name"];
		const t_string& inputs = (**i)["inputs"];
		const t_string& formula_str = (**i)["formula"];
		std::vector<std::string> args = utils::split(inputs);
		function_table.add_formula_function(name, game_logic::const_formula_ptr(new game_logic::formula(formula_str, &function_table)), game_logic::formula::create_optional_formula((**i)["precondition"], &function_table), args);
	}

	game_logic::formula f(formula_str, &function_table);
	const variant v = f.execute(*this);
	return v.to_debug_string();
}

void formula_ai::swap_move_map(move_map_backup& backup)
{
	std::swap(move_maps_valid_, backup.move_maps_valid);
	std::swap(backup.attacks_cache, attacks_cache_);
	backup.move_maps_valid = move_maps_valid_;
	backup.srcdst.swap(srcdst_);
	backup.dstsrc.swap(dstsrc_);
	backup.full_srcdst.swap(full_srcdst_);
	backup.full_dstsrc.swap(full_dstsrc_);
	backup.enemy_srcdst.swap(enemy_srcdst_);
	backup.enemy_dstsrc.swap(enemy_dstsrc_);
}

void formula_ai::prepare_move() const
{
	if(move_maps_valid_) {
		return;
	}

	possible_moves_.clear();
	srcdst_.clear();
	dstsrc_.clear();

	calculate_possible_moves(possible_moves_, srcdst_, dstsrc_, false);

	full_srcdst_.clear();
	full_dstsrc_.clear();

	std::map<location,paths> possible_moves_dummy;
	calculate_possible_moves(possible_moves_dummy, full_srcdst_, full_dstsrc_, false, true);

	enemy_srcdst_.clear();
	enemy_dstsrc_.clear();
	possible_moves_dummy.clear();
	calculate_possible_moves(possible_moves_dummy, enemy_srcdst_, enemy_dstsrc_, true);

	attacks_cache_ = variant();
	move_maps_valid_ = true;
}

bool formula_ai::make_move()
{
	if(!move_formula_) {
		return false;
	}

	move_maps_valid_ = false;

	std::cerr << "do move...\n";
	const variant var = move_formula_->execute(*this);
	std::vector<variant> vars;
	if(var.is_list()) {
		for(int n = 0; n != var.num_elements(); ++n) {
			vars.push_back(var[n]);
		}
	} else {
		vars.push_back(var);
	}

	bool made_move = false;
	for(std::vector<variant>::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		if(i->is_null()) {
			continue;
		}
		
		const move_callable* move = i->try_convert<move_callable>();
		const attack_callable* attack = i->try_convert<attack_callable>();
		const recruit_callable* recruit_command = i->try_convert<recruit_callable>();
		const set_var_callable* set_var_command = i->try_convert<set_var_callable>();
		const fallback_callable* fallback_command = i->try_convert<fallback_callable>();

		prepare_move();
		if(move) {
			std::cerr << "moving " << move->src().x << "," << move->src().y << " -> " << move->dst().x << "," << move->dst().y << "\n";
			if(possible_moves_.count(move->src()) > 0) {
				move_unit(move->src(), move->dst(), possible_moves_);
				made_move = true;
			}
		} else if(attack) {
			if(get_info().units.count(attack->dst()) == 0) {
				//this is a legitimate situation; someone might send a series of units in
				//to attack, but if the defender dies in the middle, we'll save the unit
				//ordered to move so it can get a different command.
				continue;
			}

			if(attack->move_from() != attack->src()) {
				move_unit(attack->move_from(), attack->src(), possible_moves_);
			}
			std::cerr << "ATTACK: " << attack->src() << " -> " << attack->dst() << " " << attack->weapon() << "\n";
			attack_enemy(attack->src(), attack->dst(), attack->weapon(), attack->defender_weapon());
			made_move = true;
		} else if(recruit_command) {
			std::cerr << "RECRUIT: '" << recruit_command->type() << "'\n";
			if(recruit(recruit_command->type(), recruit_command->loc())) {
				made_move = true;
			}
		} else if(set_var_command) {
			std::cerr << "setting var: " << set_var_command->key() << " -> " << set_var_command->value().to_debug_string() << "\n";
			vars_.add(set_var_command->key(), set_var_command->value());
			made_move = true;
		} else if(i->is_string() && i->as_string() == "recruit") {
			do_recruitment();
			made_move = true;
		} else if(i->is_string() && i->as_string() == "end_turn") {
			return false;
		} else if(fallback_command) {
			ai_interface* fallback = create_ai(fallback_command->key(), get_info());
			if(fallback) {
				fallback->play_turn();
			}
			return false;
		} else {
			std::cerr << "UNRECOGNIZED MOVE\n";
		}
	}


	return made_move;
}

void formula_ai::do_recruitment()
{
	if(!recruit_formula_) {
		return;
	}

	variant var = recruit_formula_->execute(*this);
	std::vector<variant> vars;
	if(var.is_list()) {
		for(int n = 0; n != var.num_elements(); ++n) {
			vars.push_back(var[n]);
		}
	} else {
		vars.push_back(var);
	}

	for(std::vector<variant>::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		if(!i->is_string()) {
			return;
		}

		if(!recruit(i->as_string())) {
			return;
		}
	}

	do_recruitment();
}

variant formula_ai::get_value(const std::string& key) const
{
	if(key == "attacks") {
		prepare_move();
		if(attacks_cache_.is_null() == false) {
			return attacks_cache_;
		}

		std::vector<attack_analysis> attacks = const_cast<formula_ai*>(this)->analyze_targets(srcdst_, dstsrc_, enemy_srcdst_, enemy_dstsrc_);
		std::vector<variant> vars;
		for(std::vector<attack_analysis>::const_iterator i = attacks.begin(); i != attacks.end(); ++i) {
			vars.push_back(variant(new attack_analysis(*i)));
		}

		attacks_cache_ = variant(&vars);
		return attacks_cache_;
	} else if(key == "my_moves") {
		prepare_move();
		return variant(new move_map_callable(srcdst_, dstsrc_));
	} else if(key == "enemy_moves") {
		prepare_move();
		return variant(new move_map_callable(enemy_srcdst_, enemy_dstsrc_));
	} else if(key == "my_leader") {
		unit_map::const_iterator i = team_leader(get_info().team_num, get_info().units);
		if(i == get_info().units.end()) {
			return variant();
		}

		return variant(new location_callable(i->first));
	} else if(key == "vars") {
		return variant(&vars_);
	} else if(key == "keeps") {
		return get_keeps();
	}

	return ai_interface::get_value(key);
}

void formula_ai::get_inputs(std::vector<formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("attacks", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("enemy_moves", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("my_leader", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("vars", FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("keeps", FORMULA_READ_ONLY));

	ai_interface::get_inputs(inputs);
}

variant formula_ai::get_keeps() const
{
	if(keeps_cache_.is_null()) {
		std::vector<variant> vars;
		for(size_t x = 0; x != size_t(get_info().map.w()); ++x) {
			for(size_t y = 0; y != size_t(get_info().map.h()); ++y) {
				const gamemap::location loc(x,y);
				if(get_info().map.is_keep(loc)) {
					gamemap::location adj[6];
					get_adjacent_tiles(loc,adj);
					for(size_t n = 0; n != 6; ++n) {
						if(get_info().map.is_castle(adj[n])) {
							vars.push_back(variant(new location_callable(loc)));
							break;
						}
					}
				}
			}
		}
		keeps_cache_ = variant(&vars);
	}

	return keeps_cache_;
}
