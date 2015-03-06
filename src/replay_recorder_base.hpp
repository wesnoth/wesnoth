#pragma once
#include <cassert>
#include <boost/ptr_container/ptr_vector.hpp>

#include <config.hpp>

class config_writer;
class replay_recorder_base
{
public:
	replay_recorder_base();
	~replay_recorder_base();

	int get_pos() const;

	int size() const;

	config& get_command_at(int pos);

	config& add_child();

	config& get_upload_log();

	void remove_command(int index);

	config& insert_command(int index);

	void set_to_end();

	void set_pos(int pos);

	void append_config(const config& data);

	void write(config_writer& out) const;

	void write(config& out) const;
protected:
	config upload_log_;
	boost::ptr_vector<config> commands_;
	int pos_;
};
