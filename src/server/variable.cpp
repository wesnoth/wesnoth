#include "../game_events.hpp"

//this is an 'identity' implementation of variables, that just returns the name
//of the variable itself. To be used in systems for which variables shouldn't be implemented
namespace {
std::map<std::string,std::string> variables;

}

namespace game_events {
const std::string& get_variable_const(const std::string& str)
{
	const std::map<std::string,std::string>::const_iterator itor = variables.find(str);
	if(itor != variables.end()) {
		return itor->second;
	} else {
		variables[str] = "$" + str;
		return get_variable_const(str);
	}
}
}

