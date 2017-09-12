/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "utils/name_generator_factory.hpp"
#include "utils/name_generator.hpp"
#include "utils/context_free_grammar_generator.hpp"
#include "utils/markov_generator.hpp"
#include "formula/string_utils.hpp"

std::string name_generator::generate(const std::map<std::string,std::string>& variables) const {
	return utils::interpolate_variables_into_string(generate(), &variables);
}

name_generator_factory::name_generator_factory(const config& config, std::vector<std::string> ids) : name_generators_() {
	add_name_generator_from_config(config, "", "");

	for (std::vector<std::string>::iterator it = std::begin(ids); it!=std::end(ids); it++) {
		std::string id = *it;
		add_name_generator_from_config(config, id, (id + "_"));
	}
}

void name_generator_factory::add_name_generator_from_config(const config& config, const std::string id, const std::string prefix) {
 	std::string cfg_name 	= prefix + "name_generator";
	std::string markov_name = prefix + "names";

	if(config.has_attribute(cfg_name)) {
		try {
			name_generators_[id].reset(new context_free_grammar_generator(config[cfg_name]));
			return;
		}
		catch (const name_generator_invalid_exception& ex) {
			lg::wml_error() << ex.what() << '\n';
		}
	}

	if(config.has_attribute(markov_name)) {
		config::attribute_value markov_name_list = config[markov_name];

		if(!markov_name_list.blank()) {
			name_generators_[id].reset(new markov_generator(utils::split(markov_name_list), config["markov_chain_size"].to_int(2), 12));
		}
	}
}

std::shared_ptr<name_generator> name_generator_factory::get_name_generator() {
	std::map<std::string, std::shared_ptr<name_generator>>::const_iterator it = name_generators_.find("");
	if(it == name_generators_.end()) {
		//create a dummy instance, which always returns the empty string
		return std::make_shared<name_generator>();
	}

	return it->second;
}

std::shared_ptr<name_generator> name_generator_factory::get_name_generator(const std::string id) {
	std::map<std::string, std::shared_ptr<name_generator>>::const_iterator it = name_generators_.find(id);
	if(it == name_generators_.end()) {
		return get_name_generator();
	}

	return it->second;
}
