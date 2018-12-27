/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 */

#pragma once

#include "ai/composite/property_handler.hpp"
#include "ai/composite/value_translator.hpp"
#include "ai/lua/lua_object.hpp"
#include "ai/lua/core.hpp"
#include "scripting/game_lua_kernel.hpp"

#include "log.hpp"

#include "utils/functional.hpp"

#include <boost/range/adaptor/reversed.hpp>

namespace ai {

class aspect : public readonly_context_proxy, public events::observer, public component {
public:
	aspect(readonly_context &context, const config &cfg, const std::string &id);

	virtual ~aspect();

	void invalidate() const
	{
		valid_ = false;
		valid_variant_ = false;
		valid_lua_ = false;
	}


	virtual const wfl::variant& get_variant() const = 0;


	virtual std::shared_ptr<wfl::variant> get_variant_ptr() const = 0;


	virtual void recalculate() const = 0;


	virtual void on_create();


	virtual bool redeploy(const config &cfg, const std::string & id);


	virtual config to_config() const;


	virtual bool delete_all_facets();


	void handle_generic_event(const std::string &/*event_name*/)
	{
		invalidate();
	}


	virtual bool active() const;

	virtual std::string get_name() const
	{ return name_; }

	virtual std::string get_id() const
	{ return id_; }

	virtual std::string get_engine() const
	{ return engine_; }

	static lg::log_domain& log();

protected:
	std::string time_of_day_;
	std::string turns_;

	mutable bool valid_;
	mutable bool valid_variant_;
	mutable bool valid_lua_;

	config cfg_;
	bool invalidate_on_turn_start_;
	bool invalidate_on_tod_change_;
	bool invalidate_on_gamestate_change_;
	std::string engine_;
	std::string name_;
	std::string id_;

};

template<typename T>
class typesafe_aspect : public aspect {
public:
	typesafe_aspect(readonly_context &context, const config &cfg, const std::string &id)
		: aspect(context,cfg,id)
		, value_()
		, value_variant_()
		, value_lua_()
	{
	}

	virtual ~typesafe_aspect()
	{
	}


	virtual const T& get() const
	{
		return *get_ptr();
	}


	virtual const wfl::variant& get_variant() const
	{
		return *get_variant_ptr();
	}

	virtual std::shared_ptr<wfl::variant> get_variant_ptr() const
	{
		if (!valid_variant_) {
			if (!valid_) {
				recalculate();
			}

			if (!valid_variant_ && valid_ ) {
				value_variant_.reset(new wfl::variant(variant_value_translator<T>::value_to_variant(this->get())));
				valid_variant_ = true;
			} else if (!valid_variant_ && valid_lua_) {
				value_ = value_lua_->get();
				value_variant_.reset(new wfl::variant(variant_value_translator<T>::value_to_variant(this->get())));
				valid_variant_ = true; // @note: temporary workaround
			} else {
				assert(valid_variant_);
			}
		}
		return value_variant_;
	}

	virtual void recalculate() const = 0;


	virtual std::shared_ptr<T> get_ptr() const
	{
		if (!valid_) {
			if (!(valid_variant_ || valid_lua_)) {
				recalculate();
			}

			if (!valid_ ) {
				if (valid_variant_) {
					value_.reset(new T(variant_value_translator<T>::variant_to_value(get_variant())));
					valid_ = true;
				} else if (valid_lua_){
					value_ = value_lua_->get();
					valid_ = true;
				} else {
					assert(valid_);
				}
			}
		}
		return value_;
	}

protected:
	mutable std::shared_ptr<T> value_;
	mutable std::shared_ptr<wfl::variant> value_variant_;
	mutable std::shared_ptr< lua_object<T>> value_lua_;
};


class known_aspect {
public:
	known_aspect(const std::string &name);


	virtual ~known_aspect();


	virtual void set(aspect_ptr a) = 0;


	virtual void add_facet(const config &cfg) = 0;


	const std::string& get_name() const;

protected:
	const std::string name_;
};


template<class T>
class composite_aspect;

template<typename T>
class typesafe_known_aspect : public known_aspect {
public:
	typesafe_known_aspect(const std::string &name, typesafe_aspect_ptr<T>& where, aspect_map &aspects)
	: known_aspect(name), where_(where), aspects_(aspects)
	{
	}

	void set(aspect_ptr a)
	{
		typesafe_aspect_ptr<T> c = std::dynamic_pointer_cast<typesafe_aspect<T>>(a);
		if (c) {
			assert (c->get_id()== this->get_name());
			where_ = c;
			aspects_.emplace(this->get_name(),c);
		} else {
			LOG_STREAM(debug, aspect::log()) << "typesafe_known_aspect [" << this->get_name() << "] : while setting aspect, got null. this might be caused by invalid [aspect] WML" << std::endl;
		}
	}

	virtual void add_facet(const config &cfg)
	{
		std::shared_ptr< composite_aspect <T>> c = std::dynamic_pointer_cast< composite_aspect<T>>(where_);
		if (c) {
			assert (c->get_id()==this->get_name());
			c->add_facet(-1, cfg);
			c->invalidate();
		} else {
			LOG_STREAM(debug, aspect::log()) << "typesafe_known_aspect [" << this->get_name() << "] : while adding facet to aspect, got null. this might be caused by target [aspect] being not composite" << std::endl;
		}
	}

protected:
	typesafe_aspect_ptr<T>& where_;
	aspect_map &aspects_;
};


template<typename T>
class composite_aspect : public typesafe_aspect<T> {
public:

	composite_aspect(readonly_context &context, const config &cfg, const std::string &id)
		: typesafe_aspect<T>(context, cfg, id)
		, facets_()
		, default_()
		, parent_id_(id)
	{
		for (const config &cfg_element : this->cfg_.child_range("facet")) {
			add_facet(-1,cfg_element);
		}

		config _default = this->cfg_.child("default");
		if (_default) {
			_default["id"] = "default_facet";
			std::vector< aspect_ptr > default_aspects;
			engine::parse_aspect_from_config(*this,_default,parent_id_,std::back_inserter(default_aspects));
			if (!default_aspects.empty()) {
				typesafe_aspect_ptr<T> b = std::dynamic_pointer_cast< typesafe_aspect<T>>(default_aspects.front());
				if (composite_aspect<T>* c = dynamic_cast<composite_aspect<T>*>(b.get())) {
					c->parent_id_ = parent_id_;
				}
				default_ = b;
			}
		}

		std::function<void(typesafe_aspect_vector<T>&, const config&)> factory_facets =
                        std::bind(&ai::composite_aspect<T>::create_facet,*this,_1,_2);

		register_facets_property(this->property_handlers(),"facet",facets_,default_, factory_facets);

	}


	void create_facet(typesafe_aspect_vector<T>& facets, const config &cfg)
	{
		std::vector<aspect_ptr> facets_base;
		engine::parse_aspect_from_config(*this,cfg,parent_id_,std::back_inserter(facets_base));
		for (aspect_ptr a : facets_base) {
			typesafe_aspect_ptr<T> b = std::dynamic_pointer_cast< typesafe_aspect<T>> (a);
			if (composite_aspect<T>* c = dynamic_cast<composite_aspect<T>*>(b.get())) {
				c->parent_id_ = parent_id_;
			}
			facets.push_back(b);
		}
	}


	virtual void recalculate() const
	{
		for(const auto& f : boost::adaptors::reverse(facets_)) {
			if (f->active()) {
				this->value_ = f->get_ptr();
				this->valid_ = true;
				return;
			}
		}
		if (default_) {
			this->value_ = default_->get_ptr();
			this->valid_ = true;
		}
	}


	virtual config to_config() const
	{
		config cfg = aspect::to_config();
		for (const typesafe_aspect_ptr<T> f : facets_) {
			cfg.add_child("facet",f->to_config());
		}
		if (default_) {
			cfg.add_child("default",default_->to_config());
		}
		return cfg;
	}


	using typesafe_aspect<T>::add_facet;
	virtual bool add_facet(int pos, const config &cfg)
	{
		if (pos<0) {
			pos = facets_.size();
		}
		std::vector< aspect_ptr > facets;
		engine::parse_aspect_from_config(*this,cfg,parent_id_,std::back_inserter(facets));
		int j=0;
		for (aspect_ptr a : facets) {
			typesafe_aspect_ptr<T> b = std::dynamic_pointer_cast< typesafe_aspect<T>> (a);
			if (composite_aspect<T>* c = dynamic_cast<composite_aspect<T>*>(b.get())) {
				c->parent_id_ = parent_id_;
			}
			facets_.insert(facets_.begin()+pos+j,b);
			j++;
		}
		return (j>0);
	}


	virtual bool delete_all_facets()
	{
		bool b = !facets_.empty();
		facets_.clear();
		return b;
	}

	aspect& find_active()
	{
		for(aspect_ptr a : facets_) {
			if(a->active()) {
				return *a;
			}
		}
		return *default_;
	}

protected:
	typesafe_aspect_vector<T> facets_;
	typesafe_aspect_ptr<T> default_;
	std::string parent_id_;

};

template<typename T>
class standard_aspect : public typesafe_aspect<T> {
public:
	standard_aspect(readonly_context &context, const config &cfg, const std::string &id)
		: typesafe_aspect<T>(context, cfg, id)
	{
		this->name_ = "standard_aspect";
		this->value_ = std::make_shared<T>(config_value_translator<T>::cfg_to_value(this->cfg_));
		LOG_STREAM(debug, aspect::log()) << "standard aspect has value: "<< std::endl << config_value_translator<T>::value_to_cfg(this->get()) << std::endl;
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
		return cfg;
	}

};

class lua_aspect_visitor : public boost::static_visitor<std::string> {
	static std::string quote_string(const std::string& s);
public:
	std::string operator()(bool b) const {return utils::bool_string(b);}
	std::string operator()(int i) const {return std::to_string(i);}
	std::string operator()(unsigned long long i) const {return std::to_string(i);}
	std::string operator()(double i) const {return std::to_string(i);}
	std::string operator()(const std::string& s) const {return quote_string(s);}
	std::string operator()(const t_string& s) const {return quote_string(s.str());}
	std::string operator()(boost::blank) const {return "nil";}
};

template<typename T>
class lua_aspect : public typesafe_aspect<T>
{
public:
	lua_aspect(readonly_context &context, const config &cfg, const std::string &id, std::shared_ptr<lua_ai_context>& l_ctx)
		: typesafe_aspect<T>(context, cfg, id)
		, handler_(), code_(), params_(cfg.child_or_empty("args"))
	{
		this->name_ = "lua_aspect";
		if (cfg.has_attribute("code"))
		{
			code_ = cfg["code"].str();
		}
		else if (cfg.has_attribute("value"))
		{
			code_ = "return " + cfg["value"].apply_visitor(lua_aspect_visitor());
		}
		else
		{
			// error
			return;
		}
		handler_.reset(resources::lua_kernel->create_lua_ai_action_handler(code_.c_str(), *l_ctx));
	}

	void recalculate() const
	{
		this->valid_lua_ = true;
		this->value_lua_.reset(new lua_object<T>);
		handler_->handle(params_, true, this->value_lua_);
	}

	config to_config() const
	{
		config cfg = aspect::to_config();
		cfg["code"] = code_;
		if (!params_.empty()) {
			cfg.add_child("args", params_);
		}
		return cfg;
	}

private:
	std::shared_ptr<lua_ai_action_handler> handler_;
	std::string code_;
	const config params_;
};


class aspect_factory{
	bool is_duplicate(const std::string &name);
public:
	typedef std::shared_ptr< aspect_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *aspect_factories;
		if (aspect_factories==nullptr) {
			aspect_factories = new factory_map;
		}
		return *aspect_factories;
	}

	virtual aspect_ptr get_new_instance( readonly_context &context, const config &cfg, const std::string &id) = 0;

	aspect_factory( const std::string &name )
	{
		if (is_duplicate(name)) {
			return;
		}
		factory_ptr ptr_to_this(this);
		get_list().emplace(name,ptr_to_this);
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

	aspect_ptr get_new_instance( readonly_context &context, const config &cfg, const std::string &id)
	{
		aspect_ptr a = std::make_shared<ASPECT>(context, cfg, id);
		a->on_create();
		return a;
	}
};

class lua_aspect_factory{
public:
	typedef std::shared_ptr< lua_aspect_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *aspect_factories;
		if (aspect_factories==nullptr) {
			aspect_factories = new factory_map;
		}
		return *aspect_factories;
	}

	virtual aspect_ptr get_new_instance( readonly_context &context, const config &cfg, const std::string &id, std::shared_ptr<lua_ai_context>& l_ctx) = 0;

	lua_aspect_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().emplace(name,ptr_to_this);
	}

	virtual ~lua_aspect_factory() {}
};

template<class ASPECT>
class register_lua_aspect_factory : public lua_aspect_factory {
public:
	register_lua_aspect_factory( const std::string &name )
		: lua_aspect_factory( name )
	{
	}

	aspect_ptr get_new_instance( readonly_context &context, const config &cfg, const std::string &id, std::shared_ptr<lua_ai_context>& l_ctx)
	{
		aspect_ptr a = std::make_shared<ASPECT>(context, cfg, id, l_ctx);
		a->on_create();
		return a;
	}
};


} //end of namespace ai
