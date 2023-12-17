

#include "units/unit_status.hpp"

#include "config.hpp"
#include "game_config_view.hpp"

boost::container::flat_set<unit_status> unit_status::data_;

unit_status::unit_status(const config& cfg)
    : id_(cfg["id"].str())
	, icon_(cfg["icon"].str())
	, tooltip_(cfg["tooltip"].str())
	, persistent_(cfg["persistent"].to_bool())
{

}

void unit_status::read_statuses(const game_config_view& gc)
{
    data_.clear();
	for(const config& cs : gc.child_range("status_type")) {
        data_.emplace(cs);
	}
}

void unit_status::reset_statuses(std::set<std::string>& ss)
{
    for (const auto& s : data_) {
        if(s.persistent_) {
            ss.erase(s.id_);
        }
    }
}

void unit_status::heal_statuses(std::set<std::string>& ss)
{
    for (const auto& s : data_) {
        if(!s.persistent_) {
            ss.erase(s.id_);
        }
    }
}

void unit_status::report(config& res, const std::set<std::string>& ss)
{
    for (const auto& s : data_) {
        if(s.visible() && ss.count(s.id_)) {
            res.add_child("element", config {
                "image", s.icon_,
                "tooltip", s.title_ + s.tooltip_
            });
        }
    }
}
