#include "statistics.hpp"

namespace {

typedef statistics::stats stats;

struct scenario_stats
{
	scenario_stats(const std::string& name) : scenario_name(name)
	{}

	std::vector<stats> team_stats;
	std::string scenario_name;
};

std::vector<scenario_stats> master_stats;

stats& get_stats(int team)
{
	if(master_stats.empty()) {
		master_stats.push_back(scenario_stats(""));
	}

	std::vector<stats>& team_stats = master_stats.back().team_stats;
	const size_t index = size_t(team-1);
	if(index >= team_stats.size()) {
		team_stats.resize(index+1);
	}

	return team_stats[index];
}

void merge_str_int_map(stats::str_int_map& a, const stats::str_int_map& b)
{
	for(stats::str_int_map::const_iterator i = b.begin(); i != b.end(); ++i) {
		a[i->first] += i->second;
	}
}

void merge_battle_result_maps(stats::battle_result_map& a, const stats::battle_result_map& b)
{
	for(stats::battle_result_map::const_iterator i = b.begin(); i != b.end(); ++i) {
		merge_str_int_map(a[i->first],i->second);
	}
}

void merge_stats(stats& a, const stats& b)
{
	merge_str_int_map(a.recruits,b.recruits);
	merge_str_int_map(a.recalls,b.recalls);
	merge_str_int_map(a.advanced_to,b.advanced_to);
	merge_str_int_map(a.deaths,b.deaths);
	merge_str_int_map(a.killed,b.killed);

	merge_battle_result_maps(a.attacks,b.attacks);
	merge_battle_result_maps(a.defends,b.defends);

	a.recruit_cost += b.recruit_cost;
	a.recall_cost += b.recall_cost;
	a.damage_inflicted += b.damage_inflicted;
	a.damage_taken += b.damage_taken;
}

}

namespace statistics
{

stats::stats() : recruit_cost(0), recall_cost(0), damage_inflicted(0), damage_taken(0)
{}

scenario_context::scenario_context(const std::string& name)
{
	master_stats.push_back(scenario_stats(name));
}

scenario_context::~scenario_context()
{
}

attack_context::attack_context(const unit& a, const unit& d, const battle_stats& stats)
   : attacker_type(a.type().name()), defender_type(d.type().name()),
     bat_stats(stats), attacker_side(a.side()), defender_side(d.side())
{
}

attack_context::~attack_context()
{
	attacker_stats().attacks[bat_stats.chance_to_hit_defender][attacker_res]++;
	defender_stats().defends[bat_stats.chance_to_hit_attacker][defender_res]++;
}

stats& attack_context::attacker_stats()
{
	return get_stats(attacker_side);
}

stats& attack_context::defender_stats()
{
	return get_stats(defender_side);
}

void attack_context::attack_result(attack_context::ATTACK_RESULT res)
{
	attacker_res.resize(attacker_res.size()+1);
	attacker_res[attacker_res.size()-1] = (res == MISSES ? '0' : '1');

	attacker_stats().damage_inflicted += bat_stats.damage_defender_takes;
	defender_stats().damage_taken += bat_stats.damage_defender_takes;

	if(res == KILLS) {
		attacker_stats().killed[defender_type]++;
		defender_stats().deaths[attacker_type]++;
	}
}

void attack_context::defend_result(attack_context::ATTACK_RESULT res)
{
	defender_res.resize(defender_res.size()+1);
	defender_res[defender_res.size()-1] = (res == MISSES ? '0' : '1');

	attacker_stats().damage_taken += bat_stats.damage_attacker_takes;
	defender_stats().damage_inflicted += bat_stats.damage_defender_takes;

	if(res == KILLS) {
		attacker_stats().deaths[defender_type]++;
		defender_stats().killed[attacker_type]++;
	}
}

void recruit_unit(const unit& u)
{
	stats& s = get_stats(u.side());
	s.recruits[u.type().name()]++;
	s.recruit_cost += u.type().cost();
}

void recall_unit(const unit& u)
{
	stats& s = get_stats(u.side());
	s.recalls[u.type().name()]++;
	s.recall_cost += u.type().cost();
}

void advance_unit(const unit& u)
{
	stats& s = get_stats(u.side());
	s.advanced_to[u.type().name()]++;
}

std::vector<std::string> get_categories()
{
	std::vector<std::string> res;
	res.push_back("all_statistics");
	for(std::vector<scenario_stats>::const_iterator i = master_stats.begin(); i != master_stats.end(); ++i) {
		res.push_back(i->scenario_name);
	}

	return res;
}

stats calculate_stats(int category, int side)
{
	if(category == 0) {
		stats res;
		for(int i = 1; i <= int(master_stats.size()); ++i) {
			merge_stats(res,calculate_stats(i,side));
		}

		return res;
	} else {
		const size_t index = master_stats.size() - size_t(side);
		const size_t side_index = size_t(side) - 1;
		if(index < master_stats.size() && side_index < master_stats[index].team_stats.size()) {
			return master_stats[index].team_stats[side_index];
		} else {
			return stats();
		}
	}
}

}