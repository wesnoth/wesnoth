#pragma once
class CVideo;

/// Any object of this type will prevent the game from quitting immediately, instad a confirmation dialog will pop up when attepmting to close. 
class quit_confirmation
{
public:
	quit_confirmation() { ++count_; }
	quit_confirmation(const quit_confirmation&) { ++count_; }
	~quit_confirmation() { --count_; }
	/// Shows the quit confirmation if needed, throws CVideo::quit to exit.
	static void quit();
	static void quit(CVideo& video );
private:
	static int count_;
	static bool open_;
};
