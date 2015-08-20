#!/usr/bin/env python
# encoding: utf8

"""
NOTE: You should use wmlparser2.py instead which uses the C++
preprocessor.

This module represents the internal appearance of WML.

Note: We cannot store textdomain information, due to how the parser work.
For example:
    name = _"x" + {foor} + {bar}
That string may actually be composed from three different textdomains. The
textdomain stuff in here is therefore only useful to CampGen, as that does
not allow composed strings like above.
"""

import re, sys
import codecs

class Data:
    """Common subclass."""
    def __init__(self, name):
        self.name = name
        self.file = "(None)"
        self.line = -1

    def __str__( self ):
        return self.debug(show_contents = True, write = False)

    def debug(self, show_contents = False, use_color = False, indent=0, write=True):
        if use_color:
            magenta = "\x1b[35;1m"
            off = "\x1b[0m"
        else:
            magenta = off = ""
        pos = indent * "  "
        result = pos + "\ " + magenta + self.name + off + " (" + self.__class__.__name__ + ")"
        if show_contents:
            if self.is_translatable() == True:
                result += " _"
            result += " '" + self.get_value() + "'"
        if write:
            # The input usually is utf8, but it also may be not - for example
            # if a .png image is transmitted over WML. As this is only for
            # display purposes, we ask Python to replace any garbage - and then
            # re-encode as utf8 for console output.
            text = result.decode("utf8", "replace")
            text = text.encode("utf8", "replace")
            sys.stdout.write(text)
            sys.stdout.write("\n")

        else: return result

    def copy(self):
        c = self.__class__(self.name, self.data) # this makes a new instance or so I was told
        return c

    def compare(self, other):
        return self.name == other.name and self.data == other.data

    def get_value(self):
        return ""

    def is_translatable(self):
        return False

    def get_type(self):
        return self.__class__.__name__

    def set_meta(self, file, line):
        self.file = file
        self.line = line

    def get_meta(self):
        return (self.file, self.line,)

class DataText(Data):
    """Represents any text strings."""
    def __init__(self, name, text, translatable = False):
        Data.__init__(self, name)
        self.data = text
        self.translatable = translatable

    def copy(self):
        return DataText(self.name, self.data, self.translatable)

    def get_value(self):
        return self.data

    def set_value(self, data):
        self.data = data

    def is_translatable(self):
        return self.translatable

class DataBinary(Data):
    """A binary chunk of WML."""
    def __init__(self, name, binary):
        Data.__init__(self, name)
        self.data = binary

    def get_value(self):
        return self.data

    def set_value(self, data):
        self.data = data

class DataMacro(Data):
    """A macro."""
    def __init__(self, name, macro):
        Data.__init__(self, name)
        self.data = macro

    def get_value(self):
        return self.data

    def set_value(self, data):
        self.data = data

class DataDefine(Data):
    """A definition."""
    def __init__(self, name, params, define):
        Data.__init__(self, name)
        self.params = params
        self.data = define

    def copy(self):
        return DataDefine(self.name, self.params, self.data)

    def get_value(self):
        return self.data

    def set_value(self, data):
        self.data = data

class DataComment(Data):
    """A comment (normally discarded)."""
    def __init__(self, name, comment):
        Data.__init__(self, name)
        self.data = comment

    def get_value(self):
        return self.data

    def set_value(self, data):
        self.data = data

class DataClosingTag(Data):
    """Yes, those are kept."""
    def __init__(self, name):
        Data.__init__(self, name)
        self.data = None

class WMLException(Exception):
    def __init__(self, text):
        super( WMLException, self ).__init__()
        self.text = text
        print(text)
    def __str__(self):
        return self.text

class DataSub(Data):
    def __init__(self, name, sub = [], textdomain=None):
        """The sub parameter is a list of sub-elements."""
        Data.__init__(self, name)
        self.data = []
        self.dict = {}
        self.textdomain = textdomain

        for element in sub:
            self.insert(element)

    def clean_empty_ifdefs(self):
        rem = []
        for item in self.data:
            if isinstance(item, DataIfDef):
                if item.data == []:
                    rem.append(item)
            if isinstance(item, DataSub):
                item.clean_empty_ifdefs()
        while rem:
            item = rem.pop()
            print("Removing empty #ifdef %s" % item.name)
            self.remove(item)

    def write_file(self, f, indent=0, textdomain=""):
        f.write(self.make_string( indent, textdomain))

    def make_string(self, indent = 0, textdomain = ""):
        """Write the data object to the given file object."""
        ifdef = 0
        result = []

        self.clean_empty_ifdefs()

        for item in self.data:
            if ifdef:
                if not isinstance(item, DataIfDef) or not item.type == "else":
                    result.append("#endif\n")
                ifdef = 0

            if isinstance(item, DataIfDef):
                if item.type == "else":
                    result.append("#else\n")
                else:
                    result.append("#ifdef %s\n" % item.name)
                result.append( item.make_string(indent + 4, textdomain) )
                ifdef = 1

            elif isinstance(item, DataSub):
                result.append(" " * indent)
                result.append("[%s]\n" % item.name)
                result.append(item.make_string(indent + 4, textdomain))
                result.append(" " * indent)
                close = item.name
                if close[0] == "+": close = close[1:]
                result.append("[/%s]\n" % close)

            elif isinstance(item, DataText):
                result.append(" " * indent)
                text = item.data.replace('"', r'""')
                # We always enclosed in quotes
                # In theory, the parser will just remove then on reading in, so
                # the actual value is 1:1 preserved
                # TODO: is there no catch?
                if textdomain and item.textdomain == "wesnoth":
                    result.append("#textdomain wesnoth\n")
                    result.append(" " * indent)

                # FIXME: don't compile regex's if they are one shots
                if item.translatable:
                    if "=" in text:
                        text = re.compile("=(.*?)(?=[=;]|$)"
                            ).sub("=\" + _\"\\1\" + \"", text)
                        text = '"' + text + '"'
                        text = re.compile(r'\+ _""').sub("", text)
                        result.append('%s=%s\n' % (item.name, text))
                    else:
                        result.append('%s=_"%s"\n' % (item.name, text))

                else:
                    result.append('%s="%s"\n' % (item.name, text))

                if textdomain and item.textdomain == "wesnoth":
                    result.append(" " * indent)
                    result.append("#textdomain %s\n" % textdomain)

            elif isinstance(item, DataMacro):
                result.append(" " * indent)
                result.append("%s\n" % item.data)

            elif isinstance(item, DataComment):
                result.append("%s\n" % item.data)

            elif isinstance(item, DataDefine):
                result.append("#define %s %s\n%s#enddef\n" % (item.name, item.params,
                    item.data))

            elif isinstance(item, DataClosingTag):
                result.append("[/%s]\n" % item.name)

            elif isinstance(item, DataBinary):
                data = item.data.replace('"', r'""')
                result.append("%s=\"%s\"\n" % (
                    item.name, data))

            else:
                raise WMLException("Unknown item: %s" % item.__class__.__name__)

        if ifdef:
            result.append("#endif\n")

        bytes = ""
        for r in result:
            if r != None:
                # For networking, we need actual bytestream here, not unicode.
                if type(r) is str: r = r.encode("utf8")
                bytes += str(r)

        return bytes

    def is_empty(self):
        return len(self.data) == 0

    def children(self):
        return self.data

    def place_dict(self, data):
        if data.name in self.dict:
            self.dict[data.name] += [data]
        else:
            self.dict[data.name] = [data]

    def append(self, other):
        """Append all elements of other."""
        for item in other.data:
            self.insert(item)

    def insert(self, data):
        """Inserts a sub-element."""
        self.data += [data]
        self.place_dict(data)

    def insert_first(self, data):
        self.data = [data] + self.data
        if data.name in self.dict:
            self.dict[data.name] = [data] + self.dict[data.name]
        else:
            self.dict[data.name] = [data]

    def insert_after(self, previous, data):
        """Insert after given node, or else insert as first."""
        if not previous in self.data: return self.insert_first(data)
        # completely rebuild list and dict
        new_childs = []
        self.dict = {}
        for child in self.data:
            new_childs += [child]
            self.place_dict(child)
            if child == previous:
                new_childs += [data]
                self.place_dict(data)
        self.data = new_childs

    def insert_at(self, pos, data):
        """Insert at given index (or as last)."""
        if pos >= len(self.data): return self.insert(data)
        # completely rebuild list and dict
        new_childs = []
        self.dict = {}
        i = 0
        for child in self.data:
            if i == pos:
                new_childs += [data]
                self.place_dict(data)
            new_childs += [child]
            self.place_dict(child)
            i += 1

        self.data = new_childs

    def insert_as_nth(self, pos, data):
        """Insert as nth child of same name."""
        if pos == 0: return self.insert_first(data)
        already = self.get_all(data.name)
        before = already[pos - 1]
        self.insert_after(before, data)

    def remove(self, child):
        """Removes a sub-element."""
        self.data.remove(child)
        self.dict[child.name].remove(child)

    def clear(self):
        """Remove everything."""
        self.data = []
        self.dict = {}

    def copy(self):
        """Return a recursive copy of the element."""
        copy = DataSub(self.name)
        for item in self.data:
            subcopy = item.copy()
            copy.insert(subcopy)
        return copy

    def compare(self, other):
        if len(self.data) != len(other.data): return False
        for i in range(self.data):
            if not self.data[i].compare(other.data[i]): return False
        return True

    def rename_child(self, child, name):
        self.dict[child.name].remove(child)
        child.name = name
        # rebuild complete mapping for this name
        if name in self.dict: del self.dict[name]
        for item in self.data:
            if item.name == name:
                if name in self.dict:
                    self.dict[name] += [item]
                else:
                    self.dict[name] = [item]

    def insert_text(self, name, data, translatable = False,
            textdomain = ""):
        data = DataText(name, data, translatable = translatable)
        data.textdomain = textdomain
        self.insert(data)

    def insert_macro(self, name, args = None):
        macrodata = "{" + name
        if args: macrodata += " " + str(args)
        macrodata += "}"
        data = DataMacro(name, macrodata)
        self.insert(data)

    def get_first(self, name, default = None):
        """Return first of given tag, or default"""
        if not name in self.dict or not self.dict[name]: return default
        return self.dict[name][0]

    def get_all_with_attributes(self, attr_name, **kw):
        """Like get_or_create_sub_with_attributes."""
        ret = []
        for data in self.get_all(attr_name):
            if isinstance(data, DataSub):
                for key in kw:
                    if data.get_text_val(key) != kw[key]:
                        break
                else:
                    ret += [data]
        return ret

    def get_or_create_sub(self, name):
        for data in self.get_all(name):
            if isinstance(data, DataSub): return data

        sub = DataSub(name, [])
        self.insert(sub)
        return sub

    def create_sub(self, name):
        sub = DataSub(name, [])
        self.insert(sub)
        return sub

    def get_or_create_sub_with_attributes(self, name, **kw):
        """For the uber lazy. Example:

            event = scenario.get_or_create_sub_with_attribute("event", name = "prestart")

            That should find the first prestart event and return it, or else
            create and insert a new prestart event and return it.
        """
        for data in self.get_all(name):
            if isinstance(data, DataSub):
                for key in kw:
                    if data.get_text_val(key) != kw[key]:
                        break
                else:
                    return data

        sub = DataSub(name, [])
        for key in kw:
            sub.set_text_val(key, kw[key])
        self.insert(sub)
        return sub

    def get_or_create_ifdef(self, name):
        for data in self.get_all(name):
            if isinstance(data, DataIfDef): return data

        ifdef = DataIfDef(name, [], "then")
        self.insert(ifdef)
        return ifdef

    def delete_all(self, name):
        while 1:
            data = self.get_first(name)
            if not data: break
            self.remove(data)

    def remove_all(self, name):
        self.delete_all(name)

    def find_all(self, *args):
        """Return list of multiple tags"""
        return [item for item in self.data if item.name in args]

    def get_all(self, name):
        """Return a list of all sub-items matching the given name."""
        if not name in self.dict: return []
        return self.dict[name]

    def get_text(self, name):
        """Return a text element"""
        for data in self.get_all(name):
            if isinstance(data, DataText): return data
        return None

    def get_texts(self, name):
        """Gets all text elements matching the name"""
        return [text for text in self.get_all(name)
            if isinstance(text, DataText)]

    def get_all_text(self):
        """Gets all text elements"""
        return [text for text in self.data
            if isinstance(text, DataText)]

    def get_binary(self, name):
        """Return a binary element"""
        for data in self.get_all(name):
            if isinstance(data, DataBinary): return data
        return None

    def remove_text(self, name):
        text = self.get_text(name)
        if text: self.remove(text)

    def get_macros(self, name):
        """Gets all macros matching the name"""
        return [macro for macro in self.get_all(name)
            if isinstance(macro, DataMacro)]

    def get_all_macros(self):
        """Gets all macros"""
        return [macro for macro in self.data
            if isinstance(macro, DataMacro)]

    def get_ifdefs(self, name):
        """Gets all ifdefs matching the name"""
        return [ifdef for ifdef in self.get_all(name)
            if isinstance(ifdef, DataIfDef)]

    def get_subs(self, name):
        """Gets all elements matching the name"""
        return [sub for sub in self.get_all(name)
            if isinstance(sub, DataSub)]

    def get_all_subs(self):
        """Gets all elements"""
        return [sub for sub in self.data
            if isinstance(sub, DataSub)]

    def remove_macros(self, name):
        for macro in self.get_macros(name):
            self.remove(macro)

    def get_binary_val(self, name, default = None):
        """For the lazy."""
        binary = self.get_binary(name)
        if binary: return binary.data
        return default

    def get_text_val(self, name, default = None):
        """For the lazy."""
        text = self.get_text(name)
        if text: return text.data
        return default

    def set_text_val(self, name, value, delete_if = None, translatable = False,
        textdomain = ""):
        """For the lazy."""
        text = self.get_text(name)
        if text:
            if value == delete_if:
                self.remove(text)
            else:
                text.data = value
                text.textdomain = textdomain
                text.translatable = translatable
        else:
            if value != delete_if:
                self.insert_text(name, value, translatable = translatable,
                    textdomain = textdomain)

    def get_comment(self, start):
        for item in self.get_all("comment"):
            if isinstance(item, DataComment):
                if item.data.startswith(start): return item
        return None

    def set_comment_first(self, comment):
        """For the lazy."""
        for item in self.get_all("comment"):
            if isinstance(item, DataComment):
                if item.data == comment: return

        self.insert_first(DataComment("comment", comment))

    def get_quantity(self, tag, difficulty, default = None):
        """For the even lazier, looks for a value inside a difficulty ifdef.
        """
        v = self.get_text_val(tag)
        if v != None: return v

        for ifdef in self.get_ifdefs(["EASY", "NORMAL", "HARD"][difficulty]):
            v = ifdef.get_text_val(tag)
            if v != None: return v

        return default

    def set_quantity(self, name, difficulty, value):
        """Sets one of 3 values of a quantity. If it doesn't exist yet, also the
        other difficulties get the same value.
        """
        value = str(value)
        # read existing values
        q = []
        for d in range(3):
            q += [self.get_quantity(name, d, value)]
        q[difficulty] = value

        # remove current tags
        self.remove_text(name)
        for ifdef in self.get_ifdefs("EASY") + self.get_ifdefs("NORMAL") + self.get_ifdefs("HARD"):
            ifdef.remove_text(name)

        # insert updated item
        if q[0] == q[1] == q[2]:
            self.set_text_val(name, value)
        else:
            for d in range(3):
                ifdef = self.get_or_create_ifdef(["EASY", "NORMAL", "HARD"][d])
                ifdef.set_text_val(name, q[d])

    def debug(self, show_contents = False, use_color = False, indent=0, write=True):
        if use_color:
            red = "\x1b[31;1m"
            off = "\x1b[0m"
        else:
            red = off = ""
        pos = indent * "  "
        result = pos + "\ " + red + self.name + off + " (" + self.__class__.__name__ + ")\n"
        if write:
            sys.stdout.write( result )
        indent += 1
        for child in self.data:
            cresult = child.debug(show_contents, use_color, indent, write=write)
            if not write:
                result += "\n" + cresult
        indent -= 1

        if not write:
            return result


class DataIfDef(DataSub):
    """
    An #ifdef section in WML.
    """
    def __init__(self, name, sub, type):
        DataSub.__init__(self, name, sub)
        self.type = type

    def copy(self):
        copy = DataSub.copy(self)
        copy.type = self.type
        return copy

def read_file(filename, root_name = "WML"):
    """
    Read in a file from disk and return a WML data object, with the WML in the
    file placed under an entry with the name root_name.
    """
    parser = wmlparser.Parser(None)
    parser.parse_file(filename)
    data = DataSub(root_name)
    parser.parse_top(data)
    return data


###############################################################################################################################

import os, glob, sys
import re

"""
NOTE: You should use wmlparser2.py instead which uses the C++
preprocessor.

Module implementing a WML parser in pure python.
"""

class Error(Exception):
    """
    This is a custom exception the parser throws on errors. It can display
    the position of the error as a tree of all files and macros leading to
    the error, with line numbers.
    """

    def __init__(self, parser, text):
        self.text = "%s:%d: %s" % (parser.filename, parser.line, text)
        for i in range(len(parser.texts)):
            parent = parser.texts[-1 - i]
            self.text += "\n " + " " * i + "from %s:%d" % (parent.filename, parent.line)

    def __str__(self):
        return self.text

class Parser:
    """
    The main parser class. An instance of this is needed for parsing.
    """

    class Macro:
        """Class to hold one single macro."""
        def __init__(self, name, params, text, textdomain):
            self.name, self.params, self.text, self.textdomain =\
                name, params, text, textdomain

    class TextState:
        def __init__(self, filename, text, textpos, line, current_path,
            textdomain):
            self.filename, self.text, self.textpos, self.line =\
                filename, text, textpos, line
            self.current_path = current_path
            self.textdomain = textdomain

    def __init__(self, data_dir, user_dir=None, no_macros_in_string=False):
        """
        Initialize a new WMLParser instance.

        data_dir is used for resolving {filepath} and {@filepath}
        user_dir is used for resolving {~filepath} and {@filepath}
        See http://www.wesnoth.org/wiki/PreprocessorRef
        """

        self.data_dir = data_dir
        self.user_dir = user_dir
        self.no_macros_in_string = no_macros_in_string

        self.textpos = 0
        self.line = 1
        self.macros = {}
        self.texts = []

        self.text = ""
        self.filename = ""

        self.current_path = "."

        self.textdomain = ""

        # Callback which is called for each macro. If it returns False, the
        # macro is ignored.
        self.macro_callback = None
        self.macro_not_found_callback = None
        self.no_macros = False

        # If set to a function, the function is called with the current
        # textdomain and string as arguments for any translatable string, and
        # is expected to return a translation.
        self.gettext = None

        # A list containing the stacked up #ifdefs.
        self.preprocessor_nesting = []
        self.stay_in_file = False

        # If set, included files are only parsed when under the given directory.
        self.only_expand_pathes = []

        # Whether to print current file, comments, and macro replacements.
        self.verbose = False

    def read_encoded(self, filename):
        """
        Helper for gracefully handling non-utf8 files and fixing up non-unix
        line endings.
        """
        try:
            text = open(filename, encoding='utf-8').read()
        except IOError:
            sys.stderr.write("Cannot open file %s!\n" % filename)
            return ""
        #try:
        #    u = text.decode("utf8")
        #except UnicodeDecodeError:
        #    u = text.decode("latin1")
        #text = u
        text = text.replace("\r\n", "\n").replace("\t", " ").replace("\r", "\n")
        if text == "" or text[-1] != "\n":
            text += "\n"
        return text

    def set_macro_not_found_callback(self, callback):
        """
        You can set a last-resort function which is called when a macro could
        not be resolved by the Parser. The calling format is:

        callback(wmlparser, name, params)
        """
        self.macro_not_found_callback = callback

    def parse_file(self, filename):
        """
        Set the parser to parse the given file.
        """
        text = self.read_encoded(filename)
        self.push_text(filename, text, cd = os.path.dirname(filename))

    def parse_stream(self, stream, binary = False):
        """
        Set the parser to parse from a file object.
        """
        text = stream.read()
        if not binary:
            text = text.replace("\r\n", "\n").replace("\t", " ").replace("\r", "\n")
        self.push_text("inline", text)

    def parse_text(self, text, binary = False):
        """
        Set the parser to directly parse from the given string.
        """
        if not binary:
            text = text.replace("\r\n", "\n").replace("\t", " ")
        self.push_text("inline", text)

    def push_text(self, filename, text, params = None, cd = None,
        initial_textdomain = ""):
        """
        Recursively parse a sub-document, e.g. when a file is included or a
        macro is executed.
        """
        if self.verbose:
            sys.stderr.write("%s:%d: Now parsing %s.\n" % (self.filename,
                                                           self.line, filename))
        if not text: text = "\n"
        self.texts.append(self.TextState(self.filename, self.text, self.textpos,
            self.line, self.current_path, self.textdomain))
        self.filename, self.text, self.params = filename, text, params
        self.textpos = 0
        self.line = 1
        self.textdomain = initial_textdomain
        if cd: self.current_path = cd

    def pop_text(self):
        """
        Finish the current text and return to parsing the caller.
        """
        textstate = self.texts.pop()
        self.filename, self.text, self.textpos, self.line =\
            textstate.filename, textstate.text, textstate.textpos, textstate.line
        self.current_path = textstate.current_path
        self.textdomain = textstate.textdomain
        if self.verbose:
            sys.stderr.write("%s:%d: Back.\n" % (self.filename, self.line))

    def get_macros(self):
        """
        Return a list of all macros currently known to the parser.
        """
        return self.macros

    def add_macros(self, macros):
        """
        Add additional macros to the parser.
        """
        for macro in macros:
            self.macros[macro] = macros[macro]

    def read_next(self):
        """Read the next character, taking care of \r and \t."""
        c = self.text[self.textpos]
        self.textpos += 1
        if c == "\n":
            self.line += 1
        if self.textpos == len(self.text):
            if len(self.texts) and not self.stay_in_file: self.pop_text()
        return c

    def at_end(self):
        """
        Return True if the parser is at the very end of the input, that is the
        last character of the topmost input text has been read.
        """
        return (len(self.texts) == 0
            or self.stay_in_file) and self.textpos == len(self.text)

    def check_end(self):
        if self.textpos == len(self.text):
            if len(self.texts): self.pop_text()

    def peek_next(self):
        """Like read_next, but does not consume."""
        if self.textpos >= len(self.text):
            if len(self.texts) and not self.stay_in_file:
                ts = self.texts[-1]
                if ts.textpos >= len(ts.text): return ""
                return ts.text[ts.textpos]
        return self.text[self.textpos]

    def read_until(self, sep):
        """Read until a character inside the string sep is found."""
        mob = re.compile(".*?[" + sep + "]", re.S).match(self.text, self.textpos)
        if mob:
            found = mob.group(0)
            self.line += found.count("\n")
            self.textpos = mob.end(0)
            if self.textpos == len(self.text) and not self.stay_in_file:
                if len(self.texts): self.pop_text()
            return found
        else:
            found = self.text[self.textpos:]
            self.line += found.count("\n")
            self.textpos = len(self.text)
            if len(self.texts) and not self.stay_in_file:
                self.pop_text()
                found += self.read_until(sep)
            return found

    def read_while(self, sep):
        """Read while characters are inside the string sep."""
        text = ""
        while not self.at_end():
            c = self.peek_next()
            if not c in sep:
                return text
            c = self.read_next()
            text += c
        return text

    def skip_whitespace_and_newlines(self):
        self.read_while(" \t\r\n")

    preprocessor_commands = ["#define", "#undef", "#textdomain", "#ifdef",
        "#ifndef", "#else", "#enddef", "#endif"]

    def read_lines_until(self, string):
        """
        Read lines until one contains the given string, but throw away any
        comments.
        """
        text = ""
        in_string = False
        while 1:
            if self.at_end():
                return None
            line = self.read_until("\n")

            line_start = 0
            if in_string:
                string_end = line.find('"')
                if string_end < 0:
                    text += line
                    continue
                in_string = False
                line_start = string_end + 1

            elif line.lstrip().startswith("#"):
                possible_comment = line.lstrip()
                for com in self.preprocessor_commands:
                    if possible_comment.startswith(com):
                        break
                else:
                    continue


            quotation = line.find('"', line_start)
            while quotation >= 0:
                in_string = True
                string_end = line.find('"', quotation + 1)
                if string_end < 0: break
                line_start = string_end + 1
                in_string = False
                quotation = line.find('"', line_start)

            if not in_string:
                end = line.find(string, line_start)
                if end >= 0:
                    text += line[:end]
                    break
            text += line
        return text

    def read_lua(self):
        """
        Read a << .... >> string, return contents literally.
        """
        text = ""
        while not self.at_end():
            c = self.read_next()
            if c == ">" and self.peek_next() == ">":
                self.read_next() #get rid of trailing >
                return text
            text += c
        raise Error(self, "Unexpected end of file")

    def skip_whitespace_inside_statement(self):
        self.read_while(" \t\r\n")
        if not self.at_end():
            c = self.peek_next()
            if c == "#":
                for command in self.preprocessor_commands:
                    if self.check_for(command): return
                self.read_until("\n")
                self.skip_whitespace_inside_statement()

    def skip_whitespace(self):
        self.read_while(" ")

    def check_for(self, str):
        """Compare the following text with str."""
        return self.text[self.textpos:self.textpos + len(str)] == str

    def parse_macro(self):
        """No recursive macro processing is done here. If a macro is passed as
        parameter to a macro, then whoever parses the macro replacement will do
        the macro recursion.
        Actually - I'm too tired right now to think this through. Maybe it
        should be done the other way around, starting expansion with the
        innermost macro?
        """
        params = []
        balance = 1
        macro = ""
        while balance > 0:
            macro += self.read_until("{}")
            if macro[-1] == "{":
                balance += 1
            elif macro[-1] == "}":
                balance -= 1
            else:
                raise Error(self, "Unclosed macro")
                return

        preserve = macro
        macro = macro[:-1] # Get rid of final }

        if self.macro_callback:
            if not self.macro_callback(macro): return None

        # If the macro starts with ~, assume a file in userdata.
        if macro[0] == "~":
            if self.user_dir:
                dirpath = self.user_dir + "/" + macro[1:]
            else:
                dirpath = None
        # If the macro starts with @, look first in data then in userdata.
        elif macro[0] == "@":
            if self.data_dir:
                dirpath = self.data_dir + "/" + macro[1:]
                if not os.path.exists(dirpath): dirpath = None
            else:
                dirpath = None
            if not dirpath and self.user_dir:
                dirpath = self.user_dir + "/" + macro[1:]
        # If the macro starts with ., look relative to the currently parsed
        # file.
        elif macro[0] == ".":
            dirpath = self.current_path + macro[1:]
        # Otherwise, try to interpret the macro as a filename in the data dir.
        elif self.data_dir != None:
            dirpath = self.data_dir + "/" + macro
        else:
            dirpath = None

        if dirpath != None and os.path.exists(dirpath):
            dirpath = os.path.normpath(dirpath)
            if self.only_expand_pathes:
                if not [x for x in self.only_expand_pathes if os.path.commonprefix([dirpath, x]) == x]:
                    return None
            # If it is a directory, parse all cfg files within.
            if os.path.isdir(dirpath):
                # Note: glob.glob will try to return unicode filenames
                # if you pass it an unicode string - but to deal with
                # non-unicode filenames as are allowed in linux we
                # convert to a byte-string.
                # Execute all WML files in the directory.
                files = glob.glob(str(dirpath + "/*.cfg"))
                # And also execute directories with a _main.cfg.
                files += glob.glob(str(dirpath + "/*/_main.cfg"))
                files.sort()
                mc = dirpath + "/_main.cfg"
                fc = dirpath + "/_final.cfg"

                if mc in files:
                    # If there's a file called _main.cfg, only parse that.
                    files = [mc]
                elif fc in files:
                    # If there's a file called _final.cfg, parse it only after
                    # all others.
                    files.remove(fc)
                    files.append(fc)
            else:
                files = [dirpath]
            files.reverse()
            for path in files:
                self.push_text(path, self.read_encoded(path), cd = os.path.dirname(path))
            return None

        # It's OK for user directories not to exist.
        # Nothing prefixed with ~ can be a macro.
        if macro.startswith("~") and not self.verbose:
            return None

        # No file was found, try to do macro expansion. First, push the
        # macro call again. E.g. {blah 1 2 3}.
        self.push_text("macro", preserve)

        # Find all parameters.
        while 1:
            read = self.read_until('"}{ (\n')
            if not read:
                sys.stderr.write("? %s\n" % macro)
                sys.stderr.write(" (%s)\n" % params)
                raise Error(self, "Unexpected end of file")
                break
            sep = read[-1]
            read = read[:-1]

            if sep == "}":
                if read: params += [read]
                break

            elif sep == "{":
                balance = 1
                param = sep + read
                while balance:
                    c = self.read_next()
                    if c == "{":
                        balance += 1
                    elif c == "}":
                        balance -= 1
                    param += c
                params += [param]

            elif sep == '"':
                # Cannot parse strings here.. it must be passed to the macro as
                # is, so the real string parser can handle it. Else there will
                # be subtle bugs, e.g. when a MACRO evaluates to a + at the end
                # of line.
                read += '"' + self.read_until('"')
                params += [read]

            elif sep == "(":
                in_string = False
                balance = 1
                param = read
                while balance:
                    c = self.read_next()
                    # Ignore () within strings
                    if c == '"':
                        in_string = not in_string

                    if not in_string:
                        if c == "(":
                            balance += 1
                        elif c == ")":
                            balance -= 1
                    param += c
                params += [param[:-1]]

            else:
                if read:
                    params += [read]
                self.read_while(" \n")

        if self.no_macros:
            return wmldata.DataMacro("macro", " ".join(params))

        name = params[0]
        if name in self.macros:
            macro = self.macros[name]
            text = macro.text
            for i, j in enumerate(macro.params):
                if 1 + i >= len(params):
                    raise Error(self, "Not enough parameters for macro %s. " % name +
                        "%d given but %d needed %s." % (len(params) - 1,
                            len(macro.params), macro.params))
                rep = params[1 + i]
                # Handle gettext replacement here, since inside the macro
                # the textdomain will be wrong.
                if self.gettext and rep and rep[0] == "_":
                    q = rep.find('"')
                    qe = rep.find('"', q + 1)
                    rep = self.gettext(self.textdomain, rep[q + 1:qe])
                    rep = '"' + rep + '"'
                if self.verbose:
                    #s = "Replacing {%s} with %s\n" % (j, rep)
                    ##sys.stderr.write(s.encode("utf8"))
                    pass
                text = text.replace("{%s}" % j, rep)

            if text:
                self.push_text(name, text, initial_textdomain = macro.textdomain)
            else:
                pass # empty macro, nothing to do
        else:
            if self.macro_not_found_callback:
                keep_macro = self.macro_not_found_callback(self, name, params)
                if keep_macro: return keep_macro
            if self.verbose:
                sys.stderr.write("No macro %s.\n" % name.encode("utf8"))
            if self.verbose:
                sys.stderr.write(" (%s:%d)\n" % (self.filename, self.line))
            return name
        return None

    def parse_string(self):
        text = ""
        match_read_end = '"'
        if not self.no_macros_in_string:
            match_read_end += '{'
        while not self.at_end():
            text += self.read_until(match_read_end)
            if text[-1] == '"':
                if self.peek_next() == '"':
                    self.read_next()
                else:
                    return text[:-1]
            elif text[-1] == '{':
                text = text[:-1]
                not_found = self.parse_macro()
                if not isinstance(not_found, Data):
                    if not_found:
                        text += not_found
            else:
                break
        raise Error(self, "Unclosed string")

    def parse_inside(self, data, c):
        variables = []
        values = []
        variable = ""
        value = ""
        got_assign = False
        spaces = ""
        filename = "(None)"
        line = -1
        got_lua = False
        while 1:
            if c == "{":
                keep_macro = self.parse_macro()
                if keep_macro:
                    if self.no_macros:
                        values += [keep_macro.name]
                    else:
                        values += [keep_macro]
            elif c == "\n":
                break
            elif c == "#":
                # FIXME
                # Assume something like:
                # name="picture.png="+
                # #textdomain blah
                # "translateable name"
                #
                # For now, we ignore the textdomain...
                self.read_until("\n")
                break
            elif c == "+":
                value = value.rstrip() # remove whitespace before +
                self.skip_whitespace_inside_statement() # read over newline
            elif not got_assign:
                if c == "=":
                    variables += [variable.rstrip()]
                    got_assign = True
                    translatable = False
                    filename = self.filename
                    line = self.line
                    self.skip_whitespace()
                else:
                    if c == ",":
                        variables += [variable]
                        variable = ""
                    else:
                        variable += c
            else:
                if c == "<" and self.peek_next() == "<":
                    self.read_next() #skip the rest of the opening
                    value += self.read_lua()
                elif c == '"':
                    # We want the textdomain at the beginning of the string,
                    # the end of the string may be outside a macro and already
                    # in another textdomain.
                    textdomain = self.textdomain
                    # remove possible _
                    i = len(value)
                    while i > 0:
                        i -= 1
                        if value[i] != " ": break
                    got_underscore = False
                    if value and value[i] == "_":
                        got_underscore = True
                        translatable = True
                        # remove whitespace before _
                        while i > 1:
                            if value[i - 1] != " ": break
                            i -= 1
                        value = value[:i]

                    string = self.parse_string()
                    if got_underscore:
                        if self.gettext:
                            string = self.gettext(textdomain, string)
                    value += string
                    spaces = ""
                else:
                    if c == "," and len(values) + 1 < len(variables):
                        values += [value]
                        value = ""
                        spaces = ""
                    elif c == " ":
                        spaces += c
                    else:
                        if spaces:
                            value += spaces
                            spaces = ""
                        value += c
            if self.at_end(): break
            c = self.read_next()
        if not got_assign:
            raise Error(self, "= expected for \"%s\"" % variable)
            return []
        values += [value]

        data = []
        for i in range(len(variables)):
            try:
                key = DataText(variables[i], values[i], translatable)
                key.set_meta(filename, line)
                data.append(key)
            except IndexError:
                raise Error(self, "Assignement does not match: %s = %s" % (
                    str(variables), str(values)))
        return data

    def parse_top(self, data, state = None):
        while 1:
            self.skip_whitespace_and_newlines()
            if self.at_end():
                if state:
                    raise Error(self, "Tag stack non-empty (%s) at end of data" % state)
                break
            c = self.read_next()
            if c == "#": # comment or preprocessor
                if self.check_for("define "):
                    self.read_until(" ")
                    params = []
                    while 1:
                        name = self.read_until(" \n")
                        sep = name[-1]
                        name = name[:-1]
                        if name: params += [name]
                        if sep == "\n": break
                        self.read_while(" ")

                    text = self.read_lines_until("#enddef")
                    if text == None:
                        raise Error(self, "#define without #enddef")

                    self.macros[params[0]] = self.Macro(
                        params[0], params[1:], text, self.textdomain)
                    if self.verbose:
                        sys.stderr.write("New macro: %s.\n" % params[0])

                elif self.check_for("undef "):
                    self.read_until(" ")
                    name = self.read_until("\n")
                    name = name.rstrip()
                    if " " in name:
                        if self.verbose: sys.stderr.write("Stray symbols in #undef %s\n" % name)
                        name = name.split(" ")[0]
                    if name in self.macros: del self.macros[name]
                    elif self.verbose: sys.stderr.write("undef'd macro '%s' did not exist\n" % name)
                elif self.check_for("ifdef ") or self.check_for("ifndef"):

                    what = "#" + self.read_until(" ").rstrip()

                    name = self.read_until(" \n")
                    if name[-1] == " ": self.read_while(" \n")
                    name = name[:-1]

                    condition_failed = False
                    if what == "#ifdef":
                        if name in self.macros:
                            pass
                        else:
                            condition_failed = True
                    else: # what == "#ifndef"
                        if not name in self.macros:
                            pass
                        else:
                            condition_failed = True

                    self.preprocessor_nesting.append((what, condition_failed))

                    # If the condition is true, we simply continue parsing. At
                    # some point we will either hit an #else or #endif, and
                    # things continue there. If the condition failed, we skip
                    # over everything until we find the matching #else or
                    # endif.
                    if condition_failed:
                        self.stay_in_file = True
                        balance = 1
                        while balance > 0 and not self.at_end():
                            line = self.read_until("\n")
                            line = line.lstrip()
                            if line.startswith("#ifdef"): balance += 1
                            if line.startswith("#ifndef"): balance += 1
                            if line.startswith("#endif"): balance -= 1
                            if line.startswith("#else"):
                                if balance == 1:
                                    balance = -1
                                    break

                        self.stay_in_file = False
                        if balance == 0:
                            self.preprocessor_nesting.pop()
                        if balance > 0:
                            raise Error(self, "Missing  #endif for %s" % what)

                        self.check_end()

                elif self.check_for("else"):
                    if not self.preprocessor_nesting:
                        raise Error(self, "#else without #ifdef")

                    self.read_until("\n")

                    # We seen an #else - that means we are at the end of a
                    # conditional preprocessor block which has executed. So
                    # we should now ignore everything up to the #endif.
                    balance = 1
                    self.stay_in_file = True
                    while balance > 0 and not self.at_end():
                        line = self.read_until("\n")
                        line = line.lstrip()
                        if line.startswith("#ifdef"): balance += 1
                        if line.startswith("#ifndef"): balance += 1
                        if line.startswith("#endif"): balance -= 1
                    self.stay_in_file = False
                    if balance != 0:
                        raise Error(self, "Missing #endif for #else")

                    self.check_end()

                elif self.check_for("endif"):
                    if not self.preprocessor_nesting:
                        raise Error(self, "#endif without #ifdef")
                    self.preprocessor_nesting.pop()
                    self.read_until("\n")

                elif self.check_for("textdomain"):
                    self.read_until(" ")
                    self.textdomain = self.read_until("\n").strip()
                else: # comment
                    line = self.read_until("\n")
                    #comment = c + line
                    if self.verbose:
                        #msg = "Comment removed: %s" % comment
                        ##sys.stderr.write(msg.encode("utf8"))
                        pass
            elif c == '[':
                name = self.read_until("]")[:-1]
                if name[0] == '/':
                    if state == name[1:]:
                        return
                    raise Error(self, "Mismatched closing tag [%s], expected [/%s]" % (name, state))
                elif name[0] == '+':
                    name = name[1:]
                    try:
                        subdata = data.dict[name][-1]
                        self.parse_top(subdata, name)
                    except KeyError:
                        subdata = DataSub(name)
                        subdata.set_meta(self.filename, self.line)
                        self.parse_top(subdata, name)
                        data.insert(subdata)
                elif name[0] == " ":
                    # We single this case out explicitly as the wesnoth parser
                    # ignores it due to implementation reasons and this makes
                    # more sense of the error message
                    raise Error(self, "Invalid tag [%s] contains whitespace" % name)
                else:
                    subdata = DataSub(name)
                    subdata.set_meta(self.filename, self.line)
                    self.parse_top(subdata, name)
                    data.insert(subdata)
            elif c == '{':
                keep_macro = self.parse_macro()
                if isinstance(keep_macro, Data):
                    data.insert(keep_macro)
            else:
                for subdata in self.parse_inside(data, c):
                    data.insert(subdata)


import json
def jsonify(tree, verbose=False, depth=0):
    """
Convert a DataSub into JSON

If verbose, insert a linebreak after every brace and comma (put every item on its own line), otherwise, condense everything into a single line.
"""
    print("{", end=' ')
    first = True
    sdepth1 = "\n" + " " * depth
    sdepth2 = sdepth1 + " "
    for child in tree.children():
        if first:
            first = False
        else:
            sys.stdout.write(",")
        if verbose:
            sys.stdout.write(sdepth2)
        print('"%s":' % child.name, end=' ')
        if child.get_type() == "DataSub":
            jsonify(child, verbose, depth + 1)
        else:
            print(json.dumps(child.get_value()), end=' ')
    if verbose:
        sys.stdout.write(sdepth1)
    sys.stdout.write("}")

from xml.sax.saxutils import escape
def xmlify(tree, verbose=False, depth=0):
    sdepth = ""
    if verbose:
        sdepth = "  " * depth
    for child in tree.children():
        if child.get_type() == "DataSub":
            print('%s<%s>' % (sdepth, child.name))
            xmlify(child, verbose, depth + 1)
            print('%s</%s>' % (sdepth, child.name))
        else:
            if "\n" in child.get_value() or "\r" in child.get_value():
                print(sdepth + '<' + child.name + '>' + \
            '<![CDATA[' + child.get_value() + ']]>' + '</' + child.name + '>')
            else:
                print(sdepth + '<' + child.name + '>' + \
            escape(child.get_value())  + '</' + child.name + '>')

if __name__ == "__main__":
    import argparse, subprocess
    try: import psyco
    except ImportError: pass
    else: psyco.full()

    # Hack to make us not crash when we encounter characters that aren't ASCII
    sys.stdout = __import__("codecs").getwriter('utf-8')(sys.stdout)

    argumentparser = argparse.ArgumentParser("usage: %(prog)s [options]")
    argumentparser.add_argument("-p", "--path",  help = "specify wesnoth data path")
    argumentparser.add_argument("-C", "--color", action = "store_true",
        help = "use colored output")
    argumentparser.add_argument("-u", "--userpath",  help = "specify userdata path")
    argumentparser.add_argument("-e", "--execute",  help = "execute given WML")
    argumentparser.add_argument("-v", "--verbose", action = "store_true",
        help = "make the parser very verbose")
    argumentparser.add_argument("-n", "--no-macros", action = "store_true",
        help = "do not expand any macros")
    argumentparser.add_argument("-c", "--contents", action = "store_true",
        help = "display contents of every tag")
    argumentparser.add_argument("-j", "--to-json", action = "store_true",
        help = "output JSON version of tree")
    argumentparser.add_argument("-x", "--to-xml", action = "store_true",
        help = "output XML version of tree")
    argumentparser.add_argument("filename", nargs = "?",
        help = "file to parse")
    args = argumentparser.parse_args()

    if args.path:
        path = args.path
    else:
        try:
            p = subprocess.Popen(["wesnoth", "--path"], stdout = subprocess.PIPE)
            path = p.stdout.read().strip()
            path = os.path.join(path, "data")
        except OSError:
            sys.stderr.write("Could not determine Wesnoth path.\n")
            path = None

    wmlparser = Parser(path, args.userpath)
    if args.no_macros:
        wmlparser.no_macros = True

    if args.verbose:
        wmlparser.verbose = True
        def gt(domain, x):
            print("gettext: '%s' '%s'" % (domain, x))
            return x
        wmlparser.gettext = gt

    wmlparser.do_preprocessor_logic = True

    if args.execute:
        wmlparser.parse_text(args.execute)
    elif args.filename:
        wmlparser.parse_file(args.filename)
    else:
        wmlparser.parse_stream(sys.stdin)

    data = DataSub("WML")
    wmlparser.parse_top(data)

    if args.to_json:
        jsonify(data, True) # For more readable results
    elif args.to_xml:
        xmlify(data, True)
    else:
        data.debug(show_contents = args.contents, use_color = args.color)
