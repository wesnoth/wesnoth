#include "player_network.hpp"

namespace wesnothd {

player_map::const_iterator find_user(const player_map& all_players,
									 const simple_wml::string_span& name)
{
	player_map::const_iterator pl;
	for (pl = all_players.begin(); pl != all_players.end(); pl++) {
		if (name == pl->second.name().c_str()) {
			return pl;
		}
	}
	return all_players.end();
}

void send_to_one(simple_wml::document& data, const network::connection sock, std::string packet_type)
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	simple_wml::string_span s = data.output_compressed();
	network::send_raw_data(s.begin(), s.size(), sock, packet_type);
}

void send_to_many(simple_wml::document& data, const connection_vector& vec,
				  const network::connection exclude, std::string packet_type)
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	simple_wml::string_span s = data.output_compressed();
	for(connection_vector::const_iterator i = vec.begin(); i != vec.end(); ++i) {
		if (*i != exclude) {
			network::send_raw_data(s.begin(), s.size(), *i, packet_type);
		}
	}
}

void send_to_many(simple_wml::document& data, const connection_vector& vec,
				  boost::function<bool (network::connection)> except_pred, std::string packet_type)
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	simple_wml::string_span s = data.output_compressed();
	for(connection_vector::const_iterator i = vec.begin(); i != vec.end(); ++i) {
		if (!except_pred(*i)) {
			network::send_raw_data(s.begin(), s.size(), *i, packet_type);
		}
	}

}

} //end namespace wesnothd
