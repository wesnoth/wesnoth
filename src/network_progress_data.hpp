/* $Id$ */
/*
   Copyright (C) 2010 by Maciej Pawlowski <>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef NETWORK_PROGRESS_HPP_INCLUDED
#define NETWORK_PROGRESS_HPP_INCLUDED

#include <cstdlib>
#include <iostream>

#include <boost/thread/mutex.hpp>

namespace network {

	class progress_data {
	public:
		progress_data()
			: done_(0), total_(0), abort_(false), running_(false), mutex_()
		{
		}

		double done() {
			boost::mutex::scoped_lock lock(mutex_);
			return done_;
		}

		double total() {
			boost::mutex::scoped_lock lock(mutex_);
			return total_;
		}

		bool abort() {
			boost::mutex::scoped_lock lock(mutex_);
			return abort_;
		}

		bool running() {
			boost::mutex::scoped_lock lock(mutex_);
			return running_;
		}

		void set_done(double v) {
			boost::mutex::scoped_lock lock(mutex_);
			done_ = v;
		}

		void set_total(double v) {
			boost::mutex::scoped_lock lock(mutex_);
			total_ = v;
		}

		void set_abort(bool v) {
			boost::mutex::scoped_lock lock(mutex_);
			abort_ = v;
		}

		void set_running(bool v) {
			boost::mutex::scoped_lock lock(mutex_);
			running_ = v;
		}

	private:
		double done_;
		double total_;
		bool abort_;
		bool running_;

		boost::mutex mutex_;
	};


	/*

	  network::progress_data pi;
	  addonclient.start_async_get(pi, ...);
	  gui::progress_dialog dialog;
	  dialog.set_info(pi);
	  dialog.show();
	  assert(!pi.running());
	  if (!pi.abort()) {
		try do shit
	  } else {
	    aborted
	  }


	  */

} //end namespace network


#endif
