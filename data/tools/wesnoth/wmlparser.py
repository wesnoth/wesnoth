#!/usr/bin/env python
# encoding: utf8

import wmldata, os, glob, sys
import re

"""Module implementing a WML parser."""

class Error(Exception):
    def __init__(self, parser, text):
        self.text = "%s:%d: %s" % (parser.filename, parser.line, text)
        for i in range(len(parser.texts)):
            parent = parser.texts[-1 - i]
            self.text += "\n " + " " * i + "from %s:%d" % (parent.filename, parent.line)

    def __str__(self):
        return self.text

class Parser:
    class Macro:
        """Class to hold one single macro."""
        def __init__(self, name, params, text):
            self.name, self.params, self.text = name, params, text

    class TextState:
        def __init__(self, filename, text, textpos, line, current_path,
            textdomain):
            self.filename, self.text, self.textpos, self.line =\
                filename, text, textpos, line
            self.current_path = current_path
            self.textdomain = textdomain

    def __init__(self, data_dir, user_dir = None):
        """
        Initialize a new WMLParser instance.

        data_dir is used for resolving {filepath} and {@filepath}
        user_dir is used for resolving {~filepath} and {@filepath}
        See http://www.wesnoth.org/wiki/PreprocessorRef
        """
        self.data_dir = data_dir
        self.user_dir = user_dir

        self.textpos = 0
        self.line = 1
        self.macros = {}
        self.texts = []

        self.text = ""
        self.filename = ""

        self.current_path = "."

        self.textdomain = ""

        self.macro_not_found_callback = None
        self.no_macros = False

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
            text = file(filename).read()
        except IOError:
            sys.stderr.write("Cannot open file %s!\n" % filename)
            return ""
        try:
            u = text.decode("utf8")
        except UnicodeDecodeError:
            u = text.decode("latin1")
        text = u
        text = text.replace("\r\n", "\n").replace("\t", " ")
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

    def parse_stream(self, stream):
        """
        Set the parser to parse from a file object.
        """
        text = stream.read()
        text = text.replace("\r\n", "\n").replace("\t", " ")
        self.push_text("inline", text)

    def parse_text(self, text):
        """
        Set the parser to directly parse from the given string.
        """
        text = text.replace("\r\n", "\n").replace("\t", " ")
        self.push_text("inline", text)

    def push_text(self, filename, text, params = None, cd = None):
        """
        Recursively parse a sub-document, e.g. when a file is included or a
        macro is executed.
        """
        if self.verbose:
            sys.stderr.write("%s:%d: Now parsing %s.\n" % (self.filename,
                self.line, filename))
        text = text.replace("\r\n", "\n").replace("\t", " ")
        if not text: text = "\n"
        self.texts.append(self.TextState(self.filename, self.text, self.textpos,
            self.line, self.current_path, self.textdomain))
        self.filename, self.text, self.params = filename, text, params
        self.textpos = 0
        self.line = 1
        self.textdomain = ""
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
            if len(self.texts): self.pop_text()
        return c

    def at_end(self):
        """
        Return True if the parser is at the very end of the input, that is the
        last character of the topmost input text has been read.
        """
        return len(self.texts) == 0 and self.textpos == len(self.text)

    def peek_next(self):
        """Like read_next, but does not consume."""
        if self.textpos >= len(self.text):
            if len(self.texts):
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
            if self.textpos == len(self.text):
                if len(self.texts): self.pop_text()
            return found
        else:
            found = self.text[self.textpos:]
            self.line += found.count("\n")
            self.textpos = len(self.text)
            if len(self.texts):
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

    def skip_whitespace_inside_statement(self):
        self.read_while(" \t\r\n")
        if not self.at_end():
            c = self.peek_next()
            if c == "#":
                if self.check_for("#define"): return
                if self.check_for("#undef"): return
                if self.check_for("#textdomain"): return
                if self.check_for("#ifdef"): return
                if self.check_for("#ifndef"): return
                if self.check_for("#else"): return
                if self.check_for("#end"): return
                self.read_until("\n")
                self.skip_whitespace_inside_statement()

    def skip_whitespace(self):
        self.read_while(" ")

    def check_for(self, str):
        """Compare the following text with str."""
        return self.text[self.textpos:self.textpos + len(str)] == str

    def read_upto_string(self, str):
        """Read input up to and including the given string."""
        pos = self.text.find(str, self.textpos)
        if pos == -1:
            return None
        found = self.text[self.textpos:pos]
        self.textpos = pos + len(str)
        self.line += found.count("\n")
        if self.textpos == len(self.text):
            self.pop_text()
        return found

    def parse_macro(self):
        """No recursive macro processing is done here. If a macro is passed as
        parameter to a macro, then whoever parses the macro replacement will do
        the macro recursion.
        Actually - I'm too tired right now to think this through. Maybe it
        should be done the other way around, starting expansion with the
        innermost macro?
        """
        params = []
        macro = self.read_until("}")
        if macro[-1] != "}":
            raise Error(self, "Unclosed macro")
            return

        preserve = macro
        macro = macro[:-1] # Get rid of final }
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
        # Otherwise, try to interprete the macro as a filename in the data dir.
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
                # Execute all WML files in the directory.
                files = glob.glob(dirpath + "/*.cfg")
                # And also execute directories with a _main.cfg.
                files += glob.glob(dirpath + "/*/_main.cfg")
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

        # No file was found, try to do macro expansion.
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
                balance = 1
                param = read
                while balance:
                    c = self.read_next()
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
            for i in range(len(macro.params)):
                if 1 + i >= len(params):
                    raise Error(self, "Not enough parameters for macro %s. " % name +
                        "%d given but %d needed %s." % (len(params) - 1,
                            len(macro.params), macro.params))
                if self.verbose:
                    s = "Replacing {%s} with %s" % (macro.params[i], params[1 + i])
                    print s.encode("utf8")
                text = text.replace("{%s}" % macro.params[i],
                    params[1 + i])

            if text:
                self.push_text(name, text)
            else:
                pass # empty macro, nothing to do
        else:
            if self.macro_not_found_callback:
                keep_macro = self.macro_not_found_callback(self, name, params)
                if keep_macro: return keep_macro
            sys.stderr.write("No macro %s.\n" % name)
            sys.stderr.write(" (%s:%d)\n" % (self.filename, self.line))
            return name
        return None

    def parse_string(self):
        text = ""
        while not self.at_end():
            text += self.read_until('"{')
            if text[-1] == '"':
                return text[:-1]
            elif text[-1] == '{':
                text = text[:-1]
                not_found = self.parse_macro()
                if not isinstance(not_found, wmldata.Data):
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
                    self.skip_whitespace()
                else:
                    if c == ",":
                        variables += [variable]
                        variable = ""
                    else:
                        variable += c  
            else:
                if c == '"':
                    # remove possible _
                    i = len(value)
                    while i > 0:
                        i -= 1
                        if value[i] != " ": break
                    if value and value[i] == "_":
                        translatable = True
                        # This is not the assignement =, but from e.g. MENU_IMG_TXT
                        if i == 0 or value[i - 1] in [" ", "="]:
                            # remove whitespace before _
                            while i > 1:
                                if value[i - 1] != " ": break
                                i -= 1
                            value = value[:i]

                    string = self.parse_string()
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
        j = 0
        for i in range(len(variables)):
            data += [wmldata.DataText(variables[i], values[j],
                translatable = translatable, textdomain = self.textdomain)]
            j += 1
        return data

    def parse_top(self, data, state = None):
        while 1:
            self.skip_whitespace_and_newlines()
            if self.at_end():
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
                    text = self.read_upto_string("#enddef")
                    if text == None:
                        raise Error(self, "#define without #enddef")
                        return
                    self.macros[params[0]] = self.Macro(params[0], params[1:], text)

                elif self.check_for("undef "):
                    self.read_until(" ")
                    name = self.read_until(" \n")
                    self.macros[name] = None
                elif self.check_for("ifdef "):
                    self.read_until(" ")
                    name = self.read_until(" \n")
                    if name[-1] == " ": self.read_while(" \n")
                    name = name[:-1]
                    subdata = wmldata.DataIfDef(name, [], "then")
                    self.parse_top(subdata, "#ifdef")
                    data.insert(subdata)
                elif self.check_for("ifndef "):
                    self.read_until(" ")
                    name = self.read_until(" \n")
                    if name[-1] == " ": self.read_while(" \n")
                    name = name[:-1]
                    subdata = wmldata.DataIfDef(name, [], "then")
                    self.parse_top(subdata, "#ifndef")
                    data.insert(subdata)

                elif self.check_for("else"):
                    self.read_until("\n")
                    if state != "#ifdef" and state != "#ifndef":
                        raise Error(self, "#else without #ifdef")
                    subdata = wmldata.DataIfDef("else", [], "else")
                    self.parse_top(subdata, "#else")
                    data.insert(subdata)
                    return

                elif self.check_for("endif"):
                    self.read_until("\n")
                    if state != "#ifdef" and state != "#else" and state !=\
                        "#ifndef":
                        self.read_until("\n")
                        raise Error(self, "#endif without #ifdef or #else")
                    return

                elif self.check_for("textdomain"):
                    self.read_until(" ")
                    name = self.read_until("\n").strip()
                    if name == "wesnoth":
                        self.textdomain = "wesnoth"
                    else:
                        self.textdomain = ""
                else: # comment
                    line = self.read_until("\n")
                    comment = c + line
                    if self.verbose:
                        msg = "Comment removed: %s" % comment
                        sys.stderr.write(msg.encode("utf8"))
            elif c == '[':
                name = self.read_until("]")[:-1]
                if name[0] == '/':
                    if state == name[1:] or state == "+" + name[1:]:
                        return
                    raise Error(self, "Mismatched closing tag [%s], expected [/%s]" % (name, state))
                subdata = wmldata.DataSub(name)
                self.parse_top(subdata, name)
                data.insert(subdata)
            elif c == '{':
                keep_macro = self.parse_macro()
                if isinstance(keep_macro, wmldata.Data):
                    data.insert(keep_macro)
            else:
                for subdata in self.parse_inside(data, c):
                    data.insert(subdata)


if __name__ == "__main__":
    import optparse, subprocess
    try: import psyco
    except ImportError: pass
    else: psyco.full()

    optionparser = optparse.OptionParser()
    optionparser.set_usage("usage: %prog [options] [filename]")
    optionparser.add_option("-p", "--path",  help = "specify wesnoth data path")
    optionparser.add_option("-C", "--color", action = "store_true",
        help = "use colored output")
    optionparser.add_option("-u", "--userpath",  help = "specify userdata path")
    optionparser.add_option("-e", "--execute",  help = "execute given WML")
    optionparser.add_option("-v", "--verbose", action = "store_true",
        help = "make the parser very verbose")
    optionparser.add_option("-n", "--no-macros", action = "store_true",
        help = "do not expand any macros")
    optionparser.add_option("-c", "--contents", action = "store_true",
        help = "display contents of every tag")
    options, args = optionparser.parse_args()

    if options.path:
        path = options.path
    else:
        try:
            p = subprocess.Popen(["wesnoth", "--path"], stdout = subprocess.PIPE)
            path = p.stdout.read().strip()
            path = os.path.join(path, "data")
        except OSError:
            sys.stderr.write("Could not determine Wesnoth path.\n")
            path = None

    wmlparser = Parser(path, options.userpath)
    if options.no_macros:
        wmlparser.no_macros = True

    if options.verbose:
        wmlparser.verbose = True

    if options.execute:
        wmlparser.parse_text(options.execute)
    elif args:
        wmlparser.parse_file(args[0])
    else:
        wmlparser.parse_stream(sys.stdin)

    data = wmldata.DataSub("WML")
    wmlparser.parse_top(data)

    data.debug(show_contents = options.contents, use_color = options.color)

