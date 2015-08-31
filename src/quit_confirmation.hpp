#pragma once
class CVideo;

class quit_confirmation
{
public:
	quit_confirmation() { ++count_; }
	quit_confirmation(const quit_confirmation&) { ++count_; }
	~quit_confirmation() { --count_; }
	static void quit();
	static void quit(CVideo& video );
private:
	static int count_;
	static bool open_;
};
