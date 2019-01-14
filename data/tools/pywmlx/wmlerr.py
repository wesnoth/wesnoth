import os
import sys
import warnings
import ctypes
import struct

enabled_text_col = True
is_utest = False
_warnall = False


# these constants are used by the Win32 API
FG_RED = 4
FG_GREEN = 2
FG_BLUE = 1
FG_INTENSITY = 8
BG_RED = 64
BG_GREEN = 32
BG_BLUE = 16
BG_INTENSITY = 128
STD_INPUT_HANDLE = -10
STD_OUTPUT_HANDLE = -11
STD_ERROR_HANDLE = -12

# there are three ways to store the console's default status
# the first requires to redefine the C structures used by the Win32 API
# by using the ctypes.Structure class, and pass them by reference
# by using ctypes.byref
# the second requires to use ctypes.create_string_buffer
# the third one just requires to create an empty bytes object
# both of them must be able to contain exactly 22 bytes/characters
# and you need to use the struct module to decode the values
console_defaults = bytes(22)


def wmlerr_debug():
    global is_utest
    is_utest = True



def ansi_setEnabled(value):
    global enabled_text_col
    enabled_text_col = value



def wincol_setEnabled(value):
    global enabled_text_col
    global handle
    global default_console_status
    global default_color
    enabled_text_col = value
    if sys.platform.startswith('win') and enabled_text_col:
        # and now, let's start playing with the Win32 API
        # first of all, we need to get a handle to the console output
        handle = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
        # and then we store the current console status
        # to be able to reset the original color
        ctypes.windll.kernel32.GetConsoleScreenBufferInfo(handle,
                                                          console_defaults)
        # by using struct.unpack_from, platform endianness is automatically
        # handled
        # this is why I'm using it, instead of a bitwise operation
        default_color = struct.unpack_from("H", console_defaults, 8)[0]



def warnall():
    return _warnall



def set_warnall(value):
    _warnall = value



class WmlError(ValueError):
    pass

class WmlWarning(UserWarning):
    pass



def print_wmlerr(finfo, message, iserr):
    # red if error, blue if warning
    ansi_color = '\033[91;1m' if iserr else '\033[94m'
    errtype = "error:" if iserr else "warning:"
    # now we have ascii_color and errtype values
    # here we print the error/warning.
    # 1) On Windows, write "error" in red and "warning" in blue
    #    by using the Win32 API (except if --no-text-colors is used)
    if sys.platform.startswith('win') and enabled_text_col:
        # a syntactic sugar to make lines shorter
        kernel32 = ctypes.windll.kernel32
        # first flush the stderr, otherwise colors might be changed in
        # unpredictable ways
        sys.stderr.flush()
        if iserr:
            kernel32.SetConsoleTextAttribute(handle, FG_RED | FG_INTENSITY)
        else:
            kernel32.SetConsoleTextAttribute(handle, FG_BLUE | FG_INTENSITY)
        # then write the error type and continue on the same line
        print(errtype + " ", end="", file=sys.stderr)
        # flush again and write file name in yellow on the same line
        sys.stderr.flush()
        kernel32.SetConsoleTextAttribute(handle,
                                         FG_RED | FG_GREEN | FG_INTENSITY)
        print(finfo + ": ", end="", file=sys.stderr)
        # then flush again, reset the color and finish writing
        sys.stderr.flush()
        ctypes.windll.kernel32.SetConsoleTextAttribute(handle, default_color)
        print(message, file=sys.stderr)
        # finally flush again for good measure
        sys.stderr.flush()
    # 2) On posix we write "error" in red and "warning" in blue
    #    by using ansi escape codes (except if --no-text-colors is used)
    elif os.name == "posix" and enabled_text_col:
        msg = (ansi_color + errtype + ' \033[0m\033[93m' + finfo +
               ':\033[0m ' + message)
        print(msg, file=sys.stderr)
    # 3) On non-posix and non-windows system we don't use colors
    #    (is it ever possible?).
    #    If --no-text-colors option is used, than we don't use colors
    #    regardless of OS.
    else:
        msg = errtype + ' ' + finfo + ': ' + message
        print(msg, file=sys.stderr)



def my_showwarning(message, category, filename, lineno, file=None, line=None):
    try:
        finfo, msg = message.args[0].split(": ", 1)
        print_wmlerr(finfo, msg, False)
    except OSError:
        pass # the file (probably stderr) is invalid - this warning gets lost.



warnings.showwarning = my_showwarning



def wmlerr(finfo, message, errtype=WmlError):
    if not is_utest:
        try:
            raise errtype(finfo + ": " + message)
        except errtype as err:
            print_wmlerr(finfo, message, True)
            sys.exit(1)
    else:
        raise errtype(finfo + ": " + message)



def wmlwarn(finfo, message):
    warnings.warn(finfo + ": " + message, WmlWarning)
