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
- display, which I don't think has to be exposed to Python. This is used by C++ AIs mostly for
  debugging purposes. We might want to allow Python scripts to write messages to the display for
  debugging purposes also, but they would only need very limited access.
- gamemap, defined in map.hpp, which contains the definition of the map. Definitely needs to be
 exposed to Python.
- game_data, defined in unit_types.hpp, which contains definitions of the available unit types,
 attacks, races, and so forth. Needs to be exposed to Python.
* unit_map, a map<gamemap::location,unit>. gamemap::location is defined in map.hpp and unit is
 defined in unit.hpp. This contains all the units currently in the game. Will need to be exposed.
- vector<team>, a listing of the teams (i.e. sides) in the game. Defined in team.hpp. Will
 need to be exposed.
- gamestatus, the current turn and time of day, etc. Defined in gamestatus.hpp. Will need to
 be exposed.
- turn_info, does not need to be exposed.

Additionally, useful utility functions such as those found in pathutils.hpp should be exposed. 

*/

#include "global.hpp"

#include "ai.hpp"
#include "ai_python.hpp"

static python_ai* running_instance;
bool python_ai::init_ = false;

typedef struct {
	PyObject_HEAD
	const unit* unit_;
} wesnoth_unit;

static PyObject* unit_get_name(wesnoth_unit* unit, void* closure)
{
	return Py_BuildValue("s",( const char* )unit->unit_->name().c_str());
}

static PyGetSetDef unit_getseters[] = {
	{ "name",       (getter)unit_get_name,     NULL, NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef unit_methods[] = {
	{ NULL, NULL }
};

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
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags*/
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
	const gamemap::location* location_;
} wesnoth_location;

static PyObject* location_get_x(wesnoth_location* location, void* closure)
{
	return Py_BuildValue("i", location->location_->x);
}
static PyObject* location_get_y(wesnoth_location* location, void* closure)
{
	return Py_BuildValue("i", location->location_->y);
}

static PyGetSetDef location_getseters[] = {
	{ "x",       (getter)location_get_x,     NULL, NULL, NULL },
	{ "y",       (getter)location_get_y,     NULL, NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef location_methods[] = {
    { NULL, NULL }
};

static PyTypeObject wesnoth_location_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size*/
	"wesnoth.location",        /* tp_name*/
	sizeof(wesnoth_location),  /* tp_basicsize*/
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
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags*/
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


static PyGetSetDef gamemap_getseters[] = {
	{ NULL, NULL, NULL, NULL, NULL }
};

PyObject* python_ai::wesnoth_getmap_is_village( wesnoth_gamemap* map, PyObject* args )
{
	wesnoth_location* location;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &location ) )
		return NULL;
	return Py_BuildValue("i", running_instance->get_info().map.is_village(*location->location_) ? 1 : 0);
}

PyObject* python_ai::wesnoth_getmap_is_keep( wesnoth_gamemap* map, PyObject* args )
{
	wesnoth_location* location;
	if ( !PyArg_ParseTuple( args, "O!", &wesnoth_location_type, &location ) )
		return NULL;
	return Py_BuildValue("i", running_instance->get_info().map.is_keep(*location->location_) ? 1 : 0);
}

static PyMethodDef gamemap_methods[] = {
	{ "is_village",         (PyCFunction)python_ai::wesnoth_getmap_is_village,       METH_VARARGS},
	{ "is_keep",         (PyCFunction)python_ai::wesnoth_getmap_is_keep,       METH_VARARGS},
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
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags*/
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
/*	0,                         /* tp_base *
    0,                         /* tp_dict *
    0,                         /* tp_descr_get *
    0,                         /* tp_descr_set *
    0,                         /* tp_dictoffset *
    (initproc)wesnoth_unit_init,                         /* tp_init *
    0,                         /* tp_alloc *
    0,      /* tp_new */
};



PyObject* python_ai::wrapper_log_message(PyObject* self, PyObject* args)
{
	const char* msg;
    if ( !PyArg_ParseTuple( args, "s", &msg ) )
        return NULL;
	running_instance->diagnostic(msg);
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

	wesnoth_location* key;
	wesnoth_unit* unit;
	int ret;

	for(unit_map::const_iterator i = running_instance->get_info().units.begin(); i != running_instance->get_info().units.end(); ++i) {
		key = (wesnoth_location*)PyObject_NEW(wesnoth_location, &wesnoth_location_type);
		key->location_ = &i->first;
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
    if ( !PyArg_ParseTuple( args, "(ii)", &x, &y ) )
        return NULL;

	location* wrap = new gamemap::location();
	wrap->x = x;
	wrap->y = y;
	running_instance->wrapped_location_.push_back(wrap);
	wesnoth_location* loc = (wesnoth_location*)PyObject_NEW(wesnoth_location, &wesnoth_location_type);
	loc->location_ = wrap;
	return (PyObject*)loc;
}

PyObject* python_ai::wrapper_get_map(PyObject* self, PyObject* args)
{
	wesnoth_gamemap* loc = (wesnoth_gamemap*)PyObject_NEW(wesnoth_gamemap, &wesnoth_gamemap_type);
	return (PyObject*)loc;
}

static PyMethodDef wesnoth_python_methods[] = {
	{"log_message",		python_ai::wrapper_log_message,		METH_VARARGS},
	{"get_units",		python_ai::wrapper_get_units,		METH_VARARGS},
	{"get_location",	python_ai::wrapper_get_location,			METH_VARARGS},
	{"get_map",			python_ai::wrapper_get_map,			METH_VARARGS},
	{NULL, NULL}
};

python_ai::python_ai(ai_interface::info& info) : ai_interface(info)
{
	running_instance = this;
	if ( !init_ )
	{
		PyObject* module;
		Py_Initialize( );
		module = Py_InitModule("wesnoth", wesnoth_python_methods);
		PyType_Ready(&wesnoth_unit_type);
		Py_INCREF(&wesnoth_unit_type);
		PyType_Ready(&wesnoth_location_type);
		Py_INCREF(&wesnoth_location_type);
		PyType_Ready(&wesnoth_gamemap_type);
		Py_INCREF(&wesnoth_gamemap_type);
		PyModule_AddObject(module, "unit", (PyObject*)&wesnoth_unit_type);
		PyModule_AddObject(module, "location", (PyObject*)&wesnoth_location_type);
		PyModule_AddObject(module, "gamemap", (PyObject*)&wesnoth_gamemap_type);
		init_ = true;
	}
}

python_ai::~python_ai()
	{
/*	Py_DECREF(&Wesnoth_UnitType);
	Py_DECREF(&wesnoth_location_type);
	Py_Finalize();*/
	for ( int i = wrapped_location_.size( ) - 1; i >= 0; i-- )
	{
		delete wrapped_location_[ i ];
		wrapped_location_.pop_back( );
	}
	running_instance = NULL;
	}

void python_ai::play_turn()
{
	//PyRun_SimpleString("import Wesnoth\nWesnoth.LogMessage('test')");
/*	int result = PyRun_SimpleString("import Wesnoth\n"
		"units = Wesnoth.GetUnits()\n"
		"Wesnoth.LogMessage('%s units'%len(units))\n"
		"for i in [1..len(units)]:\n"
		"\tu = units.get(i)\n");/*
		"\tWesnoth.LogMessage('%s'%(u.Name))\n");
/*		"for u in units:\n"
		"\tWesnoth.LogMessage('%s'%(u.Name))\n");
		"Wesnoth.LogMessage('test1')\n");
	PyObject* error = PyErr_Occurred();*/
	PyObject* file = PyFile_FromString("c:\\w.py","rt");
	//FILE* f = fopen("c:\\w.py","rt");
	PyRun_SimpleFile(PyFile_AsFile(file),"c:\\w.py");
	Py_DECREF(file);
}
