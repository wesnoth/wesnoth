#ifndef AI_PTYHON_HPP_INCLUDED
#define AI_PTYHON_HPP_INCLUDED

#include "ai_interface.hpp"
#include <Python.h>

typedef struct {
	PyObject_HEAD
} wesnoth_gamemap;

class python_ai : public ai_interface
{
public:
	python_ai(ai_interface::info& info);
	virtual ~python_ai();
	virtual void play_turn();

	static PyObject* wrapper_get_units(PyObject* self, PyObject* args);
	static PyObject* wrapper_log_message(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_location(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_map(PyObject* self, PyObject* args);

	static PyObject* python_ai::wesnoth_getmap_is_village( wesnoth_gamemap* map, PyObject* args );
	static PyObject* python_ai::wesnoth_getmap_is_keep( wesnoth_gamemap* map, PyObject* args );

protected:
	static bool init_;
	std::vector<gamemap::location*> wrapped_location_;
};


#endif
