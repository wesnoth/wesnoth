#!/usr/bin/env python2
# encoding: utf-8

"""
This parser uses the --preprocess option of wesnoth so a working
wesnoth executable must be available at runtime.

If you are using this you shold instead use wmlparser3.py and upgrade
your code to Python 3.
"""

import os, glob, sys, re, subprocess, argparse, tempfile, shutil
import atexit

tempdirs_to_clean = []

@atexit.register
def cleaner():
    for temp_dir in tempdirs_to_clean:
        shutil.rmtree(temp_dir, ignore_errors=True)

class WMLError(Exception):
    """
    Catch this exception to retrieve the first error message from
    the parser.
    """
    def __init__(self, parser=None, message=None):
        if parser:
            self.line = parser.parser_line
            self.wml_line = parser.last_wml_line
            self.message = message
            self.preprocessed = parser.preprocessed

    def __str__(self):
        return """WMLError:
    %s %s
    %s
    %s
""" % (str(self.line), self.preprocessed, self.wml_line, self.message)

class StringNode:
    """
    One part of an attribute's value. Because a single WML string
    can be made from multiple translatable strings we need to model
    it this way (as a list of several StringNode).
    """
    def __init__(self, data):
        self.textdomain = None # non-translatable by default
        self.data = data

    def debug(self):
        if self.textdomain:
            return "_<%s>%s" % (self.textdomain, repr(self.data))
        else:
            return repr(self.data)

class AttributeNode:
    """
    A WML attribute. For example the "id=Elfish Archer" in:
        [unit]
            id=Elfish Archer
        [/unit]
    """
    def __init__(self, name, location=None):
        self.name = name
        self.location = location
        self.value = [] # List of StringNode

    def debug(self):
        return self.name + "=" + " .. ".join(
            [v.debug() for v in self.value])

    def get_text(self, translation=None):
        r = u""
        for s in self.value:
            ustr = s.data.decode("utf8", "ignore")
            if translation:
                r += translation(ustr, s.textdomain)
            else:
                r += ustr
        return r

class TagNode:
    """
    A WML tag. For example the "unit" in this example:
        [unit]
            id=Elfish Archer
        [/unit]
    """
    def __init__(self, name, location=None):
        self.name = name
        self.location = location
        # List of child elements, which are either of type TagNode or
        # AttributeNode.
        self.data = []

        self.speedy_tags = {}

    def debug(self):
        s = "[%s]\n" % self.name
        for sub in self.data:
            for subline in sub.debug().splitlines():
                s += "    %s\n" % subline
        s += "[/%s]\n" % self.name
        return s

    def get_all(self, **kw):
        """
        This gets all child tags or child attributes of the tag.
        For example:

        [unit]
            name=A
            name=B
            [attack]
            [/attack]
            [attack]
            [/attack]
        [/unit]

        unit.get_all(att = "name")
        will return two nodes for "name=A" and "name=B"

        unit.get_all(tag = "attack")
        will return two nodes for the two [attack] tags.

        unit.get_all()
        will return 4 nodes for all 4 sub-elements.

        unit.get_all(att = "")
        Will return the two attribute nodes.

        unit.get_all(tag = "")
        Will return the two tag nodes.

        If no elements are found an empty list is returned.
        """
        if len(kw) == 1 and "tag" in kw and kw["tag"]:
            return self.speedy_tags.get(kw["tag"], [])

        r = []
        for sub in self.data:
            ok = True
            for k, v in kw.items():
                if k == "tag":
                    if not isinstance(sub, TagNode): ok = False
                    elif v != "" and sub.name != v: ok = False
                elif k == "att":
                    if not isinstance(sub, AttributeNode): ok = False
                    elif v != "" and sub.name != v: ok = False
            if ok:
                r.append(sub)
        return r

    def get_text_val(self, name, default=None, translation=None, val=-1):
        """
        Returns the value of the specified attribute. If the attribute
        is given multiple times, the value number val is returned (default
        behaviour being to return the last value). If the
        attribute is not found, the default parameter is returned.

        If a translation is specified, it should be a function which
        when passed a unicode string and text-domain returns a
        translation of the unicode string. The easiest way is to pass
        it to gettext.translation if you have the binary message
        catalogues loaded.
        """
        x = self.get_all(att=name)
        if not x: return default
        return x[val].get_text(translation)

    def append(self, node):
        self.data.append(node)

        if isinstance(node, TagNode):
            if node.name not in self.speedy_tags:
                self.speedy_tags[node.name] = []
            self.speedy_tags[node.name].append(node)

class RootNode(TagNode):
    """
    The root node. There is exactly one such node.
    """
    def __init__(self):
        TagNode.__init__(self, None)

    def debug(self):
        s = ""
        for sub in self.data:
            for subline in sub.debug().splitlines():
                s += subline + "\n"
        return s

class Parser:
    def __init__(self, wesnoth_exe, config_dir, data_dir,
        no_preprocess):
        """
        path - Path to the file to parse.
        wesnoth_exe - Wesnoth executable to use. This should have been
            configured to use the desired data and config directories.
        """
        self.wesnoth_exe = wesnoth_exe
        self.config_dir = None
        if config_dir: self.config_dir = os.path.abspath(config_dir)
        self.data_dir = None
        if data_dir: self.data_dir = os.path.abspath(data_dir)
        self.keep_temp_dir = None
        self.temp_dir = None
        self.no_preprocess = no_preprocess
        self.preprocessed = None
        self.verbose = False

        self.last_wml_line = "?"
        self.parser_line = 0
        self.line_in_file = 42424242
        self.chunk_start = "?"

    def parse_file(self, path, defines=""):
        self.path = path
        if not self.no_preprocess:
            self.preprocess(defines)
        self.parse()

    def parse_text(self, text, defines=""):
        temp = tempfile.NamedTemporaryFile(prefix="wmlparser_",
            suffix=".cfg")
        temp.write(text)
        temp.flush()
        self.path = temp.name
        if not self.no_preprocess:
            self.preprocess(defines)
        self.parse()

    def preprocess(self, defines):
        """
        Call wesnoth --preprocess to get preprocessed WML which we
        can subsequently parse.

        If this is not called then the .parse method will assume the
        WML is already preprocessed.
        """
        if self.keep_temp_dir:
            output = self.keep_temp_dir
        else:
            output = tempfile.mkdtemp(prefix="wmlparser_")
            tempdirs_to_clean.append(output)

        self.temp_dir = output
        commandline = [self.wesnoth_exe]
        if self.data_dir:
            commandline += ["--data-dir", self.data_dir]
        if self.config_dir:
            commandline += ["--config-dir", self.config_dir]
        commandline += ["--preprocess", self.path, output]
        if defines:
            commandline += ["--preprocess-defines", defines]
        if self.verbose:
            print(" ".join(commandline))
        p = subprocess.Popen(commandline,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        if self.verbose:
            print(out + err)
        self.preprocessed = output + "/" + os.path.basename(self.path) +\
            ".plain"
        if not os.path.exists(self.preprocessed):
            first_line = open(self.path).readline().strip()
            raise WMLError(self, "Preprocessor error:\n" +
                " ".join(commandline) + "\n" +
                "First line: " + first_line + "\n" +
                out +
                err)

    def parse_line_without_commands(self, line):
        """
        Once the .plain commands are handled WML lines are passed to
        this.
        """
        if not line: return

        if line.strip():
            self.skip_newlines_after_plus = False

        if self.in_tag:
            self.handle_tag(line)
            return

        if self.in_arrows:
            arrows = line.find('>>')
            if arrows >= 0:
                self.in_arrows = False
                self.temp_string += line[:arrows]
                self.temp_string_node = StringNode(self.temp_string)
                self.temp_string = ""
                self.temp_key_nodes[self.commas].value.append(
                    self.temp_string_node)
                self.in_arrows = False
                self.parse_line_without_commands(line[arrows + 2:])
            else:
                self.temp_string += line
            return

        quote = line.find('"')

        if not self.in_string:
            arrows = line.find('<<')
            if arrows >= 0 and (quote < 0 or quote > arrows):
                self.parse_line_without_commands(line[:arrows])
                self.in_arrows = True
                self.parse_line_without_commands(line[arrows + 2:])
                return

        if quote >= 0:
            if self.in_string:
                # double quote
                if quote < len(line) - 1 and line[quote + 1] == '"':
                    self.temp_string += line[:quote + 1]
                    self.parse_line_without_commands(line[quote + 2:])
                    return
                self.temp_string += line[:quote]
                self.temp_string_node = StringNode(self.temp_string)
                if self.translatable:
                    self.temp_string_node.textdomain = self.textdomain
                    self.translatable = False
                self.temp_string = ""
                if not self.temp_key_nodes:
                    raise WMLError(self, "Unexpected string value.")

                self.temp_key_nodes[self.commas].value.append(
                    self.temp_string_node)

                self.in_string = False
                self.parse_line_without_commands(line[quote + 1:])
            else:
                self.parse_outside_strings(line[:quote])
                self.in_string = True
                self.parse_line_without_commands(line[quote + 1:])
        else:
            if self.in_string:
                self.temp_string += line
            else:
                self.parse_outside_strings(line)

    def parse_outside_strings(self, line):
        """
        Parse a WML fragment outside of strings.
        """
        if not line: return
        if not self.temp_key_nodes:
            line = line.lstrip()
            if not line: return

            if line.startswith("#textdomain "):
                self.textdomain = line[12:].strip()
                return

            # Is it a tag?
            if line[0] == "[":
                self.handle_tag(line)
            # No tag, must be an attribute.
            else:
                self.handle_attribute(line)
        else:
            for i, segment in enumerate(line.split("+")):
                segment = segment.lstrip(" \t")

                if i > 0:
                    # If the last segment is empty (there was a plus sign
                    # at the end) we need to skip newlines.
                    self.skip_newlines_after_plus = not segment.strip()

                if not segment: continue

                if segment.rstrip() == '_':
                    self.translatable = True
                    segment = segment[1:].lstrip(" ")
                    if not segment: continue
                self.handle_value(segment)


    def handle_tag(self, line):
        end = line.find("]")
        if end < 0:
            if line.endswith("\n"):
                raise WMLError(self, "Expected closing bracket.")
            self.in_tag += line
            return
        tag = (self.in_tag + line[:end])[1:]
        self.in_tag = ""
        if tag[0] == "/":
            self.parent_node = self.parent_node[:-1]
        else:
            node = TagNode(tag, location=(self.line_in_file, self.chunk_start))
            if self.parent_node:
                self.parent_node[-1].append(node)
            self.parent_node.append(node)
        self.parse_outside_strings(line[end + 1:])

    def handle_attribute(self, line):
        assign = line.find("=")
        remainder = None
        if assign >= 0:
            remainder = line[assign + 1:]
            line = line[:assign]

        self.commas = 0
        self.temp_key_nodes = []
        for att in line.split(","):
            att = att.strip()
            node = AttributeNode(att, location=(self.line_in_file, self.chunk_start))
            self.temp_key_nodes.append(node)
            if self.parent_node:
                self.parent_node[-1].append(node)

        if remainder:
            self.parse_outside_strings(remainder)

    def handle_value(self, segment):
        def add_text(segment):
            segment = segment.rstrip()
            if not segment: return
            n = len(self.temp_key_nodes)
            maxsplit = n - self.commas - 1
            if maxsplit < 0: maxsplit = 0
            for subsegment in segment.split(",", maxsplit):
                self.temp_string += subsegment.strip()
                self.temp_string_node = StringNode(self.temp_string)
                self.temp_string = ""
                self.temp_key_nodes[self.commas].value.append(
                    self.temp_string_node)
                if self.commas < n - 1:
                    self.commas += 1

        # Finish assignment on newline, except if there is a
        # plus sign before the newline.
        add_text(segment)
        if segment[-1] == "\n" and not self.skip_newlines_after_plus:
            self.temp_key_nodes = []

    def parse(self):
        """
        Parse preprocessed WML into a tree of tags and attributes.
        """

        # parsing state
        self.temp_string = ""
        self.temp_string_node = None
        self.commas = 0
        self.temp_key_nodes = []
        self.in_string = False
        self.in_arrows = False
        self.textdomain = "wesnoth"
        self.translatable = False
        self.root = RootNode()
        self.parent_node = [self.root]
        self.skip_newlines_after_plus = False
        self.in_tag = ""

        command_marker_byte = chr(254)

        input = self.preprocessed
        if not input: input = self.path

        for rawline in open(input, "rb"):
            compos = rawline.find(command_marker_byte)
            self.parser_line += 1
            # Everything from chr(254) to newline is the command.
            if compos != 0:
                self.line_in_file += 1
            if compos >= 0:
                self.parse_line_without_commands(rawline[:compos])
                self.handle_command(rawline[compos + 1:-1])
            else:
                self.parse_line_without_commands(rawline)

        if self.keep_temp_dir is None and self.temp_dir:
            if self.verbose:
                print("removing " + self.temp_dir)
            shutil.rmtree(self.temp_dir, ignore_errors=True)

    def handle_command(self, com):
        if com.startswith("line "):
            self.last_wml_line = com[5:]
            _ = self.last_wml_line.split(" ")
            self.chunk_start = [(_[i+1], int(_[i])) for i in range(0, len(_), 2)]
            self.line_in_file = self.chunk_start[0][1]
        elif com.startswith("textdomain "):
            self.textdomain = com[11:]
        else:
            raise WMLError(self, "Unknown parser command: " + com)

    def get_all(self, **kw):
        return self.root.get_all(**kw)

    def get_text_val(self, name, default=None, translation=None):
        return self.root.get_text_val(name, default, translation)


import json
def jsonify(tree, verbose=False, depth=0):
    """
Convert a DataSub into JSON

If verbose, insert a linebreak after every brace and comma (put every item on its own line), otherwise, condense everything into a single line.
"""
    print "{",
    first = True
    sdepth1 = "\n" + " " * depth
    sdepth2 = sdepth1 + " "
    for pair in tree.speedy_tags.iteritems():
        if first:
            first = False
        else:
            sys.stdout.write(",")
        if verbose:
            sys.stdout.write(sdepth2)
        print '"%s":' % pair[0],
        if verbose:
            sys.stdout.write(sdepth1)
        print '[',
        first_tag = True
        for tag in pair[1]:
            if first_tag:
                first_tag = False
            else:
                sys.stdout.write(",")
            if verbose:
                sys.stdout.write(sdepth2)
            jsonify(tag, verbose, depth + 2)
        if verbose:
            sys.stdout.write(sdepth2)
        sys.stdout.write("]")
    for child in tree.data:
        if isinstance(child, TagNode):
            continue
        if first:
            first = False
        else:
            sys.stdout.write(",")
        if verbose:
            sys.stdout.write(sdepth2)
        print '"%s":' % child.name,
        print json.dumps(child.get_text()),
    if verbose:
        sys.stdout.write(sdepth1)
    sys.stdout.write("}")

from xml.sax.saxutils import escape
def xmlify(tree, verbose=False, depth=0):
    sdepth = ""
    if verbose:
        sdepth = "  " * depth
    for child in tree.data:
        if isinstance(child, TagNode):
            print '%s<%s>' % (sdepth, child.name)
            xmlify(child, verbose, depth + 1)
            print '%s</%s>' % (sdepth, child.name)
        else:
            if "\n" in child.get_text() or "\r" in child.get_text():
                print sdepth + '<' + child.name + '>' + \
            '<![CDATA[' + child.get_text() + ']]>' + '</' + child.name + '>'
            else:
                print sdepth + '<' + child.name + '>' + \
            escape(child.get_text()) + '</' + child.name + '>'

if __name__ == "__main__":
    # Hack to make us not crash when we encounter characters that aren't ASCII
    sys.stdout = __import__("codecs").getwriter('utf-8')(sys.stdout)
    arg = argparse.ArgumentParser()
    arg.add_argument("-a", "--data-dir", help="directly passed on to wesnoth.exe")
    arg.add_argument("-c", "--config-dir", help="directly passed on to wesnoth.exe")
    arg.add_argument("-i", "--input", help="a WML file to parse")
    arg.add_argument("-k", "--keep-temp", help="specify directory where to keep temp files")
    arg.add_argument("-t", "--text", help="WML text to parse")
    arg.add_argument("-w", "--wesnoth", help="path to wesnoth.exe")
    arg.add_argument("-d", "--defines", help="comma separated list of WML defines")
    arg.add_argument("-T", "--test", action="store_true")
    arg.add_argument("-j", "--to-json", action="store_true")
    arg.add_argument("-n", "--no-preprocess", action="store_true")
    arg.add_argument("-v", "--verbose", action="store_true")
    arg.add_argument("-x", "--to-xml", action="store_true")
    args = arg.parse_args()

    if not args.input and not args.text and not args.test:
        sys.stderr.write("No input given. Use -h for help.\n")
        sys.exit(1)

    if not args.no_preprocess and (not args.wesnoth or not
        os.path.exists(args.wesnoth)):
        sys.stderr.write("Wesnoth executable not found.\n")
        sys.exit(1)

    if args.test:
        print("Running tests")
        p = Parser(args.wesnoth, args.config_dir,
            args.data_dir, args.no_preprocess)
        if args.keep_temp:
            p.keep_temp_dir = args.keep_temp
        if args.verbose: p.verbose = True

        only = None
        def test2(input, expected, note, function):
            if only and note != only: return
            input = input.strip()
            expected = expected.strip()
            p.parse_text(input)
            output = function(p).strip()
            if output != expected:
                print("__________")
                print("FAILED " + note)
                print("INPUT:")
                print(input)
                print("OUTPUT:")
                print(output)
                print("EXPECTED:")
                print(expected)
                print("__________")
            else:
                print("PASSED " + note)

        def test(input, expected, note):
            test2(input, expected, note, lambda p: p.root.debug())

        test(
"""
[test]
a=1
[/test]
""", """
[test]
    a='1'
[/test]
""", "simple")

        test(
"""
[test]
a, b, c = 1, 2, 3
[/test]
""", """
[test]
    a='1'
    b='2'
    c='3'
[/test]
""", "multi assign")

        test(
"""
[test]
a, b = 1, 2, 3
[/test]
""", """
[test]
    a='1'
    b='2, 3'
[/test]
""", "multi assign 2")

        test(
"""
[test]
a, b, c = 1, 2
[/test]
""", """
[test]
    a='1'
    b='2'
    c=
[/test]
""", "multi assign 3")

        test(
"""
#textdomain A
#define X
    _ "abc"
#enddef
#textdomain B
[test]
x = _ "abc" + {X}
[/test]
""", """
[test]
    x=_<B>'abc' .. _<A>'abc'
[/test]
""", "textdomain")

        test(
"""
[test]
x,y = _1,_2
[/test]
""", """
[test]
    x='_1'
    y='_2'
[/test]
""", "underscores")

        test(
"""
[test]
a = "a ""quoted"" word"
[/test]
""",
"""
[test]
    a='a "quoted" word'
[/test]
""", "quoted")

        test(
"""
[test]
code = <<
    "quotes" here
    ""blah""
>>
[/test]
""",
"""
[test]
    code='\\n    "quotes" here\\n    ""blah""\\n'
[/test]
""", "quoted2")

        test(
"""
foo="bar"+



"baz"
""",
"""
foo='bar' .. 'baz'
""", "multi line string")

        test(
"""
#define baz

"baz"
#enddef
foo="bar"+{baz}
""",
"""
foo='bar' .. 'baz'
""", "defined multi line string")

        test(
"""
foo="bar" + "baz" # blah
""",
"""
foo='bar' .. 'baz'
""", "comment after +")

        test(
"""
#define baz
"baz"
#enddef
foo="bar" {baz}
""",
"""
foo='bar' .. 'baz'
""", "defined string concatenation")

        test(
"""
#define A BLOCK
[{BLOCK}]
[/{BLOCK}]
#enddef
{A blah}
""",
"""
[blah]
[/blah]
""", "defined tag")

        test2(
"""
[test]
    a=1
    b=2
    a=3
    b=4
[/test]
""", "3, 4", "multiatt",
    lambda p:
        p.get_all(tag = "test")[0].get_text_val("a") + ", " +
        p.get_all(tag = "test")[0].get_text_val("b"))

        sys.exit(0)

    p = Parser(args.wesnoth, args.config_dir, args.data_dir,
        args.no_preprocess)
    if args.keep_temp:
        p.keep_temp_dir = args.keep_temp
    if args.verbose: p.verbose = True
    if args.input: p.parse_file(args.input, args.defines)
    elif args.text: p.parse_text(args.text, args.defines)
    if args.to_json:
        jsonify(p.root, True)
        print
    elif args.to_xml:
        print '<?xml version="1.0" encoding="UTF-8" ?>'
        print '<root>'
        xmlify(p.root, True, 1)
        print '</root>'
    else:
        print(p.root.debug())
