#include "global.hpp"

#include "cavegen.hpp"
#include "log.hpp"
#include "map_create.hpp"
#include "mapgen_dialog.hpp"
#include "scoped_resource.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>

#define ERR_CF lg::err(lg::config)

map_generator* create_map_generator(const std::string& name, const config* cfg)
{
	if(name == "default" || name == "") {
		return new default_map_generator(cfg);
	} else if(name == "cave") {
		return new cave_map_generator(cfg);
	} else {
		return NULL;
	}
}

//function to generate a random map, from a string which describes
//the generator to use and its arguments
std::string random_generate_map(const std::string& parms, const config* cfg)
{
	//the first token is the name of the generator, tokens after
	//that are arguments to the generator
	std::vector<std::string> parameters = utils::split(parms, ' ');
	util::scoped_ptr<map_generator> generator(create_map_generator(parameters.front(),cfg));
	if(generator == NULL) {
		ERR_CF << "could not find map generator '" << parameters.front() << "'\n";
		return std::string();
	}

	parameters.erase(parameters.begin());
	return generator.get()->create_map(parameters);
}

config random_generate_scenario(const std::string& parms, const config* cfg)
{
	//the first token is the name of the generator, tokens after
	//that are arguments to the generator
	std::vector<std::string> parameters = utils::split(parms, ' ');
	util::scoped_ptr<map_generator> generator(create_map_generator(parameters.front(),cfg));
	if(generator == NULL) {
		ERR_CF << "could not find map generator '" << parameters.front() << "'\n";
		return config();
	}

	parameters.erase(parameters.begin());
	return generator->create_scenario(parameters);
}
