#ifndef INPUT_STREAM_HPP_INCLUDED
#define INPUT_STREAM_HPP_INCLUDED

#include <deque>
#include <string>

class input_stream
{
public:
	input_stream(const std::string& path);
	~input_stream();

	bool read_line(std::string& str);

private:
	input_stream(const input_stream&);
	void operator=(const input_stream&);

	int fd_;
	std::string path_;
	std::deque<char> data_;
};

#endif
