/*
	Copyright (C) 2020 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <memory>
#include <stdexcept>

namespace utils {

template <typename T>
class shared_reference
{
	std::shared_ptr<T> m_ptr;

public:
	template<typename Y>
	shared_reference(const shared_reference<Y>& p)
		: m_ptr(p.m_ptr)
	{ }
	template<typename Y>
	shared_reference(shared_reference<Y>&& p)
		: m_ptr(std::move(p.m_ptr))
	{ }
	template<typename Y>
	explicit shared_reference(const std::shared_ptr<Y>& p)
		: m_ptr(p)
	{
		if(!p)  {
			throw std::invalid_argument("invalid shared_reference");
		}
	}
	template<typename Y>
	explicit shared_reference(std::shared_ptr<Y>&& p)
		: m_ptr(p)
	{
		if(!p)  {
			throw std::invalid_argument("invalid shared_reference");
		}
	}

	template<typename Y>
	shared_reference& operator=(shared_reference<Y>&& p)
	{
		m_ptr = std::move(p.m_ptr);
		return *this;
	}
	template<typename Y>
	shared_reference& operator=(const shared_reference<Y>& p)
	{
		m_ptr = p.m_ptr;
		return *this;
	}

	~shared_reference() = default;

	operator std::shared_ptr<T>()
	{ return m_ptr; }

	T* operator->() { return m_ptr.get(); }
	const T* operator->() const { return m_ptr.get(); }

	T& operator*() { return *m_ptr.get(); }
	const T& operator*() const { return *m_ptr.get(); }

	template <typename XT, typename...XTypes>
	friend shared_reference<XT> make_shared_reference(XTypes&&...args);

};


template <typename T, typename...Types>
shared_reference<T> make_shared_reference(Types&&...args)
{
	return shared_reference<T>(std::make_shared<T>(std::forward<Types>(args)...));
}


} // namespace utils
