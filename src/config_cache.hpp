/*
   Copyright (C) 2008 - 2016 by Pauli Nieminen <paniemin@cc.hut.fi>
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
 * this file implements game config caching
 * to speed up startup
 ***/

#ifndef CONFIG_CACHE_HPP_INCLUDED
#define CONFIG_CACHE_HPP_INCLUDED

#include <list>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "serialization/preprocessor.hpp"

class config;

namespace game_config {


	/**
	 * Used to set and unset scoped defines to preproc_map
	 * This is preferred form to set defines that aren't global
	 **/
	template <class T>
		class scoped_preproc_define_internal : private boost::noncopyable {
			// Protected to let test code access
			protected:
				std::string name_;
				bool add_;
			public:
				/**
				 * Adds normal preproc define.
				 * @param name name of preproc define to add.
				 * @param add true if we should add this.
				 */
				scoped_preproc_define_internal(const std::string& name, bool add = true) : name_(name), add_(add)
				{
					if (add_)
						T::instance().add_define(name_);
				}

				/**
				 * This removes preproc define from cacher
				 **/
				~scoped_preproc_define_internal()
				{
					if(add_)
						T::instance().remove_define(name_);
				}
		};

	class config_cache;

	typedef scoped_preproc_define_internal<config_cache> scoped_preproc_define;

	/**
	 * Singleton class to manage game config file caching.
	 * It uses paths to config files as key to find correct cache
	 * @todo Make smarter filetree checksum caching so only required parts
	 *       of tree are checked at startup. Trees are overlapping so have
	 *       to split trees to subtrees to only do check once per file.
	 * @todo Make cache system easily allow validation of in memory cache objects
	 *       using hash checksum of preproc_map.
	 **/
	class config_cache : private boost::noncopyable {
		private:

		bool force_valid_cache_, use_cache_, fake_invalid_cache_;
		preproc_map defines_map_;

		std::string cache_file_prefix_;

		void read_file(const std::string& file, config& cfg);
		void write_file(std::string file, const config& cfg);
		void write_file(std::string file, const preproc_map&);

		void read_cache(const std::string& path, config& cfg);

		void read_configs(const std::string& path, config& cfg, preproc_map& defines);
		void load_configs(const std::string& path, config& cfg);
		void read_defines_queue();
		void read_defines_file(const std::string& path);

		preproc_map& make_copy_map();
		void add_defines_map_diff(preproc_map&);

		bool delete_cache_files(const std::vector<std::string>& paths,
								const std::string& exclude_pattern = "");

		// Protected to let test code access
		protected:
		config_cache();

		void set_force_invalid_cache(bool);


		public:
		/**
		 * Get reference to the singleton object
		 **/
		static config_cache& instance();

		const preproc_map& get_preproc_map() const;
		/**
		 * Gets a config object from given @a path.
		 * @param path file to load. Should be _main.cfg.
		 * @param cfg config object that is written to. Should be empty on entry.
		 */
		void get_config(const std::string& path, config& cfg);

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

		/**
		 * Deletes stale cache files not in use by the game.
		 */
		bool clean_cache();

		/**
		 * Deletes all cache files.
		 */
		bool purge_cache();

	};

	class fake_transaction;
	/**
	 * Used to share macros between cache objects
	 * You have to create transaction object to load all
	 * macros to memory and share them subsequent cache loads.
	 * If transaction is locked all stored macros are still
	 * available but new macros aren't added.
	 **/
	class config_cache_transaction  : private boost::noncopyable {
		public:
		config_cache_transaction();
		~config_cache_transaction();
		/**
		 * Lock the transaction so no more macros are added
		 **/
		void lock();

		enum state { FREE,
			NEW,
			ACTIVE,
			LOCKED
		};
		typedef std::vector<std::string> filenames;

		/**
		 * Used to let std::for_each insert new defines to active_map
		 * map to active
		 **/
		void insert_to_active(const preproc_map::value_type& def);

		private:
		static state state_;
		static config_cache_transaction* active_;
		filenames define_filenames_;
		preproc_map active_map_;

		static state get_state()
		{return state_; }
		static bool is_active()
		{return active_ != 0;}
		static config_cache_transaction& instance()
		{ assert(active_); return *active_; }
		friend class config_cache;
		friend class fake_transaction;
		const filenames& get_define_files() const;
		void add_define_file(const std::string&);
		preproc_map& get_active_map(const preproc_map&);
		void add_defines_map_diff(preproc_map& defines_map);
	};

	/**
	 * Holds a fake cache transaction if no real one is used
	 **/
	class fake_transaction : private boost::noncopyable {
		typedef boost::scoped_ptr<config_cache_transaction> value_type;
		value_type trans_;
		fake_transaction() : trans_()
		{
			if (!config_cache_transaction::is_active())
			{
				trans_.reset(new config_cache_transaction());
			}
		}
		friend class config_cache;
	};
}
#endif
