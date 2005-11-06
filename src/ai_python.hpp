#ifndef AI_PTYHON_HPP_INCLUDED
#define AI_PTYHON_HPP_INCLUDED

#include "ai_interface.hpp"
#include <Python.h>

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
	static PyObject* wrapper_get_teams(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_current_team(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_src_dst(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_dst_src(PyObject* self, PyObject* args);
	static PyObject* wrapper_move_unit(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_adjacent_tiles(PyObject* self, PyObject* args);

protected:
	static bool init_;
	ai_interface::move_map src_dst_;
	ai_interface::move_map dst_src_;
	std::map<location,paths> possible_moves_;
};


#endif
