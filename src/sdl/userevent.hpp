/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <SDL_events.h>

namespace sdl
{
    class UserEvent : public SDL_UserEvent
    {
        public:
            UserEvent() : SDL_UserEvent() {
            }

            UserEvent(int type) : UserEvent() {
                this->type = type;
            }

            UserEvent(int type, int code) : UserEvent(type) {
                this->code = code;
            }

            UserEvent(int type, int data1, int data2) : UserEvent(type) {
                this->data1 = reinterpret_cast<void*>(data1);
                this->data2 = reinterpret_cast<void*>(data2);
            }

            UserEvent(int type, void* data1) : UserEvent(type) {
                this->data1 = data1;
            }
    };
}
