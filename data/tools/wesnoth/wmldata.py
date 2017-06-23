#!/usr/bin/env python2
# encoding: utf-8

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

from __future__ import print_function
import re, sys
import wmlparser
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
                if type(r) is unicode: r = r.encode("utf8")
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
        for i in xrange(self.data):
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
        for d in xrange(3):
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
            for d in xrange(3):
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
