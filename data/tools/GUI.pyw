#!/usr/bin/env python2
# -*- coding: utf-8 -*-

# By Elvish_Hunter, April 2014

# You may notice that this script, unlike all our other Python mainline scripts,
# has a .pyw extension, instead of .py. *This is deliberate*. On Windows, .pyw
# scripts are started directly in GUI mode, without opening a prompt.
# This is, after all, the behavior that we want.

# threading and subprocess are needed to run wmllint without freezing the window
# codecs is used to save files as UTF8
# Queue (queue in Python3) is needed to exchange informations between threads
# if we use the run_tool thread to do GUI stuff we obtain weird crashes
# This happens because Tk is a single-thread GUI
import sys,os,threading,subprocess,codecs

# sys.version_info checks the interpreter version
# this is used to have a script that can run on both Python2 and Python3
# not that useful until the mainline tools are updated, but still...
if sys.version_info.major >= 3:
    import queue
    # tkinter modules
    from tkinter import *
    from tkinter.messagebox import *
    from tkinter.filedialog import *
    # ttk must be called last
    from tkinter.ttk import *
else: # we are on Python 2
    import Queue
    # tkinter modules
    from Tkinter import *
    from tkMessageBox import *
    from tkFileDialog import *
    # ttk must be called last
    from ttk import *

# we need to know in what series we are
# so set it in a constant and change it for every new series
# it must be a string
WESNOTH_SERIES="1.13"

# get the location where the script is placed
# we'll check later if this is a Wesnoth directory
# and use it to generate the command lines
# os.path.realpath gets the full path of this script,
# while removing any symlink
# This allows users to create a link to the app on their desktop
APP_DIR,APP_NAME=os.path.split(os.path.realpath(sys.argv[0]))
upper_dir=APP_DIR.split(os.sep)
upper_dir.pop()
WESNOTH_DATA_DIR=os.sep.join(upper_dir)
WESNOTH_CORE_DIR=os.path.join(WESNOTH_DATA_DIR,"core")

def wrap_elem(line):
    """If the supplied line contains spaces, return it wrapped between double quotes"""
    if ' ' in line:
        return "\"{0}\"".format(line)
    return line

def run_tool(tool,queue,command):
    """Runs a maintenance tool with the desired arguments and pushes the output in the supplied queue"""
    if sys.platform=="win32":
        # Windows wants a string, Linux wants a list and Polly wants a cracker
        # Windows wants also strings flavoured with double quotes
        wrapped_line=map(wrap_elem,command)
        queue.put_nowait(' '.join(wrapped_line)+"\n")
        si=subprocess.STARTUPINFO()
        si.dwFlags=subprocess.STARTF_USESHOWWINDOW|subprocess.SW_HIDE # to avoid showing a DOS prompt
        try:
            output=subprocess.check_output(' '.join(wrapped_line),stderr=subprocess.STDOUT,startupinfo=si)
            queue.put_nowait(output)
        except subprocess.CalledProcessError as error:
            # post the precise message
            showerror("Error","""There was an error while executing {0}.

Error code: {1}""".format(tool,error.returncode))
            queue.put_nowait(error.output)
    else: # STARTUPINFO is not available, nor needed, outside of Windows
        queue.put_nowait(' '.join(command)+"\n")
        try:
            output=subprocess.check_output(command,stderr=subprocess.STDOUT)
            queue.put_nowait(output)
        except subprocess.CalledProcessError as error:
            # post the precise message
            showerror("Error","""There was an error while executing {0}.

Error code: {1}""".format(tool,error.returncode))
            queue.put_nowait(error.output)

def is_wesnoth_tools_path(path):
    """Checks if the supplied path may be a wesnoth/data/tools directory"""
    lower_path=path.lower()
    if "wesnoth" in lower_path and \
       "data" in lower_path and \
       "tools" in lower_path:
        return True
    return False

class Popup(Toplevel):
    def __init__(self,parent,tool,thread):
        """Creates a popup that informs the user that the desired tool is running.
Self destroys when the tool thread is over"""
        self.thread=thread
        if sys.version_info.major>=3:
            super().__init__(parent)
        else:
            Toplevel.__init__(self,parent)
        self.transient(parent)
        self.grab_set()
        self.protocol("WM_DELETE_WINDOW",
                      lambda: None) # disable close button
        self.resizable(width=False,
                       height=False)
        frame=Frame(self)
        frame.pack(fill=BOTH,expand=YES)
        wait_label=Label(frame,
                         text="{0} is running\nPlease wait...".format(tool),
                         justify=CENTER)
        wait_label.grid(row=0,
                        column=0,
                        padx=5,
                        pady=5)
        wait_progress=Progressbar(frame,
                                  mode="indeterminate")
        wait_progress.grid(row=1,
                           column=0,
                           sticky=E+W,
                           padx=5,
                           pady=5)
        frame.columnconfigure(0,weight=1)
        # place the popup in the middle of the main window
        # get the main window position and dimension
        self.geometry("{0}x{1}+{2}+{3}".format(400,
                                               80,
                                               int(root.winfo_rootx()+(root.winfo_width()-400)/2),
                                               int(root.winfo_rooty()+(root.winfo_height()-80)/2)))
        wait_progress.start(10)
        self.check_thread_alive()
    def check_thread_alive(self):
        """Checks if the thread is still alive, and destroys the window if it isn't"""
        # placing this in a for or while cycle freezes the app
        # so we need to use the .after method and recursively call the function
        # that's one of the many quirks of Tkinter
        if self.thread.isAlive():
            self.after(100,self.check_thread_alive)
        else:
            self.after(1,self.destroy)

class ContextMenu(Menu):
    def __init__(self,x,y,widget):
        """A subclass of Menu, used to display a context menu in Text and Entry widgets
If the widget isn't active, some options do not appear"""
        if sys.version_info.major>=3:
            super().__init__(None,tearoff=0) # otherwise Tk allows splitting it in a new window
        else:
            Menu.__init__(self,None,tearoff=0)
        self.widget=widget
        # str is necessary because in some instances a Tcl_Obj is returned instead of a string
        if str(widget.cget('state')) in (ACTIVE,NORMAL): # do not add if state is readonly or disabled
            self.add_command(label="Cut",
                             image=ICONS['cut'],
                             compound=LEFT,
                             accelerator='Ctrl+X',
                             command=lambda: self.widget.event_generate("<<Cut>>"))
        self.add_command(label="Copy",
                         image=ICONS['copy'],
                         compound=LEFT,
                         accelerator='Ctrl+C',
                         command=lambda: self.widget.event_generate("<<Copy>>"))
        if str(widget.cget('state')) in (ACTIVE,NORMAL):
            self.add_command(label="Paste",
                             image=ICONS['paste'],
                             compound=LEFT,
                             accelerator='Ctrl+V',
                             command=lambda: self.widget.event_generate("<<Paste>>"))
        self.add_separator()
        self.add_command(label="Select all",
                         image=ICONS['select_all'],
                         compound=LEFT,
                         accelerator='Ctrl+A',
                         command=lambda: self.widget.event_generate("<<SelectAll>>"))
        self.tk_popup(x,y) # self.post does not destroy the menu when clicking out of it

class EntryContext(Entry):
    def __init__(self,parent,**kwargs):
        """An enhanced Entry widget that has a right-click menu
Use like any other Entry widget"""
        if sys.version_info.major>=3:
            super().__init__(parent,**kwargs)
        else:
            Entry.__init__(self,parent,**kwargs)
        # on Mac the right button fires a Button-2 event, or so I'm told
        # some mice don't even have two buttons, so the user is forced
        # to use Control + the only button
        # bear in mind that I don't have a Mac, so this point may be bugged
        if sys.platform=="darwin":
            self.bind("<Button-2>",self.on_right_click)
            self.bind("<Control-Button-1>",self.on_right_click)
        else:
            self.bind("<Button-3>",self.on_right_click)
    def on_right_click(self,event):
        if str(self.cget('state')) != DISABLED:
            ContextMenu(event.x_root,event.y_root,event.widget)

class SpinboxContext(Spinbox):
    def __init__(self,parent,**kwargs):
        """An enhanced Spinbox widget that has a right-click menu
Use like any other Spinbox widget"""
        if sys.version_info.major>=3:
            super().__init__(parent,**kwargs)
        else:
            Spinbox.__init__(self,parent,**kwargs)
        # on Mac the right button fires a Button-2 event, or so I'm told
        # some mice don't even have two buttons, so the user is forced
        # to use Control + the only button
        # bear in mind that I don't have a Mac, so this point may be bugged
        if sys.platform=="darwin":
            self.bind("<Button-2>",self.on_right_click)
            self.bind("<Control-Button-1>",self.on_right_click)
        else:
            self.bind("<Button-3>",self.on_right_click)
    def on_right_click(self,event):
        if str(self.cget('state')) != DISABLED:
            ContextMenu(event.x_root,event.y_root,event.widget)

class SelectDirectory(LabelFrame):
    def __init__(self,parent,textvariable=None,**kwargs):
        """A subclass of LabelFrame sporting a readonly Entry and a Button with a folder icon.
It comes complete with a context menu and a directory selection screen"""
        if sys.version_info.major>=3:
            super().__init__(parent,text="Select a directory",**kwargs)
        else:
            LabelFrame.__init__(self,parent,text="Select a directory",**kwargs)
        self.textvariable=textvariable
        self.dir_entry=EntryContext(self,
                                    width=40,
                                    textvariable=self.textvariable,
                                    state="readonly")
        self.dir_entry.pack(side=LEFT,
                            fill=X,
                            expand=YES)
        self.dir_button=Button(self,
                               image=ICONS['browse'],
                               compound=LEFT,
                               text="Browse...",
                               command=self.on_browse)
        self.dir_button.pack(side=LEFT)
    def on_browse(self):
        # os.path.expanduser gets the current user's home directory on every platform
        if sys.platform=="win32":
            # get userdata directory on Windows
            # it assumes that you choose to store userdata in the My Games directory
            # while installing Wesnoth
            userdata=os.path.join(os.path.expanduser("~"),
                                  "Documents",
                                  "My Games",
                                  "Wesnoth"+WESNOTH_SERIES,
                                  "data",
                                  "add-ons")
        elif sys.platform.startswith("linux"): # we're on Linux; usually this string is 'linux2'
            userdata=os.path.join(os.path.expanduser("~"),
                                  ".local",
                                  "share",
                                  "wesnoth",
                                  WESNOTH_SERIES,
                                  "data",
                                  "add-ons")
        elif sys.platform=="darwin": # we're on MacOS
            # bear in mind that I don't have a Mac, so this point may be bugged
            userdata=os.path.join(os.path.expanduser("~"),
                                  "Library",
                                  "Application Support",
                                  "Wesnoth_"+WESNOTH_SERIES,
                                  "data",
                                  "add-ons")
        else: # unknown system; if someone else wants to add other rules, be my guest
            userdata="."
        
        if os.path.exists(userdata): # we may have gotten it wrong
            directory=askdirectory(initialdir=userdata)
        else:
            directory=askdirectory(initialdir=".")
        
        if directory:
            self.textvariable.set(directory)

class WmllintTab(Frame):
    def __init__(self,parent):
        # it means super(WmllintTab,self), that in turn means
        # Frame.__init__(self,parent)
        if sys.version_info.major>=3:
            super().__init__(parent)
        else:
            Frame.__init__(self,parent)
        self.mode_variable=IntVar()
        self.mode_frame=LabelFrame(self,
                                   text="Select wmllint mode")
        self.mode_frame.grid(row=0,
                             column=0,
                             sticky=N+E+S+W)
        self.radio_normal=Radiobutton(self.mode_frame,
                                      text="Normal",
                                      variable=self.mode_variable,
                                      value=0)
        self.radio_normal.grid(row=0,
                               column=0,
                               sticky=W,
                               padx=10)
        self.radio_dryrun=Radiobutton(self.mode_frame,
                                      text="Dry run\nDo not perform changes",
                                      variable=self.mode_variable,
                                      value=1)
        self.radio_dryrun.grid(row=1,
                               column=0,
                               sticky=W,
                               padx=10)
        self.radio_clean=Radiobutton(self.mode_frame,
                                     text="Clean\nDelete *.bak files",
                                     variable=self.mode_variable,
                                     value=2)
        self.radio_clean.grid(row=2,
                              column=0,
                              sticky=W,
                              padx=10)
        self.radio_diff=Radiobutton(self.mode_frame,
                                    text="Diff\nShow differences in converted files",
                                    variable=self.mode_variable,
                                    value=3)
        self.radio_diff.grid(row=3,
                             column=0,
                             sticky=W,
                             padx=10)
        self.radio_revert=Radiobutton(self.mode_frame,
                                      text="Revert\nRevert conversions using *.bak files",
                                      variable=self.mode_variable,
                                      value=4)
        self.radio_revert.grid(row=4,
                               column=0,
                               sticky=W,
                               padx=10)
        self.verbosity_frame=LabelFrame(self,
                                        text="Verbosity level")
        self.verbosity_frame.grid(row=0,
                                  column=1,
                                  sticky=N+E+S+W)
        self.verbosity_variable=IntVar(0)
        self.radio_v0=Radiobutton(self.verbosity_frame,
                                  text="Normal",
                                  variable=self.verbosity_variable,
                                  value=0)
        self.radio_v0.grid(row=0,
                           column=0,
                           sticky=W,
                           padx=10)
        self.radio_v1=Radiobutton(self.verbosity_frame,
                                  text="List changes",
                                  variable=self.verbosity_variable,
                                  value=1)
        self.radio_v1.grid(row=1,
                           column=0,
                           sticky=W,
                           padx=10)
        self.radio_v2=Radiobutton(self.verbosity_frame,
                                  text="Name files\nbefore processing",
                                  variable=self.verbosity_variable,
                                  value=2)
        self.radio_v2.grid(row=2,
                           column=0,
                           sticky=W,
                           padx=10)
        self.radio_v3=Radiobutton(self.verbosity_frame,
                                  text="Show parse details",
                                  variable=self.verbosity_variable,
                                  value=3)
        self.radio_v3.grid(row=3,
                           column=0,
                           sticky=W,
                           padx=10)
        self.options_frame=LabelFrame(self,
                                      text="Set wmllint options")
        self.options_frame.grid(row=0,
                                column=2,
                                sticky=N+E+S+W)
        self.stripcr_variable=IntVar(0)
        self.stripcr_check=Checkbutton(self.options_frame,
                                       text="Convert EOL characters to Unix style",
                                       variable=self.stripcr_variable)
        self.stripcr_check.grid(row=0,
                                column=0,
                                sticky=W,
                                padx=10)
        self.progress_variable=IntVar(0)
        self.progress_check=Checkbutton(self.options_frame,
                                        text="List files before processing",
                                        variable=self.progress_variable)
        self.progress_check.grid(row=1,
                                 column=0,
                                 sticky=W,
                                 padx=10)
        self.future_variable=IntVar(0)
        self.future_check=Checkbutton(self.options_frame,
                                      text="Enable experimental conversions",
                                      variable=self.future_variable)
        self.future_check.grid(row=2,
                               column=0,
                               sticky=W,
                               padx=10)
        self.known_variable=IntVar(0)
        self.known_check=Checkbutton(self.options_frame,
                                     text="Disable checks for unknown units",
                                     variable=self.known_variable)
        self.known_check.grid(row=3,
                              column=0,
                              sticky=W,
                              padx=10)
        self.spell_variable=IntVar(0)
        self.spell_check=Checkbutton(self.options_frame,
                                     text="Disable spellchecking",
                                     variable=self.spell_variable)
        self.spell_check.grid(row=4,
                              column=0,
                              sticky=W,
                              padx=10)
        self.freeze_variable=IntVar(0)
        self.freeze_check=Checkbutton(self.options_frame,
                                      text="Ignore newlines in messages",
                                      variable=self.freeze_variable)
        self.freeze_check.grid(row=5,
                               column=0,
                               sticky=W,
                               padx=10)
        self.columnconfigure(0,weight=1)
        self.columnconfigure(1,weight=1)
        self.columnconfigure(2,weight=1)

class WmlscopeTab(Frame):
    def __init__(self,parent):
        if sys.version_info.major>=3:
            super().__init__(parent)
        else:
            Frame.__init__(self,parent)
        self.options_frame=LabelFrame(self,
                                      text="Set wmlscope options")
        self.options_frame.grid(row=0,
                                column=0,
                                sticky=N+E+S+W)
        self.normal_options=Frame(self.options_frame)
        self.normal_options.grid(row=0,
                                 column=0,
                                 sticky=N+E+S+W)
        self.crossreference_variable=IntVar(0) # equivalent to warnlevel 1
        self.crossreference_check=Checkbutton(self.normal_options,
                                              text="Check for duplicate macro definitions",
                                              variable=self.crossreference_variable)
        self.crossreference_check.grid(row=0,
                                       column=0,
                                       sticky=W,
                                       padx=10)
        self.collisions_variable=IntVar(0)
        self.collisions_check=Checkbutton(self.normal_options,
                                          text="Check for duplicate resource files",
                                          variable=self.collisions_variable)
        self.collisions_check.grid(row=1,
                                   column=0,
                                   sticky=W,
                                   padx=10)
        self.definitions_variable=IntVar(0)
        self.definitions_check=Checkbutton(self.normal_options,
                                           text="Make definition list",
                                           variable=self.definitions_variable)
        self.definitions_check.grid(row=2,
                                    column=0,
                                    sticky=W,
                                    padx=10)
        self.listfiles_variable=IntVar(0)
        self.listfiles_check=Checkbutton(self.normal_options,
                                         text="List files that will be processed",
                                         variable=self.listfiles_variable)
        self.listfiles_check.grid(row=3,
                                  column=0,
                                  sticky=W,
                                  padx=10)
        self.unresolved_variable=IntVar(0)
        self.unresolved_check=Checkbutton(self.normal_options,
                                          text="Report unresolved macro references",
                                          variable=self.unresolved_variable)
        self.unresolved_check.grid(row=4,
                                   column=0,
                                   sticky=W,
                                   padx=10)
        self.extracthelp_variable=IntVar(0)
        self.extracthelp_check=Checkbutton(self.normal_options,
                                           text="Extract help from macro definition comments",
                                           variable=self.extracthelp_variable)
        self.extracthelp_check.grid(row=5,
                                    column=0,
                                    sticky=W,
                                    padx=10)
        self.unchecked_variable=IntVar(0)
        self.unchecked_check=Checkbutton(self.normal_options,
                                         text="Report all macros with untyped formals",
                                         variable=self.unchecked_variable)
        self.unchecked_check.grid(row=6,
                                  column=0,
                                  sticky=W,
                                  padx=10)
        self.progress_variable=IntVar(0)
        self.progress_check=Checkbutton(self.normal_options,
                                        text="Show progress",
                                        variable=self.progress_variable)
        self.progress_check.grid(row=7,
                                 column=0,
                                 sticky=W,
                                 padx=10)
        self.separator=Separator(self.options_frame,
                                 orient=VERTICAL)
        self.separator.grid(row=0,
                            column=1,
                            sticky=N+S)
        self.options_with_regexp=Frame(self.options_frame)
        self.options_with_regexp.grid(row=0,
                                      column=2,
                                      sticky=N+E+S+W)
        self.exclude_variable=IntVar(0)
        self.exclude_check=Checkbutton(self.options_with_regexp,
                                       text="Exclude files matching regexp:",
                                       variable=self.exclude_variable,
                                       command=self.exclude_callback)
        self.exclude_check.grid(row=0,
                                column=0,
                                sticky=W,
                                padx=10)
        self.exclude_regexp=StringVar()
        self.exclude_entry=EntryContext(self.options_with_regexp,
                                        textvariable=self.exclude_regexp,
                                        state=DISABLED)
        self.exclude_entry.grid(row=0,
                                column=1,
                                sticky=E+W,
                                padx=10)
        self.from_variable=IntVar(0)
        self.from_check=Checkbutton(self.options_with_regexp,
                                    text="Exclude files not matching regexp:",
                                    variable=self.from_variable,
                                    command=self.from_callback)
        self.from_check.grid(row=1,
                             column=0,
                             sticky=W,
                             padx=10)
        self.from_regexp=StringVar()
        self.from_entry=EntryContext(self.options_with_regexp,
                                     textvariable=self.from_regexp,
                                     state=DISABLED)
        self.from_entry.grid(row=1,
                             column=1,
                             sticky=E+W,
                             padx=10)
        self.refcount_variable=IntVar(0)
        self.refcount_check=Checkbutton(self.options_with_regexp,
                                        text="Report only on macros referenced\nin at least n files:",
                                        variable=self.refcount_variable,
                                        command=self.refcount_callback)
        self.refcount_check.grid(row=2,
                                 column=0,
                                 sticky=W,
                                 padx=10)
        self.refcount_number=IntVar(0)
        self.refcount_spin=SpinboxContext(self.options_with_regexp,
                                          from_=0,to=999,
                                          textvariable=self.refcount_number,
                                          width=3,
                                          state=DISABLED)
        self.refcount_spin.grid(row=2,
                                column=1,
                                sticky=E+W,
                                padx=10)
        self.typelist_variable=IntVar(0)
        self.typelist_check=Checkbutton(self.options_with_regexp,
                                        text="List actual & formal argtypes\nfor calls in fname",
                                        variable=self.typelist_variable,
                                        command=self.typelist_callback)
        self.typelist_check.grid(row=3,
                                 column=0,
                                 sticky=W,
                                 padx=10)
        self.typelist_string=StringVar()
        self.typelist_entry=EntryContext(self.options_with_regexp,
                                         textvariable=self.typelist_string,
                                         state=DISABLED)
        self.typelist_entry.grid(row=3,
                                 column=1,
                                 sticky=E+W,
                                 padx=10)
        self.force_variable=IntVar(0)
        self.force_check=Checkbutton(self.options_with_regexp,
                                     text="Ignore refcount 0 on names\nmatching regexp:",
                                     variable=self.force_variable,
                                     command=self.force_callback)
        self.force_check.grid(row=4,
                              column=0,
                              sticky=W,
                              padx=10)
        self.force_regexp=StringVar()
        self.force_entry=EntryContext(self.options_with_regexp,
                                      textvariable=self.force_regexp,
                                      state=DISABLED)
        self.force_entry.grid(row=4,
                              column=1,
                              sticky=E+W,
                              padx=10)
        self.columnconfigure(0,weight=1)
        self.options_frame.columnconfigure(0,weight=1)
        self.options_frame.columnconfigure(2,weight=1)
        self.options_with_regexp.columnconfigure(1,weight=1)
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
    def __init__(self,parent):
        if sys.version_info.major>=3:
            super().__init__(parent)
        else:
            Frame.__init__(self,parent)
        self.mode_variable=IntVar()
        self.mode_frame=LabelFrame(self,
                                   text="Select wmlindent mode")
        self.mode_frame.grid(row=0,
                             column=0,
                             sticky=N+E+S+W)
        self.radio_normal=Radiobutton(self.mode_frame,
                                      text="Normal",
                                      variable=self.mode_variable,
                                      value=0)
        self.radio_normal.grid(row=0,
                               column=0,
                               sticky=W,
                               padx=10)
        self.radio_dryrun=Radiobutton(self.mode_frame,
                                      text="Dry run\nDo not perform changes",
                                      variable=self.mode_variable,
                                      value=1)
        self.radio_dryrun.grid(row=1,
                               column=0,
                               sticky=W,
                               padx=10)
        self.options_frame=LabelFrame(self,
                                      text="Set wmlindent options")
        self.options_frame.grid(row=0,
                                column=1,
                                sticky=N+E+S+W)
        self.verbose_variable=IntVar(0)
        self.verbose_check=Checkbutton(self.options_frame,
                                       text="Report also unchanged files",
                                       variable=self.verbose_variable)
        self.verbose_check.grid(row=0,
                                column=0,
                                sticky=W,
                                padx=10)
        self.exclude_variable=IntVar(0)
        self.exclude_check=Checkbutton(self.options_frame,
                                       text="Exclude files matching regexp:",
                                       variable=self.exclude_variable,
                                       command=self.exclude_callback)
        self.exclude_check.grid(row=1,
                                column=0,
                                columnspan=2,
                                sticky=W,
                                padx=10)
        self.regexp_variable=StringVar()
        self.regexp_entry=EntryContext(self.options_frame,
                                       textvariable=self.regexp_variable,
                                       state=DISABLED)
        self.regexp_entry.grid(row=1,
                               column=1,
                               sticky=E+W,
                               padx=10)
        self.quiet_variable=IntVar(0)
        self.quiet_check=Checkbutton(self.options_frame,
                                     text="Do not generate output",
                                     variable=self.quiet_variable)
        self.quiet_check.grid(row=2,
                              column=0,
                              sticky=W,
                              padx=10)
        self.columnconfigure(0,weight=1)
        self.columnconfigure(1,weight=1)
        self.options_frame.columnconfigure(1,weight=1)
    def exclude_callback(self):
        if self.exclude_variable.get():
            self.regexp_entry.configure(state=NORMAL)
        else:
            self.regexp_entry.configure(state=DISABLED)

class MainFrame(Frame):
    def __init__(self,parent):
        self.parent=parent
        if sys.version_info.major>=3:
            self.queue=queue.Queue()
            super().__init__(parent)
        else:
            self.queue=Queue.Queue()
            Frame.__init__(self,parent)
        self.grid(sticky=N+E+S+W)
        self.buttonbox=Frame(self)
        self.buttonbox.grid(row=0,
                            column=0,
                            sticky=E+W)
        self.run_button=Button(self.buttonbox,
                               text="Run wmllint",
                               image=ICONS['run'],
                               compound=LEFT,
                               width=15, # to avoid changing size when callback is called
                               command=self.on_run_wmllint)
        self.run_button.pack(side=LEFT,
                             padx=5,
                             pady=5)
        self.save_button=Button(self.buttonbox,
                                text="Save as text...",
                                image=ICONS['save'],
                                compound=LEFT,
                                command=self.on_save)
        self.save_button.pack(side=LEFT,
                              padx=5,
                              pady=5)
        self.clear_button=Button(self.buttonbox,
                                 text="Clear output",
                                 image=ICONS['clear'],
                                 compound=LEFT,
                                 command=self.on_clear)
        self.clear_button.pack(side=LEFT,
                               padx=5,
                               pady=5)
        self.about_button=Button(self.buttonbox,
                                 text="About...",
                                 image=ICONS['about'],
                                 compound=LEFT,
                                 command=self.on_about)
        self.about_button.pack(side=LEFT,
                               padx=5,
                               pady=5)
        self.exit_button=Button(self.buttonbox,
                                text="Exit",
                                image=ICONS['exit'],
                                compound=LEFT,
                                command=parent.destroy)
        self.exit_button.pack(side=LEFT,
                              padx=5,
                              pady=5)
        self.dir_variable=StringVar()
        self.dir_frame=SelectDirectory(self,
                                       textvariable=self.dir_variable)
        self.dir_frame.grid(row=1,
                            column=0,
                            sticky=E+W)
        # Notebook is one of the new widgets introduced by ttk
        # it isn't available on Python 2.6 and lower, like the rest of ttk widgets
        # please note that the Frames that become tabs don't need to be packed or gridded
        self.notebook=Notebook(self)
        self.notebook.grid(row=2,
                           column=0,
                           sticky=E+W)
        self.wmllint_tab=WmllintTab(None)
        self.notebook.add(self.wmllint_tab,
                          text="wmllint",
                          sticky=N+E+S+W)
        self.wmlscope_tab=WmlscopeTab(None)
        self.notebook.add(self.wmlscope_tab,
                          text="wmlscope",
                          sticky=N+E+S+W)
        self.wmlindent_tab=WmlindentTab(None)
        self.notebook.add(self.wmlindent_tab,
                          text="wmlindent",
                          sticky=N+E+S+W)
        self.output_frame=LabelFrame(self,
                                     text="Maintenance tools output")
        self.output_frame.grid(row=3,
                               column=0,
                               sticky=N+E+S+W)
        self.text=Text(self.output_frame,
                       wrap=WORD,
                       state=DISABLED)
        self.text.grid(row=0,
                       column=0,
                       sticky=N+E+S+W)
        self.update_text()
        self.yscrollbar=Scrollbar(self.output_frame,
                                  command=self.text.yview)
        self.yscrollbar.grid(row=0,
                             column=1,
                             sticky=N+S)
        self.text["yscrollcommand"]=self.yscrollbar.set
        self.xscrollbar=Scrollbar(self.output_frame,
                                  orient=HORIZONTAL,
                                  command=self.text.xview)
        self.xscrollbar.grid(row=1,
                             column=0,
                             sticky=E+W)
        self.text["xscrollcommand"]=self.xscrollbar.set
        self.grip=Sizegrip(self.output_frame)
        self.grip.grid(row=1,column=1)
        self.output_frame.rowconfigure(0,weight=1)
        self.output_frame.columnconfigure(0,weight=1)
        self.columnconfigure(0,weight=1)
        self.rowconfigure(3,weight=1)
        self.notebook.bind("<<NotebookTabChanged>>",self.tab_callback)

    def tab_callback(self,event):
        # we check the ID of the active tab and ask its position
        # the order of the tabs is pretty obvious
        active_tab=self.notebook.index(self.notebook.select())
        if active_tab==0:
            self.run_button.configure(text="Run wmllint",command=self.on_run_wmllint)
        elif active_tab==1:
            self.run_button.configure(text="Run wmlscope",command=self.on_run_wmlscope)
        elif active_tab==2:
            self.run_button.configure(text="Run wmlindent",command=self.on_run_wmlindent)

    def on_run_wmllint(self):
        # build the command line
        wmllint_command_string=[]
        # get the path of the Python interpreter
        wmllint_command_string.append(sys.executable)
        # get the path of the desired tool (wmllint, in this case)
        wmllint_command_string.append(os.path.join(APP_DIR,"wmllint"))
        mode=self.wmllint_tab.mode_variable.get()
        if mode==0:
            pass
        elif mode==1:
            wmllint_command_string.append("--dryrun")
        elif mode==2:
            wmllint_command_string.append("--clean")
        elif mode==3:
            wmllint_command_string.append("--diff")
        elif mode==4:
            wmllint_command_string.append("--revert")
        verbosity=self.wmllint_tab.verbosity_variable.get()
        for n in range(verbosity):
            wmllint_command_string.append("-v")
        if self.wmllint_tab.stripcr_variable.get():
            wmllint_command_string.append("--stripcr")
        if self.wmllint_tab.progress_variable.get():
            wmllint_command_string.append("--progress")
        if self.wmllint_tab.future_variable.get():
            wmllint_command_string.append("--future")
        if self.wmllint_tab.known_variable.get():
            wmllint_command_string.append("--known")
        if self.wmllint_tab.spell_variable.get():
            wmllint_command_string.append("--nospellcheck")
        if self.wmllint_tab.freeze_variable.get():
            wmllint_command_string.append("--stringfreeze")
        wmllint_command_string.append(WESNOTH_CORE_DIR)
        umc_dir=self.dir_variable.get()
        if os.path.exists(umc_dir): # add-on exists
            wmllint_command_string.append(umc_dir)
        elif not umc_dir: # path does not exists because the box was left empty
            showwarning("Warning","""You didn't select a directory.

wmllint will be run only on the Wesnoth core directory""")
        else: # path doesn't exist and isn't empty
            showerror("Error","""The selected directory does not exists""")
            return # stop here
        # start thread and wmllint subprocess
        wmllint_thread=threading.Thread(target=run_tool,args=("wmllint",self.queue,wmllint_command_string))
        wmllint_thread.start()
        # build popup
        dialog=Popup(self.parent,"wmllint",wmllint_thread)

    def on_run_wmlscope(self):
        # build the command line
        wmlscope_command_string=[]
        wmlscope_command_string.append(sys.executable)
        wmlscope_command_string.append(os.path.join(APP_DIR,"wmlscope"))
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
            wmlscope_command_string.append('--exclude=r"{0}"'.format(self.wmlscope_tab.exclude_regexp.get()))
        if self.wmlscope_tab.from_variable.get():
            wmlscope_command_string.append('--from=r"{0}"'.format(self.wmlscope_tab.from_regexp.get()))
        if self.wmlscope_tab.refcount_variable.get():
            try:
                wmlscope_command_string.append("--refcount=" + str(self.wmlscope_tab.refcount_number.get()))
            except ValueError as error:
                # normally it should be impossible to raise this exception
                # due to the fact that the Spinbox is read-only
                showerror("Error","""You typed an invalid value. Value must be an integer in the range 0-999""")
                return
        if self.wmlscope_tab.typelist_variable.get():
            wmlscope_command_string.append('--typelist=r"{0}"'.format(self.wmlscope_tab.typelist_string.get()))
        if self.wmlscope_tab.force_variable.get():
            wmlscope_command_string.append('--force-used=r"{0}"'.format(self.wmlscope_tab.force_regexp.get()))
        wmlscope_command_string.append(WESNOTH_CORE_DIR)
        umc_dir=self.dir_variable.get()
        if os.path.exists(umc_dir): # add-on exists
            wmlscope_command_string.append(umc_dir)
        elif not umc_dir: # path does not exists because the box was left empty
            showwarning("Warning","""You didn't select a directory.

wmlscope will be run only on the Wesnoth core directory""")
        else: # path doesn't exist and isn't empty
            showerror("Error","""The selected directory does not exists""")
            return # stop here
        # start thread and wmlscope subprocess
        wmlscope_thread=threading.Thread(target=run_tool,args=("wmlscope",self.queue,wmlscope_command_string))
        wmlscope_thread.start()
        # build popup
        dialog=Popup(self.parent,"wmlscope",wmlscope_thread)

    def on_run_wmlindent(self):
        # build the command line
        wmlindent_command_string=[]
        wmlindent_command_string.append(sys.executable)
        wmlindent_command_string.append(os.path.join(APP_DIR,"wmlindent"))
        mode=self.wmlindent_tab.mode_variable.get()
        if mode==0:
            pass
        elif mode==1:
            wmlindent_command_string.append("--dryrun")
        if self.wmlindent_tab.verbose_variable.get():
            wmlindent_command_string.append("-v -v")
        if self.wmlindent_tab.exclude_variable.get():
            # the r before double quotes marks the regexp as a raw string
            wmlindent_command_string.append('exclude=r"{0}"'.format(self.wmlindent_tab.regexp_variable.get()))
        if self.wmlindent_tab.quiet_variable.get():
            wmlindent_command_string.append("--quiet")
        umc_dir=self.dir_variable.get()
        if os.path.exists(umc_dir): # add-on exists
            wmlindent_command_string.append(umc_dir)
        elif not umc_dir: # path does not exists because the box was left empty
            showwarning("Warning","""You didn't select a directory.

wmlindent will be run on the Wesnoth core directory""")
            wmlindent_command_string.append(WESNOTH_CORE_DIR)
        else: # path doesn't exist and isn't empty
            showerror("Error","""The selected directory does not exists""")
            return # stop here
        # start thread and wmllint subprocess
        wmlindent_thread=threading.Thread(target=run_tool,args=("wmlindent",self.queue,wmlindent_command_string))
        wmlindent_thread.start()
        # build popup
        dialog=Popup(self.parent,"wmlindent",wmlindent_thread)

    def update_text(self):
        """Checks periodically if the queue is empty, and pushes its content in the Text widget if it isn't"""
        if not self.queue.empty():
            text_to_push=self.queue.get_nowait()
            self.text.configure(state=NORMAL)
            self.text.insert(END,text_to_push)
            self.text.configure(state=DISABLED)
        self.text.after(100,self.update_text)
        
    def on_save(self):
        fn=asksaveasfilename(defaultextension=".txt",filetypes=[("Text file","*.txt")],initialdir=".")
        if fn:
            try:
                out=codecs.open(fn,"w","utf-8")
                out.write(self.text.get(1.0,END))
                out.close()
            except IOError as error: # in case that we attempt to write without permissions
                showerror("Error","""Error while writing to:
{0}

Error code: {1}

{2}""".format(fn,error.errno,error.strerror))

    def on_clear(self):
        self.text.configure(state=NORMAL)
        self.text.delete(1.0,END)
        self.text.configure(state=DISABLED)

    def on_about(self):
        showinfo("About Maintenance tools GUI","""(C) Elvish_Hunter, 2014

Part of The Battle for Wesnoth project and released under the GNU GPL v2 license

Icons are taken from the Tango project, and are released in the Public Domain""")

if is_wesnoth_tools_path(APP_DIR):
    root=Tk()
    # a dictionary with all the icons
    # they're saved in GIF format (the only one supported by Tkinter)
    # and then encoded in base64
    # this is done to avoid having small files floating around
    ICONS={
           "about":PhotoImage(data=b'''
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
           "run":PhotoImage(data=b'''
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
           "save":PhotoImage(data=b'''
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
           "clear":PhotoImage(data=b'''
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
           "exit":PhotoImage(data=b'''
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
           "browse":PhotoImage(data=b'''
R0lGODlhIAAgAOfhADVhmjNlojRlozVlpDhmojhopjlopTlppDtppDxqpT1qo2VlZTxsp2dnZ2ho
aENuo2lpaUBvqEBvqUFvp2pqakRwq1puiUhxpGxsbG1tbUF1rEd0rXBwcHFxcVd1mnJyck14rUx5
sHR0dEp7s3V1dU57s3d3d3h4eHp6emB+o1KBtHt7e1iAsliAs1OCtHx8fFSDtWeAoX5+fmuBnX9/
f3WBkF6FtlyGuXKEnFWJwVaKwYODg32Fj12LvoWFhVqMw12MwVuNw22KsISHioeHhoeHh1+Nw12P
xGGPwImJiWePvWOQwIqKil6RyV+RyWSRwouLi22PvGKSxoqMj2mRwYyMjIyMjWGTyo2NjWOUy4WP
nI6Ojo+Pj2WWy2aWzGiWyJCQkGeXzJGRkWmYzJKSkmqZzZOTk2yazpSUlHiYv2ubzmybzXKayJWV
lZCWnpaWlpeXl3Cdz5iYmHCf0HKfz3Kf0JmZmZqamnSh0Zubm3Wh0H2gzJycnHai0oSgxZ2dnXuj
znik0p6ennmk0nqk0p+fn6CgoHym032m1KGhoX6n1KKioqOjo4Kp1qSkpKWlpKWlpYOr1oSr1ISr
1qampoWs1qenp6ioqKmpqYmv2Kqqqoqv2Kurq4uw2ZevzKysrK2trK2trZKx1Y6y2a6uro+z2pCz
2pC02pyyzrCwr7CwsJO127GxsZO227Ozs5e43Zm63bS2uLW2t7a2tba2tpq73be3t5y83ri6vLu7
vqW/3r29vb6+wL+/v7HC18DAwL/BwqvE4MHBwarF48LCwcPDw8TExMXFxMjIyMnJybjP58zMzM3N
zczOzsjP18XQ39DQ0L/U6cDU6cHV697e3t/f3+Dg4OPj4+Tk5OPl6+bm5ufn5+jo6Ozs7O3t7e7u
7u/v7///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////yH5BAEKAP8ALAAAAAAgACAA
AAj+AP8JHEiwoMGDCBMqXMiw4cE3JyJKlLhil8OC1HYwM1asmDBhv0L2enGRoDUfrgoJ+sMnzx07
du6goVVSIDYZx1SRCvWJEyZMlzAxOkGChIijSEkkQ7iNBCs+L+G8edMGjVUzhT6FIkVKFStWjIok
5EDskyZMlihBcsRokdu3jBxBomTJDo2EEFLJmYrGjBgwXLhsGbwlMBgxZNAwOZGwwa5LlNgmMqTy
D0s+lgUVMpRokZ0OeDW18QtmcBUoUJioVg2lCpbCSSjgnQWJUaJCf1zakQOnNxw5cmJihrMgoQNI
ZMBMeZAgAYLn0KMjaJ6AgITrEtIQxABqUSE3E6T+iR9Pvjz5aMEqbC8kmAeIaa3iy59PX34tXS0I
ZnhUiI8WFsqcIuCABBYoYCmwiBIFQRzIsQUUNdgwTCcUVmjhhZ1s0sgre/hB0AeY3YGDErdUYuKJ
KKY4ySGDrEIFKgSZAEYVSXjAhimK5KjjjjsigkcddYxygy8EoSDGH3ZcAEgkgTTp5JNN9jGHGlSq
kUkIzxBEAxFVDGGAJHrQIeaYY8axxhhdpJnmGIQUkA1BOxTxxwwlEHLGnWeUEYYXWVzhRBOABgpo
F0bkR1ASMlihABBfSHHEDzrkIOmklFIaxAieFFSFDDFEsMQTSIQq6qikitoDA84UBIUFB2igAgw5
sMYq66yxurCBENAUZAsAAwjg66/ABhtsACnEsoxBzeSCCy/MNuvss80CI8suyNRk7bXYZqvtRQEB
ADs='''),
           "cut":PhotoImage(data=b'''
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
           "copy":PhotoImage(data=b'''
R0lGODlhEAAQAKUeAIiKhYmLhoyOiZialZialpyemqGjn6mqp62uq6+wrbu8ury9ur2+u8PEw8fH
xs/QzdDRz9TU1NnZ2dra2tvb2+Pj4uPk4uzs7O3t7e7u7e7u7u/v7u/v7/Dw7/Dw8PHx8PHx8fPz
8/T09Pb29fb29vf39vf39/j49/r6+fr6+vv7+/z8+/7+/f////Dw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8CH5BAEKAD8ALAAAAAAQABAAAAaKQIhi
SCw+fsgfw8RsOhVJpYmzqVY5HBM0ueRQDoCweIzocgKttHoN6FIArY58Lm+1pxt0x8Hvd+xLGxNw
dHSAJhoZen18dW0kGBKEhR0pIW0iGBeLjBolDW0fIB56dAKnAgAQC6wALHQpAyknFgBRAShzKbEp
FQAFtyIlIysqEQS1W0kJAWNhBltBADs='''),
           "paste":PhotoImage(data=b'''
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
           "select_all":PhotoImage(data=b'''
R0lGODlhEAAQAKUZAAAAAIeJhIiKhYqMh4uNiIGXr4KYsLS1s7W2s7W2tKi+1qm/16rA2KvB2azC
2q3D267E3K/F3bDG3rHH37LI4LPJ4evr6+zs7O7u7vDw8PLy8vT09PX19fb29vf39/j4+Pn5+fr6
+vv7+/z8/P39/f7+/vDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw
8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8PDw8CH5BAEKAD8ALAAAAAAQABAAAAaWwN9B
QCwaiYjfL0AqlUik0UgkCoVAA6WApFgsGIyGWPwhaEuLgrrgWDvKWhJjXXjY7fDfdr5+QP4WeQIj
DXQQahEXgiMODn4RERKSGIuOEJCSExMZgiJtdGoUGh5meiKPkgAUABUbpFohqBOrHB0dr3ogDwa8
BqseHx64ArqXErMAHiDBpQEfz9AAH9LPWT8IR9kCCT9BADs=''')
           }
    ROOT_W,ROOT_H=800,480
    # the following string may be confusing, so here there's an explanation
    # Python supports two ways to perform string interpolation
    # the first one is the C-like style
    # the second one is the following, where each number enclosed in brackets points to an argument of the format method
    root.geometry("{0}x{1}+{2}+{3}".format(ROOT_W,
                                           ROOT_H,
                                           int((root.winfo_screenwidth()-ROOT_W)/2),
                                           int((root.winfo_screenheight()-ROOT_H)/2)))
    root.title("Maintenance tools GUI")
    root.rowconfigure(0,weight=1)
    root.columnconfigure(0,weight=1)
    # use a better style on Linux instead of the Motif-like one
    style=Style()
    if sys.platform.startswith("linux") and "clam" in style.theme_names():
        style.theme_use("clam")
    app=MainFrame(root)
    root.mainloop()
    sys.exit(0)
else:
    showerror("Error","This application must be placed into the wesnoth/data/tools directory")
    sys.exit(1)
