/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/composite/aspect.hpp
 */

#ifndef AI_COMPOSITE_ASPECT_HPP_INCLUDED
#define AI_COMPOSITE_ASPECT_HPP_INCLUDED

#include "../../global.hpp"

#include "engine.hpp"
#include "stage.hpp"

#include "../contexts.hpp"
#include "../default/contexts.hpp"
#include "../game_info.hpp"
#include "../manager.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"
#include "../../terrain_filter.hpp"

#include <map>
#include <stack>
#include <vector>
#include <deque>
#include <boost/lexical_cast.hpp>
#include <boost/pointer_cast.hpp>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

//----
// @todo: move to some other place
template<typename T>
class config_value_translator {
public:

	static void cfg_to_value(const config &cfg, T &value)
	{
		value = boost::lexical_cast<T>(cfg["value"]);
	}

	static void value_to_cfg(const T &value, config &cfg)
	{
		cfg["value"] = boost::lexical_cast<std::string>(value);
	}

	static config value_to_cfg(const T &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static T cfg_to_value(const config &cfg)
	{
		T value;
		cfg_to_value(cfg,value);
		return value;
	}
};


template<>
class config_value_translator<bool> {
public:

	static void cfg_to_value(const config &cfg, bool &value)
	{
		value = utils::string_bool(cfg["value"]);
	}

	static void value_to_cfg(const bool &value, config &cfg)
	{
		cfg["value"] = value ? "yes" : "no";
	}

	static config value_to_cfg(const bool &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static bool cfg_to_value(const config &cfg)
	{
		bool value;
		cfg_to_value(cfg,value);
		return value;
	}
};


template<>
class config_value_translator<int> {
public:

	static void cfg_to_value(const config &cfg, int &value)
	{
		value = atoi(cfg["value"].c_str());
	}

	static void value_to_cfg(const int &value, config &cfg)
	{
		cfg["value"] = boost::lexical_cast<std::string>(value);
	}

	static config value_to_cfg(const int &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static int cfg_to_value(const config &cfg)
	{
		int value;
		cfg_to_value(cfg,value);
		return value;
	}
};


template<>
class config_value_translator< std::vector<std::string> > {
public:

	static void cfg_to_value(const config &cfg, std::vector<std::string> &value)
	{
		value = utils::split(cfg["value"]);
	}

	static void value_to_cfg(const std::vector<std::string> &value, config &cfg)
	{
		std::stringstream buf;
		for(std::vector<std::string>::const_iterator p = value.begin(); p != value.end(); ++p) {
			if (p != value.begin()) {
				buf << ",";
			}
			buf << *p;
		}
		cfg["value"] = buf.str();
	}

	static config value_to_cfg(const std::vector<std::string> &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static std::vector<std::string> cfg_to_value(const config &cfg)
	{
		std::vector<std::string> value;
		cfg_to_value(cfg,value);
		return value;
	}
};

template<>
class config_value_translator<config> {
public:

	static void cfg_to_value(const config &cfg, config &value)
	{
		if (cfg.child("value")) {
			value = cfg.child("value");
		} else {
			value = config();
		}
	}

	static void value_to_cfg(const config &value, config &cfg)
	{
		cfg.add_child("value",value);
	}

	static config value_to_cfg(const config &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static config cfg_to_value(const config &cfg)
	{
		config value;
		cfg_to_value(cfg,value);
		return value;
	}
};

template<>
class config_value_translator<ministage> {
public:

	static void cfg_to_value(const config &cfg, ministage &value)
	{
		value = ministage(cfg);
	}

	static void value_to_cfg(const ministage &value, config &cfg)
	{
		cfg.add_child("value",value.to_config());
	}

	static config value_to_cfg(const ministage &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static ministage cfg_to_value(const config &cfg)
	{
		config c;
		ministage value(c);
		cfg_to_value(cfg,value);
		return value;
	}
};

template<>
class config_value_translator<terrain_filter> {
public:

	static void cfg_to_value(const config &cfg, terrain_filter &value)
	{
		if (cfg.child("value")) {
			value = terrain_filter(vconfig(cfg.child("value")),manager::get_ai_info().units);
		} else {
			static config c;
			if (!c.child("not")) {
				c.add_child("not");
			}
			value = terrain_filter(vconfig(c),manager::get_ai_info().units);
		}
	}

	static void value_to_cfg(const terrain_filter &value, config &cfg)
	{
		cfg.add_child("value",value.to_config());
	}

	static config value_to_cfg(const terrain_filter &value)
	{
		config cfg;
		value_to_cfg(value,cfg);
		return cfg;
	}

	static terrain_filter cfg_to_value(const config &cfg)
	{
		if (cfg.child("value")) {
			return terrain_filter(vconfig(cfg.child("value")),manager::get_ai_info().units);
		}
		static config c;
		if (!c.child("not")) {
			c.add_child("not");
		}
		return terrain_filter(vconfig(c),manager::get_ai_info().units);
	}
};


// variant value translator

template<typename T>
class variant_value_translator {
public:

	static void variant_to_value(const variant &/*var*/, T &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const T &/*value*/, variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static variant value_to_variant(const T &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static T variant_to_value(const variant &var)
	{
		T value;
		variant_to_value(var,value);
		return value;
	}
};

template<>
class variant_value_translator<ministage> {
public:

	static void variant_to_value(const variant &/*var*/, ministage &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const ministage &/*value*/, variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static variant value_to_variant(const ministage &/*value*/)
	{
		assert(false);
		return variant();
	}

	static ministage variant_to_value(const variant &/*var*/)
	{
		assert(false);
		config cfg;
		return ministage(cfg);
	}
};

template<>
class variant_value_translator<int> {
public:

	static void variant_to_value(const variant &var, int &value)
	{
	        value = var.as_int();
	}

	static void value_to_variant(const int &value, variant &var)
	{
		var = variant(value);
	}

	static variant value_to_variant(const int &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static int variant_to_value(const variant &var)
	{
		int value;
		variant_to_value(var,value);
		return value;
	}
};


template<>
class variant_value_translator<bool> {
public:

	static void variant_to_value(const variant &var, bool &value)
	{
	        value = var.as_bool();
	}

	static void value_to_variant(const bool &value, variant &var)
	{
		var = variant(value);
	}

	static variant value_to_variant(const bool &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static bool variant_to_value(const variant &var)
	{
		bool value;
		variant_to_value(var,value);
		return value;
	}
};



template<>
class variant_value_translator<std::string> {
public:

	static void variant_to_value(const variant &var, std::string &value)
	{
	        value = var.as_string();
	}

	static void value_to_variant(const std::string &value, variant &var)
	{
		var = variant(value);
	}

	static variant value_to_variant(const std::string &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static std::string variant_to_value(const variant &var)
	{
		std::string value;
		variant_to_value(var,value);
		return value;
	}
};



template<>
class variant_value_translator<attacks_vector> {
public:

	static void variant_to_value(const variant &/*var*/, attacks_vector &/*value*/)
	{ 
		assert(false);//not implemented
	}

	static void value_to_variant(const attacks_vector &value, variant &var)
	{
                std::vector<variant> vars;
                for(attacks_vector::const_iterator i = value.begin(); i != value.end(); ++i) {
                        vars.push_back(variant(new attack_analysis(*i)));
                }
		var = variant(&vars);
	}

	static variant value_to_variant(const attacks_vector &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static attacks_vector variant_to_value(const variant &var)
	{
		attacks_vector value;
		variant_to_value(var,value);
		return value;
	}
};


template<>
class variant_value_translator<terrain_filter> {
public:

	static void variant_to_value(const variant &/*var*/, terrain_filter &/*value*/)
	{
	        assert(false);//not implemented
	}

	static void value_to_variant(const terrain_filter &/*value*/, variant &/*var*/)
	{
		assert(false);//not implemented
	}

	static variant value_to_variant(const terrain_filter &value)
	{
		variant var;
		value_to_variant(value,var);
		return var;
	}

	static terrain_filter variant_to_value(const variant &var)
	{
		static config c;
		if (!c.child("not")) {
			c.add_child("not");
		}

		terrain_filter value(vconfig(c),manager::get_ai_info().units);
		variant_to_value(var,value);
		return value;
	}
};


//----

class aspect : public readonly_context_proxy, public events::observer {
public:
	aspect(readonly_context &context, const config &cfg, const std::string &id);

	virtual ~aspect();

	void invalidate() const
	{
		valid_ = false;
		valid_variant_ = false;
	}


	virtual const variant& get_variant() const = 0;


	virtual boost::shared_ptr<variant> get_variant_ptr() const = 0;


	virtual void recalculate() const = 0;


	virtual void on_create();


	virtual config to_config() const;


	void handle_generic_event(const std::string &/*event_name*/)
	{
		invalidate();
	}


	virtual bool active() const
	{
		return true;
	}


	const std::string& get_name() const;


	const std::string& get_id() const;

	static lg::log_domain& log();
protected:
	mutable bool valid_;
	mutable bool valid_variant_;

	config cfg_;
	bool invalidate_on_turn_start_;
	bool invalidate_on_tod_change_;
	bool invalidate_on_gamestate_change_;
	bool invalidate_on_minor_gamestate_change_;
	std::string engine_;
	std::string name_;
	std::string id_;

};

template<typename T>
class typesafe_aspect : public aspect {
public:
	typesafe_aspect(readonly_context &context, const config &cfg, const std::string &id)
		: aspect(context,cfg,id)
	{
	}

	virtual ~typesafe_aspect()
	{
	}


	virtual const T& get() const
	{
		return *get_ptr();
	}


	virtual const variant& get_variant() const
	{
		return *get_variant_ptr();
	}

	virtual boost::shared_ptr<variant> get_variant_ptr() const
	{
		if (!valid_variant_) {
			if (!valid_) {
				recalculate();
			}

			if (!valid_variant_ && valid_ ) {
				value_variant_ = boost::shared_ptr<variant>(new variant(variant_value_translator<T>::value_to_variant(this->get())));
				valid_variant_ = true;
			} else {
				assert(valid_variant_);
			}
		}
		return value_variant_;
	}

	virtual void recalculate() const = 0;
	

	virtual boost::shared_ptr<T> get_ptr() const
	{
		if (!valid_) {
			if (!valid_variant_) {
				recalculate();
			}

			if (!valid_ && valid_variant_) {
				value_ = boost::shared_ptr<T>(new T(variant_value_translator<T>::variant_to_value(get_variant())));
				valid_ = true;
			} else {
				assert(valid_);
			}
		}
		return value_;
	}

protected:
	mutable boost::shared_ptr<T> value_;
	mutable boost::shared_ptr<variant> value_variant_;
};


class known_aspect {
public:
	known_aspect(const std::string &name);

	virtual ~known_aspect();

	virtual void set(aspect_ptr a) = 0;

	const std::string& get_name() const;

protected:
	const std::string name_;
};


template<typename T>
class typesafe_known_aspect : public known_aspect {
public:
	typesafe_known_aspect(const std::string &name, boost::shared_ptr< typesafe_aspect<T> > &where, aspect_map &aspects)
	: known_aspect(name), where_(where), aspects_(aspects)
	{
	}

	void set(aspect_ptr a)
	{
		boost::shared_ptr< typesafe_aspect <T> > c = boost::dynamic_pointer_cast< typesafe_aspect<T> >(a);
		if (c) {
			assert (c->get_id()== this->get_name());
			where_ = c;
			aspects_.insert(make_pair(this->get_name(),c));
		} else {
			LOG_STREAM(debug, aspect::log()) << "typesafe_known_aspect [" << this->get_name() << "] : while setting aspect, got null. this might be caused by invalid [aspect] WML" << std::endl;
		}
	}

protected:
	boost::shared_ptr<typesafe_aspect <T> > &where_;
	aspect_map &aspects_;

};


template<typename T>
class composite_aspect : public typesafe_aspect<T> {
public:

	composite_aspect(readonly_context &context, const config &cfg, const std::string &id)
		: typesafe_aspect<T>(context, cfg, id)
	{
		foreach (const config &cfg_element, this->cfg_.child_range("facet") ){
			std::vector< aspect_ptr > facets;
			engine::parse_aspect_from_config(*this,cfg_element,this->get_id(),std::back_inserter(facets));
			foreach (aspect_ptr a, facets ){
				boost::shared_ptr< typesafe_aspect<T> > b = boost::dynamic_pointer_cast< typesafe_aspect<T> > (a);
				facets_.push_back(b);
			}
		}

		const config &_default = this->cfg_.child("default");
		if (_default) {
			std::vector< aspect_ptr > default_aspects;
			engine::parse_aspect_from_config(*this,_default,this->get_id(),std::back_inserter(default_aspects));
			if (!default_aspects.empty()) {
				boost::shared_ptr< typesafe_aspect<T> > b = boost::dynamic_pointer_cast< typesafe_aspect<T> >(default_aspects.front());
				default_ = b;
			}
		}
	}

	virtual void recalculate() const
	{
		//@todo 1.7.4 optimize in case of an aspect which returns variant
		foreach (const boost::shared_ptr< typesafe_aspect<T> > f, facets_) {
			if (f->active()) {
				this->value_ = boost::shared_ptr<T>(f->get_ptr());
				this->valid_ = true;
				return;
			}
		}
		this->value_ = boost::shared_ptr<T>(default_->get_ptr());
		this->valid_ = true;
	}


	virtual config to_config() const
	{
		config cfg = aspect::to_config();
		foreach (const boost::shared_ptr< typesafe_aspect<T> > f, facets_) {
			cfg.add_child("facet",f->to_config());
		}
		if (default_) {
			cfg.add_child("default",default_->to_config());
		}
		return cfg;
	}

protected:
	std::vector< boost::shared_ptr< typesafe_aspect<T> > > facets_;
	boost::shared_ptr< typesafe_aspect<T> > default_;

};

template<typename T>
class standard_aspect : public typesafe_aspect<T> {
public:
	standard_aspect(readonly_context &context, const config &cfg, const std::string &id)
		: typesafe_aspect<T>(context, cfg, id), time_of_day_(cfg["time_of_day"]),turns_(cfg["turns"])
	{
		boost::shared_ptr<T> value(new T(config_value_translator<T>::cfg_to_value(this->cfg_)));
		this->value_= value;
		LOG_STREAM(debug, aspect::log()) << "standard aspect has time_of_day=["<<time_of_day_<<"], turns=["<<turns_<<"], and value: "<< std::endl << config_value_translator<T>::value_to_cfg(this->get()) << std::endl;
	}


	virtual bool active() const
	{
		if(time_of_day_.empty() == false) {
			const std::vector<std::string>& times = utils::split(time_of_day_);
			if(std::count(times.begin(),times.end(),this->get_info().tod_manager_.get_time_of_day().name) == 0) {
				return false;
			}
		}

		if(turns_.empty() == false) {
			int turn = this->get_info().tod_manager_.turn();
			const std::vector<std::string>& turns_list = utils::split(turns_);
			for(std::vector<std::string>::const_iterator j = turns_list.begin(); j != turns_list.end() ; j++ ) {
				const std::pair<int,int> range = utils::parse_range(*j);
				if(turn >= range.first && turn <= range.second) {
				      return true;
				}
			}
			return false;
		}
		return false;
	}


	void recalculate() const
	{
		//nothing to recalculate
		this->valid_ = true;
	}


	config to_config() const
	{
		config cfg = aspect::to_config();
		config_value_translator<T>::value_to_cfg(this->get(),cfg);
		cfg["time_of_day"] = time_of_day_;
		cfg["turns"] = turns_;
		return cfg;
	}

protected:
	std::string time_of_day_;
	std::string turns_;

};


class aspect_factory{
public:
	typedef boost::shared_ptr< aspect_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *aspect_factories;
		if (aspect_factories==NULL) {
			aspect_factories = new factory_map;
		}
		return *aspect_factories;
	}

	virtual aspect_ptr get_new_instance( readonly_context &context, const config &cfg, const std::string &id) = 0;

	aspect_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
	}

	virtual ~aspect_factory() {}
};


template<class ASPECT>
class register_aspect_factory : public aspect_factory {
public:
	register_aspect_factory( const std::string &name )
		: aspect_factory( name )
	{
	}

	virtual aspect_ptr get_new_instance( readonly_context &context, const config &cfg, const std::string &id){
		boost::shared_ptr<ASPECT> _a(new ASPECT(context,cfg,id));
		aspect_ptr a = _a;
		a->on_create();
		return a;
	}
};


} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
