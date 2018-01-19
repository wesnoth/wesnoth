/*
   Copyright (C) 2009 - 2018 by Daniel Franke.
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "save_blocker.hpp"
#include <exception>
#include <iostream>

play_controller* save_blocker::controller_ = nullptr;
void (play_controller::*save_blocker::callback_)() = nullptr;
SDL_sem* save_blocker::sem_ = SDL_CreateSemaphore(1);

save_blocker::save_blocker() {
	block();
}

save_blocker::~save_blocker() {
	try {
	unblock();
	if(controller_ && callback_) {
		(controller_->*callback_)();
		controller_ = nullptr;
		callback_ = nullptr;
	}
	} catch (std::exception & e) {
		std::cerr << "Save blocker dtor swallowing an exception: " << e.what() << "\n";
	}
}

void save_blocker::on_unblock(play_controller* controller, void (play_controller::*callback)()) {
	if(try_block()) {
		unblock();
		(controller->*callback)();
	} else {
		controller_ = controller;
		callback_ = callback;
	}
}

bool save_blocker::saves_are_blocked() {
	return SDL_SemValue(sem_) == 0;
}

void save_blocker::block() {
	SDL_SemWait(sem_);
}

bool save_blocker::try_block() {
	return SDL_SemTryWait(sem_) == 0;
}

void save_blocker::unblock() {
	assert(SDL_SemValue(sem_) == 0);
	SDL_SemPost(sem_);
}
