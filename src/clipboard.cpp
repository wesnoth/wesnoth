#include "global.hpp"

#include "clipboard.hpp"
#include "serialization/string_utils.hpp"
#if defined(_X11) && !defined(__APPLE__)

#define CLIPBOARD_FUNCS_DEFINED

#include <X11/Xlib.h>
#include <unistd.h> 
#include <iostream>

#include "SDL_syswm.h"

/**
 The following are two classes which wrap the SDL's interface to X, including 
 locking/unlocking, and which manage the atom internment. They exist mainly to make
 the actual clipboard code somewhat readable
*/
class XHelper
{
private:
	XHelper()
	{
		acquireCount_ = 0;
		acquire();
		
		//Intern some atoms;
		const char* atoms[] = {
			"CLIPBOARD",
			"TEXT",
			"COMPOUND_TEXT",
			"UTF8_STRING",
			"WESNOTH_PASTE",
			"TARGETS"
		};
		
		XInternAtoms(dpy(), (char**)atoms, 6, false, atomTable_);
		
		release();
	}
	
	static XHelper* s_instance_;
	
	SDL_SysWMinfo wmInf_;
	
	Atom          atomTable_[6];	
	int           acquireCount_;
public:
	static XHelper* instance()
	{
		if (!s_instance_)
			s_instance_ = new XHelper;
		return s_instance_;
	}


	Atom XA_CLIPBOARD()
	{
		return atomTable_[0];
	}
	
	Atom XA_TEXT()
	{
		return atomTable_[1];
	}
	
	Atom XA_COMPOUND_TEXT()
	{
		return atomTable_[2];
	}
	
	Atom UTF8_STRING()
	{
		return atomTable_[3];
	}
	
	Atom WES_PASTE()
	{
		return atomTable_[4];
	}
	
	Atom XA_TARGETS()
	{
		return atomTable_[5];
	}
	
	Display* dpy()
	{
		return wmInf_.info.x11.display;
	}
	
	Window window()
	{
		return wmInf_.info.x11.window;
	}
	
	void acquire(void)
	{
		++acquireCount_;
		if (acquireCount_ == 1) {
			SDL_VERSION  (&wmInf_.version);
			SDL_GetWMInfo(&wmInf_);
		
			wmInf_.info.x11.lock_func();
		}
	}
	
	void release(void)
	{
		--acquireCount_;
		if (acquireCount_ == 0)
			wmInf_.info.x11.unlock_func();
	}
};

XHelper* XHelper::s_instance_ = 0;

class UseX
{
public:
	UseX()
	{
		XHelper::instance()->acquire();
	}
	
	~UseX()
	{
		XHelper::instance()->release();
	}

	XHelper* operator->()
	{
		return XHelper::instance();
	}
};

/**
 Note: unfortunately, SDL does not keep track of event timestamps.
 This means we are forced to use CurrentTime in many spots and are 
 unable to perform many safety checks. Hence, the code below is 
 not compliant to the ICCCM, and may ocassionally suffer from
 race conditions if an X client is connected to the server over 
 a slow/high-latency link. This implementation is also very minimal. 
 The text is assumed to be reasonably small as INCR transactions are not 
 supported. MULTIPLE is not supported either.
 
 We provide UTF8_STRING, COMPOUND_TEXT, and TEXT, and 
 try to grab all of them, plus STRING (which is latin1).
*/


/**
 We primarily. keep a copy of the string to response to data requests, 
 but it also has an another function: in case we're both the source
 and destination, we just copy it accross; this is so that we 
 don't have to handle SelectionRequest events while waiting for SelectionNotify.
 To make this work, however, this gets cleared when we loose CLIPBOARD
*/
static std::string clipboard_string;


void handle_system_event(const SDL_Event& event)
{
	XEvent& xev = event.syswm.msg->event.xevent;
	if (xev.type == SelectionRequest) {
		UseX x11;
		
		//Since wesnoth does not notify us of selections,
		//we set both selection + clipboard when copying.
		if ((xev.xselectionrequest.owner     == x11->window()) &&
		    ((xev.xselectionrequest.selection == XA_PRIMARY) || 
		     (xev.xselectionrequest.selection == x11->XA_CLIPBOARD()))) {
			XEvent responseEvent;
			responseEvent.xselection.type      = SelectionNotify;
			responseEvent.xselection.display   = x11->dpy();
			responseEvent.xselection.requestor = xev.xselectionrequest.requestor;
			responseEvent.xselection.selection = xev.xselectionrequest.selection;
			responseEvent.xselection.target    = xev.xselectionrequest.target;
			responseEvent.xselection.property  = None; //nothing available, by default
			responseEvent.xselection.time      = xev.xselectionrequest.time;
			
			//std::cout<<"Request for target:"<<XGetAtomName(x11->dpy(), xev.xselectionrequest.target)<<"\n";
			
			//### presently don't handle XA_STRING as it must be latin1
			
			if (xev.xselectionrequest.target == x11->XA_TARGETS()) {
				responseEvent.xselection.property = xev.xselectionrequest.property;
				
				Atom supported[] = {
					x11->XA_TEXT(),
					x11->XA_COMPOUND_TEXT(),
					x11->UTF8_STRING(),
					x11->XA_TARGETS()
				};
				
				XChangeProperty(x11->dpy(), responseEvent.xselection.requestor,
					xev.xselectionrequest.property, XA_ATOM, 32, PropModeReplace,
					(unsigned char*)supported, 4);
			}
			
			//The encoding of XA_TEXT and XA_COMPOUND_TEXT is not specified
			//by the ICCCM... So we assume wesnoth native/utf-8 for simplicity.
			//modern apps are going to use UTF8_STRING anyway
			if (xev.xselectionrequest.target == x11->XA_TEXT() ||
			    xev.xselectionrequest.target == x11->XA_COMPOUND_TEXT() || 
			    xev.xselectionrequest.target == x11->UTF8_STRING()) {
				responseEvent.xselection.property = xev.xselectionrequest.property;
				
				XChangeProperty(x11->dpy(), responseEvent.xselection.requestor,
					xev.xselectionrequest.property,
					xev.xselectionrequest.target, 8, PropModeReplace, 
					(const unsigned char*) clipboard_string.c_str(), clipboard_string.length());
			}
			
			XSendEvent(x11->dpy(), xev.xselectionrequest.requestor, False, NoEventMask,
			   &responseEvent);
		}
	} 
	
	if (xev.type == SelectionClear) {
		UseX x11;
		
		if (xev.xselectionclear.selection == x11->XA_CLIPBOARD())
			clipboard_string = ""; //We no longer own the clipboard, don't try in-process C&P
	}
}

void copy_to_clipboard(const std::string& text)
{
	if (text.empty()) {
		return;
	}

	clipboard_string = text;

	UseX x11;
	
	XSetSelectionOwner(x11->dpy(), XA_PRIMARY, x11->window(), CurrentTime);
	XSetSelectionOwner(x11->dpy(), x11->XA_CLIPBOARD(), x11->window(), CurrentTime);
}


//Tries to grab a given target. Returns true if successful, false otherwise
static bool try_grab_target(Atom target, std::string& ret)
{
	UseX x11;
	
	//Cleanup previous data
	XDeleteProperty(x11->dpy(), x11->window(), x11->WES_PASTE());
	XSync          (x11->dpy(), False);
	
	//std::cout<<"We request target:"<<XGetAtomName(x11->dpy(), target)<<"\n";
	
	//Request information
	XConvertSelection(x11->dpy(), x11->XA_CLIPBOARD(), target, 
	                  x11->WES_PASTE(), x11->window(), CurrentTime);
			  
	//Wait (with timeout) for a response SelectionNotify
	for (int attempt = 0; attempt < 15; attempt++) {
		if (XPending(x11->dpy())) {
			XEvent selectNotify;
			while (XCheckTypedWindowEvent(x11->dpy(), x11->window(), SelectionNotify, &selectNotify)) {
				if (selectNotify.xselection.property == None)
					//Not supported. Say so.
					return false;
				else if (selectNotify.xselection.property == x11->WES_PASTE() &&
				         selectNotify.xselection.target   == target) {
					//The size
					unsigned long length = 0;
					unsigned char* data;
					
					//these 3 XGetWindowProperty returns but we don't use
					Atom         typeRet;
					int          formatRet;
					unsigned long remaining;
					
//					std::cout<<"Grab:"<<XGetAtomName(x11->dpy(), target)<<"\n";
					
					//Grab the text out of the property
					XGetWindowProperty(x11->dpy(), x11->window(),
					                   selectNotify.xselection.property,
					                   0, 65535/4, True, target, 
					                   &typeRet, &formatRet, &length, &remaining, &data);
							   
					if (data && length) {
						ret = (char*)data;
						XFree(data);
						return true;
					} else {
						return false;
					}
				}
			}
		}
		
		usleep(10000);
	}
	
	//Timed out -- return empty string
	return false;
}

std::string copy_from_clipboard()
{
	if (!clipboard_string.empty())
		return clipboard_string; //in-wesnoth copy-paste

	std::string ret;
	
	UseX x11;
	
	if (try_grab_target(x11->UTF8_STRING(), ret))
		return ret;
		
	if (try_grab_target(x11->XA_COMPOUND_TEXT(), ret))
		return ret;
		
	if (try_grab_target(x11->XA_TEXT(), ret))
		return ret;
		
	if (try_grab_target(XA_STRING, ret)) //acroread only provides this
		return ret;

	
	return "";
}

#endif
#ifdef WIN32
#include <windows.h>

#define CLIPBOARD_FUNCS_DEFINED

void handle_system_event(const SDL_Event& )
{}

void copy_to_clipboard(const std::string& text)
{
	if(text.empty()) {
		return;
	}
	

	if(!OpenClipboard(0)) {
		return;
	}

	EmptyClipboard();

	const HGLOBAL clip_buffer = GlobalAlloc(GMEM_DDESHARE,text.size()+1);
	char* const buffer = reinterpret_cast<char*>(GlobalLock(clip_buffer));
	strcpy(buffer,text.c_str());

	GlobalUnlock(clip_buffer);
	SetClipboardData(CF_TEXT,clip_buffer);
	CloseClipboard();
}

std::string copy_from_clipboard()
{
	if(OpenClipboard(NULL)) {
		const HANDLE data = GetClipboardData(CF_TEXT);
		char* const buffer = reinterpret_cast<char*>(GlobalLock(data));
		GlobalUnlock(data);
		CloseClipboard();

		return buffer;
	}

	return "";
}

#endif

#ifdef __BEOS__
#include <Clipboard.h>

#define CLIPBOARD_FUNCS_DEFINED

void copy_to_clipboard(const std::string& text)
{
	BMessage *clip;
	if (be_clipboard->Lock())
	{
		be_clipboard->Clear();
		if ((clip = be_clipboard->Data()))
		{
			clip->AddData("text/plain", B_MIME_TYPE, text.c_str(), text.size()+1);
			be_clipboard->Commit();
		}
		be_clipboard->Unlock();
	}
}

std::string copy_from_clipboard()
{
	const char* data;
	ssize_t size;
	BMessage *clip = NULL;
	if (be_clipboard->Lock())
	{
		clip = be_clipboard->Data();
		be_clipboard->Unlock();
	}
	if (clip != NULL && clip->FindData("text/plain", B_MIME_TYPE, (const void**)&data, &size) == B_OK)
		return (const char*)data;
	else
		return "";
}
#endif

#ifdef __APPLE__
#define CLIPBOARD_FUNCS_DEFINED

#include <Carbon/Carbon.h>
void copy_ucs2_to_clipboard(const ucs2_string& text) {
	ucs2_string str;
	str.reserve(text.size() + 1);
	for(int i = 0; i < text.size(); ++i) {
		if(text[i] == '\n')
			str.push_back('\r');
		else
			str.push_back(text[i]);
	}
	str.push_back(0);
	OSStatus err = noErr;
	ScrapRef scrap = kScrapRefNone;
	err = ClearCurrentScrap();
	if(err != noErr)
		return;
	err = GetCurrentScrap(&scrap);
	if(err != noErr)
		return;
	PutScrapFlavor(scrap, kScrapFlavorTypeUnicode, kScrapFlavorMaskNone, str.size(), &str.front());
}

ucs2_string copy_ucs2_from_clipboard() {
	ucs2_string str;
	OSStatus err = noErr;
	ScrapRef scrap = kScrapRefNone;
	err = GetCurrentScrap(&scrap);
	if(err != noErr)
		return str;
	Size scrapsize;
	err = GetScrapFlavorSize(scrap, kScrapFlavorTypeUnicode, &scrapsize);
	if(err != noErr)
		return str;
	str.reserve(scrapsize);
	str.resize(scrapsize);
	err = GetScrapFlavorData(scrap, kScrapFlavorTypeUnicode, &scrapsize, const_cast<Uint16*>(&str.front()));
	if(err != noErr)
		return str.clear();
	for(int i = 0; i < str.size(); ++i) {
		if(str[i] == '\r')
			str[i] = '\n';
	}
	return str;
}

void copy_to_clipboard(const std::string& text)
{
	std::string new_str;
	new_str.reserve(text.size());
	for (int i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			new_str.push_back('\r');
		} else {
			new_str.push_back(text[i]);
		}
	}
	OSStatus err = noErr;
	ScrapRef scrap = kScrapRefNone;
	err = ClearCurrentScrap();
	if (err != noErr) return;
	err = GetCurrentScrap(&scrap);
	if (err != noErr) return;
	PutScrapFlavor(scrap, kScrapFlavorTypeText, kScrapFlavorMaskNone, text.size(), new_str.c_str());
}

std::string copy_from_clipboard()
{
	ScrapRef curscrap = kScrapRefNone;
	Size scrapsize = 0;
	OSStatus err = noErr;
	err = GetCurrentScrap(&curscrap);
	if (err != noErr) return "";
	err = GetScrapFlavorSize(curscrap, kScrapFlavorTypeText, &scrapsize);
	if (err != noErr) return "";
	std::string str;
	str.reserve(scrapsize);
	str.resize(scrapsize);
	err = GetScrapFlavorData(curscrap, kScrapFlavorTypeText, &scrapsize, const_cast<char*>(str.data()));
	if (err != noErr) return "";
	for (int i = 0; i < str.size(); ++i)
	{
		if (str[i] == '\r') str[i] = '\n';
	}
	return str;
}

void handle_system_event(const SDL_Event& event)
{
}

#endif

#ifndef CLIPBOARD_FUNCS_DEFINED

void copy_to_clipboard(const std::string& text)
{
}

std::string copy_from_clipboard()
{
	return "";
}

void handle_system_event(const SDL_Event& event)
{
}

#endif

