/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Manage WML-variables.
 */
#include "variable_info.hpp"
#include "game_config.hpp"
#include "config_assign.hpp"

#include <stdexcept>

using namespace variable_info_detail;

/// general helpers
namespace
{
	/// TConfig is eigher 'config' or 'const config'
	template<typename TConfig>
	auto get_child_range(TConfig& cfg, const std::string& key, int start, int count) -> decltype(cfg.child_range(key))
	{
		auto res = cfg.child_range(key);
		return { res.begin() + start, res.begin() + start + count};
	}

	void resolve_negative_value(int size, int& val)
	{
		if(val < 0)
		{
			val = size + val;
		}
		//val is still < 0? We don't accept!
		if(val < 0)
		{
			throw invalid_variablename_exception();
		}
	}

	template<const variable_info_type vit>
	typename maybe_const<vit, config>::type& get_child_at(typename maybe_const<vit, config>::type& cfg, const std::string& key, int index = 0);

	template<>
	config& get_child_at<vit_create_if_not_existent>(config& cfg, const std::string& key, int index)
	{
		assert(index >= 0);
		// the 'create_if_not_existent' logic.
		while(static_cast<int>(cfg.child_count(key)) <= index)
		{
			cfg.add_child(key);
		}
		return cfg.child(key, index);
	}

	//helper variable for get_child_at<vit_const>
	const config empty_const_cfg;
	const config non_empty_const_cfg = config_of("_", config());

	template<>
	const config& get_child_at<vit_const>(const config& cfg, const std::string& key, int index)
	{
		assert(index >= 0);
		//cfg.child_or_empty does not support index parameter
		if(const config& child = cfg.child(key, index))
		{
			return child;
		}
		else
		{
			return empty_const_cfg;
		}
	}

	template<>
	config& get_child_at<vit_throw_if_not_existent>(config& cfg, const std::string& key, int index)
	{
		assert(index >= 0);
		if(config& child = cfg.child(key, index))
		{
			return child;
		}
		else
		{
			throw invalid_variablename_exception();
		}
	}

	template <typename TVal>
	config::attribute_value attribute_value_of(const TVal& val)
	{
		config::attribute_value v;
		v = val;
		return v;
	}

	/// @param visitor
	/// TVisitor should have 4 methods:
	///    from_named if the variable ended with a .somename
	///    from_indexed if the variable enden with .somename[someindex]
	///    from_temporary if the variable is a readonly value (.somename.length)
	///    from_start if the variablename was previously empty, this can only happen
	///      during calculate_value()
	/// TVisitor should derive from variable_info_visitor(_const) which makes default implementation for these (as not supported)
	template <typename TVisitor>
	typename TVisitor::result_type apply_visitor(const TVisitor& visitor, typename TVisitor::param_type state)
	{
		switch(state.type_)
		{
		case state_start:
			return visitor.from_start(state);
		case state_named:
			return visitor.from_named(state);
		case state_indexed:
			return visitor.from_indexed(state);
		case state_temporary:
			return visitor.from_temporary(state);
		}
		throw std::range_error("Failed to convert the TVisitor::param_type type");
	}

	template <const variable_info_type vit, typename TResult>
	class variable_info_visitor
	{
	public:
		typedef TResult result_type;
		typedef variable_info_state<vit>& param_type;
#define DEFAULTHANDLER(name) result_type name(param_type) const { throw invalid_variablename_exception(); }
		DEFAULTHANDLER(from_start)
		DEFAULTHANDLER(from_named)
		DEFAULTHANDLER(from_indexed)
		DEFAULTHANDLER(from_temporary)
#undef DEFAULTHANDLER
	};

	template <const variable_info_type vit, typename TResult>
	class variable_info_visitor_const
	{
	public:
		typedef TResult result_type;
		typedef const variable_info_state<vit>& param_type;
#define DEFAULTHANDLER(name) result_type name(param_type) const { throw invalid_variablename_exception(); }
		DEFAULTHANDLER(from_start)
		DEFAULTHANDLER(from_named)
		DEFAULTHANDLER(from_indexed)
		DEFAULTHANDLER(from_temporary)
#undef DEFAULTHANDLER
	};
}
/// calculate_value() helpers
namespace
{
	/// Parses a ']' terminated string.
	/// This is a important optimisation of lexical_cast_default
	int parse_index(const char* index_str)
	{
		char* endptr;
		int res = strtol(index_str, &endptr, 10);

		if (*endptr != ']' || res > int(game_config::max_loop) || endptr == index_str)
		{
			throw invalid_variablename_exception();
		}
		return res;
	}

	/// Adds a '.<key>' to the current variable
	template<const variable_info_type vit>
	class get_variable_key_visitor
		: public variable_info_visitor<vit, void>
	{
	public:
		get_variable_key_visitor(const std::string& key) : key_(key) {
			if (!config::valid_id(key_)) {
				throw invalid_variablename_exception();
			}
		}
		void from_named(typename get_variable_key_visitor::param_type state) const
		{
			if(key_ == "length")
			{
				state.temp_val_ = state.child_->child_count(state.key_);
				state.type_ = state_temporary;
				return;
			}
			else
			{
				return do_from_config(get_child_at<vit>(*state.child_, state.key_, 0), state);
			}
		}
		void from_start(typename get_variable_key_visitor::param_type state) const
		{
			return do_from_config(*state.child_, state);
		}
		void from_indexed(typename get_variable_key_visitor::param_type state) const
		{
			//we do not support aaa[0].length
			return do_from_config(get_child_at<vit>(*state.child_, state.key_, state.index_), state);
		}
	private:
		void do_from_config(typename maybe_const<vit, config>::type & cfg, typename get_variable_key_visitor::param_type state) const
		{
			state.type_ = state_named;
			state.key_ = key_;
			state.child_ = &cfg;
		}
		const std::string& key_;
	};

	/// appens a [index] to the variable.
	/// we only support from_named since [index][index2] or a.length[index] both doesn't make sense.
	template<const variable_info_type vit>
	class get_variable_index_visitor
		: public variable_info_visitor<vit, void>
	{
	public:
		get_variable_index_visitor(int n) : n_(n) {}
		void from_named(typename get_variable_index_visitor::param_type state) const
		{
			state.index_ = n_;
			resolve_negative_value(state.child_->child_count(state.key_), state.index_);
			state.type_ = state_indexed;
		}
	private:
		const int n_;
	};
}
/// as...  visitors
namespace
{
	///tries to convert it to an (maybe const) attribute value
	template<const variable_info_type vit>
	class as_skalar_visitor
		: public variable_info_visitor_const<vit, typename maybe_const<vit, config::attribute_value>::type&>
	{
	public:
		typename as_skalar_visitor::result_type from_named(typename as_skalar_visitor::param_type state) const
		{
			return (*state.child_)[state.key_];
		}
		///Defined below for different cases.
		typename as_skalar_visitor::result_type from_temporary(typename as_skalar_visitor::param_type state) const;
	};
	/// this type of value like '.length' are readonly values so we only support them for reading.
	/// espacily we don't want to return non const references.
	template<>
	const config::attribute_value & as_skalar_visitor<vit_const>::from_temporary(as_skalar_visitor::param_type state) const
	{
		return state.temp_val_;
	}
	template<>
	config::attribute_value & as_skalar_visitor<vit_create_if_not_existent>::from_temporary(as_skalar_visitor::param_type) const
	{
		throw invalid_variablename_exception();
	}
	template<>
	config::attribute_value & as_skalar_visitor<vit_throw_if_not_existent>::from_temporary(as_skalar_visitor::param_type) const
	{
		throw invalid_variablename_exception();
	}

	/// tries to convert to a (const) config&, unlike range based operation this also supports 'from_start'
	/// Note: Currently getting the 'from_start' case here is impossible, because we always apply at least one string key.
	template<const variable_info_type vit>
	class as_container_visitor
		: public variable_info_visitor_const<vit, typename maybe_const<vit, config>::type&>
	{
	public:
		typename as_container_visitor::result_type from_named(typename as_container_visitor::param_type state) const
		{
			return get_child_at<vit>(*state.child_, state.key_, 0);
		}
		typename as_container_visitor::result_type from_start(typename as_container_visitor::param_type state) const
		{
			return *state.child_;
		}
		typename as_container_visitor::result_type from_indexed(typename as_container_visitor::param_type state) const
		{
			return get_child_at<vit>(*state.child_, state.key_, state.index_);
		}
	};

	// This currently isn't implemented as a range based operation because doing it on something like range
	// 2-5 on vit_const if child_ has only 4 elements would be too hard to implement.
	template<const variable_info_type vit>
	class as_array_visitor
		: public variable_info_visitor_const<vit, typename maybe_const<vit, config::child_itors>::type>
	{
	public:
		typename as_array_visitor::result_type from_named(typename as_array_visitor::param_type state) const
		{
			return get_child_range(*state.child_, state.key_, 0, state.child_->child_count(state.key_));
		}
		typename as_array_visitor::result_type from_indexed(typename as_array_visitor::param_type state) const;
	};
	template<>
	config::const_child_itors as_array_visitor<vit_const>::from_indexed(as_array_visitor::param_type state) const
	{
		if (int(state.child_->child_count(state.key_)) <= state.index_)
		{
			return get_child_range(non_empty_const_cfg, "_", 0, 1);
		}
		else
		{
			return get_child_range(*state.child_, state.key_, state.index_, 1);
		}
	}
	template<>
	config::child_itors as_array_visitor<vit_create_if_not_existent>::from_indexed(as_array_visitor::param_type state) const
	{
		//Ensure we have a config at the given explicit position.
		get_child_at<vit_create_if_not_existent>(*state.child_, state.key_, state.index_);
		return get_child_range(*state.child_, state.key_, state.index_, 1);
	}
	template<>
	config::child_itors as_array_visitor<vit_throw_if_not_existent>::from_indexed(as_array_visitor::param_type state) const
	{
		//Ensure we have a config at the given explicit position.
		get_child_at<vit_throw_if_not_existent>(*state.child_, state.key_, state.index_);
		return get_child_range(*state.child_, state.key_, state.index_, 1);
	}
}
/// range_based operations
namespace {

	/// @tparam THandler a function
	///        (config& cfg, const std::string& name, int range_begin, int range_end) -> THandler::result_type
	///        that does the actual work on the range of children of cfg with name name.
	/// Note that currently this is only used by the insert/append/replace/merge operations
	/// so vit is always vit_create_if_not_existent
	template<const variable_info_type vit, typename THandler>
	class as_range_visitor_base
		: public variable_info_visitor_const<vit, typename THandler::result_type>
	{
	public:
		as_range_visitor_base(const THandler& handler) : handler_(handler) {}
		typename as_range_visitor_base::result_type from_named(typename as_range_visitor_base::param_type state) const
		{
			return handler_(*state.child_, state.key_, 0, state.child_->child_count(state.key_));
		}
		typename as_range_visitor_base::result_type from_indexed(typename as_range_visitor_base::param_type state) const
		{
			return this->handler_(*state.child_, state.key_, state.index_, state.index_ + 1);
		}

	protected:
		const THandler& handler_;
	};

	/**
		replaces the child in [startindex, endindex) with 'source'
		'insert' and 'append' are subcases of this.
	*/
	class replace_range_h
	{
	public:
		typedef config::child_itors result_type;
		replace_range_h(std::vector<config>& source) : datasource_(source) { }
		result_type operator()(config& child, const std::string& key, int startindex, int endindex) const
		{
			assert(endindex - startindex >= 0);
			if (endindex > 0)
			{
				//NOTE: currently this is nonly called from as_range_visitor_base<vit_create_if_not_existent>
				//based on that assumption we use get_child_at<vit_create_if_not_existent> here
				//instead of making this a template<typename vit> function/type
				get_child_at<vit_create_if_not_existent>(child, key, endindex - 1);
			}
			int size_diff = datasource_.size() - (endindex - startindex);
			//remove configs first
			while(size_diff < 0)
			{
				child.remove_child(key, startindex);
				++size_diff;
			}
			size_t index = 0;
			for(index = 0; index < static_cast<size_t>(size_diff); ++index)
			{
				child.add_child_at(key, config(), startindex + index).swap(datasource_[index]);
			}
			for(; index < datasource_.size(); ++index)
			{
				child.child(key, startindex + index).swap(datasource_[index]);
			}
			return get_child_range(child, key, startindex, datasource_.size());
		}
	private:
		std::vector<config>& datasource_;
	};

	class insert_range_h : replace_range_h
	{
	public:
		typedef config::child_itors result_type;
		insert_range_h(std::vector<config>& source) : replace_range_h(source) { }

		result_type operator()(config& child, const std::string& key, int startindex, int /*endindex*/) const
		{
			//insert == replace empty range with data.
			return replace_range_h::operator()(child, key, startindex, startindex);
		}
	};

	class append_range_h : insert_range_h
	{
	public:
		typedef config::child_itors result_type;
		append_range_h(std::vector<config>& source) : insert_range_h(source) { }
		result_type operator()(config& child, const std::string& key, int /*startindex*/, int /*endindex*/) const
		{
			//append == insert at end.
			int inser_pos = child.child_count(key);
			return insert_range_h::operator()(child, key, inser_pos, inser_pos /*ignored by insert_range_h*/);
		}
	};

	class merge_range_h
	{
	public:
		typedef void result_type;
		merge_range_h(std::vector<config>& source) : datasource_(source) { }
		void operator()(config& child, const std::string& key, int startindex, int /*endindex*/) const
		{
			//the merge_with function accepts only configs so we convert vector -> config.
			config datatemp;
			//Add emtpy config to 'shift' the merge to startindex
			for(int index = 0; index < startindex; ++index)
			{
				datatemp.add_child(key);
			}
			//move datasource_ -> datatemp
			for(size_t index = 0; index < datasource_.size(); ++index)
			{
				datatemp.add_child(key).swap(datasource_[index]);
			}
			child.merge_with(datatemp);
		}
	private:
		std::vector<config>& datasource_;
	};
}//annonymous namespace end based operations
/// misc
namespace
{
	template<const variable_info_type vit>
	class clear_value_visitor
		: public variable_info_visitor_const<vit, void>
	{
	public:
		clear_value_visitor(bool only_tables) : only_tables_(only_tables) {}
		void from_named(typename clear_value_visitor::param_type state) const
		{
			if(!only_tables_)
			{
				state.child_->remove_attribute(state.key_);
			}
			state.child_->clear_children(state.key_);
		}
		void from_indexed(typename clear_value_visitor::param_type state) const
		{
			state.child_->remove_child(state.key_, state.index_);
		}
	private:
		bool only_tables_;
	};

	template<const variable_info_type vit>
	class exists_as_container_visitor
		: public variable_info_visitor_const<vit, bool>
	{
	public:
		typename exists_as_container_visitor::result_type from_named(typename exists_as_container_visitor::param_type state) const
		{
			return state.child_->has_child(state.key_);
		}
		typename exists_as_container_visitor::result_type from_indexed(typename exists_as_container_visitor::param_type state) const
		{
			return state.child_->child_count(state.key_) > static_cast<size_t>(state.index_);
		}
		typename exists_as_container_visitor::result_type from_start(typename exists_as_container_visitor::param_type) const
		{
			return true;
		}
		typename exists_as_container_visitor::result_type from_temporary(typename exists_as_container_visitor::param_type) const
		{
			return false;
		}
	};
}

template<const variable_info_type vit>
variable_info<vit>::variable_info(const std::string& varname, config_var& vars)
	: name_(varname)
	, state_(vars)
	, valid_(true)
{
	try
	{
		this->calculate_value();
	}
	catch(const invalid_variablename_exception&)
	{
		this->valid_ = false;
	}
}

template<const variable_info_type vit>
variable_info<vit>::~variable_info()
{
}

template<const variable_info_type vit>
void variable_info<vit>::calculate_value()
{
	// this->state_ is initialized in the constructor.
	size_t previous_index = 0;
	size_t name_size = this->name_.size();
	for(size_t loop_index = 0; loop_index < name_size; loop_index++)
	{
		switch(this->name_[loop_index])
		{
		case '.':
		case '[':
			// '.', '[' mark the end of a string key.
			// the result is oviously that '.' and '[' are
			// treated equally so 'aaa.9].bbbb[zzz.uu.7]'
			// is interpreted as  'aaa[9].bbbb.zzz.uu[7]'
			// use is_valid_variable function for stricter variablename checking.
			apply_visitor(get_variable_key_visitor<vit>(this->name_.substr(previous_index, loop_index-previous_index)), this->state_);
			previous_index = loop_index + 1;
			break;
		case ']':
			// ']' marks the end of an integer key.
			apply_visitor(get_variable_index_visitor<vit>(parse_index(&this->name_[previous_index])), this->state_);
			//after ']' we always expect a '.' or the end of the string
			//ignore the next char which is a '.'
			loop_index++;
			if(loop_index < this->name_.length() && this->name_[loop_index] != '.')
			{
				throw invalid_variablename_exception();
			}
			previous_index = loop_index + 1;
			break;
		default:
			break;
		}
	}
	if(previous_index != this->name_.length() + 1)
	{
		// the string ended not with ']'
		// in this case we still didn't add the key behind the last '.'
		apply_visitor(get_variable_key_visitor<vit>(this->name_.substr(previous_index)), this->state_);
	}
}

template<const variable_info_type vit>
bool variable_info<vit>::explicit_index() const
{
	throw_on_invalid();
	return this->state_.type_ == state_start || this->state_.type_ == state_indexed;
}

template<const variable_info_type vit>
typename maybe_const<vit, config::attribute_value>::type& variable_info<vit>::as_scalar() const
{
	throw_on_invalid();
	return apply_visitor(as_skalar_visitor<vit>(), this->state_);
}

template<const variable_info_type vit>
typename maybe_const<vit, config>::type& variable_info<vit>::as_container() const
{
	throw_on_invalid();
	return apply_visitor(as_container_visitor<vit>(), this->state_);
}

template<const variable_info_type vit>
typename maybe_const<vit, config::child_itors>::type variable_info<vit>::as_array() const
{
	throw_on_invalid();
	return apply_visitor(as_array_visitor<vit>(), this->state_);
}

template<const variable_info_type vit>
void variable_info<vit>::throw_on_invalid() const
{
	if(!this->valid_)
	{
		throw invalid_variablename_exception();
	}
}

template<>
std::string variable_info<vit_const>::get_error_message() const
{
	return "Cannot resolve variablename '" + this->name_ + "' for reading.";
}

template<>
std::string variable_info<vit_create_if_not_existent>::get_error_message() const
{
	return "Cannot resolve variablename '" + this->name_ + "' for writing.";
}

template<>
std::string variable_info<vit_throw_if_not_existent>::get_error_message() const
{
	return "Cannot resolve variablename '" + this->name_ + "' for writing without creating new childs.";
}

template<const variable_info_type vit>
void non_const_variable_info<vit>::clear(bool only_tables) const
{
	this->throw_on_invalid();
	return apply_visitor(clear_value_visitor<vit>(only_tables), this->state_);
}

template<const variable_info_type vit>
config::child_itors non_const_variable_info<vit>::append_array(std::vector<config> childs) const
{
	this->throw_on_invalid();
	return apply_visitor(as_range_visitor_base<vit,append_range_h>(append_range_h(childs)), this->state_);
}

template<const variable_info_type vit>
config::child_itors non_const_variable_info<vit>::insert_array(std::vector<config> childs) const
{
	this->throw_on_invalid();
	return apply_visitor(as_range_visitor_base<vit,insert_range_h>(insert_range_h(childs)), this->state_);
}

template<const variable_info_type vit>
config::child_itors non_const_variable_info<vit>::replace_array(std::vector<config> childs) const
{
	this->throw_on_invalid();
	return apply_visitor(as_range_visitor_base<vit,replace_range_h>(replace_range_h(childs)), this->state_);
}

template<const variable_info_type vit>
void non_const_variable_info<vit>::merge_array(std::vector<config> childs) const
{
	this->throw_on_invalid();
	apply_visitor(as_range_visitor_base<vit,merge_range_h>(merge_range_h(childs)), this->state_);
}

template<const variable_info_type vit>
bool variable_info<vit>::exists_as_attribute() const
{
	this->throw_on_invalid();
	return (this->state_.type_ == state_temporary) || ((this->state_.type_ == state_named) && this->state_.child_->has_attribute(this->state_.key_));
}
template<const variable_info_type vit>
bool variable_info<vit>::exists_as_container() const
{
	this->throw_on_invalid();
	return apply_visitor(exists_as_container_visitor<vit>(), this->state_);
}

///explicit instantiations
template class variable_info<vit_const>;
template class variable_info<vit_create_if_not_existent>;
template class variable_info<vit_throw_if_not_existent>;
template class non_const_variable_info<vit_create_if_not_existent>;
template class non_const_variable_info<vit_throw_if_not_existent>;
