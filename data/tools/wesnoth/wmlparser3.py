#!/usr/bin/env python3
# encoding: utf-8

"""
This parser uses the --preprocess option of wesnoth so a working
wesnoth executable must be available at runtime if the WML to parse
contains preprocessing directives.

Pure WML can be parsed as is.

For example:

    wml = ""
    [unit]
        id=elve
        name=Elve
        [abilities]
            [damage]
                id=Ensnare
            [/dama  ge]
        [/abilities]
    [/unit]
    ""

    p = Parser()
    cfg = p.parse_text(wml)

    for unit in cfg.get_all(tag = "unit"):
        print(unit.get_text_val("id"))
        print(unit.get_text_val("name"))
        for abilities in unit.get_all(tag = "abilitities"):
            for ability in abilities.get_all(tag = ""):
                print(ability.get_name())
                print(ability.get_text_val("id"))

Because no preprocessing is required, we did not have to pass the
location of the wesnoth executable to Parser.

The get_all method always returns a list over matching tags or
attributes.

The get_name method can be used to get the name and the get_text_val
method can be used to query the value of an attribute.
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
    can be made from multiple translatable strings we model
    it as a list of several StringNode each with its own text domain.
    """
    def __init__(self, data : bytes):
        self.textdomain = None # non-translatable by default
        self.data = data

    def wml(self) -> bytes:
        if not self.data:
            return b""
        return self.data

    def debug(self):
        if self.textdomain:
            return "_<%s>'%s'" % (self.textdomain,
                self.data.decode("utf8", "ignore"))
        else:
            return "'%s'" % self.data.decode("utf8", "ignore")

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

    def wml(self) -> bytes:
        s = self.name + b"=\""
        for v in self.value:
            s += v.wml().replace(b"\"", b"\"\"")
        s += b"\""
        return s
    
    def debug(self):
        return self.name.decode("utf8") + "=" + " .. ".join(
            [v.debug() for v in self.value])

    def get_text(self, translation = None) -> str:
        """
        Returns a text representation of the node's value. The
        translation callback, if provided, will be called on each
        partial string with the string and its corresponding textdomain
        and the returned translation will be used.
        """
        r = ""
        for s in self.value:
            ustr = s.data.decode("utf8", "ignore")
            if translation:
                r += translation(ustr, s.textdomain)
            else:
                r += ustr
        return r

    def get_binary(self):
        """
        Returns the unmodified binary representation of the value.
        """
        r = b""
        for s in self.value:
            r += s.data
        return r

    def get_name(self):
        return self.name.decode("utf8")

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

    def wml(self) -> bytes:
        """
        Returns a (binary) WML representation of the entire node.
        All attribute values are enclosed in quotes and quotes are
        escaped (as double quotes). Note that no other escaping is
        performed (see the BinaryWML specification for additional
        escaping you may require).
        """
        s = b"[" + self.name + b"]\n"
        for sub in self.data:
            s += sub.wml() + b"\n"
        s += b"[/" + self.name + b"]\n"
        return s

    def debug(self):
        s = "[%s]\n" % self.name.decode("utf8")
        for sub in self.data:
            for subline in sub.debug().splitlines():
                s += "    %s\n" % subline
        s += "[/%s]\n" % self.name.decode("utf8")
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
            return self.speedy_tags.get(kw["tag"].encode("utf8"), [])

        r = []
        for sub in self.data:
            ok = True
            for k, v in list(kw.items()):
                v = v.encode("utf8")
                if k == "tag":
                    if not isinstance(sub, TagNode): ok = False
                    elif v != b"" and sub.name != v: ok = False
                elif k == "att":
                    if not isinstance(sub, AttributeNode): ok = False
                    elif v != b"" and sub.name != v: ok = False
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
        x = self.get_all(att = name)
        if not x: return default
        return x[val].get_text(translation)

    def get_binary(self, name, default = None):
        """
        Returns the unmodified binary data for the first attribute
        of the given name or the passed default value if it is not
        found.
        """
        x = self.get_all(att = name)
        if not x: return default
        return x[0].get_binary()

    def append(self, node):
        """
        Appends a child node (must be either a TagNode or
        AttributeNode).
        """
        self.data.append(node)

        if isinstance(node, TagNode):
            if node.name not in self.speedy_tags:
                self.speedy_tags[node.name] = []
            self.speedy_tags[node.name].append(node)

    def get_name(self):
        return self.name.decode("utf8")

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
    trans_pat = re.compile(r'^_\s*"')

    def __init__(self, wesnoth_exe = None, config_dir = None,
            data_dir = None):
        """
        wesnoth_exe - Wesnoth executable to use. This should have been
            configured to use the desired data and config directories.
        config_dir - The Wesnoth configuration directory, can be
            None to use the wesnoth default.
        data_dir - The Wesnoth data  directory, can be None to use
            the wesnoth default.

        After parsing is done the root node of the result will be
        in the root attribute.
        """
        self.wesnoth_exe = wesnoth_exe
        self.config_dir = None
        if config_dir: self.config_dir = os.path.abspath(config_dir)
        self.data_dir = None
        if data_dir: self.data_dir = os.path.abspath(data_dir)
        self.keep_temp_dir = None
        self.temp_dir = None
        self.no_preprocess = (wesnoth_exe == None)
        self.preprocessed = None
        self.verbose = False

        self.last_wml_line = "?"
        self.parser_line = 0
        self.line_in_file = 42424242
        self.chunk_start = "?"

    def parse_file(self, path, defines = "") -> RootNode:
        """
        Parse the given file found under path.
        """
        self.path = path
        if not self.no_preprocess:
            self.preprocess(defines)
        return self.parse()

    def parse_binary(self, binary : bytes, defines = "") -> RootNode:
        """
        Parse a chunk of binary WML.
        """
        temp = tempfile.NamedTemporaryFile(prefix = "wmlparser_",
            suffix=".cfg")
        temp.write(binary)
        temp.flush()
        self.path = temp.name
        if not self.no_preprocess:
            self.preprocess(defines)
        return self.parse()

    def parse_text(self, text, defines = "") -> RootNode:
        """
        Parse a text string.
        """
        return self.parse_binary(text.encode("utf8"), defines)

    def preprocess(self, defines):
        """
        This is called by the parse functions to preprocess the
        input from a normal WML .cfg file into a preprocessed
        .plain file.
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
            print((" ".join(commandline)))
        p = subprocess.Popen(commandline,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        if self.verbose:
            print((out + err))
        self.preprocessed = output + "/" + os.path.basename(self.path) +\
            ".plain"
        if not os.path.exists(self.preprocessed):
            first_line = open(self.path).readline().strip()
            raise WMLError(self, "Preprocessor error:\n" +
                " ".join(commandline) + "\n" +
                "First line: " + first_line + "\n" +
                out.decode("utf8") +
                err.decode("utf8"))

    def parse_line_without_commands_loop(self, line : str) -> str:
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
            arrows = line.find(b'>>')
            if arrows >= 0:
                self.in_arrows = False
                self.temp_string += line[:arrows]
                self.temp_string_node = StringNode(self.temp_string)
                self.temp_string = b""
                self.temp_key_nodes[self.commas].value.append(
                    self.temp_string_node)
                self.in_arrows = False
                return line[arrows + 2:]
            else:
                self.temp_string += line
            return

        quote = line.find(b'"')

        if not self.in_string:
            arrows = line.find(b'<<')
            if arrows >= 0 and (quote < 0 or quote > arrows):
                self.parse_line_without_commands(line[:arrows])
                self.in_arrows = True
                return line[arrows + 2:]

        if quote >= 0:
            if self.in_string:
                # double quote
                if quote < len(line) - 1 and line[quote + 1] == b'"'[0]:
                    self.temp_string += line[:quote + 1]
                    return line[quote + 2:]
                self.temp_string += line[:quote]
                self.temp_string_node = StringNode(self.temp_string)
                if self.translatable:
                    self.temp_string_node.textdomain = self.textdomain
                    self.translatable = False
                self.temp_string = b""
                if not self.temp_key_nodes:
                    raise WMLError(self, "Unexpected string value.")

                self.temp_key_nodes[self.commas].value.append(
                    self.temp_string_node)

                self.in_string = False
                return line[quote + 1:]
            else:
                self.parse_outside_strings(line[:quote])
                self.in_string = True
                return line[quote + 1:]
        else:
            if self.in_string:
                self.temp_string += line
            else:
                self.parse_outside_strings(line)

    def parse_line_without_commands(self, line):
        while True:
            line = self.parse_line_without_commands_loop(line)
            if not line:
                break

    def parse_outside_strings(self, line):
        """
        Parse a WML fragment outside of strings.
        """
        if not line: return
        if not self.temp_key_nodes:
            line = line.lstrip()
            if not line: return

            if line.startswith(b"#textdomain "):
                self.textdomain = line[12:].strip().decode("utf8")
                return

            # Is it a tag?
            if line.startswith(b"["):
                self.handle_tag(line)
            # No tag, must be an attribute.
            else:
                self.handle_attribute(line)
        else:
            for i, segment in enumerate(line.split(b"+")):
                segment = segment.lstrip(b" ")

                if i > 0:
                    # If the last segment is empty (there was a plus sign
                    # at the end) we need to skip newlines.
                    self.skip_newlines_after_plus = not segment.strip()

                if not segment: continue

                if self.trans_pat.match(segment):
                    self.translatable = True
                    segment = segment[1:].lstrip(b" ")[1:-1]
                    if not segment: continue
                self.handle_value(segment)


    def handle_tag(self, line):
        end = line.find(b"]")
        if end < 0:
            if line.endswith(b"\n"):
                raise WMLError(self, "Expected closing bracket.")
            self.in_tag += line
            return
        tag = (self.in_tag + line[:end])[1:]
        self.in_tag = b""
        if tag.startswith(b"/"):
            self.parent_node = self.parent_node[:-1]
        else:
            node = TagNode(tag, location=(self.line_in_file, self.chunk_start))
            if self.parent_node:
                self.parent_node[-1].append(node)
            self.parent_node.append(node)
        self.parse_outside_strings(line[end + 1:])

    def handle_attribute(self, line):
        assign = line.find(b"=")
        remainder = None
        if assign >= 0:
            remainder = line[assign + 1:]
            line = line[:assign]

        self.commas = 0
        self.temp_key_nodes = []
        for att in line.split(b","):
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
            for subsegment in segment.split(b",", maxsplit):
                self.temp_string += subsegment.strip()
                self.temp_string_node = StringNode(self.temp_string)
                self.temp_string = b""
                self.temp_key_nodes[self.commas].value.append(
                    self.temp_string_node)
                if self.commas < n - 1:
                    self.commas += 1

        # Finish assignment on newline, except if there is a
        # plus sign before the newline.
        add_text(segment)
        if segment.endswith(b"\n") and not self.skip_newlines_after_plus:
            self.temp_key_nodes = []

    def parse(self) -> RootNode:
        """
        Parse preprocessed WML into a tree of tags and attributes.
        """

        # parsing state
        self.temp_string = b""
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
        self.in_tag = b""

        command_marker_byte = bytes([254])

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
                print(("removing " + self.temp_dir))
            shutil.rmtree(self.temp_dir, ignore_errors=True)

        return self.root

    def handle_command(self, com):
        if com.startswith(b"line "):
            self.last_wml_line = com[5:]
            _ = self.last_wml_line.split(b" ")
            self.chunk_start = [(_[i+1], int(_[i])) for i in range(0, len(_), 2)]
            self.line_in_file = self.chunk_start[0][1]
        elif com.startswith(b"textdomain "):
            self.textdomain = com[11:].decode("utf8")
        else:
            raise WMLError(self, "Unknown parser command: " + com)

    def get_all(self, **kw):
        return self.root.get_all(**kw)

    def get_text_val(self, name, default=None, translation=None):
        return self.root.get_text_val(name, default, translation)

def jsonify(tree, verbose=False, depth=1):
    """
Convert a Parser tree into JSON

If verbose, insert a linebreak after every brace and comma (put every
item on its own line), otherwise, condense everything into a single line.
"""
    import json
    def node_to_dict(n):
        d = {}
        tags = set(x.get_name() for x in n.get_all(tag = ""))
        for tag in tags:
            d[tag] = [node_to_dict(x) for x in n.get_all(tag = tag)]
        for att in n.get_all(att = ""):
            d[att.get_name()] = att.get_text()
        return d

    print(json.dumps(node_to_dict(tree), indent = depth if verbose else None))

def xmlify(tree, verbose=False, depth=0):
    import xml.etree.ElementTree as ET

    def node_to_et(n):
        et = ET.Element(n.get_name())
        for att in n.get_all(att = ""):
            attel = ET.Element(att.get_name())
            attel.text = att.get_text()
            et.append(attel)
        for tag in n.get_all(tag = ""):
            et.append(node_to_et(tag))
        return et
    
    ET.ElementTree(node_to_et(tree.get_all()[0])).write(
        sys.stdout, encoding = "unicode")

if __name__ == "__main__":
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
    arg.add_argument("-v", "--verbose", action="store_true")
    arg.add_argument("-x", "--to-xml", action="store_true")
    args = arg.parse_args()

    if not args.input and not args.text and not args.test:
        sys.stderr.write("No input given. Use -h for help.\n")
        sys.exit(1)

    if (args.wesnoth and not os.path.exists(args.wesnoth)):
        sys.stderr.write("Wesnoth executable not found.\n")
        sys.exit(1)

    if not args.wesnoth:
        print("Warning: Without the -w option WML is not preprocessed!",
            file = sys.stderr)

    if args.test:
        print("Running tests")
        p = Parser(args.wesnoth, args.config_dir,
            args.data_dir)
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
                print(("FAILED " + note))
                print("INPUT:")
                print(input)
                print("OUTPUT:")
                print(output)
                print("EXPECTED:")
                print(expected)
                print("__________")
            else:
                print(("PASSED " + note))

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
    code='
        "quotes" here
        ""blah""
    '
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

    p = Parser(args.wesnoth, args.config_dir, args.data_dir)
    if args.keep_temp:
        p.keep_temp_dir = args.keep_temp
    if args.verbose: p.verbose = True
    if args.input: p.parse_file(args.input, args.defines)
    elif args.text: p.parse_text(args.text, args.defines)
    if args.to_json:
        jsonify(p.root, True)
        print()
    elif args.to_xml:
        print('<?xml version="1.0" encoding="UTF-8" ?>')
        print('<root>')
        xmlify(p.root, True, 1)
        print('</root>')
    else:
        print((p.root.debug()))
