/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/* Note about Python exceptions:

   Originally, runtime exceptions were used for things like using out of bounds
   coordinates or moving a unit which already got killed, and were supposed to
   be catched by scripts. The subset of the Python language allowed in
   scripts now has no exceptions anymore, to keep things simple. Therefore the
   API was changed so scripts are not supposed to catch exceptions anymore.

   This means, invalid positions or units will now only be indicated by things
   like return	values, but are ignored otherwise.

   Most API functions still will cause exceptions (like if the game ends, or
   the wrong number of parameters is passed to a function) - but those are not
   supposed to be catched by user scripts.
*/

#ifdef HAVE_PYTHON

#include "global.hpp"

#include "ai.hpp"
#include "ai_python.hpp"
#include "attack_prediction.hpp"
#include "wassert.hpp"
#include "gamestatus.hpp"
#include "filesystem.hpp"
#include "menu_events.hpp"
#include "game_events.hpp"
#include "game_config.hpp"

#include <fstream>

#define LOG_AI LOG_STREAM(info, ai)

static python_ai* running_instance;
bool python_ai::init_ = false;

#define return_none do {Py_INCREF(Py_None); return Py_None;} while(false)

#define wrap(code) \
	try { code } \
	catch(end_level_exception& e) { \
		running_instance->exception = e; \
		PyErr_SetString(PyExc_RuntimeError, "C++ exception!"); \
		return NULL; \
	}

static PyObject* wrap_unittype(const unit_type& type);
static PyObject* wrap_attacktype(const attack_type& type);
static PyObject* wrapper_unittype_resistance_against(wesnoth_unittype* type, PyObject* args);

static PyObject* wrapper_unittype_get_name(wesnoth_unittype* unit, void* /*closure*/)
{
	return Py_BuildValue("s",( const char* )unit->unit_type_->id().c_str());
}

#define ut_get_general( t, n, x ) \
static PyObject* wrapper_unittype_get_##n( wesnoth_unittype* type, void* /*closure*/ ) \
{	\
	return Py_BuildValue(t,type->unit_type_->x);	\
}

#define ut_get( x ) \
	ut_get_general( "i", x, x() )

#define ut_get_ability( x ) \
	ut_get_general( "i", x, has_ability(#x))

#define ut_get_ability_by_id( x ) \
	ut_get_general( "i", x, has_ability_by_id(#x))

ut_get_ability( heals )
ut_get_ability( regenerate )
ut_get_ability( leadership )
ut_get_ability( illuminates )
ut_get_ability( skirmisher )
ut_get_ability( teleport )
ut_get_ability_by_id( curing )
ut_get_ability_by_id( steadfast )
ut_get( not_living )
ut_get( can_advance )
ut_get( has_zoc )
ut_get( level )
ut_get( movement )
ut_get( cost )
ut_get( alignment )
ut_get( hitpoints )
static PyObject* wrapper_unittype_get_usage( wesnoth_unittype* type, void* /*closure*/ )
{
	return Py_BuildValue("s",type->unit_type_->usage().c_str());
}

#define ut_gs( x, doc ) \
	{ #x,	(getter)wrapper_unittype_get_##x,	NULL,	doc,	NULL },

static PyGetSetDef unittype_getseters[] = {
	ut_gs( name, "Name of the unit type." )
	ut_gs( heals, "If type can heal others (either healing or curing ability)." )
	ut_gs( curing, "If type has curing ability (remove poison from others)." )
	ut_gs( regenerate, "If type has regenerate ability." )
	ut_gs( leadership, "If type has leadership ability." )
	ut_gs( illuminates, "If type has illuminates ability." )
	ut_gs( skirmisher, "If type has skirmisher ability." )
	ut_gs( teleport, "If type has teleport ability." )
	ut_gs( steadfast, "If type has steadfast ability." )
	ut_gs( not_living, "If type has not-living ability." )
	ut_gs( can_advance, "If type can advance." )
	ut_gs( has_zoc, "If type has a ZOC." )
	ut_gs( level, "Level of the type." )
	ut_gs( hitpoints, "Hitpoints of the type." )
	ut_gs( usage, "AI's usage hint of the type, one of: 'archer', 'fighter', "
		"'healer', 'mixed fighter', 'scout'." )
	ut_gs( movement, "Movement points of the type." )
	ut_gs( cost, "Cost of the type." )
	ut_gs( alignment, "Alignment of the type: 0=lawful, 1=neutral, 2=chaotic." )
	{ NULL, NULL, NULL, NULL, NULL }
};

PyObject* python_ai::unittype_advances_to( wesnoth_unittype* type, PyObject* args )
{
	if (!PyArg_ParseTuple(args, "" ))
		return NULL;

	PyObject* list = PyList_New(type->unit_type_->advances_to().size());
	int r;
	for (size_t advance = 0; advance < type->unit_type_->advances_to().size(); advance++)
	{
		std::map<std::string,unit_type>::const_iterator t = running_instance->get_info().gameinfo.unit_types.find(type->unit_type_->advances_to()[advance]);
		wassert(t != running_instance->get_info().gameinfo.unit_types.end());
		r = PyList_SetItem(list,advance,wrap_unittype(t->second));
	}
	return list;
}

static PyObject* wrapper_unittype_attacks( wesnoth_unittype* type, PyObject* args )
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;

	PyObject* list = PyList_New(type->unit_type_->attacks().size());
	for ( size_t attack = 0; attack < type->unit_type_->attacks().size(); attack++)
		PyList_SetItem(list,attack,wrap_attacktype(type->unit_type_->attacks()[attack]));
	return (PyObject*)list;
}

static PyMethodDef unittype_methods[] = {
	{ "advances_to",	(PyCFunction)python_ai::unittype_advances_to,	METH_VARARGS,
		"Returns: unittype[]\n"
		"Returns a list of wesnoth.unittype of possible advancements."},
	{ "attacks",		(PyCFunction)wrapper_unittype_attacks,			METH_VARARGS,
		"Returns: attacktype[]\n"
		"Returns list of possible attack types.\n"},
	{ "movement_cost",		(PyCFunction)python_ai::wrapper_unittype_movement_cost,			METH_VARARGS,
		"Parameters: location\n"
		"Returns: cost\n"
		"Returns the cost of moving over the given location."},
	{ "defense_modifier",		(PyCFunction)python_ai::wrapper_unittype_defense_modifier,			METH_VARARGS,
		"Parameters: location\n"
		"Returns: percent\n"
		"Returns the defense modifier in % (probability the unit will be hit) on the given location."},
	{ "damage_from",		(PyCFunction)wrapper_unittype_resistance_against,			METH_VARARGS,
		"Parameters: attacktype\n"
		"Returns: percent\n"
		"Returns the damage in percent a unit of this type receives when "
		"attacked with the given attack type. (0 means no damage at all, 100 "
		"means full damage, 200 means double damage.)"},
	{ NULL, NULL, 0, NULL }
};

static int unittype_internal_compare(wesnoth_unittype* left, wesnoth_unittype* right)
{
	return (long)(left->unit_type_) - (long)right->unit_type_;
}

static PyTypeObject wesnoth_unittype_type = {
	PyObject_HEAD_INIT(NULL)
	0,										/* ob_size*/
	"wesnoth.unittype",						/* tp_name*/
	sizeof(wesnoth_unittype),				/* tp_basicsize*/
	0,										/* tp_itemsize*/
	0,										/* tp_dealloc*/
	0,										/* tp_print*/
	0,										/* tp_getattr*/
	0,										/* tp_setattr*/
	(cmpfunc)unittype_internal_compare,		/* tp_compare*/
	0,										/* tp_repr*/
	0,										/* tp_as_number*/
	0,										/* tp_as_sequence*/
	0,										/* tp_as_mapping*/
	0,										/* tp_hash */
	0,										/* tp_call*/
	0,										/* tp_str*/
	0,										/* tp_getattro*/
	0,										/* tp_setattro*/
	0,										/* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,						/* tp_flags*/
	"Describes a unit type.",					/* tp_doc */
	0,										/* tp_traverse */
	0,										/* tp_clear */
	0,										/* tp_richcompare */
	0,										/* tp_weaklistoffset */
	0,										/* tp_iter */
	0,										/* tp_iternext */
	unittype_methods,						/* tp_methods */
	0,										/* tp_members */
	unittype_getseters,						/* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static PyObject* wrap_unittype(const unit_type& type)
{
	wesnoth_unittype* wrap = (wesnoth_unittype*)PyObject_NEW(wesnoth_unittype, &wesnoth_unittype_type);
	if (wrap)
		wrap->unit_type_ = &type;
	return (PyObject*)wrap;
}

typedef struct {
	PyObject_HEAD
	const attack_type* attack_type_;
} wesnoth_attacktype;

#define at_get_general( t, n, x ) \
static PyObject* attacktype_get_##n(wesnoth_attacktype* type, void* /*closure*/) \
{ \
	return Py_BuildValue(t,type->attack_type_->x); \
}

#define at_get( t, x ) \
	at_get_general( t, x, x() )

#define at_get_ability_by_id( t, x ) \
	at_get_general( t, x, has_special_by_id(#x) )

#define at_get_string_prop( x ) \
	at_get_general( "s", x, x().c_str() )

at_get( "i", damage )
at_get( "i", num_attacks )
at_get( "d", attack_weight )
at_get( "d", defense_weight )

at_get_ability_by_id( "i", backstab )
at_get_ability_by_id( "i", slow )
at_get_ability_by_id( "i", berserk )
at_get_ability_by_id( "i", stones )
at_get_ability_by_id( "i", plague )
at_get_ability_by_id( "i", marksman )
at_get_ability_by_id( "i", magical )
at_get_ability_by_id( "i", charge )
at_get_ability_by_id( "i", drains )

at_get_string_prop( name )
at_get_string_prop( range )

static void wesnoth_attacktype_dealloc(wesnoth_attacktype* self)
{
	delete self->attack_type_;
	self->ob_type->tp_free((PyObject*)self);
}

#define at_gs( x, doc ) \
	{ #x,	(getter)attacktype_get_##x,	NULL,	doc,	NULL },

static PyGetSetDef attacktype_getseters[] = {
	at_gs( name,			"Name of the attack." )
	at_gs( damage,			"Attack damage." )
	at_gs( num_attacks,		"Number of hits." )
	at_gs( attack_weight,	"AI setting, floating point number." )
	at_gs( defense_weight,	"AI setting, floating point number." )
	at_gs( backstab,		"This attack has backstab." )
	at_gs( slow,			"This attack causes slow." )
	at_gs( berserk,			"This attack uses berserk." )
	at_gs( stones,			"This attack has 'stones' special." )
	at_gs( plague,			"This attack has 'plague' special." )
	at_gs( marksman,		"This attack has 'marksman' special." )
	at_gs( magical,			"This attack is magical." )
	at_gs( charge,			"This attack has the 'charge' special." )
	at_gs( drains,			"This attack has the 'drains' special." )
	at_gs( range,			"String with the name of the attack range." )
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef attacktype_methods[] = {
	{ NULL, NULL, 0, NULL }
};

static PyTypeObject wesnoth_attacktype_type = {
	PyObject_HEAD_INIT(NULL)
	0,						   /* ob_size*/
	"wesnoth.attacktype",		 /* tp_name*/
	sizeof(wesnoth_attacktype),  /* tp_basicsize*/
	0,						   /* tp_itemsize*/
	(destructor)wesnoth_attacktype_dealloc, /* tp_dealloc*/
	0,						   /* tp_print*/
	0,						   /* tp_getattr*/
	0,						   /* tp_setattr*/
	0,						   /* tp_compare*/
	0,						   /* tp_repr*/
	0, //UniConvert,			 /* tp_as_number*/
	0,						   /* tp_as_sequence*/
	0,						   /* tp_as_mapping*/
	0,						   /* tp_hash */
	0,						   /* tp_call*/
	0,						   /* tp_str*/
	0,	 /* tp_getattro*/
	0,	 /* tp_setattro*/
	0,						   /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		   /* tp_flags*/
	"Describes an attack type.",	   /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	attacktype_methods,				/* tp_methods */
	0,						   /* tp_members */
	attacktype_getseters,		   /* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static PyObject* wrap_attacktype(const attack_type& type)
{
	wesnoth_attacktype* attack;
	attack = (wesnoth_attacktype*)PyObject_NEW(wesnoth_attacktype, &wesnoth_attacktype_type);

	attack->attack_type_ = new attack_type(type);
	return (PyObject*)attack;
}

#define u_check if (!running_instance->is_unit_valid(unit->unit_)) return_none

bool python_ai::is_unit_valid(const unit* unit)
{
	if (!unit)
	{
		return false;
	}
	for(unit_map::const_iterator i = running_instance->get_info().units.begin(); i != running_instance->get_info().units.end(); ++i) {
		if (unit == &i->second)
			return true;
	}
	return false;
}

static PyObject* unit_get_name(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("s",( const char* )unit->unit_->name().c_str());
}

static PyObject* unit_is_enemy(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", running_instance->current_team().is_enemy(unit->unit_->side()) == true ? 1 : 0);
}

static PyObject* unit_can_recruit(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", unit->unit_->can_recruit() == true ? 1 : 0);
}

static PyObject* unit_side(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", unit->unit_->side());
}

static PyObject* unit_movement_left(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", unit->unit_->movement_left());
}

static PyObject* unit_can_attack(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", unit->unit_->attacks_left());
}

static PyObject* unit_hitpoints(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", (int)unit->unit_->hitpoints());
}

static PyObject* unit_max_hitpoints(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", (int)unit->unit_->max_hitpoints());
}

static PyObject* unit_experience(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", (int)unit->unit_->experience());
}

static PyObject* unit_max_experience(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", (int)unit->unit_->max_experience());
}

static PyObject* unit_poisoned(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", utils::string_bool(unit->unit_->get_state("poisoned")));
}

static PyObject* unit_stoned(wesnoth_unit* unit, void* /*closure*/)
{
	u_check;
	return Py_BuildValue("i", utils::string_bool(unit->unit_->get_state("stoned")));
}

static PyObject* unit_query_valid(wesnoth_unit* unit, void* /*closure*/)
{
	return Py_BuildValue("i", running_instance->is_unit_valid(unit->unit_) == true ? 1 : 0);
}

static PyGetSetDef unit_getseters[] = {
	{ "name",			(getter)unit_get_name,		NULL,
		"Name of the unit (''description'' from WML).",	NULL },
	{ "is_enemy",		(getter)unit_is_enemy,		NULL,
		"True if this is an enemy unit, False if it is allied.",	NULL },
	{ "can_recruit",	(getter)unit_can_recruit,	NULL,
		"If the unit can recruit.",	NULL },
	{ "hitpoints",		(getter)unit_hitpoints,	NULL,
		"Current hitpoints of the unit.",	NULL },
	{ "max_hitpoints",	(getter)unit_max_hitpoints,	NULL,
		"Maximum hitpoints of the unit.",	NULL },
	{ "experience",		(getter)unit_experience,	NULL,
		"Current experience of the unit.",	NULL },
	{ "max_experience",	(getter)unit_max_experience,	NULL,
		"Maximum experience of the unit.",	NULL },
	{ "is_valid",		(getter)unit_query_valid,	NULL,
		"Indicates if the unit is still valid in the game. This is the only accessible "
		"field of an invalid unit, all others trigger an exception.",	NULL },
	{ "side",			(getter)unit_side,	NULL,
		"The side of the unit, starting with 1.",	NULL },
	{ "movement_left",	(getter)unit_movement_left, NULL,
		"How many movement points the unit has left.",	NULL },
	{ "can_attack",			(getter)unit_can_attack, NULL,
		"If the unit can still attack.",	NULL},
	{ "poisoned",		(getter)unit_poisoned,	NULL,
		"If the unit is poisoned.",	NULL },
	{ "stoned",		(getter)unit_stoned,	NULL,
		"If the unit is stoned.",	NULL },
	{ NULL,				NULL,						NULL,	NULL,	NULL }
};

static PyObject* wrapper_unit_type( wesnoth_unit* unit, PyObject* args )
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	u_check;
	wassert(unit->unit_->type());
	return wrap_unittype(*unit->unit_->type());
}

static PyObject* wrapper_unit_attacks( wesnoth_unit* unit, PyObject* args )
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	u_check;
	PyObject* list = PyList_New(unit->unit_->attacks().size());
	for ( size_t attack = 0; attack < unit->unit_->attacks().size(); attack++)
		PyList_SetItem(list,attack,wrap_attacktype(unit->unit_->attacks()[attack]));
	return (PyObject*)list;
}

static PyObject* wrapper_unit_damage_from( wesnoth_unit* unit, PyObject* args )
{
	wesnoth_attacktype* attack;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_attacktype_type, &attack ) )
		return NULL;
	u_check;
	static gamemap::location no_loc;
	return Py_BuildValue("i",unit->unit_->damage_from(*attack->attack_type_,true,no_loc));
}

static PyObject* wrapper_unittype_resistance_against( wesnoth_unittype* type, PyObject* args )
{
	wesnoth_attacktype* attack;
	if (!PyArg_ParseTuple(args, "O!", &wesnoth_attacktype_type, &attack))
		return NULL;
	return Py_BuildValue("i",type->unit_type_->movement_type().resistance_against(*attack->attack_type_));
}

static PyMethodDef unit_methods[] = {
	{ "type",				(PyCFunction)wrapper_unit_type,				METH_VARARGS,
		"Returns: unittype\n"
		"Returns the type of the unit."},
	{ "attacks",			(PyCFunction)wrapper_unit_attacks,			METH_VARARGS,
		"Returns: attacktype[]\n"
		"Returns list of possible attack types.\n"},
	{ "movement_cost",		(PyCFunction)python_ai::wrapper_unit_movement_cost,	METH_VARARGS,
		"Parameters: location\n"
		"Returns: cost\n"
		"Returns the cost of moving over the given location."},
	{ "defense_modifier",	(PyCFunction)python_ai::wrapper_unit_defense_modifier,	METH_VARARGS,
		"Parameters: location\n"
		"Returns: percent\n"
		"Returns the defense modifier in % (probability the unit will be hit) on the given location."},
	{ "damage_from",		(PyCFunction)wrapper_unit_damage_from,	METH_VARARGS,
		"Parameters: attacktype\n"
		"Returns: percent\n"
		"Returns the damage in percent the unit receives when attacked with "
		"the given attack type. (0 means no damage at all, 100 means full "
		"damage, 200 means double damage.)"},
	{ "find_path",			(PyCFunction)python_ai::wrapper_unit_find_path,		METH_VARARGS,
		"Parameters: location from, location to, float max_cost = unit.movement_left\n"
		"Returns: location[] path\n"
		"Finds a path from 'from' to 'to' costing less than 'max_cost' "
		"movement points to reach and returns it as a list of locations. "
		"path[0] will be 'from', path[-1] will be 'to'. "
		"If no path can be found (for example, if the target is not reachable, "
		"or it would cost more than max_cost), an empty list is returned."},
	{ "attack_statistics", (PyCFunction)python_ai::wrapper_unit_attack_statistics, METH_VARARGS,
		"Parameters: location from, location to, int attack = -1\n"
		"Returns: own_hp, enemy_hp\n"
		"Returns two dictionaries with the expected battle results when the "
		"unit attacks from 'from' to the unit at 'to', optionally using the "
		"attack with index 'attack', or if no attack is given the attack which "
		"would be presented to the player in the attack dialog. The "
		"dictionaries contain the expected hitpoints after "
		"the fight, as a mapping from hitpoints to percent, where percent are "
		"specified as floating point value from 0 to 1. For example, a return of: "
		"{0:1}, {50:0.5, 40:0.5} would mean, the attacking unit "
		"is certain to die (probability for 0 hitpoints is 1), and the enemy "
		"unit will either remain at 50 or 40 HP after the fight, with equal "
		"probability of 0.5."},
	{ NULL,					NULL,										0, NULL }
};

static int unit_internal_compare(wesnoth_unit* left, wesnoth_unit* right)
{
	return (long)left->unit_ - (long)right->unit_;
}

static PyTypeObject wesnoth_unit_type = {
	PyObject_HEAD_INIT(NULL)
	0,						   /* ob_size*/
	"wesnoth.unit",		   /* tp_name*/
	sizeof(wesnoth_unit),  /* tp_basicsize*/
	0,						   /* tp_itemsize*/
	0,						   /* tp_dealloc*/
	0,						   /* tp_print*/
	0,						   /* tp_getattr*/
	0,						   /* tp_setattr*/
	(cmpfunc)unit_internal_compare,							/* tp_compare*/
	0,						   /* tp_repr*/
	0, //UniConvert,			 /* tp_as_number*/
	0,						   /* tp_as_sequence*/
	0,						   /* tp_as_mapping*/
	0,						   /* tp_hash */
	0,						   /* tp_call*/
	0,						   /* tp_str*/
	0,	 /* tp_getattro*/
	0,	 /* tp_setattro*/
	0,						   /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		   /* tp_flags*/
	"Represents a single unit. Trying to use a method or access a property, "
	"with the exception of is_valid, will result in an exception if the unit "
	"is invalid (was destroyed last move, and so on).",		  /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	unit_methods,			  /* tp_methods */
	0,						   /* tp_members */
	unit_getseters,			 /* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

typedef struct {
	PyObject_HEAD
	gamemap::location* location_;
} wesnoth_location;

static void wesnoth_location_dealloc(wesnoth_location* self)
{
	delete self->location_;
	self->location_ = NULL;
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject* location_get_x(wesnoth_location* location, void* /*closure*/)
{
	return Py_BuildValue("i", location->location_->x);
}
static PyObject* location_get_y(wesnoth_location* location, void* /*closure*/)
{
	return Py_BuildValue("i", location->location_->y);
}

static int location_internal_compare(wesnoth_location* left, wesnoth_location* right)
{
	if (*left->location_ == *right->location_)
		return 0;
	return *left->location_ < *right->location_ ? -1 : 1;
}

static long location_internal_hash(wesnoth_location* obj)
{
	// Never return -1, which is reserved for raising an exception. Note that
	// both x and y can get values < 0, e.g. when checking all positions in
	// a certain radius at the map border.
	unsigned char x = (unsigned)obj->location_->x;
	unsigned char y = (unsigned)obj->location_->y;
	return x << 8 + y;
}

static PyGetSetDef location_getseters[] = {
	{ "x",		 (getter)location_get_x,	 NULL, "X position, starting with 0 for leftmost column.", NULL },
	{ "y",		 (getter)location_get_y,	 NULL, "Y position, starting with 0 for topmost row.", NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyObject* wrapper_location_adjacent_to( wesnoth_location* left, PyObject* args );
static PyObject* wrapper_location_distance_to( wesnoth_location* left, PyObject* args );
static PyMethodDef location_methods[] = {
	{ "adjacent_to",		 (PyCFunction)wrapper_location_adjacent_to,		  METH_VARARGS,
		"Parameters: location\n"
		"Returns: result\n"
		"Returns True if the location is adjacent to this one, False otherwise."},
	{ "distance_to",		 (PyCFunction)wrapper_location_distance_to,		  METH_VARARGS,
		"Parameters: location\n"
		"Returns: int distance\n"
		"Returns the distance in hexes to the other location."},
	{ NULL, NULL, 0, NULL }
};

static PyTypeObject wesnoth_location_type = {
	PyObject_HEAD_INIT(NULL)
	0,						   /* ob_size*/
	"wesnoth.location",		   /* tp_name*/
	sizeof(wesnoth_location),  /* tp_basicsize*/
	0,						   /* tp_itemsize*/
	(destructor)wesnoth_location_dealloc,						  /* tp_dealloc*/
	0,						   /* tp_print*/
	0,						   /* tp_getattr*/
	0,						   /* tp_setattr*/
	(cmpfunc)location_internal_compare,							/* tp_compare*/
	0,						   /* tp_repr*/
	0, //UniConvert,			 /* tp_as_number*/
	0,						   /* tp_as_sequence*/
	0,						   /* tp_as_mapping*/
	(hashfunc)location_internal_hash,						  /* tp_hash */
	0,						   /* tp_call*/
	0,						   /* tp_str*/
	0,	 /* tp_getattro*/
	0,	 /* tp_setattro*/
	0,						   /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		   /* tp_flags*/
	"Represents a single location on the map.",		  /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	location_methods,			  /* tp_methods */
	0,						   /* tp_members */
	location_getseters,			 /* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

void recalculate_movemaps()
{
    python_ai *ai = running_instance;
    ai->src_dst_.clear();
    ai->dst_src_.clear();
    ai->possible_moves_.clear();
    ai->calculate_possible_moves(ai->possible_moves_,
        ai->src_dst_, ai->dst_src_, false);

    ai->enemy_src_dst_.clear();
    ai->enemy_dst_src_.clear();
    ai->enemy_possible_moves_.clear();
    ai->calculate_possible_moves(ai->enemy_possible_moves_,
        ai->enemy_src_dst_, ai->enemy_dst_src_, true);
}

static PyObject* wrapper_location_adjacent_to( wesnoth_location* left, PyObject* args )
{
	wesnoth_location* right;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &right ) )
		return NULL;
	return Py_BuildValue("i", tiles_adjacent(*left->location_,*right->location_) ? 1 : 0);
}

static PyObject* wrapper_location_distance_to( wesnoth_location* left, PyObject* args )
{
	wesnoth_location* right;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &right ) )
		return NULL;
	return Py_BuildValue("i", distance_between(*left->location_,*right->location_));
}



typedef struct {
	PyObject_HEAD
	const gamemap* map_;
} wesnoth_gamemap;

static PyObject* gamemap_get_x(wesnoth_gamemap* map, void* /*closure*/)
{
	return Py_BuildValue("i", map->map_->x());
}
static PyObject* gamemap_get_y(wesnoth_gamemap* map, void* /*closure*/)
{
	return Py_BuildValue("i", map->map_->y());
}

static PyGetSetDef gamemap_getseters[] = {
	{ "x",	(getter)gamemap_get_x,	NULL,	"Width of the map in hexes.",	NULL },
	{ "y",	(getter)gamemap_get_y,	NULL,	"Height of the map in hexes.",	NULL },
	{ NULL,	NULL,					NULL,	NULL,	NULL },
};

static PyObject* wrapper_getmap_is_village( wesnoth_gamemap* map, PyObject* args )
{
	wesnoth_location* location;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &location ) )
		return NULL;
	return Py_BuildValue("i", map->map_->is_village(*location->location_) ? 1 : 0);
}

static PyObject* wrapper_getmap_is_keep( wesnoth_gamemap* map, PyObject* args )
{
	wesnoth_location* location;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &location ) )
		return NULL;
	return Py_BuildValue("i", map->map_->is_keep(*location->location_) ? 1 : 0);
}

static PyObject* wrapper_getmap_is_castle( wesnoth_gamemap* map, PyObject* args )
{
	wesnoth_location* location;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &location ) )
		return NULL;
	return Py_BuildValue("i", map->map_->is_castle(*location->location_) ? 1 : 0);
}

static PyMethodDef gamemap_methods[] = {
	{ "is_village",	(PyCFunction)wrapper_getmap_is_village,	METH_VARARGS,
		"Parameters: location\n"
		"Returns: result\n"
		"Returns True if a village is at the given location, False otherwise."},
	{ "is_keep",	(PyCFunction)wrapper_getmap_is_keep,	METH_VARARGS,
		"Parameters: location\n"
		"Returns: result\n"
		"Returns True if a keep (where a leader must stand to recruit) is at "
		"the given location, False otherwise."},
	{ "is_castle",	(PyCFunction)wrapper_getmap_is_castle,	METH_VARARGS,
		"Parameters: location\n"
		"Returns: result\n"
		"Returns True if the given location is a castle tile (where units are "
		"recruited to), False otherwise."},
	{ NULL, NULL, 0, NULL }
};

static PyTypeObject wesnoth_gamemap_type = {
	PyObject_HEAD_INIT(NULL)
	0,						   /* ob_size*/
	"wesnoth.gamemap",		  /* tp_name*/
	sizeof(wesnoth_gamemap),  /* tp_basicsize*/
	0,						   /* tp_itemsize*/
	0,						   /* tp_dealloc*/
	0,						   /* tp_print*/
	0,						   /* tp_getattr*/
	0,						   /* tp_setattr*/
	0,						   /* tp_compare*/
	0,						   /* tp_repr*/
	0, //UniConvert,			 /* tp_as_number*/
	0,						   /* tp_as_sequence*/
	0,						   /* tp_as_mapping*/
	0,						   /* tp_hash */
	0,						   /* tp_call*/
	0,						   /* tp_str*/
	0,	 /* tp_getattro*/
	0,							/* tp_setattro*/
	0,						   /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		   /* tp_flags*/
	"Represents the current map.",		 /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	gamemap_methods,			 /* tp_methods */
	0,						   /* tp_members */
	gamemap_getseters,			/* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static PyObject* wrapper_team_name(wesnoth_team* team, void* /*closure*/)
{
	return Py_BuildValue("s", team->team_->team_name().c_str());
}

static PyObject* wrapper_team_income(wesnoth_team* team, void* /*closure*/)
{
	return Py_BuildValue("i", team->team_->income());
}

static PyObject* wrapper_team_gold(wesnoth_team* team, void* /*closure*/)
{
	return Py_BuildValue("i", team->team_->gold());
}

static PyObject* wrapper_team_is_enemy(wesnoth_team* team, void* /*closure*/)
{
	int result;

	// find side number of team
	int side = 0;
	for (size_t t = 0; t < running_instance->get_teams().size(); t++) {
		if (team->team_ == &running_instance->get_teams()[t]) {
			side = 1 + t;
			break;
		}
	}

	result = running_instance->current_team().is_enemy(side) == true ? 1 : 0;
	return Py_BuildValue("i", result);
}

static PyObject* wrapper_team_side(wesnoth_team* team, void* /*closure*/)
{
	int side = 0;
	for (size_t t = 0; t < running_instance->get_teams().size(); t++) {
		if (team->team_ == &running_instance->get_teams()[t]) {
			side = 1 + t;
			break;
		}
	}
	return Py_BuildValue("i", side);
}

static int wrapper_team_internal_compare(wesnoth_team* left, wesnoth_team* right)
{
	return (long)left->team_ - (long)right->team_;
}

static PyObject* wrapper_team_owns_village( wesnoth_team* team, PyObject* args )
{
	wesnoth_location* location;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &location ) )
		return NULL;
	return Py_BuildValue("i", team->team_->owns_village(*location->location_) ? 1 : 0);
}

PyObject* python_ai::wrapper_team_recruits( wesnoth_team* team, PyObject* args )
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;

	PyObject* list = PyList_New(team->team_->recruits().size());
	int r;
	int idx = 0;
	for (std::set<std::string>::const_iterator recruit = team->team_->recruits().begin(); recruit != team->team_->recruits().end(); ++recruit)
	{
		std::map<std::string,unit_type>::const_iterator t = running_instance->get_info().gameinfo.unit_types.find(*recruit);
		wassert(t != running_instance->get_info().gameinfo.unit_types.end());
		r = PyList_SetItem(list,idx++,wrap_unittype(t->second));
	}
	return list;
}

static PyMethodDef team_methods[] = {
	{ "owns_village",		  (PyCFunction)wrapper_team_owns_village,		METH_VARARGS,
		"Parameters: location\n"
		"Returns: result\n"
		"True if the team owns a village at the given location."},
	{ "recruits",		  (PyCFunction)python_ai::wrapper_team_recruits,	   METH_VARARGS,
		"Returns: recruits\n"
		"Returns a list of wesnoth.unittype objects of all possible recruits for this team."},
	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef team_getseters[] = {
	{ "name",	(getter)wrapper_team_name,		NULL,	"The name of this team.",	NULL },
	{ "gold",	(getter)wrapper_team_gold,		NULL,	"The current amount of gold this team has.",	NULL },
	{ "income",	(getter)wrapper_team_income,	NULL,	"The current per-turn income if this team.",	NULL },
	{ "side",		(getter)wrapper_team_side,				NULL,	"Side number of this team, starting with 1.",	NULL},
	{ "is_enemy", (getter)wrapper_team_is_enemy, NULL, "Whether this team is an enemy.", NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject wesnoth_team_type = {
	PyObject_HEAD_INIT(NULL)
	0,						   /* ob_size*/
	"wesnoth.team",		   /* tp_name*/
	sizeof(wesnoth_team),  /* tp_basicsize*/
	0,						   /* tp_itemsize*/
	0,						   /* tp_dealloc*/
	0,						   /* tp_print*/
	0,						   /* tp_getattr*/
	0,						   /* tp_setattr*/
	(cmpfunc)wrapper_team_internal_compare,							/* tp_compare*/
	0,						   /* tp_repr*/
	0, //UniConvert,			 /* tp_as_number*/
	0,						   /* tp_as_sequence*/
	0,						   /* tp_as_mapping*/
	0,						   /* tp_hash */
	0,						   /* tp_call*/
	0,						   /* tp_str*/
	0,	 /* tp_getattro*/
	0,	 /* tp_setattro*/
	0,						   /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		   /* tp_flags*/
	"Represents one team/player/side.",		  /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	team_methods,			  /* tp_methods */
	0,						   /* tp_members */
	team_getseters,			 /* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

PyObject* python_ai::wrapper_unit_movement_cost( wesnoth_unit* unit, PyObject* args )
{
	gamemap const &map = running_instance->get_info().map;
	wesnoth_location* loc;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &loc ) )
		return NULL;
	return Py_BuildValue("i",unit->unit_->movement_cost(map.get_terrain(*loc->location_)));
}

PyObject* python_ai::wrapper_unit_defense_modifier( wesnoth_unit* unit, PyObject* args )
{
	gamemap const &map = running_instance->get_info().map;
	wesnoth_location* loc;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &loc ) )
		return NULL;
	return Py_BuildValue("i",unit->unit_->defense_modifier(map.get_terrain(*loc->location_)));
}

PyObject* python_ai::wrapper_unittype_movement_cost( wesnoth_unittype* type, PyObject* args )
{
	gamemap const &map = running_instance->get_info().map;
	wesnoth_location* loc;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &loc ) )
		return NULL;
	return Py_BuildValue("i",type->unit_type_->movement_type().movement_cost(
		map, map.get_terrain(*loc->location_)));
}

PyObject* python_ai::wrapper_unittype_defense_modifier( wesnoth_unittype* type, PyObject* args )
{
	gamemap const &map = running_instance->get_info().map;
	wesnoth_location* loc;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &loc ) )
		return NULL;
	return Py_BuildValue("i",type->unit_type_->movement_type().defense_modifier(
		map, map.get_terrain(*loc->location_)));
}

static PyObject* wrap_location(const gamemap::location& loc)
{
	wesnoth_location* location;
	location = (wesnoth_location*)PyObject_NEW(wesnoth_location, &wesnoth_location_type);
	location->location_ = new gamemap::location(loc.x, loc.y);
	return (PyObject*)location;
}

typedef struct {
	PyObject_HEAD
	const gamestatus* status_;
} wesnoth_gamestatus;

static PyMethodDef gamestatus_methods[] = {
	{ NULL,		NULL,	0, NULL }
};

static PyObject* wrapper_gamestatus_turn(wesnoth_gamestatus* status, void* /*closure*/)
{
	return Py_BuildValue("i", status->status_->turn());
}

static PyObject* wrapper_gamestatus_number_of_turns(wesnoth_gamestatus* status, void* /*closure*/)
{
	return Py_BuildValue("i", status->status_->number_of_turns());
}

static PyObject* wrapper_gamestatus_lawful_bonus(wesnoth_gamestatus* status, void* /*closure*/)
{
	return Py_BuildValue("i", status->status_->get_time_of_day().lawful_bonus);
}

static PyObject* wrapper_gamestatus_previous_lawful_bonus(wesnoth_gamestatus* status, void* /*closure*/)
{
	return Py_BuildValue("i", status->status_->get_previous_time_of_day().lawful_bonus);
}

static PyGetSetDef gamestatus_getseters[] = {
	{ "turn",					(getter)wrapper_gamestatus_turn,					NULL,	"The current turn.",	NULL },
	{ "number_of_turns",		(getter)wrapper_gamestatus_number_of_turns,			NULL,
		"The maximum number of turns of the whole game.",	NULL },
	{ "lawful_bonus",			(getter)wrapper_gamestatus_lawful_bonus,			NULL,
		"The bonus for lawful units in the current turn. This is the percentage to add to "
		"the attack damage of lawful units (alignment = 0), and to subtract from chaotic "
		"units (alignment = 2). Neutral units (alignment = 1) are not affected.", NULL },
	{ "previous_lawful_bonus",	(getter)wrapper_gamestatus_previous_lawful_bonus,	NULL,
		"The value of lawful_bonus in the previous turn.",	NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject wesnoth_gamestatus_type = {
	PyObject_HEAD_INIT(NULL)
	0,						   /* ob_size*/
	"wesnoth.team",		   /* tp_name*/
	sizeof(wesnoth_gamestatus),  /* tp_basicsize*/
	0,						   /* tp_itemsize*/
	0,						   /* tp_dealloc*/
	0,						   /* tp_print*/
	0,						   /* tp_getattr*/
	0,						   /* tp_setattr*/
	0,						   /* tp_compare*/
	0,						   /* tp_repr*/
	0, //UniConvert,			 /* tp_as_number*/
	0,						   /* tp_as_sequence*/
	0,						   /* tp_as_mapping*/
	0,						   /* tp_hash */
	0,						   /* tp_call*/
	0,						   /* tp_str*/
	0,	 /* tp_getattro*/
	0,	 /* tp_setattro*/
	0,						   /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		   /* tp_flags*/
	"This class has information about the game status.",	   /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	gamestatus_methods,				/* tp_methods */
	0,						   /* tp_members */
	gamestatus_getseters,		   /* tp_getset */
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


static PyObject* wrap_move_map(const ai_interface::move_map& wrap)
{
	PyObject* dict = PyDict_New();
	PyObject* list;
	PyObject* loc;
	ai_interface::move_map::const_iterator pos;
	for (pos = wrap.begin(); pos != wrap.end(); pos++)
	{
		loc = wrap_location(pos->first);
		list = PyDict_GetItem(dict,loc);
		if (!list)
		{
			list = PyList_New(0);
			PyDict_SetItem(dict,loc,list);
			Py_DECREF(list);
		}
		PyList_Append(list,wrap_location(pos->second));
		Py_DECREF(loc);
	}
	return dict;
}

PyObject* python_ai::wrapper_log_message(PyObject* /*self*/, PyObject* args)
{
	const char* msg;
	if ( !PyArg_ParseTuple( args, "s", &msg ) )
		return NULL;
	running_instance->log_message(msg);
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* python_ai::wrapper_get_units(PyObject* /*self*/, PyObject* args)
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;

	PyObject* dict = PyDict_New();
	if ( !dict )
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* key;
	wesnoth_unit* unit;
	int ret;

	for(unit_map::const_iterator i = running_instance->get_info().units.begin(); i != running_instance->get_info().units.end(); ++i) {
		key = wrap_location(i->first);
		unit = (wesnoth_unit*)PyObject_NEW(wesnoth_unit, &wesnoth_unit_type);
		unit->unit_ = &i->second;
		ret = PyDict_SetItem(dict,(PyObject*)key,(PyObject*)unit);
		Py_DECREF(unit);
		Py_DECREF(key);
	}

	return dict;
}

PyObject* python_ai::wrapper_get_location(PyObject* /*self*/, PyObject* args)
{
	int x, y;
	if (!PyArg_ParseTuple( args, "ii", &x, &y ))
		return NULL;
	if (x < 0 || x >= running_instance->get_info().map.x()) return_none;
	if (y < 0 || y >= running_instance->get_info().map.y()) return_none;

	gamemap::location loc(x,y);
	return wrap_location(loc);
}

PyObject* python_ai::wrapper_get_map(PyObject* /*self*/, PyObject* args)
{
	if (!PyArg_ParseTuple(args, "" )) return NULL;

	wesnoth_gamemap* map = (wesnoth_gamemap*)PyObject_NEW(wesnoth_gamemap, &wesnoth_gamemap_type);
	map->map_ = &running_instance->get_info().map;
	return (PyObject*)map;
}

PyObject* python_ai::wrapper_get_teams(PyObject* /*self*/, PyObject* args)
{
	if (!PyArg_ParseTuple(args, "" )) return NULL;

	PyObject* list = PyList_New(running_instance->get_info().teams.size());
	wesnoth_team* the_team;

	for (size_t team = 0; team < running_instance->get_info().teams.size(); team++)
	{
		the_team = (wesnoth_team*)PyObject_NEW(wesnoth_team, &wesnoth_team_type);
		the_team->team_ = &running_instance->get_info().teams[team];
		PyList_SetItem(list,team,(PyObject*)the_team);
	}

	return list;
}

PyObject* python_ai::wrapper_get_current_team(PyObject* /*self*/, PyObject* /*args*/)
{
	wesnoth_team* the_team;
	the_team = (wesnoth_team*)PyObject_NEW(wesnoth_team, &wesnoth_team_type);
	the_team->team_ = &running_instance->current_team();
	return (PyObject*)the_team;
}

PyObject* python_ai::wrapper_get_src_dst(PyObject* /*self*/, PyObject* /*args*/)
{
	return wrap_move_map(running_instance->src_dst_);
}

PyObject* python_ai::wrapper_get_dst_src(PyObject* /*self*/, PyObject* /*args*/)
{
	return wrap_move_map(running_instance->dst_src_);
}

PyObject* python_ai::wrapper_get_enemy_src_dst(PyObject* /*self*/, PyObject* /*args*/)
{
	return wrap_move_map(running_instance->enemy_src_dst_);
}

PyObject* python_ai::wrapper_get_enemy_dst_src(PyObject* /*self*/, PyObject* /*args*/)
{
	return wrap_move_map(running_instance->enemy_dst_src_);
}

PyObject* python_ai::wrapper_move_unit(PyObject* /*self*/, PyObject* args)
{
	wesnoth_location* from;
	wesnoth_location* to;
	if (!PyArg_ParseTuple( args, "O!O!", &wesnoth_location_type, &from,
		&wesnoth_location_type, &to ) )
		return NULL;

	bool valid = false;
	ai_interface::move_map::const_iterator pos;
	for (pos = running_instance->src_dst_.begin(); pos != running_instance->src_dst_.end(); pos++)
	{
		if ( pos->first == ( *from->location_ ) )
		{
			valid = true;
			if ( pos->second == ( *to->location_ ) )
				break;
		}
	}
	if (!valid) return_none;
	if (pos == running_instance->src_dst_.end()) return_none;

	location location;
	wrap(
		location = running_instance->move_unit_partial(
			*from->location_, *to->location_, running_instance->possible_moves_);
	)

	PyObject* loc = wrap_location(location);

    recalculate_movemaps();

    return loc;
}

PyObject* python_ai::wrapper_attack_unit(PyObject* /*self*/, PyObject* args)
{
	wesnoth_location* from;
	wesnoth_location* to;
	int weapon = -1; // auto-choose
	if (!PyArg_ParseTuple( args, "O!O!|i", &wesnoth_location_type, &from,
		&wesnoth_location_type, &to, &weapon ) )
		return NULL;

	// FIXME: Remove this check and let the C++ code do the check if the attack
	// is valid at all (there may be ranged attacks or similar later, and then
	// the below will horribly fail).
	if (!tiles_adjacent(*from->location_, *to->location_))
		return_none;

	info& inf = running_instance->get_info();

	battle_context bc(
		inf.map,
		inf.teams,
		inf.units,
		inf.state,
		inf.gameinfo,
		*from->location_,
		*to->location_,
		weapon);

	wrap(
		running_instance->attack_enemy(*from->location_,*to->location_,
			bc.get_attacker_stats().attack_num,
			bc.get_defender_stats().attack_num);
	)


    recalculate_movemaps();

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* python_ai::wrapper_get_adjacent_tiles(PyObject* /*self*/, PyObject* args)
{
	wesnoth_location* where;
	if (!PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &where))
		 return NULL;

	gamemap const &map = running_instance->get_info().map;
	PyObject* list = PyList_New(0);
	gamemap::location loc[6];
	get_adjacent_tiles(*where->location_,loc);
	for ( int tile = 0; tile < 6; tile++ )
		if (loc[tile].valid(map.x(), map.y()))
			PyList_Append(list,wrap_location(loc[tile]));
	return list;
}

PyObject* python_ai::wrapper_recruit_unit(PyObject* /*self*/, PyObject* args)
{
	wesnoth_location* where;
	const char* name;
	if (!PyArg_ParseTuple( args, "sO!", &name, &wesnoth_location_type, &where))
		return NULL;

    int r;
    wrap(
        r = running_instance->recruit(name,*where->location_) == true ? 1 : 0;
    )

    recalculate_movemaps();

    return Py_BuildValue("i", r);
}

PyObject* python_ai::wrapper_get_gamestatus(PyObject* /*self*/, PyObject* args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	wesnoth_gamestatus* status;
	status = (wesnoth_gamestatus*)PyObject_NEW(wesnoth_gamestatus, &wesnoth_gamestatus_type);
	status->status_ = &running_instance->get_info().state;
	return (PyObject*)status;
}

PyObject* python_ai::wrapper_unit_find_path(wesnoth_unit* self, PyObject* args)
{
	wesnoth_location* from;
	wesnoth_location* to;
	double max_cost = self->unit_->movement_left();
	if ( !PyArg_ParseTuple( args, "O!O!|d", &wesnoth_location_type, &from,
		&wesnoth_location_type, &to, &max_cost ) )
		return NULL;
	if (!running_instance->is_unit_valid(self->unit_))
		return NULL;

	info& inf = running_instance->get_info();

	max_cost += 1; // should be at least 1

	const shortest_path_calculator calc(*self->unit_,
		running_instance->current_team(),
		inf.units, inf.teams, inf.map);
	const paths::route& route = a_star_search(*from->location_, *to->location_,
		max_cost, &calc, inf.map.x(), inf.map.y());

	PyObject* steps = PyList_New(route.steps.size());
	for (size_t step = 0; step < route.steps.size(); step++)
		PyList_SetItem(steps,step,wrap_location(route.steps[step]));

	return steps;
}

PyObject* python_ai::wrapper_unit_attack_statistics(wesnoth_unit* self, PyObject* args)
{
	wesnoth_location* from;
	wesnoth_location* to;
	int weapon = -1;

	if (!PyArg_ParseTuple(args, "O!O!|i", &wesnoth_location_type, &from,
		&wesnoth_location_type, &to, &weapon))
		return NULL;
	if (!running_instance->is_unit_valid(self->unit_))
		return_none;
	if (weapon < -1 || weapon >= (int) self->unit_->attacks().size()){
		return_none;
	}

	info& inf = running_instance->get_info();

	// We need to temporarily move our unit to where the attack calculation is
	// supposed to take place.
	std::pair<gamemap::location,unit> *temp = inf.units.extract(*from->location_);
	std::pair<gamemap::location,unit> *backup = temp;
	std::pair<gamemap::location,unit> replace(*from->location_,*self->unit_);
	inf.units.add(&replace);

	battle_context bc(
		inf.map,
		inf.teams,
		inf.units,
		inf.state,
		inf.gameinfo,
		*from->location_,
		*to->location_,
		weapon);

	unsigned int i;
	std::vector<double>attacker = bc.get_attacker_combatant().hp_dist;
	PyObject* adict = PyDict_New();
	for (i = 0; i < attacker.size(); i++) {
		if (attacker[i] > 0)
			PyDict_SetItem(adict, PyInt_FromLong(i), PyFloat_FromDouble(attacker[i]));
	}

	std::vector<double>defender = bc.get_defender_combatant().hp_dist;
	PyObject* ddict = PyDict_New();
	for (i = 0; i < defender.size(); i++) {
		if (defender[i] > 0)
			PyDict_SetItem(ddict, PyInt_FromLong(i), PyFloat_FromDouble(defender[i]));
	}

	// restore old position again
	temp = inf.units.extract(*from->location_);
	if (backup)
		inf.units.add(backup);

	PyObject *ret = Py_BuildValue("(OO)", adict, ddict);

	return ret;
}

PyObject* python_ai::wrapper_set_variable(PyObject* /*self*/, PyObject* args)
{
	char const *variable, *value;
	if (!PyArg_ParseTuple(args, "ss", &variable, &value))
		return NULL;
	config const &old_memory = running_instance->current_team().ai_memory();
	config new_memory(old_memory);
	new_memory[variable] = value;
	running_instance->current_team().set_ai_memory(new_memory);

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* python_ai::wrapper_get_variable(PyObject* /*self*/, PyObject* args)
{
	char const *variable;
	if (!PyArg_ParseTuple(args, "s", &variable))
		return NULL;
	config const &memory = running_instance->current_team().ai_memory();
	return Py_BuildValue("s", memory[variable].c_str());
}

PyObject* python_ai::wrapper_get_version(PyObject* /*self*/, PyObject* args)
{
	if (!PyArg_ParseTuple(args, "" ))
		return NULL;
	return Py_BuildValue("s", game_config::version.c_str());
}

static PyMethodDef wesnoth_python_methods[] = {
	{ "log_message",				python_ai::wrapper_log_message,			METH_VARARGS,
		"Parameters: string\n"
		"Logs a message, displayed as a chat message, if debug is enabled." },
	{ "get_units", python_ai::wrapper_get_units, METH_VARARGS,
		"Returns: units\n"
		"Returns a dictionary containing (location, unit) pairs."},
	{ "get_location", python_ai::wrapper_get_location, METH_VARARGS,
		"Parameters: x, y\n"
		"Returns: location\n"
		"Returns a wesnoth.location object pointing to position (x, y) on the map."},
	{ "get_map", python_ai::wrapper_get_map, METH_VARARGS,
		"Returns: map\n"
		"Returns a wesnoth.gamemap object representing the current map."},
	{ "get_teams", python_ai::wrapper_get_teams, METH_VARARGS,
		"Returns: teams\n"
		"Returns a list containing wesnoth.team objects, representing the current teams in the game."},
	{ "get_current_team", python_ai::wrapper_get_current_team, METH_VARARGS,
		"Returns: team\n"
		"Returns a wesnoth.team object representing the current team."},
	{ "get_destinations_by_unit", python_ai::wrapper_get_src_dst, METH_VARARGS,
		"Returns: moves\n"
		"Returns a dictionary. Keys are wesnoth.location objects pointing to player's units, "
		"values are lists of possible destinations for this unit."},
	{ "get_units_by_destination", python_ai::wrapper_get_dst_src, METH_VARARGS,
		"Returns: moves\n"
		"Returns a dictionary. Keys are wesnoth.location objects pointing to positions the "
		"player's units can reach, values are lists of locations where units that can reach "
		"this position are."},
	{ "get_enemy_destinations_by_unit", python_ai::wrapper_get_enemy_src_dst, METH_VARARGS,
		"Returns: moves\n"
		"Returns a dictionary. Keys are wesnoth.location objects pointing to player's units, "
		"values are lists of possible destinations for this unit."},
	{ "get_enemy_units_by_destination", python_ai::wrapper_get_enemy_dst_src, METH_VARARGS,
		"Returns: moves\n"
		"Returns a dictionary. Keys are wesnoth.location objects pointing to positions the "
		"enemie's units can reach, values are lists of locations where units that can reach "
		"this position are."},
	{ "move_unit", python_ai::wrapper_move_unit, METH_VARARGS,
		"Parameters: location from, location to\n"
		"Returns: new_position\n"
		"Moves the unit on 'from' to the location specified by 'to', and "
		"returns a wesnoth.location object representing the position the unit was moved to."},
	{ "attack_unit", python_ai::wrapper_attack_unit, METH_VARARGS,
		"Parameters: location attacker, location defender, int weapon = -1\n"
		"Unit at position 'attacker' attacks unit at position 'defender' with weapon 'weapon'. "
		"The weapon parameter is optional, and the same weapon which would be "
		"highlighted for a human player is used if it is omitted."},
	{ "get_adjacent_tiles", python_ai::wrapper_get_adjacent_tiles, METH_VARARGS,
		"Parameters: location\n"
		"Returns: positions\n"
		"Returns a list of wesnoth.location representing tiles adjacent to the specified location."},
	{ "recruit_unit", python_ai::wrapper_recruit_unit, METH_VARARGS,
		"Parameters: string name, location where\n"
		"Returns: result\n"
		"Recruits the unit of type 'name', at the location 'where'. Returns 1 on success, 0 on failure."},
	{ "get_gamestatus", python_ai::wrapper_get_gamestatus, METH_VARARGS,
		"Returns: status\n"
		"Returns a wesnoth.gamestatus object representing the current game status."},
	{ "set_variable", python_ai::wrapper_set_variable, METH_VARARGS,
		"Parameters: variable, value\n"
		"Sets a persistent variable 'variable' to 'value'. This can be "
		"used to make the AI save strings over multiple turns."},
	{ "get_variable", python_ai::wrapper_get_variable, METH_VARARGS,
		"Parameters: variable\n"
		"Returns: value\n"
		"Retrieves a persistent variable 'variable' from the AI, which has "
		"previously been set with set_variable."},
	{ "get_version", python_ai::wrapper_get_version, METH_VARARGS,
		"Returns a string containing current Wesnoth version"},
		{ NULL, NULL, 0, NULL}
};


#define Py_Register( x, n ) { \
	PyType_Ready(&x); \
	Py_INCREF(&x); \
	PyTypeObject *type = &x; \
	PyObject* pyob = reinterpret_cast<PyObject *>(type); \
	PyModule_AddObject(module, const_cast<char *>(n), pyob); }

void python_ai::initialize_python()
{
	if (init_) return;
		init_ = true;
	Py_Initialize( );
	PyObject* module = Py_InitModule3("wesnoth", wesnoth_python_methods,
		"This is the wesnoth AI module. "
		"The python script will be executed once for each turn of the side with the "
		"python AI using the script.");
	Py_Register(wesnoth_unit_type, "unit");
	Py_Register(wesnoth_location_type, "location");
	Py_Register(wesnoth_gamemap_type, "gamemap");
	Py_Register(wesnoth_unittype_type, "unittype");
	Py_Register(wesnoth_team_type, "team");
	Py_Register(wesnoth_attacktype_type, "attacktype");
	Py_Register(wesnoth_gamestatus_type, "gamestatus");
}

void python_ai::invoke(std::string name)
{
	initialize_python();
	PyErr_Clear();
	PyObject* globals = PyDict_New();
	PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
	std::string python_code;
	python_code +=
		"import sys\n"
		"backup = sys.path[:]\n"
		"sys.path.append(\"" + game_config::path + "/data/ais\")\n"
		"try:\n"
		"\timport " + name + "\n"
		"finally:\n"
		"\tsys.path = backup\n";
	PyObject *ret = PyRun_String(python_code.c_str(), Py_file_input, globals,
		globals);
	Py_XDECREF(ret);
	Py_DECREF(globals);
}

python_ai::python_ai(ai_interface::info& info) : ai_interface(info), exception(QUIT)
{
	LOG_AI << "Running Python instance.\n";
	running_instance = this;
	initialize_python();
	calculate_possible_moves(possible_moves_,src_dst_,dst_src_,false);
	calculate_possible_moves(enemy_possible_moves_,enemy_src_dst_,enemy_dst_src_,true);
}

python_ai::~python_ai()
{
	// This is called whenever the AI is destroyed after its turn - the Python
	// interpreter itself will be auto cleaned up at program exit.
	LOG_AI << "Closing Python instance.\n";
	running_instance = NULL;
}

void python_ai::play_turn()
{
	game_events::fire("ai turn");
	
	std::string script_name = current_team().ai_parameters()["python_script"];
	if (script_name.substr(script_name.length() - 3) != ".py") {
		// Make sure the script ends in .py here - Wesnoth will not execute any
		// other files.
		std::cerr << "\"" << script_name << "\" is not a valid script name.\n";
		return;
	}
	std::string script = get_binary_file_location("data", "ais/" + script_name);

	PyErr_Clear();

	PyObject* globals = PyDict_New();
	PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

	LOG_AI << "Executing Python script \"" << script << "\".\n";
	// Run the python script. We actually execute a short inline python
	// script, which sets up the module search path to the data path,
	// runs the script, and then resets the path.
	std::string python_code;
	python_code +=
		"import sys\n"
		"backup = sys.path[:]\n"
		"sys.path.append(\"" + game_config::path + "/data/ais\")\n"
		"try:\n"
		"\timport wesnoth, parse, safe, heapq, random\n"
		"\tcode = parse.parse(\"" + script + "\")\n"
		"\tsafe.safe_exec(code, {\n"
		"\t\"wesnoth\" : wesnoth,\n"
		"\t\"heapq\" : heapq,\n"
		"\t\"random\" : random})\n"
		"finally:\n"
		"\tsys.path = backup\n";
	PyObject *ret = PyRun_String(python_code.c_str(), Py_file_input,
		globals, globals);

	Py_XDECREF(ret);
	Py_DECREF(globals);

	if (PyErr_Occurred()) {
		// RuntimeError is the game-won exception, no need to print it.
		// Anything else likely is a mistake by the script author.
		if (!PyErr_ExceptionMatches(PyExc_RuntimeError)) {
			LOG_AI << "Python script has crashed.\n";
			PyErr_Print();
		}
		// Otherwise, re-throw the exception here, so it will get handled
		// properly further up.
		else {
			LOG_AI << "Python script has been interrupted.\n";
			throw exception;
		}
	}
}

// Finds all python AI scripts available in the current binary path.
// They have to end with .py, and have #!WPY as first line.
std::vector<std::string> python_ai::get_available_scripts()
{
	std::vector<std::string> scripts;
	const std::vector<std::string>& paths = get_binary_paths("data");
	for(std::vector<std::string>::const_iterator i = paths.begin(); i != paths.end(); ++i) {
		std::vector<std::string> files;
		get_files_in_dir(*i + "ais", &files, NULL, ENTIRE_FILE_PATH);
		for(std::vector<std::string>::const_iterator j = files.begin(); j != files.end(); ++j) {
			// file ends with .py
			if (j->substr(j->length() - 3) == ".py") {
				std::string name(j->substr(j->rfind("/") + 1)); // extract name
				// read first line
				std::ifstream s(j->c_str()); std::string mark; s >> mark; s.close();
				if (mark == "#!WPY" &&
					std::find(scripts.begin(), scripts.end(), name) == scripts.end())
					scripts.push_back(name);
			}
		}
	}
	return scripts;
}

#endif // HAVE_PYTHON
