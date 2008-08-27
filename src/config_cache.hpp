/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONFIG_CACHE_HPP_INCLUDED
#define CONFIG_CACHE_HPP_INCLUDED

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "config.hpp"
#include "serialization/preprocessor.hpp"

namespace game_config {


	/**
	 * Singleton object to manage game configs
	 * and cache reading.
	 * @TODO: Make smarter filetree checksum caching so only required parts
	 * 		  of tree are checked at startup. Trees are overlapping so have
	 * 		  to split trees to subtrees to only do check once per file.
	 **/
	class config_cache : private boost::noncopyable {
		static config_cache cache_;

		bool force_valid_cache_, use_cache_;
		preproc_map defines_map_;

		void read_configs(config&, std::string&);

		void read_file(const std::string& file, config& cfg);
		void write_file(std::string file, const config& cfg);

		void read_cache(const std::string& path, config& cfg);

		void read_configs(const std::string& path, config& cfg);

		// Protected to let test code access
		protected:
		config_cache();


		const preproc_map& get_preproc_map() const;
		void load_configs(const std::string& path, config& cfg);

		public:
		/**
		 * Get reference to the singleton object
		 **/
		static config_cache& instance();

		/**
		 * get config object from given path
		 * @param path which to load. Should be _main.cfg.
		 * @param config object that is writen to, It should be empty
		 * 	      because there is no quarentee how filled in config is handled
		 **/
		void get_config(const std::string& path, config& cfg);
		/**
		 * get config_ptr from given path
		 * @return shread_ptr config object
		 * @param config object that is writen to
		 **/
		config_ptr get_config(const std::string& path);

		/**
		 * Clear stored defines map to default values
		 **/
		void clear_defines();
		/**
		 * Add a entry to preproc defines map
		 **/
		void add_define(const std::string& define);
		/**
		 * Remove a entry to preproc defines map
		 **/
		void remove_define(const std::string& define);

		/**
		 * Enable/disable caching
		 **/
		void set_use_cache(bool use);
		/**
		 * Enable/disable cache validation
		 **/
		void set_force_valid_cache(bool force);
		/**
		 * Force cache checksum validation.
		 **/
		void recheck_filetree_checksum();
	};

	/**
	 * Used to set and unset scoped defines to preproc_map
	 * This is prefered form to set defines that aren't global
	 **/
	template <class T>
		class scoped_preproc_define_internal : private boost::noncopyable {
			// Protected to let test code access
			protected:
				std::string name_;
			public:
				scoped_preproc_define_internal(const std::string& name) : name_(name)
			{
				T::instance().add_define(name_);
			}
				~scoped_preproc_define_internal()
				{
					T::instance().remove_define(name_);
				}
		};
	typedef scoped_preproc_define_internal<config_cache> scoped_preproc_define;
	typedef boost::scoped_ptr<scoped_preproc_define> scoped_preproc_define_sptr;

}
#endif
