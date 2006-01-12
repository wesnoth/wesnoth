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

/*
 TODO (from the forum):
* display, which I don't think has to be exposed to Python. This is used by C++ AIs mostly for
  debugging purposes. We might want to allow Python scripts to write messages to the display for
  debugging purposes also, but they would only need very limited access.
* gamemap, defined in map.hpp, which contains the definition of the map. Definitely needs to be
 exposed to Python.
* game_data, defined in unit_types.hpp, which contains definitions of the available unit types,
 attacks, races, and so forth. Needs to be exposed to Python.
* unit_map, a map<gamemap::location,unit>. gamemap::location is defined in map.hpp and unit is
 defined in unit.hpp. This contains all the units currently in the game. Will need to be exposed.
* vector<team>, a listing of the teams (i.e. sides) in the game. Defined in team.hpp. Will
 need to be exposed.
* gamestatus, the current turn and time of day, etc. Defined in gamestatus.hpp. Will need to
 be exposed.
- turn_info, does not need to be exposed.

Additionally, useful utility functions such as those found in pathutils.hpp should be exposed.

*/

#ifdef HAVE_PYTHON

#include "global.hpp"

#include "ai.hpp"
#include "ai_python.hpp"
#include "wassert.hpp"
#include "gamestatus.hpp"

static python_ai* running_instance;
bool python_ai::init_ = false;
PyObject* python_ai::python_error_ = NULL;

void python_ai::set_error(const char *fmt, ...)
{
	char buf[1024];
	va_list arg;

	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf), fmt, arg);
	va_end(arg);

	PyErr_SetString(PyExc_ValueError, buf);
}

static PyObject* wrap_unittype(const unit_type& type);

static PyObject* wrapper_unittype_get_name(wesnoth_unittype* unit, void* closure)
{
	return Py_BuildValue("s",( const char* )unit->unit_type_->language_name().c_str());
}

#define ut_get( x ) \
static PyObject* wrapper_unittype_get_##x( wesnoth_unittype* type, void* closure ) \
{	\
	return Py_BuildValue("i",type->unit_type_->x());	\
}

ut_get( max_unit_healing )
ut_get( heals )
ut_get( regenerates )
ut_get( is_leader )
ut_get( illuminates )
ut_get( is_skirmisher )
ut_get( teleports )
ut_get( steadfast )
ut_get( not_living )
ut_get( can_advance )
ut_get( has_zoc )
ut_get( level )
ut_get( movement )
ut_get( cost )
ut_get( alignment )

#define ut_gs( x ) \
	{ #x,	(getter)wrapper_unittype_get_##x,	NULL,	NULL,	NULL },

static PyGetSetDef unittype_getseters[] = {
	ut_gs( name )
	ut_gs( max_unit_healing )
	ut_gs( heals )
	ut_gs( regenerates )
	ut_gs( is_leader )
	ut_gs( illuminates )
	ut_gs( is_skirmisher )
	ut_gs( teleports )
	ut_gs( steadfast )
	ut_gs( not_living )
	ut_gs( can_advance )
	ut_gs( has_zoc )
	ut_gs( level )
	ut_gs( movement )
	ut_gs( cost )
	ut_gs( alignment )
	{ NULL, NULL, NULL, NULL, NULL }
};

PyObject* python_ai::unittype_advances_to( wesnoth_unittype* type, PyObject* args )
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;

	PyObject* list = PyList_New(type->unit_type_->advances_to().size());
	int r;
	for (int advance = 0; advance < type->unit_type_->advances_to().size(); advance++)
	{
		std::map<std::string,unit_type>::const_iterator t = running_instance->get_info().gameinfo.unit_types.find(type->unit_type_->advances_to()[advance]);
		wassert(t != running_instance->get_info().gameinfo.unit_types.end());
		r = PyList_SetItem(list,advance,wrap_unittype(t->second));
	}
	return list;
}

static PyMethodDef unittype_methods[] = {
	{ "advances_to",	(PyCFunction)python_ai::unittype_advances_to,	METH_VARARGS},
	{ NULL, NULL, 0 }
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
	"Wesnoth unit types",					/* tp_doc */
	0,										/* tp_traverse */
	0,										/* tp_clear */
	0,										/* tp_richcompare */
	0,										/* tp_weaklistoffset */
	0,										/* tp_iter */
	0,										/* tp_iternext */
	unittype_methods,						/* tp_methods */
	0,										/* tp_members */
	unittype_getseters,						/* tp_getset */
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

static PyObject* attacktype_get_name(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("s",type->attack_type_->name().c_str());
}

static PyObject* attacktype_get_damage(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("i",type->attack_type_->damage());
}

static PyObject* attacktype_get_num_attacks(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("i",type->attack_type_->num_attacks());
}

static PyObject* attacktype_get_attack_weight(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("d",type->attack_type_->attack_weight());
}

static PyObject* attacktype_get_defense_weight(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("d",type->attack_type_->defense_weight());
}

static PyObject* attacktype_get_backstab(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("i",type->attack_type_->backstab());
}

static PyObject* attacktype_get_slow(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("i",type->attack_type_->slow());
}

static PyObject* attacktype_get_range(wesnoth_attacktype* type, void* closure)
{
	return Py_BuildValue("s",type->attack_type_->range().c_str());
}

static PyGetSetDef attacktype_getseters[] = {
	{ "name",			(getter)attacktype_get_name,			NULL,	NULL,	NULL },
	{ "damage",			(getter)attacktype_get_damage,			NULL,	NULL,	NULL },
	{ "num_attacks",    (getter)attacktype_get_num_attacks,		NULL,	NULL,	NULL },
	{ "attack_weight",  (getter)attacktype_get_attack_weight,	NULL,	NULL,	NULL },
	{ "defense_weight",	(getter)attacktype_get_defense_weight,	NULL,	NULL,	NULL },
	{ "backstab",		(getter)attacktype_get_backstab,		NULL,	NULL,	NULL },
	{ "slow",			(getter)attacktype_get_slow,			NULL,	NULL,	NULL },
	{ "range",			(getter)attacktype_get_range,			NULL,	NULL,	NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef attacktype_methods[] = {
	{ NULL, NULL, 0 }
};

static PyTypeObject wesnoth_attacktype_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.attacktype",        /* tp_name*/
	sizeof(wesnoth_attacktype),  /* tp_basicsize*/
	0,                         /* tp_itemsize*/
	0,                         /* tp_dealloc*/
	0,                         /* tp_print*/
	0,                         /* tp_getattr*/
	0,                         /* tp_setattr*/
	0,                         /* tp_compare*/
	0,                         /* tp_repr*/
	0, //UniConvert,             /* tp_as_number*/
	0,                         /* tp_as_sequence*/
	0,                         /* tp_as_mapping*/
	0,                         /* tp_hash */
	0,                         /* tp_call*/
	0,                         /* tp_str*/
	0,   /* tp_getattro*/
	0,   /* tp_setattro*/
	0,                         /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /* tp_flags*/
	"Wesnoth attack type",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	attacktype_methods,             /* tp_methods */
	0,                         /* tp_members */
	attacktype_getseters,          /* tp_getset */
};

static PyObject* wrap_attacktype(const attack_type& type)
{
	wesnoth_attacktype* attack;
	attack = (wesnoth_attacktype*)PyObject_NEW(wesnoth_attacktype, &wesnoth_attacktype_type);
	attack->attack_type_ = &type;
	return (PyObject*)attack;
}


bool python_ai::is_unit_valid(const unit* unit, bool do_set_error)
{
	if (!unit)
	{
		if (do_set_error)
			set_error("Undefined unit.");
		return false;
	}
	for(unit_map::const_iterator i = running_instance->get_info().units.begin(); i != running_instance->get_info().units.end(); ++i) {
		if (unit == &i->second)
			return true;
	}
	if (do_set_error)
		set_error("Unit was destroyed.");
	return false;
}

static PyObject* unit_get_name(wesnoth_unit* unit, void* closure)
{
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;
	return Py_BuildValue("s",( const char* )unit->unit_->name().c_str());
}

static PyObject* unit_is_enemy(wesnoth_unit* unit, void* closure)
{
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;
	return Py_BuildValue("i",running_instance->current_team().is_enemy(unit->unit_->side()) == true ? 1 : 0);
}

static PyObject* unit_can_recruit(wesnoth_unit* unit, void* closure)
{
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;
	return Py_BuildValue("i",unit->unit_->can_recruit() == true ? 1 : 0);
}

static PyObject* unit_query_valid(wesnoth_unit* unit, void* closure)
{
	return Py_BuildValue("i",running_instance->is_unit_valid(unit->unit_) == true ? 1 : 0);
}

static PyGetSetDef unit_getseters[] = {
	{ "name",			(getter)unit_get_name,		NULL,	NULL,	NULL },
	{ "is_enemy",		(getter)unit_is_enemy,		NULL,	NULL,	NULL },
	{ "can_recruit",	(getter)unit_can_recruit,	NULL,	NULL,	NULL },
	{ "is_valid",		(getter)unit_query_valid,	NULL,	NULL,	NULL },
	{ NULL,				NULL,						NULL,	NULL,	NULL }
};

static PyObject* wrapper_unit_type( wesnoth_unit* unit, PyObject* args )
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;
	return wrap_unittype(unit->unit_->type());
}

static PyObject* wrapper_unit_attacks( wesnoth_unit* unit, PyObject* args )
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;
	PyObject* list = PyList_New(unit->unit_->attacks().size());
	for ( int attack = 0; attack < unit->unit_->attacks().size(); attack++)
		PyList_SetItem(list,attack,wrap_attacktype(unit->unit_->attacks()[attack]));
	return (PyObject*)list;
}

static PyObject* wrapper_unit_movement_cost( wesnoth_unit* unit, PyObject* args );
static PyObject* wrapper_unit_defense_modifier( wesnoth_unit* unit, PyObject* args );

static PyObject* wrapper_unit_damage_against( wesnoth_unit* unit, PyObject* args )
{
	wesnoth_attacktype* attack;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_attacktype_type, &attack ) )
		return NULL;
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;
	return Py_BuildValue("i",unit->unit_->damage_against(*attack->attack_type_));
}

static PyMethodDef unit_methods[] = {
	{ "type",				(PyCFunction)wrapper_unit_type,				METH_VARARGS},
	{ "attacks",			(PyCFunction)wrapper_unit_attacks,			METH_VARARGS},
	{ "movement_cost",		(PyCFunction)wrapper_unit_movement_cost,	METH_VARARGS},
	{ "defense_modifier",	(PyCFunction)wrapper_unit_defense_modifier,	METH_VARARGS},
	{ "damage_against",		(PyCFunction)wrapper_unit_damage_against,	METH_VARARGS},
	{ "find_path",			(PyCFunction)python_ai::wrapper_unit_find_path,		METH_VARARGS},
	{ NULL,					NULL,										0 }
};

static int unit_internal_compare(wesnoth_unit* left, wesnoth_unit* right)
{
	return (long)left->unit_ - (long)right->unit_;
}

static PyTypeObject wesnoth_unit_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.unit",        /* tp_name*/
	sizeof(wesnoth_unit),  /* tp_basicsize*/
	0,                         /* tp_itemsize*/
	0,                         /* tp_dealloc*/
	0,                         /* tp_print*/
	0,                         /* tp_getattr*/
	0,                         /* tp_setattr*/
	(cmpfunc)unit_internal_compare,                         /* tp_compare*/
	0,                         /* tp_repr*/
	0, //UniConvert,             /* tp_as_number*/
	0,                         /* tp_as_sequence*/
	0,                         /* tp_as_mapping*/
	0,                         /* tp_hash */
	0,                         /* tp_call*/
	0,                         /* tp_str*/
	0,   /* tp_getattro*/
	0,   /* tp_setattro*/
	0,                         /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /* tp_flags*/
	"Wesnoth units",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	unit_methods,             /* tp_methods */
	0,                         /* tp_members */
	unit_getseters,          /* tp_getset */
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

static PyObject* location_get_x(wesnoth_location* location, void* closure)
{
	return Py_BuildValue("i", location->location_->x);
}
static PyObject* location_get_y(wesnoth_location* location, void* closure)
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
	return obj->location_->x * 1000 + obj->location_->y;
}

static PyGetSetDef location_getseters[] = {
	{ "x",       (getter)location_get_x,     NULL, NULL, NULL },
	{ "y",       (getter)location_get_y,     NULL, NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyObject* wrapper_location_adjacent_to( wesnoth_location* left, PyObject* args );
static PyObject* wrapper_location_distance_to( wesnoth_location* left, PyObject* args );
static PyMethodDef location_methods[] = {
	{ "adjacent_to",         (PyCFunction)wrapper_location_adjacent_to,       METH_VARARGS},
	{ "distance_to",         (PyCFunction)wrapper_location_distance_to,       METH_VARARGS},
    { NULL, NULL, 0 }
};

static PyTypeObject wesnoth_location_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.location",        /* tp_name*/
	sizeof(wesnoth_location),  /* tp_basicsize*/
	0,                         /* tp_itemsize*/
	(destructor)wesnoth_location_dealloc,                         /* tp_dealloc*/
	0,                         /* tp_print*/
	0,                         /* tp_getattr*/
	0,                         /* tp_setattr*/
	(cmpfunc)location_internal_compare,                         /* tp_compare*/
	0,                         /* tp_repr*/
	0, //UniConvert,             /* tp_as_number*/
	0,                         /* tp_as_sequence*/
	0,                         /* tp_as_mapping*/
	(hashfunc)location_internal_hash,                         /* tp_hash */
	0,                         /* tp_call*/
	0,                         /* tp_str*/
	0,   /* tp_getattro*/
	0,   /* tp_setattro*/
	0,                         /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /* tp_flags*/
	"Wesnoth location",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	location_methods,             /* tp_methods */
	0,                         /* tp_members */
	location_getseters,          /* tp_getset */
};

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

static PyObject* gamemap_get_x(wesnoth_gamemap* map, void* closure)
{
	return Py_BuildValue("i", map->map_->x());
}
static PyObject* gamemap_get_y(wesnoth_gamemap* map, void* closure)
{
	return Py_BuildValue("i", map->map_->y());
}

static PyGetSetDef gamemap_getseters[] = {
	{ "x",	(getter)gamemap_get_x,	NULL,	NULL,	NULL },
	{ "y",	(getter)gamemap_get_y,	NULL,	NULL,	NULL },
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
	{ "is_village",	(PyCFunction)wrapper_getmap_is_village,	METH_VARARGS},
	{ "is_keep",	(PyCFunction)wrapper_getmap_is_keep,	METH_VARARGS},
	{ "is_castle",	(PyCFunction)wrapper_getmap_is_castle,	METH_VARARGS},
	{ NULL, NULL, 0 }
};

static PyTypeObject wesnoth_gamemap_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.gamemap",        /* tp_name*/
	sizeof(wesnoth_gamemap),  /* tp_basicsize*/
	0,                         /* tp_itemsize*/
	0,                         /* tp_dealloc*/
	0,                         /* tp_print*/
	0,                         /* tp_getattr*/
	0,                         /* tp_setattr*/
	0,                         /* tp_compare*/
	0,                         /* tp_repr*/
	0, //UniConvert,             /* tp_as_number*/
	0,                         /* tp_as_sequence*/
	0,                         /* tp_as_mapping*/
	0,                         /* tp_hash */
	0,                         /* tp_call*/
	0,                         /* tp_str*/
	0,   /* tp_getattro*/
	0,							/* tp_setattro*/
	0,                         /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /* tp_flags*/
	"Wesnoth map information",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	gamemap_methods,             /* tp_methods */
	0,                         /* tp_members */
	gamemap_getseters,          /* tp_getset */
};

static PyObject* wrapper_team_name(wesnoth_team* team, void* closure)
{
	return Py_BuildValue("s", team->team_->team_name().c_str());
}

static PyObject* wrapper_team_income(wesnoth_team* team, void* closure)
{
	return Py_BuildValue("i", team->team_->income());
}

static PyObject* wrapper_team_gold(wesnoth_team* team, void* closure)
{
	return Py_BuildValue("i", team->team_->gold());
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

	PyObject* list = PyList_New(running_instance->current_team().recruits().size());
	int r;
	int idx = 0;
	for (std::set<std::string>::const_iterator recruit = running_instance->current_team().recruits().begin(); recruit != running_instance->current_team().recruits().end(); ++recruit)
	{
		std::map<std::string,unit_type>::const_iterator t = running_instance->get_info().gameinfo.unit_types.find(*recruit);
		wassert(t != running_instance->get_info().gameinfo.unit_types.end());
		r = PyList_SetItem(list,idx++,wrap_unittype(t->second));
	}
	return list;
}

static PyMethodDef team_methods[] = {
	{ "owns_village",         (PyCFunction)wrapper_team_owns_village,       METH_VARARGS},
	{ "recruits",         (PyCFunction)python_ai::wrapper_team_recruits,       METH_VARARGS},
    { NULL, NULL, 0 }
};

static PyGetSetDef team_getseters[] = {
	{ "name",	(getter)wrapper_team_name,		NULL,	NULL,	NULL },
	{ "gold",	(getter)wrapper_team_gold,		NULL,	NULL,	NULL },
	{ "income",	(getter)wrapper_team_income,	NULL,	NULL,	NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject wesnoth_team_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.team",        /* tp_name*/
	sizeof(wesnoth_team),  /* tp_basicsize*/
	0,                         /* tp_itemsize*/
	0,                         /* tp_dealloc*/
	0,                         /* tp_print*/
	0,                         /* tp_getattr*/
	0,                         /* tp_setattr*/
	(cmpfunc)wrapper_team_internal_compare,                         /* tp_compare*/
	0,                         /* tp_repr*/
	0, //UniConvert,             /* tp_as_number*/
	0,                         /* tp_as_sequence*/
	0,                         /* tp_as_mapping*/
	0,                         /* tp_hash */
	0,                         /* tp_call*/
	0,                         /* tp_str*/
	0,   /* tp_getattro*/
	0,   /* tp_setattro*/
	0,                         /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /* tp_flags*/
	"Wesnoth team",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	team_methods,             /* tp_methods */
	0,                         /* tp_members */
	team_getseters,          /* tp_getset */
};

static PyObject* wrapper_unit_movement_cost( wesnoth_unit* unit, PyObject* args )
{
	wesnoth_gamemap* map;
	wesnoth_location* loc;
	if ( !PyArg_ParseTuple( args, "O!O!", &wesnoth_gamemap_type, &map, &wesnoth_location_type, &loc ) )
		return NULL;
	return Py_BuildValue("i",unit->unit_->movement_cost(*map->map_,map->map_->get_terrain(*loc->location_)));
}

static PyObject* wrapper_unit_defense_modifier( wesnoth_unit* unit, PyObject* args )
{
	wesnoth_gamemap* map;
	wesnoth_location* loc;
	if ( !PyArg_ParseTuple( args, "O!O!", &wesnoth_gamemap_type, &map, &wesnoth_location_type, &loc ) )
		return NULL;
	return Py_BuildValue("i",unit->unit_->defense_modifier(*map->map_,map->map_->get_terrain(*loc->location_)));
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
	{ NULL,		NULL,	0 }
};

static PyObject* wrapper_gamestatus_turn(wesnoth_gamestatus* status, void* closure)
{
	return Py_BuildValue("i", status->status_->turn());
}

static PyObject* wrapper_gamestatus_number_of_turns(wesnoth_gamestatus* status, void* closure)
{
	return Py_BuildValue("i", status->status_->number_of_turns());
}

static PyObject* wrapper_gamestatus_lawful_bonus(wesnoth_gamestatus* status, void* closure)
{
	return Py_BuildValue("i", status->status_->get_time_of_day().lawful_bonus);
}

static PyObject* wrapper_gamestatus_previous_lawful_bonus(wesnoth_gamestatus* status, void* closure)
{
	return Py_BuildValue("i", status->status_->get_previous_time_of_day().lawful_bonus);
}

static PyGetSetDef gamestatus_getseters[] = {
	{ "turn",					(getter)wrapper_gamestatus_turn,					NULL,	NULL,	NULL },
	{ "number_of_turns",		(getter)wrapper_gamestatus_number_of_turns,			NULL,	NULL,	NULL },
	{ "lawful_bonus",			(getter)wrapper_gamestatus_lawful_bonus,			NULL,	NULL,	NULL },
	{ "previous_lawful_bonus",	(getter)wrapper_gamestatus_previous_lawful_bonus,	NULL,	NULL,	NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject wesnoth_gamestatus_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.team",        /* tp_name*/
	sizeof(wesnoth_gamestatus),  /* tp_basicsize*/
	0,                         /* tp_itemsize*/
	0,                         /* tp_dealloc*/
	0,                         /* tp_print*/
	0,                         /* tp_getattr*/
	0,                         /* tp_setattr*/
	0,                         /* tp_compare*/
	0,                         /* tp_repr*/
	0, //UniConvert,             /* tp_as_number*/
	0,                         /* tp_as_sequence*/
	0,                         /* tp_as_mapping*/
	0,                         /* tp_hash */
	0,                         /* tp_call*/
	0,                         /* tp_str*/
	0,   /* tp_getattro*/
	0,   /* tp_setattro*/
	0,                         /* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /* tp_flags*/
	"Wesnoth game status",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	gamestatus_methods,             /* tp_methods */
	0,                         /* tp_members */
	gamestatus_getseters,          /* tp_getset */
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
	}
	return dict;
}

PyObject* python_ai::wrapper_log_message(PyObject* self, PyObject* args)
{
	const char* msg;
    if ( !PyArg_ParseTuple( args, "s", &msg ) )
        return NULL;
	running_instance->log_message(msg);
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* python_ai::wrapper_get_units(PyObject* self, PyObject* args)
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

PyObject* python_ai::wrapper_get_location(PyObject* self, PyObject* args)
{
	int x, y;
    if ( !PyArg_ParseTuple( args, "ii", &x, &y ) )
        return NULL;

	return wrap_location(gamemap::location(x,y));
}

PyObject* python_ai::wrapper_get_map(PyObject* self, PyObject* args)
{
    if ( !PyArg_ParseTuple( args, "" ) )
        return NULL;
	wesnoth_gamemap* map = (wesnoth_gamemap*)PyObject_NEW(wesnoth_gamemap, &wesnoth_gamemap_type);
	map->map_ = &running_instance->get_info().map;
	return (PyObject*)map;
}

PyObject* python_ai::wrapper_get_teams(PyObject* self, PyObject* args)
{
    if ( !PyArg_ParseTuple( args, "" ) )
        return NULL;

	PyObject* list = PyList_New(running_instance->get_info().teams.size());
	wesnoth_team* the_team;

	for (int team = 0; team < running_instance->get_info().teams.size(); team++)
	{
		the_team = (wesnoth_team*)PyObject_NEW(wesnoth_team, &wesnoth_team_type);
		the_team->team_ = &running_instance->get_info().teams[team];
		PyList_SetItem(list,team,(PyObject*)the_team);
	}

	return list;
}

PyObject* python_ai::wrapper_get_current_team(PyObject* self, PyObject* args)
{
	wesnoth_team* the_team;
	the_team = (wesnoth_team*)PyObject_NEW(wesnoth_team, &wesnoth_team_type);
	the_team->team_ = &running_instance->current_team();
	return (PyObject*)the_team;
}

PyObject* python_ai::wrapper_get_src_dst(PyObject* self, PyObject* args)
{
	return wrap_move_map(running_instance->src_dst_);
}

PyObject* python_ai::wrapper_get_dst_src(PyObject* self, PyObject* args)
{
	return wrap_move_map(running_instance->dst_src_);
}

PyObject* python_ai::wrapper_get_enemy_src_dst(PyObject* self, PyObject* args)
{
	return wrap_move_map(running_instance->enemy_src_dst_);
}

PyObject* python_ai::wrapper_get_enemy_dst_src(PyObject* self, PyObject* args)
{
	return wrap_move_map(running_instance->enemy_dst_src_);
}

PyObject* python_ai::wrapper_move_unit(PyObject* self, PyObject* args)
{
	wesnoth_location* from;
	wesnoth_location* to;
	if ( !PyArg_ParseTuple( args, "O!O!", &wesnoth_location_type, &from, &wesnoth_location_type, &to ) )
		return NULL;

	PyObject* loc = wrap_location(running_instance->move_unit_partial(*from->location_,*to->location_,running_instance->possible_moves_));
	running_instance->calculate_possible_moves(running_instance->possible_moves_,running_instance->src_dst_,running_instance->dst_src_,false);
	running_instance->calculate_possible_moves(running_instance->enemy_possible_moves_,running_instance->enemy_src_dst_,running_instance->enemy_dst_src_,true);
	return loc;
}

PyObject* python_ai::wrapper_attack_unit(PyObject* self, PyObject* args)
{
	wesnoth_location* from;
	wesnoth_location* to;
	int weapon;
	if ( !PyArg_ParseTuple( args, "O!O!i", &wesnoth_location_type, &from, &wesnoth_location_type, &to, &weapon ) )
		return NULL;

	running_instance->attack_enemy(*from->location_,*to->location_,weapon);
	running_instance->calculate_possible_moves(running_instance->possible_moves_,running_instance->src_dst_,running_instance->dst_src_,false);
	running_instance->calculate_possible_moves(running_instance->enemy_possible_moves_,running_instance->enemy_src_dst_,running_instance->enemy_dst_src_,true);

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* python_ai::wrapper_get_adjacent_tiles(PyObject* self, PyObject* args)
{
	wesnoth_location* where;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &where ) )
		return NULL;

	PyObject* list = PyList_New(6);
	gamemap::location loc[6];
	get_adjacent_tiles(*where->location_,loc);
	for ( int tile = 0; tile < 6; tile++ )
		PyList_SetItem(list,tile,wrap_location(loc[tile]));
	return list;
}

PyObject* python_ai::wrapper_recruit_unit(PyObject* self, PyObject* args)
{
	wesnoth_location* where;
	const char* name;
	if ( !PyArg_ParseTuple( args, "sO!", &name, &wesnoth_location_type, &where ) )
		return NULL;
	return Py_BuildValue("i", running_instance->recruit(name,*where->location_) == true ? 1 : 0);
}

PyObject* python_ai::wrapper_get_gamestatus(PyObject* self, PyObject* args)
{
	if ( !PyArg_ParseTuple( args, "" ) )
		return NULL;
	wesnoth_gamestatus* status;
	status = (wesnoth_gamestatus*)PyObject_NEW(wesnoth_gamestatus, &wesnoth_gamestatus_type);
	status->status_ = &running_instance->get_info().state;
	return (PyObject*)status;
}

PyObject* python_ai::wrapper_unit_find_path( wesnoth_unit* unit, PyObject* args )
{
	wesnoth_location* from;
	wesnoth_location* to;
	double percent = 100.0;
	if ( !PyArg_ParseTuple( args, "O!O!|d", &wesnoth_location_type, &from, &wesnoth_location_type, &to, &percent ) )
		return NULL;
	if (!running_instance->is_unit_valid(unit->unit_))
		return NULL;

	const shortest_path_calculator calc(*unit->unit_,
		running_instance->current_team(),
		running_instance->get_info().units,
		running_instance->get_info().teams,
		running_instance->get_info().map,
		running_instance->get_info().state);
	const paths::route& route = a_star_search(*from->location_, *to->location_, percent, &calc, running_instance->get_info().map.x(), running_instance->get_info().map.y());

	PyObject* steps = PyList_New(route.steps.size());
	for (int step = 0; step < route.steps.size(); step++)
		PyList_SetItem(steps,step,wrap_location(route.steps[step]));

	return steps;
}

static PyMethodDef wesnoth_python_methods[] = {
	{ "log_message",				python_ai::wrapper_log_message,			METH_VARARGS},
	{ "get_units",					python_ai::wrapper_get_units,			METH_VARARGS},
	{ "get_location",				python_ai::wrapper_get_location,		METH_VARARGS},
	{ "get_map",					python_ai::wrapper_get_map,				METH_VARARGS},
	{ "get_teams",					python_ai::wrapper_get_teams,			METH_VARARGS},
	{ "get_current_team",			python_ai::wrapper_get_current_team,	METH_VARARGS},
	{ "get_destinations_by_unit",	python_ai::wrapper_get_src_dst,			METH_VARARGS},
	{ "get_units_by_destination",	python_ai::wrapper_get_dst_src,			METH_VARARGS},
	{ "move_unit",					python_ai::wrapper_move_unit,			METH_VARARGS},
	{ "attack_unit",				python_ai::wrapper_attack_unit,			METH_VARARGS},
	{ "get_adjacent_tiles",			python_ai::wrapper_get_adjacent_tiles,	METH_VARARGS},
	{ "recruit_unit",				python_ai::wrapper_recruit_unit,		METH_VARARGS},
	{ "get_gamestatus",				python_ai::wrapper_get_gamestatus,		METH_VARARGS},
	{ NULL,							NULL,									0 }
};


#define Py_Register( x, n ) \
	PyType_Ready(&x); \
	Py_INCREF(&x); \
	PyModule_AddObject(module, n, (PyObject*)&x);

python_ai::python_ai(ai_interface::info& info) : ai_interface(info)
{
	running_instance = this;
	if ( !init_ )
	{
		Py_Initialize( );
		PyObject* module = Py_InitModule("wesnoth", wesnoth_python_methods);
		Py_Register(wesnoth_unit_type, "unit");
		Py_Register(wesnoth_location_type, "location");
		Py_Register(wesnoth_gamemap_type, "gamemap");
		Py_Register(wesnoth_unittype_type, "unittype");
		Py_Register(wesnoth_team_type, "team");
		Py_Register(wesnoth_attacktype_type, "attacktype");
		Py_Register(wesnoth_gamestatus_type, "gamestatus");
		python_error_ = PyErr_NewException("wesnoth.error",NULL,NULL);
		PyDict_SetItemString(PyModule_GetDict(module),"error",python_error_);
		init_ = true;
	}
	calculate_possible_moves(possible_moves_,src_dst_,dst_src_,false);
}

python_ai::~python_ai()
	{
/*	Py_DECREF(&Wesnoth_UnitType);
	Py_DECREF(&wesnoth_location_type);
	Py_Finalize();*/
	running_instance = NULL;
	}

void python_ai::play_turn()
{
	std::string script = current_team().ai_parameters()["python_script"];
	PyObject* file = PyFile_FromString((char*)script.c_str(),"rt");
	PyRun_SimpleFile(PyFile_AsFile(file),(char*)script.c_str());
	if (PyErr_Occurred())
		PyErr_Print();
	Py_DECREF(file);
}

#endif // HAVE_PYTHON
