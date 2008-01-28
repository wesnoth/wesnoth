#include "actions.hpp"
#include "callable_objects.hpp"
#include "formula.hpp"
#include "formula_ai.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"

namespace {
using namespace game_logic;

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
	  : function_expression(args, 1, 2)
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

class move_callable : public formula_callable {
	gamemap::location src_, dst_;
	variant get_value(const std::string& key) const { return variant(); }
public:
	move_callable(const gamemap::location& src, const gamemap::location& dst) :
	  src_(src), dst_(dst)
	{}

	const gamemap::location& src() const { return src_; }
	const gamemap::location& dst() const { return dst_; }
};

class move_function : public function_expression {
public:
	explicit move_function(const args_list& args)
	  : function_expression(args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap::location& src = args()[0]->evaluate(variables).convert_to<location_callable>()->loc();
		const gamemap::location& dst = args()[1]->evaluate(variables).convert_to<location_callable>()->loc();
		return variant(new move_callable(src, dst));
	}
};

class attack_callable : public formula_callable {
	gamemap::location move_from_, src_, dst_;
	int weapon_;
	battle_context bc_;
	variant get_value(const std::string& key) const {
		if(key == "attacker") {
			return variant(new location_callable(src_));
		} else if(key == "defender") {
			return variant(new location_callable(dst_));
		} else {
			return variant();
		}
	}
public:
	attack_callable(const formula_ai& ai,
					const gamemap::location& move_from,
					const gamemap::location& src, const gamemap::location& dst,
	                int weapon)
	  : move_from_(move_from), src_(src), dst_(dst), weapon_(weapon),
		bc_(ai.get_info().map, ai.get_info().teams, ai.get_info().units,
			ai.get_info().state, ai.get_info().gameinfo,
			src, dst, weapon, -1, 1.0, NULL, &ai.get_info().units.find(move_from)->second)
	{}

	const gamemap::location& move_from() const { return move_from_; }
	const gamemap::location& src() const { return src_; }
	const gamemap::location& dst() const { return dst_; }
	int weapon() const { return weapon_; }
	int defender_weapon() const { return bc_.get_defender_stats().attack_num; }
};

class attack_function : public function_expression {
public:
	explicit attack_function(const args_list& args, const formula_ai& ai)
	  : function_expression(args, 3, 4),
		ai_(ai)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap::location& move_from = args().front()->evaluate(variables).convert_to<location_callable>()->loc();
		const gamemap::location& src = args()[args().size()-3]->evaluate(variables).convert_to<location_callable>()->loc();
		const gamemap::location& dst = args()[args().size()-2]->evaluate(variables).convert_to<location_callable>()->loc();
		const int weapon = args()[args().size()-1]->evaluate(variables).as_int();

		if(ai_.get_info().units.count(move_from) == 0 || ai_.get_info().units.count(dst) == 0) {
			std::cerr << "AI ERROR: Formula produced illegal attack!\n";
			return variant();
		}
		return variant(new attack_callable(ai_, move_from, src, dst, weapon));
	}

	const formula_ai& ai_;
};

class loc_function : public function_expression {
public:
	explicit loc_function(const args_list& args)
	  : function_expression(args, 2, 2)
	{}
private:
	variant execute(const formula_callable& variables) const {
		return variant(new location_callable(gamemap::location(args()[0]->evaluate(variables).as_int()-1, args()[1]->evaluate(variables).as_int()-1)));
	}
};

class is_village_function : public function_expression {
public:
	explicit is_village_function(const args_list& args)
	  : function_expression(args, 3, 3)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap& m = args()[0]->evaluate(variables).convert_to<gamemap_callable>()->get_gamemap();
		return variant(m.is_village(gamemap::location(args()[1]->evaluate(variables).as_int(), args()[2]->evaluate(variables).as_int())));
	}
};

class unit_at_function : public function_expression {
public:
	unit_at_function(const args_list& args, const formula_ai& ai_object)
	  : function_expression(args, 1, 1), ai_(ai_object)
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
	  : function_expression(args, 1, 1), ai_(ai_object)
	{}
private:
	variant execute(const formula_callable& variables) const {
		const gamemap::location& loc = args()[0]->evaluate(variables).convert_to<location_callable>()->loc();
		const formula_ai::move_map& srcdst = ai_.srcdst();
		typedef formula_ai::move_map::const_iterator Itor;
		std::pair<Itor,Itor> range = srcdst.equal_range(loc);

		std::vector<variant> vars;
		for(Itor i = range.first; i != range.second; ++i) {
			vars.push_back(variant(new location_callable(i->second)));
		}

		return variant(&vars);
	}

	const formula_ai& ai_;
};

class ai_function_symbol_table : public function_symbol_table {
	const formula_ai& ai_;

	expression_ptr create_function(const std::string& fn,
	                               const std::vector<expression_ptr>& args) const {
		if(fn == "move") {
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
		} else {
			return function_symbol_table::create_function(fn, args);
		}
	}

public:
	explicit ai_function_symbol_table(const formula_ai& ai) : ai_(ai)
	{}
};
}

formula_ai::formula_ai(info& i) : ai(i)
{
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
		function_table.add_formula_function(name, game_logic::const_formula_ptr(new game_logic::formula(formula_str, &function_table)), args);
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
		function_table.add_formula_function(name, game_logic::const_formula_ptr(new game_logic::formula(formula_str, &function_table)), args);
	}

	game_logic::formula f(formula_str, &function_table);
	const variant v = f.execute(*this);
	return v.to_debug_string();
}

namespace {
void debug_console(const game_logic::formula_callable& info, const formula_ai& ai) {
		std::cerr << "starting debug console. Type formula to evaluate. Type 'continue' when you're ready to continue\n";
		std::cerr << variant(&info).to_debug_string() << "\n";
		ai_function_symbol_table function_table(ai);
	const config& ai_param = ai.current_team().ai_parameters();
	config::const_child_itors functions = ai_param.child_range("function");
	for(config::const_child_iterator i = functions.first; i != functions.second; ++i) {
		const t_string& name = (**i)["name"];
		const t_string& inputs = (**i)["inputs"];
		const t_string& formula_str = (**i)["formula"];

		std::vector<std::string> args = utils::split(inputs);
		function_table.add_formula_function(name, game_logic::const_formula_ptr(new game_logic::formula(formula_str, &function_table)), args);
	}

		for(;;) {
			std::cerr << "\n>>> ";
			char buf[1024];
			std::cin.getline(buf, sizeof(buf));
			std::string cmd(buf);
			if(cmd == "continue") {
				break;
			}

			try {
				formula f(cmd, &function_table);
				const variant v = f.execute(info);
				std::cerr << v.to_debug_string() << "\n";
			} catch(formula_error& e) {
				std::cerr << "ERROR IN FORMULA\n";
			}
		}
}
}

void formula_ai::prepare_move()
{
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
	calculate_possible_moves(possible_moves_dummy, full_srcdst_, full_dstsrc_, true);

	attacks_cache_ = variant();
}

bool formula_ai::make_move()
{
	if(!move_formula_) {
		return false;
	}

	prepare_move();

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
		const move_callable* move = i->try_convert<move_callable>();
		const attack_callable* attack = i->try_convert<attack_callable>();
		const recruit_callable* recruit_command = i->try_convert<recruit_callable>();
		if(move) {
			std::cerr << "moving " << move->src().x << "," << move->src().y << " -> " << move->dst().x << "," << move->dst().y << "\n";
			if(possible_moves_.count(move->src()) > 0) {
				move_unit(move->src(), move->dst(), possible_moves_); made_move = true;
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
			attack_enemy(attack->src(), attack->dst(), attack->weapon(), attack->defender_weapon());
			made_move = true;
		} else if(recruit_command) {
			std::cerr << "RECRUIT: '" << recruit_command->type() << "'\n";
			if(recruit(recruit_command->type(), recruit_command->loc())) {
				made_move = true;
			}
		} else if(i->is_string() && i->as_string() == "recruit") {
			do_recruitment();
			made_move = true;
		} else if(i->is_string() && i->as_string() == "end_turn") {
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
		return variant(new move_map_callable(srcdst_, dstsrc_));
	} else if(key == "enemy_moves") {
		return variant(new move_map_callable(enemy_srcdst_, enemy_dstsrc_));
	}

	return ai_interface::get_value(key);
}
