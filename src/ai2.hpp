#ifndef AI2_HPP_INCLUDED
#define AI2_HPP_INCLUDED

#include "ai_interface.hpp"

class ai2 : public ai_interface
{
public:
	ai2(ai_interface::info& info) : ai_interface(info)
	{}
	virtual ~ai2() {}
	virtual void play_turn() {}

};


#endif