/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

// The purpose of this header is to forward declare the unit_ptr, if it
// is an intrusive pointer then this requires some boilerplate taken
// care of here.

#pragma once

#include <boost/intrusive_ptr.hpp>
#include <memory>

class unit;

void intrusive_ptr_add_ref(const unit *);
void intrusive_ptr_release(const unit *);

class unit_ptr : public std::shared_ptr<unit> 
{
public:
   unit_ptr() = default;
   template<typename T>
   unit_ptr(T&& v) : std::shared_ptr<unit>(v) {}
   template<typename T>
   unit_ptr(unit*) = delete;
   unit_ptr(const unit*) = delete;
   unit_ptr& operator=(const unit_ptr& v)
   {
      std::shared_ptr<unit>::operator=(static_cast<const std::shared_ptr<unit>&>(v));
      return *this;
   }
   unit_ptr& operator=(const std::shared_ptr<unit>& v)
   {
      std::shared_ptr<unit>::operator=(static_cast<const std::shared_ptr<unit>&>(v));
      return *this;
   }
};

class unit_const_ptr : public std::shared_ptr<const unit> 
{
public:
   unit_const_ptr() = default;
   template<typename T>
   unit_const_ptr(T&& v) : std::shared_ptr<const unit>(v) {}
   unit_const_ptr(unit*) = delete;
   unit_const_ptr(const unit*) = delete;
   unit_const_ptr& operator=(const unit_const_ptr& v)
   {
      std::shared_ptr<const unit>::operator=(static_cast<const std::shared_ptr<const unit>&>(v));
      return *this;
   }
   unit_const_ptr& operator=(const unit_ptr& v)
   {
      std::shared_ptr<const unit>::operator=(static_cast<const std::shared_ptr<unit>&>(v));
      return *this;
   }
   unit_const_ptr& operator=(const std::shared_ptr<const unit>& v)
   {
      std::shared_ptr<const unit>::operator=(static_cast<const std::shared_ptr<const unit>&>(v));
      return *this;
   }
   unit_const_ptr& operator=(const std::shared_ptr<unit>& v)
   {
      std::shared_ptr<const unit>::operator=(static_cast<const std::shared_ptr<unit>&>(v));
      return *this;
   }
};

//typedef std::shared_ptr<unit> unit_ptr;
//typedef std::shared_ptr<const unit> unit_const_ptr;

// And attacks too!

class attack_type;

using attack_ptr = std::shared_ptr<attack_type>;
using const_attack_ptr = std::shared_ptr<const attack_type>;
