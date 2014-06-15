
#include<vector>
/*
	Automaticly registrates itself in the registry in the constructor.
*/
class syncmp_handler
{
public:
	syncmp_handler();
	virtual void pull_remote_choice() = 0;
	virtual void send_user_choice() = 0;
	virtual ~syncmp_handler();
};

class syncmp_registry
{
public:
	//called by get_user_choice while waiting for a remote user choice.
	static void pull_remote_choice();
	//called when get_user_choice was called and the client wants to send the choice to the other clients immideately
	static void send_user_choice();
private:
	friend class syncmp_handler;
	typedef std::vector<syncmp_handler*> t_handlers;
	static void remove_handler(syncmp_handler* handler);
	static void add_handler(syncmp_handler* handler);
	static t_handlers& handlers();
};
