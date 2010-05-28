#!/usr/bin/env python
# encoding: utf8

"""
This parser uses the --preprocess option of wesnoth so a working
wesnoth executable must be available at runtime.
"""

import os, glob, sys, re, subprocess, optparse, tempfile

class WMLError(Exception):
    """
    Catch this exception to retrieve the first error message from
    the parser.
    """
    def __init__(self, parser, message):
        self.line = parser.parser_line
        self.wml_line = parser.last_wml_line
        self.message = message
        self.preprocessed = parser.preprocessed
    
    def __str__(self):
        r = "WMLError:\n"
        r += "    " + str(self.line) + " " + self.preprocessed + "\n"
        r += "    " + self.wml_line + "\n"
        r += "    " + self.message + "\n"
        return r

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
            return "_<" + self.textdomain + ">" +\
                repr(self.data)
        else:
            return repr(self.data)

class AttributeNode:
    """
    A WML attribute. For example the "id=Elfish Archer" in:
        [unit]
            id=Elfish Archer
        [/unit]
    """
    def __init__(self, key):
        self.key = key
        self.value = [] # List of StringNode
    
    def debug(self):
        return self.key + "=" + " .. ".join(
            [v.debug() for v in self.value])
        
    def get_text(self, translation = None):
        r = u""
        for s in self.value:
            if translation:
                r += translation(s.data.decode("utf8"), s.textdomain)
            else:
                r += s.data.decode("utf8")
        return r

class TagNode:
    """
    A WML tag. For example the "unit" in this example:
        [unit]
            id=Elfish Archer
        [/unit]
    """
    def __init__(self, name):
        self.name = name
        # List of child elements, which are either of type TagNode or
        # AttributeNode.
        self.data = []
        
        self.speedy_tags = {}
    
    def debug(self):
        s = ""
        s += "[" + self.name + "]\n"
        for sub in self.data:
            for subline in sub.debug().splitlines():
                s += "    " + subline + "\n"
        s += "[/" + self.name + "]\n"
        return s

    def get_all(self, **kw):
        
        if len(kw) == 1 and "tag" in kw:
            return self.speedy_tags.get(kw["tag"], [])
        
        r = []
        for sub in self.data:
            ok = True
            for k, v in kw.items():
                if k == "tag":
                   if not isinstance(sub, TagNode): ok = False
                   elif not sub.name == v: ok = False
                elif k == "att":
                   if not isinstance(sub, AttributeNode): ok = False
                   elif not sub.key == v: ok = False
            if ok:
                r.append(sub)
        return r
    
    def get_text_val(self, name, default = None, translation = None):
        x = self.get_all(att = name)
        if not x: return default
        return x[0].get_text(translation)
    
    def append(self, node):
        self.data.append(node)
        
        if isinstance(node, TagNode):
            if node.name not in self.speedy_tags:
                self.speedy_tags[node.name] =[]
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
    def __init__(self, wesnoth_exe):
        """
        path - Path to the file to parse.
        wesnoth_exe - Wesnoth executable to use. This should have been
            configured to use the desired data and config directories.
        """
        self.wesnoth_exe = wesnoth_exe
        self.preprocessed = None
        
        self.last_wml_line = "?"
        self.parser_line = 0
    
    def parse_file(self, path, defines = ""):
        self.path = path
        self.preprocess(defines)
        self.parse()

    def parse_text(self, text, defines = ""):
        self.file = tempfile.NamedTemporaryFile(prefix = "wmlparser_",
            suffix = ".cfg")
        self.file.write(text)
        self.file.flush()
        self.path = self.file.name
        self.preprocess(defines)
        self.parse()

    def preprocess(self, defines):
        """
        Call wesnoth --preprocess to get preprocessed WML which we
        can subsequently parse.
        
        If this is not called then the .parse method will assume the
        WML is already preprocessed.
        """
        output = "/tmp/wmlparser"
        if not os.path.exists(output): os.mkdir(output)
        p_option = "-p=" + defines if defines else "-p "
        commandline = [self.wesnoth_exe, p_option, self.path,
            output]
        p = subprocess.Popen(commandline,
            stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = p.communicate()
        self.preprocessed = output + "/" + os.path.basename(self.path) +\
            ".plain"
        if not os.path.exists(self.preprocessed):
            raise WMLError(self, "Preprocessor error:\n" +
                " ".join(commandline) + "\n" +
                out +
                err)

    def parse_line_without_commands(self, line):
        """
        Once the .plain commands are handled WML lines are passed to
        this.
        """
        if not line: return
        
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

        arrows = line.find('<<')
        quote = line.find('"')
        
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
                segment = segment.lstrip(" ")
                if not segment: continue

                if segment[0] == "_":
                    self.translatable = True
                    segment = segment[1:].lstrip(" ")
                    if not segment: continue

                self.handle_value(segment, i == 0)
    
    def handle_tag(self, line):
        end = line.find("]")
        if end <= 0:
            raise WMLError(self, "Expected closing bracket.")
        tag = line[1:end]
        if tag[0] == "/":
            self.parent_node = self.parent_node[:-1]
        else:
            node = TagNode(tag)
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
            node = AttributeNode(att)
            self.temp_key_nodes.append(node)
            self.parent_node[-1].append(node)

        if remainder:
            self.parse_outside_strings(remainder)

    def handle_value(self, segment, is_first):
        
        def add_text(segment):
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
        if segment[-1] == "\n":
            segment = segment.rstrip()
            if segment:
                add_text(segment)
                self.temp_key_nodes = []
            elif is_first:
                self.temp_key_nodes = []
        else:
            add_text(segment)

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

        command_marker_byte = chr(254)
        
        input = self.preprocessed
        if not input: input = self.path

        for rawline in open(input, "rb"):
            self.parser_line += 1
            # Everything from þ to newline is the command.
            compos = rawline.find(command_marker_byte)
            if compos >= 0:
                self.parse_line_without_commands(rawline[:compos])
                self.handle_command(rawline[compos + 1:-1])
            else:
                self.parse_line_without_commands(rawline)
                    
    def handle_command(self, com):
        if com.startswith("line "):
            self.last_wml_line = com[5:]
        elif com.startswith("textdomain "):
            self.textdomain = com[11:]
        else:
            raise WMLError(self, "Unknown parser command: " + com);

    def get_all(self, **kw):
        return self.root.get_all(**kw)
    
    def get_text_val(self, name, default = None, translation = None):
        return self.root.get_text_val(name, default, translation)


########################################################################
#                                                                      # 
# EVERYTHING BELOW IS ONLY TESTING STUFF AND CAN BE SAFELY IGNORED OR  #
# REMOVED.                                                             #
#                                                                      #
########################################################################

if __name__ == "__main__":
    opt = optparse.OptionParser()
    opt.add_option("-i", "--input")
    opt.add_option("-t", "--text")
    opt.add_option("-w", "--wesnoth")
    opt.add_option("-d", "--defines")
    opt.add_option("-T", "--test", action = "store_true")
    options, args = opt.parse_args()

    if not options.input and not options.text and not options.test:
        sys.stderr.write("No input given. Use -h for help.\n")
        sys.exit(1)
    
    if not options.wesnoth or not os.path.exists(options.wesnoth):
        sys.stderr.write("Wesnoth executable not found.\n")
        sys.exit(1)
    
    if options.test:
        print("Running tests")
        p = Parser(options.wesnoth)
        
        only = None
        def test(input, expected, note):
            if only and note != only: return
            input = input.strip()
            expected = expected.strip()
            p.parse_text(input)
            output = p.root.debug().strip()
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
    code='\\n    "quotes" here\\n    ""blah""\\n'
[/test]
""", "quoted2")
        
        sys.exit(0)

    p = Parser(options.wesnoth)
    if options.input: p.parse_file(options.input, options.defines)
    elif options.text: p.parse_text(options.text, options.defines)
    print(p.root.debug())

