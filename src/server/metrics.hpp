#ifndef METRICS_HPP_INCLUDED
#define METRICS_HPP_INCLUDED

#include <iosfwd>

class metrics
{
public:
	metrics();

	void service_request();
	void no_requests();

	friend std::ostream& operator<<(std::ostream& out, metrics& met);

private:
	int most_consecutive_requests_;
	int current_requests_;
	int nrequests_;
	int nrequests_waited_;
	const time_t started_at_;
};

std::ostream& operator<<(std::ostream& out, metrics& met);

#endif
