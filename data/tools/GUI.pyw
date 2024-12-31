#!/usr/bin/env python3
# encoding: utf-8

# By Elvish_Hunter, April 2014

# You may notice that this script, unlike all our other Python mainline scripts,
# has a .pyw extension, instead of .py. *This is deliberate*. On Windows, .pyw
# scripts are started directly in GUI mode, without opening a prompt.
# This is, after all, the behavior that we want.

# threading and subprocess are needed to run wmllint without freezing the window
# codecs is used to save files as UTF8
# locale and gettext provides internationalization and localization (i18n, l10n)
# queue is needed to exchange information between threads
# if we use the run_tool thread to do GUI stuff we obtain weird crashes
# This happens because Tk is a single-thread GUI

import argparse
import codecs
import gettext
import locale
import os
import queue
import subprocess
import sys
import threading

# tkinter modules
import tkinter.font as font
from tkinter import *
from tkinter.filedialog import *
from tkinter.messagebox import *
from tkinter.messagebox import WARNING  # for Python >= 3.9
# ttk must be called last
from tkinter.ttk import *

from wesnoth import version

# check if additional themes are available and set a flag
# use pip or your package manager to install the "ttkthemes" package
try:
    from ttkthemes import ThemedStyle
    additional_themes = True
except ImportError:
    additional_themes = False

# we need to know in what series we are
# so set it in a constant string
WESNOTH_SERIES = "{0}.{1}".format(version.major, version.minor)

# get the location where the script is placed
# we'll check later if this is a Wesnoth directory
# and use it to generate the command lines
# os.path.realpath gets the full path of this script,
# while removing any symlink
# This allows users to create a link to the app on their desktop
# os.path.normpath allows Windows users to see their standard path separators
APP_DIR, APP_NAME = os.path.split(os.path.realpath(sys.argv[0]))
WESNOTH_ROOT_DIR = os.sep.join(APP_DIR.split(os.sep)[:-2])  # pop out "data" and "tools"
WESNOTH_DATA_DIR = os.path.join(WESNOTH_ROOT_DIR, "data")
WESNOTH_CORE_DIR = os.path.normpath(os.path.join(WESNOTH_DATA_DIR, "core"))
WESNOTH_TRAN_DIR = os.path.join(WESNOTH_ROOT_DIR, "translations")

_ = lambda x: x


def set_global_locale():
    """
    Attempts to set locale for the application session based on user-input at the command-line. If no input is given,
    fall back to the system locale.
    """
    global _

    # TODO: Replace CLI args for a proper locale selection GUI.
    # More importantly, code to dynamically update the text/layout is missing.
    parser = argparse.ArgumentParser(
        description=_("Open a Graphical User Interface (GUI) to WML Tools")
    )
    parser.add_argument(
        '--lang',
        help=_("Launch GUI.pyw in the specified language. Language code is expected as a POSIX locale name, refer to "
               "gettext.wesnoth.org for a full list.")
    )

    if not os.path.isdir(WESNOTH_TRAN_DIR):
        showerror(
            _("Error"),
            # TRANSLATORS: {0} is "translations", the directory where compiled translation files (.mo) are stored.
            _("‘{0}’ directory not found. Please run the GUI.pyw program packaged with the Wesnoth installation.").
            format("translations")
        )

    # User-specified locale
    opts = parser.parse_args(sys.argv[1:])
    if opts.lang is not None:
        try:
            _ = gettext.translation("wesnoth-tools", WESNOTH_TRAN_DIR, languages=[opts.lang], fallback=False).gettext

        except OSError:
            showerror(
                _("Error"),
                # TRANSLATORS: {0} is the language argument entered by the user.
                _("Locale {0} not recognized.").format(opts.lang)
            )

        return

    # System locale
    # On POSIX systems, getlocale() should provide the POSIX locale name that gettext uses for finding translations.
    # However, on Windows, getlocale() returns strings likely not suitable for gettext, although getdefaultlocale()
    # does.
    try:
        system_locale = locale.getlocale()[0]
        _ = gettext.translation("wesnoth-tools", WESNOTH_TRAN_DIR, languages=[system_locale], fallback=False).gettext

    except OSError:
        # Needed for compatibility with Python <3.10, and/or Windows 7/8.
        # TODO: Note that getdefaultlocale() is deprecated in Python 3.11 so an alternative arrangement needs to be
        #  implemented for Windows.
        system_locale = locale.getdefaultlocale()[0]
        _ = gettext.translation("wesnoth-tools", WESNOTH_TRAN_DIR, languages=[system_locale], fallback=False).gettext


def on_update_locale(value):
    if value is None:
        try:
            set_global_locale()
        except:
            # _ defaults to identity lambda.
            pass
    else:
        pass


on_update_locale(None)


def wrap_elem(line):
    """If the supplied line contains spaces, return it wrapped between double quotes"""
    if ' ' in line:
        return "\"{0}\"".format(line)
    return line


class ToolThread(threading.Thread):
    def __init__(self, tool, tool_queue, command):
        super().__init__()
        self.tool = tool
        self.command = command
        self.queue = tool_queue
        self.subproc = None

    def run(self):
        # we can't use check_output, because it doesn't support
        # performing operations on the subprocess
        # so we'll have to use subprocess.Popen() instead

        # set the encoding for the subprocess
        # otherwise, with the new Unicode literals used by the wml tools,
        # we may get UnicodeDecodeErrors
        env = os.environ
        env['PYTHONIOENCODING'] = "utf8"
        if sys.platform == "win32":
            # Windows wants a string, Linux wants a list and Polly wants a cracker
            # Windows wants also strings flavoured with double quotes
            # since maps return iterators, we must cast them as lists, otherwise join won't work
            # not doing this causes an OSError: [WinError 87]
            # this doesn't happen on Python 2.7, because here map() returns a list
            wrapped_line = list(map(wrap_elem, self.command))
            self.queue.put_nowait(' '.join(wrapped_line) + "\n")
            si = subprocess.STARTUPINFO()
            si.dwFlags = subprocess.STARTF_USESHOWWINDOW | subprocess.SW_HIDE  # to avoid showing a DOS prompt
            self.subproc = subprocess.Popen(' '.join(wrapped_line),
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            startupinfo=si,
                                            env=env)
        else:  # STARTUPINFO is not available, nor needed, outside of Windows
            self.queue.put_nowait(' '.join(self.command) + "\n")
            self.subproc = subprocess.Popen(self.command,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            env=env)
        out, err = self.subproc.communicate()  # waits for the subprocess to finish and gets the output
        self.queue.put_nowait(str(out, "utf8"))
        self.queue.put_nowait(str(err, "utf8"))
        # unlike check_output, Popen never raises if a subprocess terminates abnormally
        # the documentation writes about the chance of OSError being raised
        # but I tested by deleting wmllint and it doesn't happen
        # perhaps because it's a Python script and the Python interpreter
        # certainly exists
        if self.subproc.returncode != 0:
            # post a tuple into the queue to allow showing the error
            self.queue.put_nowait((self.tool, self.subproc.returncode))

    def terminate(self):
        self.subproc.kill()


def is_wesnoth_tools_path(path):
    """Checks if the supplied path may be a wesnoth/data/tools directory"""
    lower_path = path.lower()
    if "wesnoth" in lower_path and \
            "data" in lower_path and \
            "tools" in lower_path:
        return True
    return False


def get_addons_directory():
    """Returns a string containing the path of the add-ons directory"""
    # os.path.expanduser gets the current user's home directory on every platform
    if sys.platform == "win32":
        # get userdata directory on Windows
        # it assumes that you choose to store userdata in the My Games directory
        # while installing Wesnoth
        userdata = os.path.join(os.path.expanduser("~"),
                                "Documents",
                                "My Games",
                                "Wesnoth" + WESNOTH_SERIES,
                                "data",
                                "add-ons")
    elif sys.platform.startswith("linux") or "bsd" in sys.platform:
        # we're on Linux or on a BSD system like FreeBSD
        userdata = os.path.join(os.path.expanduser("~"),
                                ".local",
                                "share",
                                "wesnoth",
                                WESNOTH_SERIES,
                                "data",
                                "add-ons")
    elif sys.platform == "darwin":  # we're on macOS
        # bear in mind that I don't have a Mac, so this point may be bugged
        userdata = os.path.join(os.path.expanduser("~"),
                                "Library",
                                "Application Support",
                                "Wesnoth_" + WESNOTH_SERIES,
                                "data",
                                "add-ons")
    else:  # unknown system; if someone else wants to add other rules, be my guest
        userdata = "."

    return userdata if os.path.exists(userdata) else "."  # fallback in case the directory doesn't exist


def attach_context_menu(widget, function):
    # on Mac the right button fires a Button-2 event, or so I'm told
    # some mice don't even have two buttons, so the user is forced
    # to use Command + the only button
    # bear in mind that I don't have a Mac, so this point may be bugged
    # bind also the context menu key, for those keyboards that have it
    # that is, most of the Windows and Linux ones (however, in Win it's
    # called App, while on Linux is called Menu)
    # Mac doesn't have a context menu key on its keyboards, so no binding
    # bind also the Shift+F10 shortcut (same as Menu/App key)
    # the call to tk windowingsystem is justified by the fact
    # that it is possible to install X11 over Darwin
    windowingsystem = widget.tk.call('tk', 'windowingsystem')
    if windowingsystem == "win32":  # Windows, both 32 and 64 bit
        widget.bind("<Button-3>", function)
        widget.bind("<KeyPress-App>", function)
        widget.bind("<Shift-KeyPress-F10>", function)
    elif windowingsystem == "aqua":  # MacOS with Aqua
        widget.bind("<Button-2>", function)
        widget.bind("<Control-Button-1>", function)
    elif windowingsystem == "x11":  # Linux, FreeBSD, Darwin with X11
        widget.bind("<Button-3>", function)
        widget.bind("<KeyPress-Menu>", function)
        widget.bind("<Shift-KeyPress-F10>", function)


def attach_select_all(widget, function):
    # bind the "select all" key shortcut
    # again, Mac uses Command instead of Control
    windowingsystem = widget.tk.call('tk', 'windowingsystem')
    if windowingsystem == "win32":
        widget.bind("<Control-KeyRelease-a>", function)
    elif windowingsystem == "aqua":
        widget.bind("<Command-KeyRelease-a>", function)
    elif windowingsystem == "x11":
        widget.bind("<Control-KeyRelease-a>", function)


class Tooltip(Toplevel):
    def __init__(self, widget, text, tag=None):
        """A tooltip, or balloon. Displays the specified help text when the
mouse pointer stays on the widget for more than 500 ms."""
        # the master attribute retrieves the window where our "parent" widget is
        super().__init__(widget.master)
        self.widget = widget
        self.preshow_id = None
        self.show_id = None
        self.label = Label(self,
                           text=text,
                           background="#ffffe1",  # background color used on Windows
                           borderwidth=1,
                           relief=SOLID,
                           padding=1,
                           # Tk has a bunch of predefined fonts
                           # use the one specific for tooltips
                           font=font.nametofont("TkTooltipFont"))
        self.label.pack()
        self.overrideredirect(True)
        # allow binding the tooltips to tagged elements of a widget
        # only Text, Canvas and Treeview support tags
        # and as such, they have a tag_bind method
        # if the widget doesn't support tags, bind directly to it
        if tag and hasattr(widget, "tag_bind") and callable(widget.tag_bind):
            self.widget.tag_bind(tag, "<Enter>", self.preshow)
            self.widget.tag_bind(tag, "<Leave>", self.hide)
        else:
            self.widget.bind("<Enter>", self.preshow)
            self.widget.bind("<Leave>", self.hide)
        self.widget.bind("<ButtonPress>", self.hide)
        self.widget.bind("<ButtonRelease>", self.hide)
        self.withdraw()

    def preshow(self, event=None):
        self.after_cleanup()
        # 500 ms and 5000 ms are the default values used on Windows
        self.preshow_id = self.after(500, self.show)

    def show(self):
        self.after_cleanup()
        # check if the tooltip will end up out of the screen
        # and handle this case if so
        screen_width = self.winfo_screenwidth()
        tooltip_width = self.winfo_reqwidth()
        if tooltip_width + self.winfo_pointerx() > screen_width:
            # unfortunately, it seems like Tkinter doesn't have a way to check the pointer's size
            # so I'm using a value of 20px, which is enough for the usual 16px pointers
            self.geometry("+%d+%d" % (screen_width - tooltip_width, self.winfo_pointery() + 20))
        else:
            self.geometry("+%d+%d" % (self.winfo_pointerx(), self.winfo_pointery() + 20))
        # update_idletasks forces a geometry update
        self.update_idletasks()
        self.state("normal")
        self.lift()
        self.show_id = self.after(5000, self.hide)

    def hide(self, event=None):
        self.after_cleanup()
        self.withdraw()

    def after_cleanup(self):
        # each event should clean up after itself,
        # to avoid having two .after() calls conflicting
        # for example, one previously scheduled .after() may
        # try to hide the tooltip before five seconds are passed
        if self.show_id:
            self.after_cancel(self.show_id)
            self.show_id = None
        if self.preshow_id:
            self.after_cancel(self.preshow_id)
            self.preshow_id = None

    def set_text(self, text):
        self.label.configure(text=text)
        self.update_idletasks()


class Popup(Toplevel):
    def __init__(self, parent, tool, thread):
        """Creates a popup that informs the user that the desired tool is running.
Self destroys when the tool thread is over"""
        self.thread = thread
        super().__init__(parent)
        self.transient(parent)
        self.grab_set()
        self.protocol("WM_DELETE_WINDOW",
                      lambda: None)  # disable close button
        self.resizable(width=False,
                       height=False)
        frame = Frame(self)
        frame.pack(fill=BOTH, expand=YES)
        wait_label = Label(frame,
                           # TRANSLATORS: {0} is the name of command being executed.
                           text=_("Running: {0}\nPlease wait...").format(tool),
                           justify=CENTER)
        wait_label.grid(row=0,
                        column=0,
                        padx=5,
                        pady=5)
        wait_progress = Progressbar(frame,
                                    mode="indeterminate")
        wait_progress.grid(row=1,
                           column=0,
                           sticky=E + W,
                           padx=5,
                           pady=5)
        terminate_button = Button(frame,
                                  text=_("Terminate script"),
                                  image=ICONS["process-stop"],
                                  compound=LEFT,
                                  command=self.terminate)
        terminate_button.grid(row=2,
                              column=0,
                              padx=5,
                              pady=5)
        frame.columnconfigure(0, weight=1)
        # place the popup in the middle of the main window
        # get the main window position and dimension
        self.geometry("{0}x{1}+{2}+{3}".format(400,
                                               140,
                                               int(root.winfo_rootx() + (root.winfo_width() - 400) / 2),
                                               int(root.winfo_rooty() + (root.winfo_height() - 140) / 2)))
        wait_progress.start(10)
        self.check_thread_alive()

    def check_thread_alive(self):
        """Checks if the thread is still alive, and destroys the window if it isn't"""
        # placing this in a for or while cycle freezes the app
        # so we need to use the .after method and recursively call the function
        # that's one of the many quirks of Tkinter
        try:  # Python <= 3.8
            is_alive = self.thread.isAlive()
        except AttributeError:  # Python >= 3.9
            is_alive = self.thread.is_alive()
        finally:
            if is_alive:
                self.after(100, self.check_thread_alive)
            else:
                self.after(1, self.destroy)

    def terminate(self):
        self.thread.terminate()


class ContextMenu(Menu):
    def __init__(self, x, y, widget):
        """A subclass of Menu, used to display a context menu in Text and Entry widgets
If the widget isn't active, some options do not appear"""
        super().__init__(None, tearoff=0)  # otherwise Tk allows splitting it in a new window
        self.widget = widget
        # MacOS uses a key called Command, instead of the usual Control used by Windows and Linux
        # so prepare the accelerator strings accordingly
        # For future reference, Mac also uses Option instead of Alt
        # also, a little known fact about Python is that it *does* support using the ternary operator
        # like in this case
        control_key = "Command" if self.tk.call('tk', 'windowingsystem') == "aqua" else "Ctrl"
        # str is necessary because in some instances a Tcl_Obj is returned instead of a string
        if str(widget.cget('state')) in (ACTIVE, NORMAL):  # do not add if state is readonly or disabled
            self.add_command(label=_("Cut"),
                             image=ICONS['cut'],
                             compound=LEFT,
                             accelerator='%s+X' % control_key,
                             command=lambda: self.widget.event_generate("<<Cut>>"))
        self.add_command(label=_("Copy"),
                         image=ICONS['copy'],
                         compound=LEFT,
                         accelerator='%s+C' % control_key,
                         command=lambda: self.widget.event_generate("<<Copy>>"))
        if str(widget.cget('state')) in (ACTIVE, NORMAL):
            self.add_command(label=_("Paste"),
                             image=ICONS['paste'],
                             compound=LEFT,
                             accelerator='%s+V' % control_key,
                             command=lambda: self.widget.event_generate("<<Paste>>"))
        self.add_separator()
        self.add_command(label=_("Select All"),
                         image=ICONS['select_all'],
                         compound=LEFT,
                         accelerator='%s+A' % control_key,
                         command=self.on_select_all)
        self.tk_popup(x, y)  # self.post does not destroy the menu when clicking out of it

    def on_select_all(self):
        # disabled Text widgets have a different way to handle selection
        if isinstance(self.widget, Text):
            # adding a SEL tag to a chunk of text causes it to be selected
            self.widget.tag_add(SEL, "1.0", END)
        elif isinstance(self.widget, Entry) or \
                isinstance(self.widget, Combobox):
            # apparently, the <<SelectAll>> event doesn't fire correctly if the widget is readonly
            self.widget.select_range(0, END)
        elif isinstance(self.widget, Spinbox):
            self.widget.selection("range", 0, END)


class EntryContext(Entry):
    def __init__(self, parent, **kwargs):
        """An enhanced Entry widget that has a right-click menu
Use like any other Entry widget"""
        super().__init__(parent, **kwargs)
        attach_context_menu(self, self.on_context_menu)
        attach_select_all(self, self.on_select_all)

    def on_context_menu(self, event):
        if str(self.cget('state')) != DISABLED:
            ContextMenu(event.x_root, event.y_root, event.widget)

    def on_select_all(self, event):
        self.select_range(0, END)


class SpinboxContext(Spinbox):
    def __init__(self, parent, **kwargs):
        """An enhanced Spinbox widget that has a right-click menu
Use like any other Spinbox widget"""
        super().__init__(parent, **kwargs)
        attach_context_menu(self, self.on_context_menu)
        attach_select_all(self, self.on_select_all)

    def on_context_menu(self, event):
        if str(self.cget('state')) != DISABLED:
            ContextMenu(event.x_root, event.y_root, event.widget)

    def on_select_all(self, event):
        self.selection("range", 0, END)


class EnhancedText(Text):
    def __init__(self, *args, **kwargs):
        """A subclass of Text with a context menu
Use it like any other Text widget"""
        super().__init__(*args, **kwargs)
        attach_context_menu(self, self.on_context_menu)
        attach_select_all(self, self.on_select_all)

    def on_context_menu(self, event):
        # the disabled state in a Text widget is pretty much
        # like the readonly state in Entry, hence no state check
        ContextMenu(event.x_root, event.y_root, event.widget)

    def on_select_all(self, event):
        self.tag_add(SEL, "1.0", END)


class SelectDirectory(LabelFrame):
    def __init__(self, parent, textvariable=None, **kwargs):
        """A subclass of LabelFrame sporting a readonly Entry and a Button with a folder icon.
It comes complete with a context menu and a directory selection screen"""
        super().__init__(parent, text=_("Working directory"), **kwargs)
        self.textvariable = textvariable
        self.dir_entry = EntryContext(self,
                                      width=40,
                                      textvariable=self.textvariable,
                                      state="readonly")
        self.dir_entry.pack(side=LEFT,
                            fill=BOTH,
                            expand=YES)
        self.dir_button = Button(self,
                                 image=ICONS['browse'],
                                 compound=LEFT,
                                 text=_("Browse..."),
                                 command=self.on_browse)
        self.dir_button.pack(side=LEFT)
        self.clear_button = Button(self,
                                   image=ICONS['clear16'],
                                   compound=LEFT,
                                   # TRANSLATORS: Clear button for clearing the directory text box.
                                   text=_("Clear"),
                                   command=self.on_clear)
        self.clear_button.pack(side=LEFT)

    def on_browse(self):
        # if the user already selected a directory, try to use it
        current_dir = self.textvariable.get()
        if os.path.exists(current_dir):
            directory = askdirectory(initialdir=current_dir)
        # otherwise attempt to detect the user's userdata folder
        else:
            directory = askdirectory(initialdir=get_addons_directory())
        if directory:
            # use os.path.normpath, so on Windows the usual backwards slashes are correctly shown
            self.textvariable.set(os.path.normpath(directory))

    def on_clear(self):
        self.textvariable.set("")


class SelectOutputPath(Frame):
    def __init__(self, parent, textvariable, filetypes=None, **kwargs):
        """A subclass of Frame with a readonly Entry and a Button with a browse icon.
It has a context menu and a save file selection dialog."""
        super().__init__(parent, **kwargs)
        self.textvariable = textvariable
        self.file_entry = EntryContext(self,
                                       width=40,
                                       textvariable=self.textvariable,
                                       state="readonly")
        self.file_entry.pack(side=LEFT,
                             fill=BOTH,
                             expand=YES)
        self.file_button = Button(self,
                                  image=ICONS['browse'],
                                  compound=LEFT,
                                  text=_("Browse..."),
                                  command=self.on_browse)
        self.file_button.pack(side=LEFT)
        self.clear_button = Button(self,
                                   image=ICONS['clear16'],
                                   compound=LEFT,
                                   # TRANSLATORS: Clear button for clearing the directory text box.
                                   text=_("Clear"),
                                   command=self.on_clear)
        self.clear_button.pack(side=LEFT)
        self.filetypes = filetypes

    def on_browse_file(self):
        # if the user already selected a file, try to use its directory
        current_dir, current_file = os.path.split(self.textvariable.get())
        if os.path.exists(current_dir):
            return asksaveasfilename(filetypes=self.filetypes,
                                     initialdir=current_dir,
                                     initialfile=current_file,
                                     confirmoverwrite=False)  # the GUI will ask later if the file should be overwritten, so disable it for now
        # otherwise attempt to detect the user's userdata folder
        else:
            return asksaveasfilename(filetypes=self.filetypes,
                                     initialdir=get_addons_directory(),
                                     confirmoverwrite=False)

    def on_browse_dir(self):
        current_dir = self.textvariable.get()
        if os.path.exists(current_dir):
            return askdirectory(initialdir=current_dir)
        # otherwise attempt to detect the user's userdata folder
        else:
            return askdirectory(initialdir=get_addons_directory())

    def on_browse(self):
        if self.filetypes is None:
            directory = self.on_browse_dir()

        else:
            directory = self.on_browse_file()

        if directory:
            # use os.path.normpath, so on Windows the usual backwards slashes are correctly shown
            self.textvariable.set(os.path.normpath(directory))

    def on_clear(self):
        self.textvariable.set("")


class WmllintTab(Frame):
    def __init__(self, parent):
        # it means super(WmllintTab,self), that in turn means
        # Frame.__init__(self,parent)
        super().__init__(parent)
        self.mode_variable = IntVar()

        self.mode_frame = LabelFrame(self,
                                     text=_("wmllint mode"))
        self.mode_frame.grid(row=0,
                             column=0,
                             sticky=N + E + S + W)

        self.radio_normal = Radiobutton(self.mode_frame,
                                        # TRANSLATORS: Normal run mode for the WML tool.
                                        text=_("Normal"),
                                        variable=self.mode_variable,
                                        value=0)
        self.radio_normal.grid(row=0,
                               column=0,
                               sticky=W,
                               padx=10)
        self.tooltip_normal = Tooltip(self.radio_normal,
                                      # TRANSLATORS: Tooltip explanation for normal run mode.
                                      _("Perform conversion and save changes to file"))

        self.radio_dryrun = Radiobutton(self.mode_frame,
                                        text=_("Dry run"),
                                        variable=self.mode_variable,
                                        value=1)
        self.radio_dryrun.grid(row=1,
                               column=0,
                               sticky=W,
                               padx=10)
        self.tooltip_dryrun = Tooltip(self.radio_dryrun,
                                      # TRANSLATORS: Tooltip explanation for dry run mode.
                                      _("Perform conversion without saving changes to file"))

        self.radio_clean = Radiobutton(self.mode_frame,
                                       text=_("Clean"),
                                       variable=self.mode_variable,
                                       value=2)
        self.radio_clean.grid(row=2,
                              column=0,
                              sticky=W,
                              padx=10)
        self.tooltip_clean = Tooltip(self.radio_clean,
                                     # TRANSLATORS: Tooltip explanation for clean mode.
                                     _("Delete back-up files"))

        self.radio_diff = Radiobutton(self.mode_frame,
                                      text=_("Diff"),
                                      variable=self.mode_variable,
                                      value=3)
        self.radio_diff.grid(row=3,
                             column=0,
                             sticky=W,
                             padx=10)
        self.tooltip_diff = Tooltip(self.radio_diff,
                                    # TRANSLATORS: Tooltip explanation for diff run mode.
                                    _("Show differences in converted files"))

        self.radio_revert = Radiobutton(self.mode_frame,
                                        text=_("Revert"),
                                        variable=self.mode_variable,
                                        value=4)
        self.radio_revert.grid(row=4,
                               column=0,
                               sticky=W,
                               padx=10)
        self.tooltip_revert = Tooltip(self.radio_revert,
                                      # TRANSLATORS: Tooltip explanation for revert run mode.
                                      _("Revert conversions using back-up files"))

        self.verbosity_frame = LabelFrame(self,
                                          text=_("Verbosity level"))
        self.verbosity_frame.grid(row=0,
                                  column=1,
                                  sticky=N + E + S + W)

        self.verbosity_variable = IntVar()
        self.radio_v0 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Terse"),
                                    variable=self.verbosity_variable,
                                    value=0)

        self.radio_v0.grid(row=0,
                           column=0,
                           sticky=W,
                           padx=10)

        self.radio_v1 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Show changes"),
                                    variable=self.verbosity_variable,
                                    value=1)
        self.radio_v1.grid(row=1,
                           column=0,
                           sticky=W,
                           padx=10)

        self.radio_v2 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Name files before processing"),
                                    variable=self.verbosity_variable,
                                    value=2)
        self.radio_v2.grid(row=2,
                           column=0,
                           sticky=W,
                           padx=10)

        self.radio_v3 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Show parse details"),
                                    variable=self.verbosity_variable,
                                    value=3)
        self.radio_v3.grid(row=3,
                           column=0,
                           sticky=W,
                           padx=10)

        self.options_frame = LabelFrame(self,
                                        text=_("wmllint options"))
        self.options_frame.grid(row=0,
                                column=2,
                                sticky=N + E + S + W)

        self.stripcr_variable = BooleanVar()
        self.stripcr_check = Checkbutton(self.options_frame,
                                         # TRANSLATORS: EOL = Special characters marking 'end-of-line'.
                                         text=_("Convert EOL characters to Unix format"),
                                         variable=self.stripcr_variable)
        self.stripcr_check.grid(row=0,
                                column=0,
                                sticky=W,
                                padx=10)

        self.missing_variable = BooleanVar()
        self.missing_check = Checkbutton(self.options_frame,
                                         # TRANSLATORS: 'side=' in this context refers to WML and should not be
                                         # translated.
                                         text=_("Warn about tags without side= keys"),
                                         variable=self.missing_variable)
        self.missing_check.grid(row=1,
                                column=0,
                                sticky=W,
                                padx=10)

        self.known_variable = BooleanVar()
        self.known_check = Checkbutton(self.options_frame,
                                       text=_("Disable checks for unknown units"),
                                       variable=self.known_variable)
        self.known_check.grid(row=2,
                              column=0,
                              sticky=W,
                              padx=10)

        self.spell_variable = BooleanVar()
        self.spell_check = Checkbutton(self.options_frame,
                                       text=_("Disable spellchecking"),
                                       variable=self.spell_variable)
        self.spell_check.grid(row=3,
                              column=0,
                              sticky=W,
                              padx=10)

        self.skip_variable = BooleanVar()
        self.skip_core = Checkbutton(self.options_frame,
                                     text=_("Skip core directory"),
                                     variable=self.skip_variable,
                                     command=self.skip_core_dir_callback)
        self.skip_core.grid(row=4,
                            column=0,
                            sticky=W,
                            padx=10)

        self.columnconfigure(0, weight=1)
        self.columnconfigure(1, weight=1)
        self.columnconfigure(2, weight=1)

    def skip_core_dir_callback(self):
        # if Skip core directory is enabled
        # avoid checking for unknown unit types
        if self.skip_variable.get():
            self.known_variable.set(True)
            self.known_check.configure(state=DISABLED)
        else:
            self.known_variable.set(False)
            self.known_check.configure(state=NORMAL)


class WmlscopeTab(Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.options_frame = LabelFrame(self,
                                        text=_("wmlscope options"))
        self.options_frame.grid(row=0,
                                column=0,
                                sticky=N + E + S + W)

        self.normal_options = Frame(self.options_frame)
        self.normal_options.grid(row=0,
                                 column=0,
                                 sticky=N + E + S + W)

        self.crossreference_variable = BooleanVar()  # equivalent to warnlevel 1
        self.crossreference_check = Checkbutton(self.normal_options,
                                                text=_("Check for duplicate macro definitions"),
                                                variable=self.crossreference_variable)
        self.crossreference_check.grid(row=0,
                                       column=0,
                                       sticky=W,
                                       padx=10)

        self.collisions_variable = BooleanVar()
        self.collisions_check = Checkbutton(self.normal_options,
                                            text=_("Check for duplicate resource files"),
                                            variable=self.collisions_variable)
        self.collisions_check.grid(row=1,
                                   column=0,
                                   sticky=W,
                                   padx=10)

        self.definitions_variable = BooleanVar()
        self.definitions_check = Checkbutton(self.normal_options,
                                             text=_("Make definition list"),
                                             variable=self.definitions_variable)
        self.definitions_check.grid(row=2,
                                    column=0,
                                    sticky=W,
                                    padx=10)

        self.listfiles_variable = BooleanVar()
        self.listfiles_check = Checkbutton(self.normal_options,
                                           text=_("List files that will be processed"),
                                           variable=self.listfiles_variable)
        self.listfiles_check.grid(row=3,
                                  column=0,
                                  sticky=W,
                                  padx=10)

        self.unresolved_variable = BooleanVar()
        self.unresolved_check = Checkbutton(self.normal_options,
                                            text=_("Report unresolved macro references"),
                                            variable=self.unresolved_variable)
        self.unresolved_check.grid(row=4,
                                   column=0,
                                   sticky=W,
                                   padx=10)

        self.extracthelp_variable = BooleanVar()
        self.extracthelp_check = Checkbutton(self.normal_options,
                                             text=_("Extract help from macro definition comments"),
                                             variable=self.extracthelp_variable)
        self.extracthelp_check.grid(row=5,
                                    column=0,
                                    sticky=W,
                                    padx=10)

        self.unchecked_variable = BooleanVar()
        self.unchecked_check = Checkbutton(self.normal_options,
                                           text=_("Report all macros with untyped formals"),
                                           variable=self.unchecked_variable)
        self.unchecked_check.grid(row=6,
                                  column=0,
                                  sticky=W,
                                  padx=10)

        self.progress_variable = BooleanVar()
        self.progress_check = Checkbutton(self.normal_options,
                                          text=_("Show progress"),
                                          variable=self.progress_variable)
        self.progress_check.grid(row=7,
                                 column=0,
                                 sticky=W,
                                 padx=10)

        self.separator = Separator(self.options_frame,
                                   orient=VERTICAL)
        self.separator.grid(row=0,
                            column=1,
                            sticky=N + S)

        self.options_with_regexp = Frame(self.options_frame)
        self.options_with_regexp.grid(row=0,
                                      column=2,
                                      sticky=N + E + S + W)

        self.exclude_variable = BooleanVar()
        self.exclude_check = Checkbutton(self.options_with_regexp,
                                         text=_("Exclude file names matching regular expression:"),
                                         variable=self.exclude_variable,
                                         command=self.exclude_callback)
        self.exclude_check.grid(row=0,
                                column=0,
                                sticky=W,
                                padx=10)
        self.exclude_regexp = StringVar()
        self.exclude_entry = EntryContext(self.options_with_regexp,
                                          textvariable=self.exclude_regexp,
                                          state=DISABLED)
        self.exclude_entry.grid(row=0,
                                column=1,
                                sticky=E + W,
                                padx=10)

        self.from_variable = BooleanVar()
        self.from_check = Checkbutton(self.options_with_regexp,
                                      text=_("Exclude file names not matching regular expression:"),
                                      variable=self.from_variable,
                                      command=self.from_callback)
        self.from_check.grid(row=1,
                             column=0,
                             sticky=W,
                             padx=10)
        self.from_regexp = StringVar()
        self.from_entry = EntryContext(self.options_with_regexp,
                                       textvariable=self.from_regexp,
                                       state=DISABLED)
        self.from_entry.grid(row=1,
                             column=1,
                             sticky=E + W,
                             padx=10)

        self.refcount_variable = BooleanVar()
        self.refcount_check = Checkbutton(self.options_with_regexp,
                                          # TRANSLATORS: 'n' in this context refers to number, as in 'n number of
                                          # files'.
                                          text=_("Report only on macros referenced in exactly n files:"),
                                          variable=self.refcount_variable,
                                          command=self.refcount_callback)
        self.refcount_check.grid(row=2,
                                 column=0,
                                 sticky=W,
                                 padx=10)
        self.refcount_number = IntVar()
        self.refcount_spin = SpinboxContext(self.options_with_regexp,
                                            from_=0, to=999,
                                            textvariable=self.refcount_number,
                                            width=3,
                                            state=DISABLED)
        self.refcount_spin.grid(row=2,
                                column=1,
                                sticky=E + W,
                                padx=10)

        self.typelist_variable = BooleanVar()
        self.typelist_check = Checkbutton(self.options_with_regexp,
                                          text=_("Report macro definitions and usages in file:"),
                                          variable=self.typelist_variable,
                                          command=self.typelist_callback)
        self.typelist_check.grid(row=3,
                                 column=0,
                                 sticky=W,
                                 padx=10)
        self.typelist_string = StringVar()
        self.typelist_entry = EntryContext(self.options_with_regexp,
                                           textvariable=self.typelist_string,
                                           state=DISABLED)
        self.typelist_entry.grid(row=3,
                                 column=1,
                                 sticky=E + W,
                                 padx=10)

        self.force_variable = BooleanVar()
        self.force_check = Checkbutton(self.options_with_regexp,
                                       text=_("Allow unused macros with names matching regular expression:"),
                                       variable=self.force_variable,
                                       command=self.force_callback)
        self.force_check.grid(row=4,
                              column=0,
                              sticky=W,
                              padx=10)
        self.force_regexp = StringVar()
        self.force_entry = EntryContext(self.options_with_regexp,
                                        textvariable=self.force_regexp,
                                        state=DISABLED)
        self.force_entry.grid(row=4,
                              column=1,
                              sticky=E + W,
                              padx=10)

        self.columnconfigure(0, weight=1)
        self.options_frame.columnconfigure(0, weight=1)
        self.options_frame.columnconfigure(2, weight=1)
        self.options_with_regexp.columnconfigure(1, weight=1)
        # uniform= makes the options in the options_with_regexp frame evenly sized
        # please note that "regexp" is an ID of the widget group, not a special value
        # you can replace "regexp" with "bacon" and it still works...
        for row in range(5):
            self.options_with_regexp.rowconfigure(row, uniform="regexp")

    def exclude_callback(self):
        if self.exclude_variable.get():
            self.exclude_entry.configure(state=NORMAL)
        else:
            self.exclude_entry.configure(state=DISABLED)

    def from_callback(self):
        if self.from_variable.get():
            self.from_entry.configure(state=NORMAL)
        else:
            self.from_entry.configure(state=DISABLED)

    def refcount_callback(self):
        if self.refcount_variable.get():
            self.refcount_spin.configure(state="readonly")
        else:
            self.refcount_spin.configure(state=DISABLED)

    def typelist_callback(self):
        if self.typelist_variable.get():
            self.typelist_entry.configure(state=NORMAL)
        else:
            self.typelist_entry.configure(state=DISABLED)

    def force_callback(self):
        if self.force_variable.get():
            self.force_entry.configure(state=NORMAL)
        else:
            self.force_entry.configure(state=DISABLED)


class WmlindentTab(Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.mode_variable = IntVar()
        self.mode_frame = LabelFrame(self,
                                     text=_("wmlindent mode"))
        self.mode_frame.grid(row=0,
                             column=0,
                             sticky=N + E + S + W)

        self.radio_normal = Radiobutton(self.mode_frame,
                                        text=_("Normal"),
                                        variable=self.mode_variable,
                                        value=0)
        self.radio_normal.grid(row=0,
                               column=0,
                               sticky=W,
                               padx=10)
        self.tooltip_normal = Tooltip(self.radio_normal,
                                      # TRANSLATORS: Tooltip explanation for normal run mode.
                                      _("Perform conversion and save changes to file"))

        self.radio_dryrun = Radiobutton(self.mode_frame,
                                        text=_("Dry run"),
                                        variable=self.mode_variable,
                                        value=1)
        self.radio_dryrun.grid(row=1,
                               column=0,
                               sticky=W,
                               padx=10)
        self.tooltip_dryrun = Tooltip(self.radio_dryrun,
                                      # TRANSLATORS: Tooltip explanation for dry run mode.
                                      _("Perform conversion without saving changes to file"))

        self.verbosity_frame = LabelFrame(self,
                                          text=_("Verbosity level"))
        self.verbosity_frame.grid(row=0,
                                  column=1,
                                  sticky=N + E + S + W)
        self.verbosity_variable = IntVar()

        self.radio_v0 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Terse"),
                                    variable=self.verbosity_variable,
                                    value=0)
        self.radio_v0.grid(row=0,
                           column=0,
                           sticky=W,
                           padx=10)

        self.radio_v1 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Verbose"),
                                    variable=self.verbosity_variable,
                                    value=1)
        self.radio_v1.grid(row=1,
                           column=0,
                           sticky=W,
                           padx=10)

        self.radio_v2 = Radiobutton(self.verbosity_frame,
                                    # TRANSLATORS: Verbosity level.
                                    text=_("Report unchanged files"),
                                    variable=self.verbosity_variable,
                                    value=2)
        self.radio_v2.grid(row=2,
                           column=0,
                           sticky=W,
                           padx=10)

        self.options_frame = LabelFrame(self,
                                        text=_("wmlindent options"))
        self.options_frame.grid(row=0,
                                column=2,
                                sticky=N + E + S + W)

        self.exclude_variable = BooleanVar()
        self.exclude_check = Checkbutton(self.options_frame,
                                         text=_("Exclude file names matching regular expression:"),
                                         variable=self.exclude_variable,
                                         command=self.exclude_callback)
        self.exclude_check.grid(row=1,
                                column=0,
                                sticky=W,
                                padx=10)

        self.regexp_variable = StringVar()
        self.regexp_entry = EntryContext(self.options_frame,
                                         textvariable=self.regexp_variable,
                                         state=DISABLED)
        self.regexp_entry.grid(row=1,
                               column=1,
                               sticky=E + W,
                               padx=10)

        self.quiet_variable = BooleanVar()
        self.quiet_check = Checkbutton(self.options_frame,
                                       # TRANSLATORS: Option to run 'quietly'.
                                       text=_("Quiet mode"),
                                       variable=self.quiet_variable)
        self.quiet_check.grid(row=2,
                              column=0,
                              sticky=W,
                              padx=10)
        self.tooltip_quiet = Tooltip(self.quiet_check,
                                     # TRANSLATORS: Tooltip explanation for quiet option.
                                     _("Do not generate output messages"))

        self.columnconfigure(0, weight=1)
        self.columnconfigure(1, weight=1)
        self.columnconfigure(2, weight=1)
        self.options_frame.columnconfigure(1, weight=1)

    def exclude_callback(self):
        if self.exclude_variable.get():
            self.regexp_entry.configure(state=NORMAL)
        else:
            self.regexp_entry.configure(state=DISABLED)


class WmlxgettextTab(Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.output_wrapper_frame = Frame(self)
        self.output_wrapper_frame.grid(row=0, column=0, columnspan=2, sticky=N + E + S + W)
        self.output_label = Label(self.output_wrapper_frame,
                                  text=_("Output directory:"))
        self.output_label.grid(row=0, column=0, sticky=W)
        self.output_variable = StringVar()
        self.output_frame = SelectOutputPath(self.output_wrapper_frame, textvariable=self.output_variable)
        self.output_frame.grid(row=0, column=1, sticky=N + E + S + W)
        self.options_labelframe = LabelFrame(self,
                                             text=_("Options"))
        self.options_labelframe.grid(row=1, column=0, sticky=N + E + S + W)
        self.recursive_variable = BooleanVar()
        self.recursive_variable.set(True)
        self.recursive_check = Checkbutton(self.options_labelframe,
                                           text=_("Scan subdirectories"),
                                           variable=self.recursive_variable)
        self.recursive_check.grid(row=0, column=0, sticky=W)
        self.warnall_variable = BooleanVar()
        self.warnall_check = Checkbutton(self.options_labelframe,
                                         text=_("Show optional warnings"),
                                         variable=self.warnall_variable)
        self.warnall_check.grid(row=1, column=0, sticky=W)
        self.fuzzy_variable = BooleanVar()
        self.fuzzy_check = Checkbutton(self.options_labelframe,
                                       # TRANSLATORS: Also called "Needs work".
                                       text=_("Mark all strings as fuzzy"),
                                       variable=self.fuzzy_variable)
        self.fuzzy_check.grid(row=2, column=0, sticky=W)
        self.advanced_labelframe = LabelFrame(self,
                                              text=_("Advanced options"))
        self.advanced_labelframe.grid(row=1, column=1, sticky=N + E + S + W)
        self.package_version_variable = BooleanVar()
        self.package_version_check = Checkbutton(self.advanced_labelframe,
                                                 text=_("Package version"),
                                                 variable=self.package_version_variable)
        self.package_version_check.grid(row=0, column=0, sticky=W)
        self.initialdomain_variable = BooleanVar()
        self.textdomain_variable = BooleanVar()
        self.textdomain_check = Checkbutton(self.advanced_labelframe,
                                            text="Filter textdomains:",
                                            variable=self.textdomain_variable,
                                            command=self.textdomain_callback)
        self.textdomain_check.grid(row=1, column=0, sticky=W)
        self.textdomain_name = StringVar()
        self.textdomain_entry = Entry(self.advanced_labelframe,
                                      state=DISABLED,
                                      width=40,
                                      textvariable=self.textdomain_name)
        self.textdomain_entry.grid(row=1, column=1, sticky=E + W)
        self.initialdomain_check = Checkbutton(self.advanced_labelframe,
                                               text=_("Initial textdomain:"),
                                               variable=self.initialdomain_variable,
                                               command=self.initialdomain_callback)
        self.initialdomain_check.grid(row=2, column=0, sticky=W)
        self.initialdomain_name = StringVar()
        self.initialdomain_entry = Entry(self.advanced_labelframe,
                                         state=DISABLED,
                                         width=40,
                                         textvariable=self.initialdomain_name)
        self.initialdomain_entry.grid(row=2, column=1, sticky=E + W)
        self.output_wrapper_frame.columnconfigure(1, weight=1)
        self.output_wrapper_frame.rowconfigure(0, uniform="group")
        self.advanced_labelframe.columnconfigure(1, weight=1)
        self.advanced_labelframe.columnconfigure(2, weight=1)
        self.columnconfigure(0, weight=2)
        self.columnconfigure(1, weight=1)

    def textdomain_callback(self, event=None):
        if self.textdomain_variable.get():
            self.textdomain_entry.configure(state=NORMAL)
        else:
            self.textdomain_entry.configure(state=DISABLED)

    def initialdomain_callback(self, event=None):
        if self.initialdomain_variable.get():
            self.initialdomain_entry.configure(state=NORMAL)
        else:
            self.initialdomain_entry.configure(state=DISABLED)


class MainFrame(Frame):
    def __init__(self, parent):
        self.parent = parent
        self.queue = queue.Queue()
        super().__init__(parent)
        self.grid(sticky=N + E + S + W)
        self.buttonbox = Frame(self)
        self.buttonbox.grid(row=0,
                            column=0,
                            sticky=E + W)
        self.run_button = Button(self.buttonbox,
                                 image=ICONS['run'],
                                 command=self.on_run_wmllint)
        self.run_button.pack(side=LEFT,
                             padx=5,
                             pady=5)
        self.run_tooltip = Tooltip(self.run_button, _("Run wmllint"))
        self.save_button = Button(self.buttonbox,
                                  image=ICONS['save'],
                                  command=self.on_save)
        self.save_button.pack(side=LEFT,
                              padx=5,
                              pady=5)
        self.save_tooltip = Tooltip(self.save_button, _("Save as text..."))
        self.clear_button = Button(self.buttonbox,
                                   image=ICONS['clear'],
                                   command=self.on_clear)
        self.clear_button.pack(side=LEFT,
                               padx=5,
                               pady=5)
        self.clear_tooltip = Tooltip(self.clear_button, _("Clear output"))
        self.about_button = Button(self.buttonbox,
                                   image=ICONS['about'],
                                   command=self.on_about)
        self.about_button.pack(side=LEFT,
                               padx=5,
                               pady=5)
        self.about_tooltip = Tooltip(self.about_button, _("About..."))
        self.exit_button = Button(self.buttonbox,
                                  image=ICONS['exit'],
                                  command=self.on_quit)
        self.exit_button.pack(side=RIGHT,
                              padx=5,
                              pady=5)
        self.exit_tooltip = Tooltip(self.exit_button, _("Exit"))
        self.dir_variable = StringVar()
        self.dir_frame = SelectDirectory(self,
                                         textvariable=self.dir_variable)
        self.dir_frame.grid(row=1,
                            column=0,
                            sticky=E + W)
        # Notebook is one of the new widgets introduced by ttk
        # it isn't available on Python 2.6 and lower, like the rest of ttk widgets
        # please note that the Frames that become tabs don't need to be packed or gridded
        self.notebook = Notebook(self)
        self.notebook.grid(row=2,
                           column=0,
                           sticky=E + W)
        self.wmllint_tab = WmllintTab(None)
        self.notebook.add(self.wmllint_tab,
                          text=_("wmllint"),
                          sticky=N + E + S + W)
        self.wmlscope_tab = WmlscopeTab(None)
        self.notebook.add(self.wmlscope_tab,
                          text=_("wmlscope"),
                          sticky=N + E + S + W)
        self.wmlindent_tab = WmlindentTab(None)
        self.notebook.add(self.wmlindent_tab,
                          text=_("wmlindent"),
                          sticky=N + E + S + W)
        self.wmlxgettext_tab = WmlxgettextTab(None)
        self.notebook.add(self.wmlxgettext_tab,
                          text=_("wmlxgettext"),
                          sticky=N + E + S + W)
        self.output_frame = LabelFrame(self,
                                       text=_("Output"))
        self.output_frame.grid(row=3,
                               column=0,
                               sticky=N + E + S + W)
        # in former versions of this script, I disabled the text widget at its creation
        # it turned out that doing so on Aqua (macOS) causes the widget to ignore
        # any additional binding set after its disabling
        # the subclass EnhancedText first calls the constructor of the original Text widget
        # and only later it creates its own bindings
        # so first create the widget, and disable it later
        self.text = EnhancedText(self.output_frame,
                                 wrap=WORD,
                                 takefocus=True)
        self.text.configure(state=DISABLED)
        self.text.grid(row=0,
                       column=0,
                       sticky=N + E + S + W)
        self.update_text()
        self.yscrollbar = Scrollbar(self.output_frame,
                                    command=self.text.yview)
        self.yscrollbar.grid(row=0,
                             column=1,
                             sticky=N + S)
        self.text["yscrollcommand"] = self.yscrollbar.set
        self.xscrollbar = Scrollbar(self.output_frame,
                                    orient=HORIZONTAL,
                                    command=self.text.xview)
        self.xscrollbar.grid(row=1,
                             column=0,
                             sticky=E + W)
        self.text["xscrollcommand"] = self.xscrollbar.set
        self.grip = Sizegrip(self.output_frame)
        self.grip.grid(row=1, column=1)
        self.output_frame.rowconfigure(0, weight=1)
        self.output_frame.columnconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)
        self.rowconfigure(3, weight=1)
        self.notebook.bind("<<NotebookTabChanged>>", self.tab_callback)

        parent.protocol("WM_DELETE_WINDOW",
                        self.on_quit)

    def tab_callback(self, event):
        # we check the ID of the active tab and ask its position
        # the order of the tabs is pretty obvious
        active_tab = self.notebook.index(self.notebook.select())
        if active_tab == 0:
            self.run_tooltip.set_text(_("Run wmllint"))
            self.run_button.configure(command=self.on_run_wmllint)
        elif active_tab == 1:
            self.run_tooltip.set_text(_("Run wmlscope"))
            self.run_button.configure(command=self.on_run_wmlscope)
        elif active_tab == 2:
            self.run_tooltip.set_text(_("Run wmlindent"))
            self.run_button.configure(command=self.on_run_wmlindent)
        elif active_tab == 3:
            self.run_tooltip.set_text(_("Run wmlxgettext"))
            self.run_button.configure(command=self.on_run_wmlxgettext)

    def on_run_wmllint(self):
        # first of all, check if we have something to run wmllint on it
        # if not, stop here
        umc_dir = self.dir_variable.get()
        if not umc_dir and self.wmllint_tab.skip_variable.get():
            showerror(_("Error"), _("""No directory selected.

Please select a directory or disable the "Skip core directory" option."""))
            return
        # build the command line from Python interpreter path and wmllint tool path
        wmllint_command_string = [sys.executable, os.path.join(APP_DIR, "wmllint")]

        mode = self.wmllint_tab.mode_variable.get()
        if mode == 0:
            pass
        elif mode == 1:
            wmllint_command_string.append("--dryrun")
        elif mode == 2:
            wmllint_command_string.append("--clean")
        elif mode == 3:
            wmllint_command_string.append("--diff")
        elif mode == 4:
            wmllint_command_string.append("--revert")
        verbosity = self.wmllint_tab.verbosity_variable.get()
        for n in range(verbosity):
            wmllint_command_string.append("-v")
        if self.wmllint_tab.stripcr_variable.get():
            wmllint_command_string.append("--stripcr")
        if self.wmllint_tab.missing_variable.get():
            wmllint_command_string.append("--missing")
        if self.wmllint_tab.known_variable.get():
            wmllint_command_string.append("--known")
        if self.wmllint_tab.spell_variable.get():
            wmllint_command_string.append("--nospellcheck")
        if not self.wmllint_tab.skip_variable.get():
            wmllint_command_string.append(WESNOTH_CORE_DIR)
        if os.path.exists(umc_dir):  # add-on exists
            # the realpaths are here just in case that the user
            # attempts to fool the script by feeding it a symlink
            if os.path.realpath(WESNOTH_CORE_DIR) in os.path.realpath(umc_dir):
                answer = askokcancel(_("Warning"), _("""Core directory or one of its subdirectories selected in the add-on selection box.

The tool will be run only on the Wesnoth core directory."""), icon=WARNING)
                if not answer:
                    return
            else:
                wmllint_command_string.append(umc_dir)
        elif not umc_dir:  # path does not exist because the box was left empty
            answer = askokcancel(_("Warning"), _("""No directory selected.

The tool will be run only on the Wesnoth core directory."""), icon=WARNING)
            if not answer:
                return
        else:  # path doesn't exist and isn't empty
            showerror(_("Error"), _("""The selected directory does not exist."""))
            return  # stop here
        # start thread and wmllint subprocess
        wmllint_thread = ToolThread("wmllint", self.queue, wmllint_command_string)
        wmllint_thread.start()
        # build popup
        dialog = Popup(self.parent, _("wmllint"), wmllint_thread)

    def on_run_wmlscope(self):
        # build the command line
        wmlscope_command_string = [sys.executable, os.path.join(APP_DIR, "wmlscope")]

        if self.wmlscope_tab.crossreference_variable.get():
            wmlscope_command_string.append("--crossreference")
        if self.wmlscope_tab.collisions_variable.get():
            wmlscope_command_string.append("--collisions")
        if self.wmlscope_tab.definitions_variable.get():
            wmlscope_command_string.append("--definitions")
        if self.wmlscope_tab.listfiles_variable.get():
            wmlscope_command_string.append("--listfiles")
        if self.wmlscope_tab.unresolved_variable.get():
            wmlscope_command_string.append("--unresolved")
        if self.wmlscope_tab.extracthelp_variable.get():
            wmlscope_command_string.append("--extracthelp")
        if self.wmlscope_tab.unchecked_variable.get():
            wmlscope_command_string.append("--unchecked")
        if self.wmlscope_tab.progress_variable.get():
            wmlscope_command_string.append("--progress")
        if self.wmlscope_tab.exclude_variable.get():
            wmlscope_command_string.append("--exclude")
            wmlscope_command_string.append(self.wmlscope_tab.exclude_regexp.get())
        if self.wmlscope_tab.from_variable.get():
            wmlscope_command_string.append("--from")
            wmlscope_command_string.append(self.wmlscope_tab.from_regexp.get())

        if self.wmlscope_tab.refcount_variable.get():
            try:
                wmlscope_command_string.append("--refcount")
                wmlscope_command_string.append(str(self.wmlscope_tab.refcount_number.get()))

            except ValueError:
                # normally it should be impossible to raise this exception
                # due to the fact that the Spinbox is read-only
                showerror(_("Error"), _("""Invalid value. Value must be an integer in the range 0-999."""))
                return

        if self.wmlscope_tab.typelist_variable.get():
            wmlscope_command_string.append("--typelist")
            wmlscope_command_string.append(self.wmlscope_tab.typelist_string.get())
        if self.wmlscope_tab.force_variable.get():
            wmlscope_command_string.append("--force-used")
            wmlscope_command_string.append(self.wmlscope_tab.force_regexp.get())
        wmlscope_command_string.append(WESNOTH_CORE_DIR)
        umc_dir = self.dir_variable.get()
        if os.path.exists(umc_dir):  # add-on exists
            # the realpaths are here just in case that the user
            # attempts to fool the script by feeding it a symlink
            if os.path.realpath(WESNOTH_CORE_DIR) in os.path.realpath(umc_dir):
                answer = askokcancel(_("Warning"), _("""Core directory or one of its subdirectories selected in the add-on selection box.

The tool will be run only on the Wesnoth core directory."""), icon=WARNING)
                if not answer:
                    return
            else:
                wmlscope_command_string.append(umc_dir)
        elif not umc_dir:  # path does not exist because the box was left empty
            answer = askokcancel(_("Warning"), _("""No directory selected.

The tool will be run only on the Wesnoth core directory."""), icon=WARNING)
            if not answer:
                return
        else:  # path doesn't exist and isn't empty
            showerror(_("Error"), _("""The selected directory does not exist."""))
            return  # stop here
        # start thread and wmlscope subprocess
        wmlscope_thread = ToolThread("wmlscope", self.queue, wmlscope_command_string)
        wmlscope_thread.start()
        # build popup
        dialog = Popup(self.parent, _("wmlscope"), wmlscope_thread)

    def on_run_wmlindent(self):
        # build the command line
        wmlindent_command_string = [sys.executable, os.path.join(APP_DIR, "wmlindent")]

        mode = self.wmlindent_tab.mode_variable.get()
        if mode == 0:
            pass
        elif mode == 1:
            wmlindent_command_string.append("--dryrun")
        verbosity = self.wmlindent_tab.verbosity_variable.get()
        for n in range(verbosity):
            wmlindent_command_string.append("-v")
        if self.wmlindent_tab.exclude_variable.get():
            wmlindent_command_string.append("--exclude")
            wmlindent_command_string.append(self.wmlindent_tab.regexp_variable.get())
        if self.wmlindent_tab.quiet_variable.get():
            wmlindent_command_string.append("--quiet")
        umc_dir = self.dir_variable.get()
        if os.path.exists(umc_dir):  # add-on exists
            wmlindent_command_string.append(umc_dir)
        elif not umc_dir:  # path does not exist because the box was left empty
            answer = askokcancel(_("Warning"), _("""No directory selected.

The tool will be run on the Wesnoth core directory."""), icon=WARNING)
            if not answer:
                return
            wmlindent_command_string.append(WESNOTH_CORE_DIR)
        else:  # path doesn't exist and isn't empty
            showerror(_("Error"), _("""The selected directory does not exist."""))
            return  # stop here
        # start thread and wmllint subprocess
        wmlindent_thread = ToolThread("wmlindent", self.queue, wmlindent_command_string)
        wmlindent_thread.start()
        # build popup
        dialog = Popup(self.parent, _("wmlindent"), wmlindent_thread)

    def on_run_wmlxgettext(self):
        # build the command line and add the path of the Python interpreter and wmlxgettext
        wmlxgettext_command_string = [sys.executable, os.path.join(APP_DIR, "wmlxgettext")]

        if self.wmlxgettext_tab.textdomain_variable.get():
            wmlxgettext_command_string.extend(["--domain", self.wmlxgettext_tab.textdomain_entry.get()])
        wmlxgettext_command_string.append("--directory")
        umc_dir = self.dir_variable.get()
        if os.path.exists(umc_dir):  # add-on exists
            wmlxgettext_command_string.append(umc_dir)
        elif not umc_dir:  # path does not exist because the box was left empty
            showwarning(_("Warning"), _("""No directory selected.

The tool will not be run."""))
            return
        else:  # path doesn't exist and isn't empty
            showerror(_("Error"), _("""The selected directory does not exist."""))
            return
        if self.wmlxgettext_tab.recursive_variable.get():
            wmlxgettext_command_string.append("--recursive")
        output_file = self.wmlxgettext_tab.output_variable.get()
        if os.path.exists(output_file):
            # TRANSLATORS: Dialogue box title.
            answer = askyesno(title=_("Overwrite Confirmation"),
                              # TRANSLATORS: {0} is a placeholder for a file name, and not meant to be modified.
                              message=_("""File {0} already exists.
Do you want to overwrite it?""").format(output_file))
            if not answer:
                return
        elif not output_file:
            showwarning(_("Warning"), _("""No output file selected.

The tool will not be run."""))
            return
        wmlxgettext_command_string.extend(["-o", self.wmlxgettext_tab.output_variable.get()])
        if self.wmlxgettext_tab.warnall_variable.get():
            wmlxgettext_command_string.append("--warnall")
        if self.wmlxgettext_tab.fuzzy_variable.get():
            wmlxgettext_command_string.append("--fuzzy")
        if self.wmlxgettext_tab.package_version_variable.get():
            wmlxgettext_command_string.append("--package-version")
        wmlxgettext_command_string.append("--no-text-colors")
        if self.wmlxgettext_tab.initialdomain_variable.get():
            wmlxgettext_command_string.extend(["--initialdomain", self.wmlxgettext_tab.initialdomain_entry.get()])
        # start thread and wmlxgettext subprocess
        wmlxgettext_thread = ToolThread("wmlxgettext", self.queue, wmlxgettext_command_string)
        wmlxgettext_thread.start()
        # build popup
        dialog = Popup(self.parent, _("wmlxgettext"), wmlxgettext_thread)

    def update_text(self):
        """Checks periodically if the queue is empty.
If it contains a string, pushes it into the Text widget.
If it contains an error in form of a tuple, displays a message and pushes the remaining output in the Text widget"""
        if not self.queue.empty():
            queue_item = self.queue.get_nowait()
            # if there's a tuple in the queue, it's because a tool exited with
            # non-zero status
            if isinstance(queue_item, tuple):
                showerror(_("Error"), _("""There was an error while executing {0}.

Error code: {1}""".format(queue_item[0], queue_item[1])))
            # otherwise it's just the output
            elif isinstance(queue_item, str):
                self.text.configure(state=NORMAL)
                self.text.insert(END, queue_item)
                self.text.configure(state=DISABLED)
        self.after(100, self.update_text)

    def on_save(self):
        fn = asksaveasfilename(defaultextension=".txt", filetypes=[(_("Text file"), "*.txt")], initialdir=".")
        if fn:
            try:
                with codecs.open(fn, "w", "utf-8") as out:
                    out.write(self.text.get(1.0, END)[:-1])  # exclude the double endline at the end
                # the output is saved, if we close we don't lose anything
                self.text.edit_modified(False)
            except IOError as error:  # in case that we attempt to write without permissions
                showerror(_("Error"), _("""Error while writing to:
{0}

Error code: {1}

{2}""".format(fn, error.errno, error.strerror)))

    def on_clear(self):
        self.text.configure(state=NORMAL)
        self.text.delete(1.0, END)
        self.text.configure(state=DISABLED)
        # the edit_modified flag is set to True every time that the content
        # of the text widget is altered
        # since there's nothing useful inside of it, set it to False
        self.text.edit_modified(False)

    def on_about(self):
        showinfo(_("About Maintenance Tools GUI"),
                 # TRANSLATORS: {0} is a placeholder for Wesnoth's current version, and not meant to be modified.
                 _("""© Elvish_Hunter, 2014-2024

Version: {0}

Part of The Battle for Wesnoth project and released under the GNU GPL v2 license

Icons are taken from the Tango Desktop Project (http://tango.freedesktop.org), and are released in the Public Domain.""").format(
                     version.as_string))

    def on_quit(self):
        # check if the text widget contains something
        # and ask for a confirmation if so
        if self.text.edit_modified():
            # TRANSLATORS: Dialogue box title.
            answer = askyesno(_("Exit Confirmation"),
                              _("Do you really want to quit?"),
                              icon=WARNING)
            if answer:
                ICONS.clear()
                self.parent.destroy()
        else:
            ICONS.clear()
            self.parent.destroy()


root = Tk()

if is_wesnoth_tools_path(APP_DIR):
    # a dictionary with all the icons
    # they're saved in GIF format (the only one supported by Tkinter)
    # and then encoded in base64
    # this is done to avoid having small files floating around
    ICONS = {
        "about": PhotoImage(data=b'''
R0lGODlhIAAgAOf/AExOK05QLVRRNFRWMlhZL1lYQF1eNFpcWWRhRF9jQ19hXmdnPWdoVWNndGdn
cGVodmhpZ2ttamtsdXNwOmpufGhwd25wbWxwfm1xf3Rydm10fG9zgXB0gnd6QnJ2hHZ4dXt7T3t9
P3Z9kXl9i3SAmHyBhIWGR4WEaoCEkomJXX+GmnuHoIWIeXmIp4iJdIaIhYSIl36Ko42Lj5CPYn+O
oIuPnoWRno+QmZCSj4uTp22ZzHSXzHOax3Kb1YqZrHedypWbkXie36KgWp2clJyem36h1p6eqIKh
3Z+hnoykz3+p0IWn3KSnhZemuaWmkKemi4yoy6qpe4qr1KansZapzpCsz42u16qsqZOt5Iyx05Ov
07GxcJywyKevt7Gzd5Sx4pay1quvv5ez16+vuZqy3qizwa6ztZS42rK0saG12ra3jqW11Z624aC3
1re0uZq615253by5hLW3tJy56ru6i7W5qZy82aO62ba3waC84Lq4vLi6t6W925+/3KXA16++0ru9
ur6/qanB4K3B2sC+wq3B58PAxMDGlcDCvq3E477DxcLEwbLG37rHx8TGw67K4bXJ4sbMm7PK6brJ
6sTI2MrIzMjKx8jJ08vKwbzL7LbO4L3M4LfP4cTN1cfOw8fNz7zP6b/P4svOytHSqMHR5MvQ0rzT
5cnR2s/Rzs/Tw8PT59DWpM3S1dDSz77W6MzT6cbV6dLU0crW5dPW0sjX69LX2cnY7NXX1Mba5s/Y
4MrZ7dbY1c3Z587a6Mvb7s/b6szc8Nnb19zfp9zdxtfc39Dd69rc2d7b4Nvhr9Le7Nne4dff6NDg
9N3f3NTg7t7g3dXh7+Hg19zg8Nni6tzi5ODi39fj8drj6+LntePmwtzk7eHk4Nnl9ODk9OPl4uDm
6OrouN7n7+Ln6uXn5OPo6+bo5eDp8efp5uHq8uXq7eXp+ejq5+jp8+Ps9Ofs7+rs6Ovxvu/s8evu
6unu8Orv8uru/u3u+Ovw8+7w7PXzwu/w+vL4xfL3+fT5/Pz78v///yH5BAEKAP8ALAAAAAAgACAA
AAj+AP8JHEiwoMGDCA+uI3SDw4MHHG4QWpewokA9FGJQKTQp06Q1OS7osXhwXg0RZOYs6bFDxw8r
kkCpqDGP5EB8NVrMOVIEDiRSsEIxAmPFl48a+Gz+w0NiTpA8sGjZ4tWLly1ab6Qwo4HHprgLX4Ik
ikq1V7JeumSZqiLGFwZxJN20wPIFFixcvpLp1Yv2jRZYTdyQRJHkiCRSsqgmcwYN2t5BbQS9QkGS
ApkeoUzZMgttGrRr1qD9YuTnjjoKJBuw0fEIFq9fzqZpC6dN2zRYmkjba0DyQhoeZ2DpSgbNWjhz
6MItk0WKE6N6F0jCgKJECiSzs5GHm7YsHC9SpKT+UbZIaAWfLHb6XYN9TbY2Vp9+yeJWxgxJfCO4
JOoTjh8/cdQQI+A2pSwjTTceUETSMRj8AUoo9yxDjjvufHPMN8exM0IlSv1TywabwHINbenMEw89
97RjzwiGdChQJTYkw4sy2qQTjzyz0GPPJVO4KNA3GEDDSye5uHPPLLHcY08YivgoEAqUJNMIEZ6k
MkQ09+jjwTZOesjBKZ8AUUcgTwyTzhRGdCkQCxV0Ucs34pBTiwwZvKPmPx1sMUYJEjigwQcMIHDn
PwaYkA042IxCRxwzDDCoExMwAc8+8AgDwgInDEqAEP7ksw84akQxAwGDDhCCF4cgs0okKSyQwKAm
/7ggAAABBABAAZnC+g8OCuyxBwQv6DqQBQccEIGwBAGyB7J3BgQAOw=='''),
        "run": PhotoImage(data=b'''
R0lGODlhIAAgAOeSAEZHQ0pLR0pMR1BRTVNUUFVWUldYVVhZVVtcV15gXWBhXGFhXmVlYmZmYmZn
YmdpY2tsZ21va3BwbXBxbXBybHN0cHh4d3p8eX1/e35/en+BfH+BfYGBfoGBf4GDfIaIhoyNiJOU
kpOVkpaXk5aXlJqamZqbmZyemqKioKqrqaysq66urLO0srS1tLa2tbe3tbi5trm6t7q6ucDBvsDB
wMPDw8XFw8XFxcfHx8vLycvLys3Nzc/Pz8/QztHR0dLS0tLT0dPT09TU0tXV1dbW1tfX19fY1tra
2tvb29zc3N3d3d/f3+Dg4OHh3+Li4uTk5OLl4OXl5ePm4eTm5Obm5ufn5+bo5ejo6Obp5efp5enp
6efq5ujq5urq6unr5+rr6uvr6+ns6Ors6Ozs7Ort6evt6evt6uvt6+3t7evu6uzu6uzu6+3u7e7u
7u3v6+3v7O7v7O7v7e/v7+3w7O7w7e7w7vDw8O/x7u/x7/Dx7/Hx8PHx8fDy7/Hy8PLy8vHz8PL0
8fP08vT09PT18/T19PT29PX29PX29fb39fb39vf49vf49/j4+Pj5+Pn5+Pn6+fr7+v3+/f//////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////yH5BAEKAP8ALAAAAAAgACAA
AAj+AP8JHEiwoMGDCBMqXMiwocOHECNKnKgQwwuKCgEw6FAD40EAkGgYCLHDI0EAg9KsOZFAhUmB
KL14mWPlggQZJmPKFFMHyIIPNzDqlOmFzJ0YB0rwmAigEJkyZqKaKQPnDIkGLYZEBIDoDZ2vYL/2
mTLBAg4kDwEs4tPnDx88b9SEwSIFSpweBEwccch1i8wwYsSE8cIlSxg9LgKseChgkRk3dO7k4ZPn
zh1CRh6IQPtwQCM6fQIVQpQo0aMvGzgIkVjAUR5AhhZBiiQoRQUbFBF8Dq2IkY4ILDwqWFSGzqEm
GVC8dIDoDpsRIJK8/AfhEQwPOaYLpKDBhfaBM74Kix9Pvrz58wUDAgA7'''),
        "save": PhotoImage(data=b'''
R0lGODlhIAAgAOf8AAABACJKhSVOgydOiSJUjyxSjiNVkCZXkjBWjDFXjShZlFJUUSpalVRVU0lX
aFVWVDVakFZYVTdck1dZVjlelVpcWTtgl1tdWkJfkTRjmS1kpjxhmD5imV1fXDBmqEFlnGBiX2Fj
YEFomWJkYT1ppWNlYmRlYz1roURqm0VrnGZnZUFsqWdoZkdsnmhpZ0htnzpwrENuq0JvpTtxrWlr
aEtwomttak1ypE5zpUt3rld2o014sF12nU55sVp4pVZ6rVh8r2B+q1t/smF/rFyAs3t9el2BtHx+
e2GCqWiAqWOBrl6CtX1/fGSCsF+Dtn6AfWqCq2WDsWGFuICCf2CHtGKGuWOHumeIr12LvWSKt4SG
g2uJt2WLuG6KrIaIhWuMtGeNunCMrmyNtWiOu2mPvHCOvGqQvXyPs2yTwG2UwY+RjniUtm+Ww3yU
sZCSj3GYxXeYwJOVknmZwnqaw5WXlHubxHydxYWcuX2exn6fx3+gyIifvYChyZyem4ahxIGiyoqh
v52fnIeixoOkzKGjoIqmyZKlvYunyqOlooyoy42pzKWnpKaopY+rzpCsz5Gt0KmrqKCsuZKu0pqt
xpOv05evzautqpiwzqyuq5mxz6Gww62vrJuy0K6wrbCyrrGzr6y0vLK0sa21vaa2ybO1srS2s7W3
tKm5zLa4tbe5tri6t628z7m7uLu9ur2/vLjAyLLB1bjD0bvDy8HDv7/ExsLEwcPFwsTGw8XHxLzI
1sbIxcfJxr7K2MjKx8nLyMDM2srMycHN28vOysnO0MrP0c3Py87QzM/RzsrS29DSz9HT0NLU0dPV
0tTW09XX1NbY1dLa4tXa3Nja1tbb3dnb19rc2dXd5tvd2tne4dze29zd593f3Nvg497g3d/h3tzi
5ODi3+Hk4ODl6OPl4eTm4+Xn5Obo5efp5uXq7ejq5+nr6Ors6eft7+vu6u3v6+7w7e/x7u3y9fDy
7/Hz8PL08fP18vT38/f59vn7+Pr8+fv9+vz/+////////////////yH5BAEKAP8ALAAAAAAgACAA
AAj+AP8JHEiwoMGDCAkGO4Piw4s1vxJKJCjOh4Qgcgr5ycKhBraJCa1ZEPPoUKNHjzhx+oIAGUiD
7F4AWqPjhpKTig49kgNB20uChmpsCBMLVhIJghT94dOIypCfAtG1IJCL4KgMjfLUmeNog8ufvJDE
MgiFy583bAYpWQP11B5qRSZcmHvBgYhCacy8wSPgQoW/EbyoQxgM2pNN+fbp05cvnoJDZMCMUVTA
Xj168+bFYYSQW7oO9VYFGD16gCIsVqo8Ij26yzYX7xCmonNv3CQKcv4MOqRIipMllS6pVJJiWjsm
uBAW6bauW7g7G+ZgWWJEiJAeMEiU2QJhmLNwtbz+HKTGAp+3cebMXUFRR0iOGB40aFiS5oAsatKk
nQPhzWCfVvCcN8443/hwwhgkyKfBDnUwoAk1EEozDiGLFNROCfAMOE443WQTTQoyoJEgDHxw0EZ+
EEaoQmwDuaJGPdSY0w1+zTRDDAU/oLECHjXwYIwxyjDTzDbMjPPELARNIY051HDYDDNQGvNKAkoU
AgQGtPQCjDDGSEONMtS0osVA27AAozTOdNONM8wYU8wuoBBwgwGi2IJLL8Lkx+Uy3YzQjUCImKJO
NjQ2uY0zxviySyQBRHKnMc58WQyQ0mTTByEC2XBOJ1Mc4emnoIYq6hFTWLIMDQI9YM8TzWTj6qu9
sMYqazbNPHFOAwItAM8R5jDi66/AmgNAesQWm94RxiyQ6zpHyAPJs9BGKw8A8lRr7bXyHKGKsv8s
kM4R+nQi7rjk6gPAYuimu9gRqHC7wDngkitvJ+aqa+8RpLhrzhPz2Ovvv/rM88Qn7o4TSKejJizq
FG5gwq0Jt3jTzTYRsrnMj8Ioqosts7jSCiuqmELKJ55sEocLApUSwgIst+zyyzC/DIIlAzWDiyqh
bKLzzjz33DMpnqiSHFRECxQQADs='''),
        "clear": PhotoImage(data=b'''
R0lGODlhIAAgAOf/AHAFAHsGCJcAAHMKC3QLBHYMAKwAAYkLA3kRCbgAAIIQB8EAAHsUEbEGDIgX
DIAaG5sYF4MkDYYmB80TDrIcFr8gHpE8A8EuLtIqKZVAGOMzM5lNEKBXA+BDQZdgEJVkEu5ISaBn
D5xpDvZITZtpGJ9sHJxvJKRwIKBzKKV3Jat2HqV4NKZ5Lrd6EcB7CK5/NK2AQrKDOMKDEcWGIcKJ
KrqKP7OLSr2PGLyPI7SNU7WPWsiPOMOSP8KSTb+VTsWUScOeCcWfAMChAMiiAMahIMWlAMOkEsuk
AsenAb2mM8ifXb2gdMqpCNOgTceoJcmqG8GqN72pTc6sAMysDsysH8Smesaio8+mas2uLNOxAdWo
WcyxI9WpYcmuU9C0BM6zGMyyL8WxVNOyJ9OvSNKyMceqqbmxl9m2D9G2Kda5ALa0mdO4H9KweNa6
E9S9ANi7Fc2vrtu9AMm1ks65XNi8JdO+Jde9OtS8Ude9Q9rCCdjBHLm7uNq/PdjCKuHCENi/TdW/
Wt3FENy8b8W/mOHIANXBacHAt9nFSdzGOeHIGNG/odnDZdzKF8DCvtHDhNzHQ83Cot/IMufIG+DJ
KN6/ksfCweHBjdnGdc/GmeXMHt/NKt7KTcTGw97NNd3KVujOD+nLLeLMP97MXujPI+TFl8fJxuPO
St3Lf+TGn+XPQ+jQMefQO+HRQ9HLtsrMyeXQVO7UHO3TKd7QdeLTTd7QkOLScfHWIOfWP+fTXuzY
H/DWLdrNzs/Rzt/SmObVbebYYd/Rvt3Up+/bMejZW+fXduDVodPV0vbbNPLbT+7aZencZNXX1PXc
SOvdXu7bbeHZq+ndbOnbgPneN+/fWfjiLvXhQtnb2OPctPjgVNvd2uPfvPrlPe7jePDhhvflT93f
3Obd1vflV+7ilN7g3fvkYPvnSPXkg+7km+rhxuDi3ubh3/noYvLml+Hk4Ovkz+vj2/fojOPm4urm
1+bo5f3udurp4P/uhenr6PrwjO3s4/Dr6f/wjevu6u3v7P31sv///yH5BAEKAP8ALAAAAAAgACAA
AAj+AP8J1Jfjwwcd7wQqXMiwYcMlMVBR8mHinsOLGP+VoCTiA4sXcjKKXOiBDYp+JK6Q6DdyJAwl
JUiEIHVCUUuRwD5Q4uGiyQ8WN0VWSWFphgwuHoAFxdhvRQ0tLnaosLEoCMulDMF94EGjxY0gj4q4
w9pQ0QccQMSYY7eBgRWyC90BOZIMXw8EECgMUAf336kgdZqNieCgAwgMAeD0vRaESAYCDTSMuADA
ApZnffMhELBgQgUFEgTtc7YF0FWsDwwkOFCghz965bxx4wOmHtkyAwBwgGePHjlv1GIp80RFHtld
XUytK7cOmbdjuRLNqvUEHVlApsJtY4ZN1bFRmdLgzIpGRRvWS4imUdNVDVQsSZkmeTElbkuvpb3Q
NINlK5a0M7G8QUgkUiByDh6A5HMTOlNAkwgsflCTSBvyBRKKFH2IQ10xLfVTxC95fBIHLNUUkcka
XjDCyhRfiHNOEbS0dAcikeSRRxzCELgKE25oMgsfTJBRyE3BfIGLF4kw8ck0U+gRViDd+DLEHEs9
8QofbtCRxS2pFMHKJkygYQQkWDmyBTFedFJEHsPYYYcnQ0BhCFxOPCJLKLgUQYwnQSRhRl//tDKE
KH8c8oQQUagBqEKYPOFEGINUsuiklFYaVEAAOw=='''),
        "exit": PhotoImage(data=b'''
R0lGODlhIAAgAOfxAKQBAKMCBKUEAKUEBaYHBqcJB80AAKcKDs4ABM8ABKgMB9AAD6gMD6oOCKkP
EKsRCdMHEckMFKUYFcERD68ZG7AaHLMdHrIeI7QfJMgdHNIcGrAmJLInJcohHdMeItQgIrQqLM4m
JrcuLtEqL9UuLNcwLNkzNFZXVdM2MlZYVdo0NVdZVtU4NFhaV9U5OVlbWNY6OlpcWdc7O1tdWtg8
PMtAPFxeW15gXds/Pto/Q19hXmBiX91BRWJkYctJRWNlYtlGRWVmZNtHRmZnZYdfXWdoZtxKTWhq
Z2lraN5LTmpsac1ST2ttas5UVm1vbNxRUMpXVW5wbd5SUdBWV29xbt9TUcxZV3Byb99UWHFzcHN1
ct1ZWXR2c95aWnV3dHZ4dXd5dnh6d3l7eHp8ed5iY3x+e31/fH6AfeJlZn+BfoCCf+RnZ4GDgIKE
gYOFgoSGg4WHhIaIhYeJhoiKh+FzcomLiMl6eIqMiYuNioyOi42PjI6QjY+RjpCSj5GTkJ6Qi5KU
kZOVkpSWk+iAgZWXlJaYlZeZlpial+aFg5qbmJudmZyem52fnJ6gnZ+hnueOjqCin6GjoOmQkMaa
l6KkoaOloqSmo8Ken6WnpKaopaeppqiqp+uYlamrqM+in6utquidnqyuq+men62vrM2oqeyhobCy
ru2iorGzr+qmpLO1stqsquGtrLe5tri6t7m7uLq8ubu9ury+u72/vO6ytL7BvcDCvsHDv9u9vMLE
wcPFwsTGw8XHxN/Bv8bIxcjKx8nLyMrMycvOys3Py87QzM/RztDSz+vNy9LU0dPV0tTW09XX1NbY
1dnb19rc2dvd2tze293f3N7g3eDi3+Hk4OPl4eTm4+vl5OXn5PLk5ezm5ebo5efp5vPm5ujq5/Tn
5+nr6Ors6evu6u3v6+7w7e/x7vDy7/Hz8PL08fP18vT38/b49Pf59vj69/n7+Pr8+fv9+v//////
/////////////////////////////////////////////////////yH5BAEKAP8ALAAAAAAgACAA
AAj+AP8JHEiwoMGDBSGdWMiwYYoWOo5c8QIGDJconRAKPKEqj5szYLIwKdLjxgwdSJqhS8cSnTcv
Gv+daLVHjpoxXqIgGdJjxxAw1Naxa9eOnbo4MU/IApTnTRoxXJzsDMKkzbZ27rK6Ywco6a5EfOq8
MRNGSxQlSLLM4ebunduthpICc0RoT52PYbxQifJFD7i28N65a5coaTBKiwjpmePmqRctY/iEAyyY
sMBrvQyeEFapUSJBdt+cGRPmDCBxlAcXviaigOZhmBwlItRn8ccybQiNS01YGwgoFTQTywRpUSFA
eurAUZPmjaHd7wIP/rPBCq3gBU8Q0xSJkaFAe/D+5OnUSE8i6NK/SZgiKhUFBfAVbJBJbFPnz3zy
3BmlChUm9PB8g0ETkqDBSSmnnFIKKALQ14klshHiR3JeuPGKK+h9c8ESg2DxxBZddLFFFZw0qF0n
mERiHHJ3wJFILpVAl40FPtABBA9AGJFEEkYA8YiJxHSSCSWMJBIIH3gAAswmfpDjTjcV1ECGCibI
QAMOMsCAAw2IANmJfZ4J0gcfubRyRiJOHqOACzR88IEGCyBggAEILLCGlw9CkkghfrwyixlvQOLk
O6woMAIJcs6pqAFV4JniIoY08sodbORByaDvrKJACIjSicCnCDyBpyaUeHZkHXfwYUk5baEDjif+
BXSAqAw45IAjEAE4+GUlEdaGRx+YcMMOO+Vs08wkA2RAAgsFCOCsAA7oiqKefO6hhyCb7FLNOuZs
Aw0xRBAQAQoMCNfJuZl0Z4gggACSSCawMHNOOuFQs4walxAwQbnZBXmuJrwmIvBnmvBSjTnonAOO
NG78Q0oBCph77iaYUFKJJYEEogoy1YhDzsficPOGQLjYIfG5nYwClinENEPNNjDDXA01ZiTl7yev
VNIHKsEs00w00UgjtNDRNMNFUsOEEosphqjiyzDIJLPM1FRPrQwyTCA9Syaq1OLLL8GELczYZI8d
DDA/JOVKK7DMYsstueSiy9x0152LLTbEpFAYCiu08EIMMcwg+OAz2EB4DJHEpPjiCAUEADs='''),
        "browse": PhotoImage(data=b'''
R0lGODlhEAAQAMZ6AFpaWlxcXFxcXV1dXTNkpDRlpGBgYDZmpTdmo2FhYTdnpThopThopjlopDlo
pWNjYztppjtqpjxqpWZmZmdnZ0BtqGpqamtra2xsbElxpW1tbUhzq29vb013r0d5tHNzc095rnZ2
dlR9snp6emp/mWiAn35+fn9/f4CAgH+FjlmMw4aGhlyOxGCPw2iPvY+Um5WVlXuawJmZmZqampub
m5ycnJ2dnZ6enp+fn6CgoKGhoX6n1H+o1KOjo4Gp1aSkpIKq1aWlpKWlpYSr1qampoar1IWs1qen
p4au2Iiu2Kmpqaqqqqurq4uw2Yyw2aysrI2x2q2trY6z2q6urpK027CwsJG125K125O125K225O2
25S23LKyspS33LOzs5W43Ju73re3t52937q6uqK/3729vcDAwMHBwMTExK7I5MXFxbHK5MfHx7PL
5sjIyLTM5snJybXN5srKyrfO5rfO57fP57jP577T6cHV6sTX6/Dw8PDw8PDw8PDw8PDw8PDw8CH5
BAEKAH8ALAAAAAAQABAAAAfOgH+Cg4SFf0QoiYkrhoMmcGxqaGhcMI1/I2pVUUxLSycaGBcXFox/
IWdPS0dCP0pjam5yZhOCH2VKRD06OTg4OTo9PQmCHGG6OTY1NDM0NTc3A4IWXj0ZDtjZ2QgQIhRT
OQtvdHR1c+foawQPSjcNd1lWV1n0WVpfbQcGQTUSeF0AA3aRgiSOggA6aFTIA6Zhwy1NkkCxwwAA
jRcg0mCh4sQIjx0ggZCJMEBGCRdimgDxwbLlkCIdBJBw0IKFips4cXqIkaKAz59Af274EwgAOw=='''),
        "clear16": PhotoImage(data=b'''
R0lGODlhEAAQAOeIAKsbDbg4HX9QCaxDFKxDJYRUCoJUEKhJGYpYDIlZDIZaGLtKKIdbGothIoxi
IqZpCqlrCqttC7FsHbJyDLJ2C8htM7x6D5uHAKGIAcF9EJyLAKCQCKGRC6KRDaGSEKOSD6SSEKOT
EaOTEqOTE6SUEaWUFKWWF6aWF6qOY6eXG6qZLqydIa2eKK2fKrChJLOjJ7CjNrynL72tMbqodrum
h7mtULmuUbuwVsGyNryyW72yWsS0NMa1M8W1OMW2NdexYs29QcS7dcy+R9rCA9vDBM7ASNzEB9zF
ENHCT9zGENzGFcnBgNXHVc7Brd/KJtjIT+PLEdDFltDDsOXNFNDJlufQGNzNWNPIuNLMm93PXuTS
SeTTTOHTW+XUTu7XI+LUZ+bWVufXW9jUsOfYX/PbKu3aRtrVs+fZbdrWtOjabNvXtvbYYfbeL+zc
aOrccuvdd+ved/vkN/PjZfzlPPvlRP3mPOPgzOTh0OTi0v3pUubk1v3qX/3qY/3rYf3rbPjqiPvq
hP3sa+jn3P3sbfvti/3uff3vhP3vivDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8CH5BAEKAP8ALAAAAAAQABAA
AAjBAK84YNDkn8GDCA0qeBBBgJSEEA1MyAChwb8gEA/SQGAhQwIVGjIeRFGAAgYZL9SINDjjAhA4
Eg60WCIIIpYNT9pUAPDDjRAQOvQgpCIixoIAa/j4KfRHCIcbZhASGAAokKFBeejIeVPExYiDUXAc
2tMnTx0yVZJo+VIC4QlCc+Kw8TKFyBAwPmwgZJGlTBUoRoYM2WKlQ0IxK9IcEawkDBMSaCDC6DHG
SZczO0zYEZkCCRceH2qs/IfHQ4gcdxIGBAA7'''),
        "cut": PhotoImage(data=b'''
R0lGODlhEAAQAMZ5AKYBAaYCAqcDA6YFBacFBagFBKYGBqgHB6gKCqkLC6sREbYPDqsUFK0ZGa4a
Gq8bG6wcHK4dHcAZF60fH8AaGccaGckaGrAiIrAjI7AkJMwdHM0dHbAlJcofH8wgIM4gINAgINEg
ILEpKc8hIdAhIdIhIdMiItAjI84kJMslJNQjI9UjI7MuLp42M9onJ7UxMbc3N7pBQaJJRLtERLtF
Rb5YVs5qas5ubomJhMt4eIqMh4uNiMx7e42Pis1+fo+RjI+Rjc2AgJCSjpGTjs+GhpWXkpiZlpia
lZqbl5qcl5udmZ6fm6Gjn6aopKeopKippamqpqmqp6qsp9ehodiioq2uqrCxrrGyr7Oyr7KzsLO0
sbO1sLS1srW2srW2s7a4s7e5tLm6tr6/vMPEwcXGw8bIw8vLyuPFxczOyc3Oy9HSz9PT0tLU0NbW
1djZ2Nvc2dvc2+Hh3+ri4ufo5u7v7vT19Pb29fb29vf39/Dw8PDw8PDw8PDw8PDw8PDw8PDw8CH5
BAEKAH8ALAAAAAAQABAAAAe7gH+Ca0NMgoJaRYeLQndNY4dGak6LgjpsdFaCbVJlP5V/Q1t2SJA/
aFVKoG47b3RRXF9pOmKgf0ddeEBLXmBQtn9wOnF1SU89cMB/V2FzUj1Zyn9mOGRYLcnAORMGMgEV
GDygRAchGws1BRYgHQhTh1QEIzMUAH9yGhI0HgNEgiIqfPxBkUCQgBR/goAQIUjBhz83SrwQBGOF
jT8kGAhi4eLEhghnBJ15YGLEBg6CqFxoACEkvBgOMoQMBAA7'''),
        "copy": PhotoImage(data=b'''
R0lGODlhEAAQAKUeAIiKhYmLhoyOiZialZialpyemqGjn6mqp62uq6+wrbu8ury9ur2+u8PEw8fH
xs/QzdDRz9TU1NnZ2dra2tvb2+Pj4uPk4uzs7O3t7e7u7e7u7u/v7u/v7/Dw7/Dw8PHx8PHx8fPz
8/T09Pb29fb29vf39vf39/j49/r6+fr6+vv7+/z8+/7+/f////Dw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8CH5BAEKAD8ALAAAAAAQABAAAAaKQIhi
SCw+fsgfw8RsOhVJpYmzqVY5HBM0ueRQDoCweIzocgKttHoN6FIArY58Lm+1pxt0x8Hvd+xLGxNw
dHSAJhoZen18dW0kGBKEhR0pIW0iGBeLjBolDW0fIB56dAKnAgAQC6wALHQpAyknFgBRAShzKbEp
FQAFtyIlIysqEQS1W0kJAWNhBltBADs='''),
        "paste": PhotoImage(data=b'''
R0lGODlhEAAQAMZZAGpDAmtEA2xEAXBJB3BKB3FKB3FKC3JLC3JNDnNNDnNOEHROEHRPEHVPEFxc
W1xcXF5eXmZoZGdpZGpsaG5sZG5tZHBtY3BtZHFvZHNvZH5+e39/fKF8QKN8PYCAfbN7Iqd9O6R+
Prl/I7p/I4WFhMCEJMKGKMWHJsWHJ8aIJ5WViZeXirqrkburkb6wmL+wmLGysrK0tLO1tbe3tLi5
tbm5trm6tru7u8HCvszNys3Oy9jY1dnZ1tra2Nvb2eDg4Obk4Ofn5Ofn5ejo5unp5+rq6Ovq6Ovr
6evr6uzs6uzs6+3t6+3t7O3u7e7u7e7u7u/v7e/v7u/v7/Dw7/Hx8fLy8v7+/f7+/v////Dw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8CH5
BAEKAH8ALAAAAAAQABAAAAe6gH+Cf1IkD4cPJFKDgi0NAgEOKisrKg4BAgwvf0wHJSEXEBseHhoQ
FBwiCU9ABSkYVD83szc/VRUoAEatKRFYU8DBWBImuq0nvlMyyzJRw8W7Bci/wVBLz8YE08oyMTBF
2LvayVFMSEU+4UDjWE5KR0Q9OersSUVDOjY06gPTREI8amAZSMxYP19BduCYQRAFNCD9LDS5QjGK
FQkoMupiYmAEiAwTQobsQPIDgid/WCwQwLKlSwUu/gQCADs='''),
        "select_all": PhotoImage(data=b'''
R0lGODlhEAAQAKUZAAAAAIeJhIiKhYqMh4uNiIGXr4KYsLS1s7W2s7W2tKi+1qm/16rA2KvB2azC
2q3D267E3K/F3bDG3rHH37LI4LPJ4evr6+zs7O7u7vDw8PLy8vT09PX19fb29vf39/j4+Pn5+fr6
+vv7+/z8/P39/f7+/vDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8CH5BAEKAD8ALAAAAAAQABAAAAaWwN9B
QCwaiYjfL0AqlUik0UgkCoVAA6WApFgsGIyGWPwhaEuLgrrgWDvKWhJjXXjY7fDfdr5+QP4WeQIj
DXQQahEXgiMODn4RERKSGIuOEJCSExMZgiJtdGoUGh5meiKPkgAUABUbpFohqBOrHB0dr3ogDwa8
BqseHx64ArqXErMAHiDBpQEfz9AAH9LPWT8IR9kCCT9BADs='''),
        "window_icon": PhotoImage(data=b'''
R0lGODlhEAAQAMZqAAogQA0lRA4nQBMmQBAoQRUoQRYoQRYpQhcpQxcqQhgqQRgrRBYtQhctQh8w
SSEwRSMxQiIyRSAyTTE9RTlCSjtDSjRFYDVFYDdFXUBFSjVGYzdGZDpJYkRKWFZKQ1dKQ1ROSlRO
S1FQSVZQQFNSTlZST1tSQWpTRVdYVUxZcWxURmVZRVNcbWdbSndgQlhkfXVjVYVjOY5jNIxjPY9l
NGdrcIZnPY9lPoBpSI5oO4RuPJBrP3pwWIZxQYpvT4RxT5RxRaJ2KJ55SYt9apx8UYSCgaSAUpmC
Y62BNaCCVpmEbaGEWYmJiKOResqQL56XfaKWgc2XOKSfm6yghdidIqyoj92lK6yurq6vsN6xW7m5
turIeO7MeevMhu3Nfe7SjevWn+zZq+3aou7aofjdj/bfmfLgo/zim/fkmfzurP//////////////
/////////////////////////////////////////////////////////////////////////yH5
BAEKAH8ALAAAAAAQABAAAAeOgH+Cg4SFgx+GhB6EIVqDQzCDWiCEVypHNSkcDggZPidYhUYWL0xV
TygGDDKGSxtTaWJjZz8EO4ZCGlBmYWBlPAI0hkAXRWhfXWQkDTmGNh0sXF5bWRATMYklGEhRTkEK
Iol/LRImVFYjAyviTRUBOj0AFEqJUn9JEQcFD0R/9uI4EixwIa7QjRkFExIKBAA7='''),
        "process-stop": PhotoImage(data=b'''
R0lGODlhIAAgAOf/AIgAAZAID7gAALkAAL8AAMEAAMIAALsCAZwKD8MAAMQAAMMABsYBAMwAArwF
AsYCCMcEAI4UD6AREsgHAL4LBKIUFKEUGZEZGJoXFsoLAZsYHLURF8kMFJUdIJ0bHrgVGaYaHcIT
EJ8dGp4dH5AiJM0TDqAfILoZGpMkIM0UFqkeJIwnKcQYGc4WHr4eHcccG9EaGacmJZIsLckfHdMe
G9UgHMsjJdcjJMcqKtglK9knJsotLNspJ80wLdUuLN4tKc0xNNYwMs8yNdgxM9A0NtE1N9kzNNI2
N9o0NdQ3ONU5OdY6OuY2Nd04Pdc7O9g8POA6Oeg4N+k5N9o9PZlPT9s/Pto/Q+s7PtxAP9xARN1B
QJhUUd1BRd9CQN5CRqBVVdlGRelDQOFFSNtHRtxIR+tGR+VIRd5KSPVFRuZJS+5ISa1aXKhcW+FN
S+lLSOBNUP5GSv9HRapeXeJPUqtfXuRQU+tPUK1hYPtNUfBSU/dRUKtmaPlSUa5oau1ZWudbXvdZ
WfZZXuteW+RgXrBwcOZiYPpdYe5hY7JyculkYuhkaLR0dPdiYvJlZu1nZbd2dvpkZO9pZ/BqaO9q
bbV7d/5nZ/dpa7V9f/lrbPJtb+5wb7iAgvZxc/5vcLWDg/Fzcvlyb/Z3du95evl5ePN8ffZ/gPOD
gPqCg/OEh/WFgvyEhfaGg/iIi/KKiqCin/+Iiaahn/qKjfWMjPCOjPaNjaKkofiPj/GRlP+OjPmQ
kKWnpKaopfaUkamrqPmWk6qsqfyZlrKtq/uZnK6wrfednbCyrrK0sbO1ssqzsO6qqLe5trm7uLy+
u8i7u72/vO+ytMu+v8DCvsHDwO23t/O2uM/Cw/C6uve6vMbIxfK9vMfKxvW/v8vNyvLDwM3PzPLF
yM/RzvbHxNLU0dPV0vnMz/XQ0PfR0ufa2urd3ebh3/vc2+3g4ePl4eTm4/Dj5Ovl5OXn5Obo5e3n
5ufq5u/p6Ors6Ovu6u3v7PPu7PXv7u/x7vDy7/Hz8PL08fP18tbW1iH5BAEKAP8ALAAAAAAgACAA
AAj+AP8JHDjQE4CDCBMeZEOwoUOBmzC04kWRYi5btmixWhXjzsOPlzSYSlSlZJUnKFE6mUNKxZ6P
DSl5MKXIik0rJlM+WSkKRB+YAmWiSmSTC5ebJXWu/ASCEMxHI4YaBUMVzFGcJ1U6eaNJxKKHi6IW
4lK17NWsKpe8mSTiUUNEJlAVskIWTBo8Z6raLBkGzZMlSo68cWTh0kBCcQcVtcsomR4yVPeWwTQL
DeAjR84kwuBJIABaf0rSNWPoHLtnerxcnZyO3S00gY8QOeMIgOdcKKvYhJOMXbx4qLVYUcPp3e9z
cZocKUIEiCTb/wDgfiJai55p87JTA2Qn1Lvs6CD+aVlOpHkk6NJTmuQCyFr2etZGycu+DpMYzMyB
ADl/Wyf1Klgwso099xRYoDugnIFfc/o1gl4uTjjh3xNYYLLNPfpkKM8oCpK3ww444CAIerYsEeGE
fqiyDz8smtPJETZwsMACCSRQgANXkIiEiRGe6Ecu+LDYTz/8bKMHBDXWeOMAUqBHCxJQLiGlE4L4
kg8//uCjDpH7bBMIjTYOIGYU6MlihBFQQimIMEH6k88plYTjDz/7dNPIAgWIOYAATKDHShBnnnkI
MW3mU8oNE2BCzpz6fMMJBWIKwCd6qbQRxKVBnJLNiviccoMCCSwQijn97LPOKyHsKcAAfQrERgz0
oliKKSnX0BNLDqDWuEAp5rhjywuqDqBDC9D9s0YMmUABQwkTTJAJLj7kmuQCsgADrKQD8DBDAMgQ
xIYFedDQAATkpiCtkg5QkCq2PLwQQTMO0VGBHTRAwAAD59rogJ7ssnABNB/dIQG9+CaJLr8COMCD
Cx1UA5QcErhBg8H6IuzADyegADBM3ETzhQRlwJDkkpEm/MMHJATDDDbjPDSONs4cQwUCaqhRRhlh
hHHFFVJEwQQTG6wASzHLsPyRONgwY8wWCjUNgAxDF90yUOJwI40yxfyySy2udO2KLr0Mo4w03EwN
VEPjiOPN2tys7Y04Zj8UEAA7''')
    }
    ROOT_W, ROOT_H = 1024, 600
    # the following string may be confusing, so here there's an explanation
    # Python supports two ways to perform string interpolation
    # the first one is the C-like style
    # the second one is the following, where each number enclosed in brackets points to an argument of the format method
    root.geometry("{0}x{1}+{2}+{3}".format(ROOT_W,
                                           ROOT_H,
                                           int((root.winfo_screenwidth() - ROOT_W) / 2),
                                           int((root.winfo_screenheight() - ROOT_H) / 2)))
    root.title("Maintenance Tools GUI")
    root.rowconfigure(0, weight=1)
    root.columnconfigure(0, weight=1)
    # set the window icon
    # for now, it's just a grayscale Wesnoth icon
    # also, this line shouldn't have effect on macOS
    root.tk.call("wm", "iconphoto", root, "-default", ICONS["window_icon"])
    # use a better style on X11 systems instead of the Motif-like one
    if root.tk.call('tk', 'windowingsystem') == "x11":
        if additional_themes:
            # if ttkthemes is available try using a new theme
            # some of them have issues, but this one seems to work fine
            # TODO: at some point, add a preferences dialog to allow changing theme
            # and a preferences file to keep track of it and other settings
            style = ThemedStyle()
            if "keramik" in style.theme_names():
                style.set_theme("keramik") 
        else:
            # no ttkthemes, clam is a built-in theme
            style = Style()
            if "clam" in style.theme_names():
                style.theme_use("clam")
    app = MainFrame(root)
    root.mainloop()
    sys.exit(0)
else:
    root.withdraw()  # avoid showing a blank Tk window
    showerror(_("Error"), _("This application must be placed into the wesnoth/data/tools directory."))
    sys.exit(1)
