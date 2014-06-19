/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "global.hpp"

#include "clipboard.hpp"
#include <algorithm>

#include <SDL_events.h>
#include <SDL_version.h>

#if SDL_VERSION_ATLEAST(2,0,0)

#define CLIPBOARD_FUNCS_DEFINED

/*
 * Note SDL 2.0 has its own clipboard routines, but they don't support
 * different clipboards (yet).
 */

void copy_to_clipboard(const std::string& text, const bool)
{
	SDL_SetClipboardText(text.c_str());
}

std::string copy_from_clipboard(const bool)
{
	char* clipboard = SDL_GetClipboardText();
	if(!clipboard) {
		return std::string();
	}

	const std::string result(clipboard);
	SDL_free(clipboard);
	return result;
}

void handle_system_event(const SDL_Event& /*event*/)
{
}

#else

#if defined(_X11) && !defined(__APPLE__)

#define CLIPBOARD_FUNCS_DEFINED

#include "SDL_syswm.h"

#include <unistd.h>

/**
 The following are two classes which wrap the SDL's interface to X,
 including locking/unlocking, and which manage the atom internment.
 They exist mainly to make the actual clipboard code somewhat readable.
*/
class XHelper
{
private:
	XHelper() :
		wmInf_(),
		acquireCount_(0)
	{
		acquire();

		// Intern some atoms;
		const char* atoms[] = {
			"CLIPBOARD",
			"TEXT",
			"COMPOUND_TEXT",
			"UTF8_STRING",
			"WESNOTH_PASTE",
			"TARGETS"
		};

		XInternAtoms(dpy(), const_cast<char**>(reinterpret_cast<const char**>(atoms)), 6, false, atomTable_);

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


	Atom XA_CLIPBOARD() const
	{
		return atomTable_[0];
	}

	Atom XA_TEXT() const
	{
		return atomTable_[1];
	}

	Atom XA_COMPOUND_TEXT() const
	{
		return atomTable_[2];
	}

	Atom UTF8_STRING() const
	{
		return atomTable_[3];
	}

	Atom WES_PASTE() const
	{
		return atomTable_[4];
	}

	Atom XA_TARGETS() const
	{
		return atomTable_[5];
	}

	Display* dpy() const
	{
		return wmInf_.info.x11.display;
	}

	Window window() const
	{
		return wmInf_.info.x11.window;
	}

	void acquire()
	{
		++acquireCount_;
		if (acquireCount_ == 1) {
			SDL_VERSION  (&wmInf_.version);
			SDL_GetWMInfo(&wmInf_);

			wmInf_.info.x11.lock_func();
		}
	}

	void release()
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
 This means we are forced to use CurrentTime in many spots and
 are unable to perform many safety checks.
 Hence, the code below is not compliant to the ICCCM, and
 may occasionally suffer from race conditions if an X client
 is connected to the server over a slow/high-latency link.
 This implementation is also very minimal.
 The text is assumed to be reasonably small, as INCR transactions
 are not supported.
 MULTIPLE is not supported either.

 We provide UTF8_STRING, COMPOUND_TEXT, and TEXT,
 and try to grab all of them, plus STRING (which is latin1).
*/


/**
 We primarily. keep a copy of the string to response to data requests,
 but it also has an another function: in case we're both the source
 and destination, we just copy it across; this is so that we don't
 have to handle SelectionRequest events while waiting for SelectionNotify.
 To make this work, however, this gets cleared when we loose CLIPBOARD.
*/
static std::string clipboard_string;

/**
 The following string is used for the mouse selection aka PRIMARY
 Unix behavior is mouse selection is stored in primary
 active selection goes to CLIPBOARD.
*/
static std::string primary_string;

void handle_system_event(const SDL_Event& event)
{
	XEvent& xev = event.syswm.msg->event.xevent;
	if (xev.type == SelectionRequest) {
		UseX x11;

		// Since wesnoth does not notify us of selections,
		// we set both selection + clipboard when copying.
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
					const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(supported)), 4);
			}

			// The encoding of XA_TEXT and XA_COMPOUND_TEXT is not specified
			// by the ICCCM... So we assume wesnoth native/utf-8 for simplicity.
			// Modern apps are going to use UTF8_STRING anyway.
			if (xev.xselectionrequest.target == x11->XA_TEXT()
			    	|| xev.xselectionrequest.target == x11->XA_COMPOUND_TEXT()
			    	|| xev.xselectionrequest.target == x11->UTF8_STRING()) {

				responseEvent.xselection.property = xev.xselectionrequest.property;

				std::string& selection = (xev.xselectionrequest.selection == XA_PRIMARY) ?
					primary_string : clipboard_string;

				XChangeProperty(x11->dpy(), responseEvent.xselection.requestor,
					xev.xselectionrequest.property,
					xev.xselectionrequest.target, 8, PropModeReplace,
					reinterpret_cast<const unsigned char*>(selection.c_str()), selection.length());
			}

			XSendEvent(x11->dpy(), xev.xselectionrequest.requestor, False, NoEventMask,
			   &responseEvent);
		}
	}

	if (xev.type == SelectionClear) {
		//We no longer own the clipboard, don't try in-process C&P
		UseX x11;

		if(xev.xselectionclear.selection == x11->XA_CLIPBOARD()) {
			clipboard_string.clear();
		} else if(xev.xselectionclear.selection == XA_PRIMARY) {
			primary_string.clear();
		}
	}
}

void copy_to_clipboard(const std::string& text, const bool mouse)
{
	if (text.empty()) {
		return;
	}

	UseX x11;

	if(mouse) {
		primary_string = text;
		XSetSelectionOwner(x11->dpy(), XA_PRIMARY, x11->window(), CurrentTime);
	} else {
		clipboard_string = text;
		XSetSelectionOwner(x11->dpy(), x11->XA_CLIPBOARD(), x11->window(), CurrentTime);
	}
}

/**
 * Tries to grab a given target.
 * Returns true if successful, false otherwise.
 */
static bool try_grab_target(Atom source, Atom target, std::string& ret)
{
	UseX x11;

	// Cleanup previous data
	XDeleteProperty(x11->dpy(), x11->window(), x11->WES_PASTE());
	XSync          (x11->dpy(), False);

	//std::cout<<"We request target:"<<XGetAtomName(x11->dpy(), target)<<"\n";

	// Request information
	XConvertSelection(x11->dpy(), source, target,
	                  x11->WES_PASTE(), x11->window(), CurrentTime);

	// Wait (with timeout) for a response SelectionNotify
	for (int attempt = 0; attempt < 15; attempt++) {
		if (XPending(x11->dpy())) {
			XEvent selectNotify;
			while (XCheckTypedWindowEvent(x11->dpy(), x11->window(), SelectionNotify, &selectNotify)) {
				if (selectNotify.xselection.property == None)
					//Not supported. Say so.
					return false;
				else if (selectNotify.xselection.property == x11->WES_PASTE() &&
				         selectNotify.xselection.target   == target) {
					// The size
					unsigned long length = 0;
					unsigned char* data;

					// These 3 XGetWindowProperty returns but we don't use
					Atom         typeRet;
					int          formatRet;
					unsigned long remaining;

//					std::cout<<"Grab:"<<XGetAtomName(x11->dpy(), target)<<"\n";

					// Grab the text out of the property
					XGetWindowProperty(x11->dpy(), x11->window(),
					                   selectNotify.xselection.property,
					                   0, 65535/4, True, target,
					                   &typeRet, &formatRet, &length, &remaining, &data);

					if (data && length) {
						ret = reinterpret_cast<char*>(data);
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

	// Timed out -- return empty string
	return false;
}

std::string copy_from_clipboard(const bool mouse)
{
	// in-wesnoth copy-paste
	if(mouse && !primary_string.empty()) {
		return primary_string;
	}
	if (!mouse && !clipboard_string.empty()) {
		return clipboard_string;
	}

	UseX x11;

	std::string ret;
	const Atom& source = mouse ?  XA_PRIMARY : x11->XA_CLIPBOARD();

	if (try_grab_target(source, x11->UTF8_STRING(), ret))
		return ret;

	if (try_grab_target(source, x11->XA_COMPOUND_TEXT(), ret))
		return ret;

	if (try_grab_target(source, x11->XA_TEXT(), ret))
		return ret;

	if (try_grab_target(source, XA_STRING, ret)) 	// acroread only provides this
		return ret;


	return "";
}

#endif
#ifdef _WIN32
#include <windows.h>
#define CLIPBOARD_FUNCS_DEFINED

void handle_system_event(const SDL_Event& )
{}

void copy_to_clipboard(const std::string& text, const bool)
{
	if(text.empty())
		return;
	if(!OpenClipboard(NULL))
		return;
	EmptyClipboard();

	// Convert newlines
	std::string str;
	str.reserve(text.size());
	std::string::const_iterator last = text.begin();
	while(last != text.end()) {
		if(*last != '\n') {
			str.push_back(*last);
		} else {
			str.append("\r\n");
		}
		++last;
	}

	const HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, (str.size() + 1) * sizeof(TCHAR));
	if(hglb == NULL) {
		CloseClipboard();
		return;
	}
	char* const buffer = reinterpret_cast<char* const>(GlobalLock(hglb));
	strcpy(buffer, str.c_str());
	GlobalUnlock(hglb);
	SetClipboardData(CF_TEXT, hglb);
	CloseClipboard();
}

std::string copy_from_clipboard(const bool)
{
	if(!IsClipboardFormatAvailable(CF_TEXT))
		return "";
	if(!OpenClipboard(NULL))
		return "";

	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if(hglb == NULL) {
		CloseClipboard();
		return "";
	}
	char const * buffer = reinterpret_cast<char*>(GlobalLock(hglb));
	if(buffer == NULL) {
		CloseClipboard();
		return "";
	}

	// Convert newlines
	std::string str(buffer);
	str.erase(std::remove(str.begin(),str.end(),'\r'),str.end());

	GlobalUnlock(hglb);
	CloseClipboard();
	return str;
}

#endif

#ifdef __APPLE__
#define CLIPBOARD_FUNCS_DEFINED

#include <Carbon/Carbon.h>

void copy_to_clipboard(const std::string& text, const bool)
{
	std::string new_str;
	new_str.reserve(text.size());
	for (unsigned int i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			new_str.push_back('\r');
		} else {
			new_str.push_back(text[i]);
		}
	}
	OSStatus err = noErr;
	PasteboardRef clipboard;
	PasteboardSyncFlags syncFlags;
	CFDataRef textData = NULL;
	err = PasteboardCreate(kPasteboardClipboard, &clipboard);
	if (err != noErr) return;
	err = PasteboardClear(clipboard);
	if (err != noErr) return;
	syncFlags = PasteboardSynchronize(clipboard);
	if ((syncFlags&kPasteboardModified) && !(syncFlags&kPasteboardClientIsOwner)) return;
	textData = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)new_str.c_str(), text.size());
	PasteboardPutItemFlavor(clipboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), textData, 0);
}

std::string copy_from_clipboard(const bool)
{
	OSStatus err = noErr;
	PasteboardRef clipboard;
	PasteboardSyncFlags syncFlags;
	ItemCount count;
	err = PasteboardCreate(kPasteboardClipboard, &clipboard);
	if (err != noErr) return "";
	syncFlags = PasteboardSynchronize(clipboard);
	if (syncFlags&kPasteboardModified) return "";
	err = PasteboardGetItemCount(clipboard, &count);
	if (err != noErr) return "";
	for (UInt32 k = 1; k <= count; k++) {
		PasteboardItemID itemID;
		CFArrayRef flavorTypeArray;
		CFIndex flavorCount;
		err = PasteboardGetItemIdentifier(clipboard, k, &itemID);
		if (err != noErr) return "";
		err = PasteboardCopyItemFlavors(clipboard, itemID, &flavorTypeArray);
		if (err != noErr) return "";
		flavorCount = CFArrayGetCount(flavorTypeArray);
		for (CFIndex j = 0; j < flavorCount; j++) {
			CFStringRef flavorType;
			CFDataRef flavorData;
			CFIndex flavorDataSize;
			flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, j);
			if (UTTypeConformsTo(flavorType, CFSTR("public.utf8-plain-text"))) {
				err = PasteboardCopyItemFlavorData(clipboard, itemID, flavorType, &flavorData);
				if (err != noErr) {
					CFRelease(flavorTypeArray);
					return "";
				}
				flavorDataSize = CFDataGetLength(flavorData);
				std::string str;
				str.reserve(flavorDataSize);
				str.resize(flavorDataSize);
				CFDataGetBytes(flavorData, CFRangeMake(0, flavorDataSize), (UInt8 *)str.data());
				for (unsigned int i = 0; i < str.size(); ++i) {
					if (str[i] == '\r') str[i] = '\n';
				}
				return str;
			}
		}
	}
	return "";
}

void handle_system_event(const SDL_Event& /*event*/)
{
}

#endif

#ifndef CLIPBOARD_FUNCS_DEFINED

void copy_to_clipboard(const std::string& /*text*/, const bool)
{
}

std::string copy_from_clipboard(const bool)
{
	return "";
}

void handle_system_event(const SDL_Event& /*event*/)
{
}

#endif
#endif

