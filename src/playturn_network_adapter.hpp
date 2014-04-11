

#ifndef PLAYTURN_NETWORK_ADAPTER_HPP_INCLUDED
#define PLAYTURN_NETWORK_ADAPTER_HPP_INCLUDED

#include "config.hpp"
#include <list>
#include <boost/function.hpp>

/*
	The purpose if this class is to preprocess incoming network data, and provide a steam that always returns just one command/action at a time.
	Especialy we want each replay command in his own [turn].
*/
class playturn_network_adapter
{
public:
	typedef boost::function1<bool, config&> source_type; 
	
	playturn_network_adapter(source_type source = read_network);
	~playturn_network_adapter();

	//returns true on success.
	//dst has to be empty befor the call.
	//after the call dst contains one child chen returned true otherise it's empty.
	bool read(config& dst);
	//returns false if there is still data in the internal buffer.
	bool is_at_end();
	void set_source(source_type source);
	//returns a function to be passed to set_source.
	static source_type get_source_from_config(config& src);
	//a function to be passed to set_source.
	static bool read_network(config& dst);
private:
	//reads data from the network stream.
	void read_from_network();
	//this always contains one empty config becasue we want a vaid value for next_.
	std::list<config> data_;
	//the position of the next to be received element in data_->front().
	config::all_children_iterator next_;
	//if we are processing a [turn] with mutiple [command] we want to split them. 
	//In this case next_command_num_ is the next to be processed turn into a command otherwise it's 0;
	unsigned int next_command_num_;
	//a function to receive data from the network.
	source_type network_reader_;
};
#endif
