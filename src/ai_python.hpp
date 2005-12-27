#ifndef AI_PYTHON_HPP_INCLUDED
#define AI_PYTHON_HPP_INCLUDED

#ifdef HAVE_PYTHON

#include "ai_interface.hpp"
#include <Python.h>

typedef struct {
	PyObject_HEAD
	const unit_type* unit_type_;
} wesnoth_unittype;

typedef struct {
	PyObject_HEAD
	const team* team_;
} wesnoth_team;

typedef struct {
	PyObject_HEAD
	const unit* unit_;
} wesnoth_unit;

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
	static PyObject* wrapper_get_enemy_src_dst(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_enemy_dst_src(PyObject* self, PyObject* args);
	static PyObject* wrapper_move_unit(PyObject* self, PyObject* args);
	static PyObject* wrapper_attack_unit(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_adjacent_tiles(PyObject* self, PyObject* args);
	static PyObject* wrapper_recruit_unit(PyObject* self, PyObject* args);
	static PyObject* wrapper_get_gamestatus(PyObject* self, PyObject* args);

	static PyObject* unittype_advances_to( wesnoth_unittype* type, PyObject* args );
	static PyObject* wrapper_team_recruits( wesnoth_team* team, PyObject* args );
	static PyObject* wrapper_unit_find_path( wesnoth_unit* unit, PyObject* args );

	static void set_error(const char *fmt, ...);

	static bool is_unit_valid(const unit* unit, bool do_set_error = true);
protected:
	static bool init_;
	static PyObject* python_error_;
	ai_interface::move_map src_dst_;
	ai_interface::move_map dst_src_;
	std::map<location,paths> possible_moves_;
	ai_interface::move_map enemy_src_dst_;
	ai_interface::move_map enemy_dst_src_;
	std::map<location,paths> enemy_possible_moves_;
};

#endif // HAVE_PYTHON

#endif
