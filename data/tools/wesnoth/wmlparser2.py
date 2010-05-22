#!/usr/bin/env python
# encoding: utf8

"""
This module is the third WML parser I wrote from scratch. Unlike the
first two this one does not have a pre-processor because now we can
use the --preprocess option of wesnoth itself for it.

Advantages are:

- It's extremely fast (compared to previous versions).

- Translations work 100%, a single string can be made up of multiple
  non-translatable and translatable parts from different text domains.

- Results are always 100% identical to the builtin game parser.

The only disadvantage is:

- A wesnoth binary with a working --preprocess option must be present
  else only WML without preprocessor instructions can be parsed.

"""

import os, glob, sys, re, subprocess, optparse

class WMLError(Exception):
    """
    Catch this exception to retrieve the first error message from
    the parser.
    """
    def __init__(self, parser, message):
        self.line = parser.parser_line
        self.wml_line = parser.last_wml_line
        self.message = message

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
    
    def debug(self):
        s = ""
        s += "[" + self.name + "]\n"
        for sub in self.data:
            for subline in sub.debug().splitlines():
                s += "    " + subline + "\n"
        s += "[/" + self.name + "]\n"
        return s

class RootNode:
    """
    The root node. There is exactly one such node.
    """
    def __init__(self):
        self.name = None
        self.data = []

    def debug(self):
        s = ""
        for sub in self.data:
            for subline in sub.debug().splitlines():
                s += subline + "\n"
        return s

class Parser:
    def __init__(self, path, wesnoth_exe, defines):
        """
        path - Path to the file to parse.
        wesnoth_exe - Wesnoth executable to use. This should have been
            configured to use the desired data and config directories.
        """
        self.path = path
        self.wesnoth_exe = wesnoth_exe
        self.defines = defines
        self.preprocess = None
        
        self.get_datadir()
        self.get_userdir()

    def get_datadir(self):
        p = subprocess.Popen([self.wesnoth_exe, "--path"],
            stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = p.communicate()
        self.datadir = out
    
    def get_userdir(self):
        p = subprocess.Popen([self.wesnoth_exe, "--config-path"],
            stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = p.communicate()
        self.userdir = out
    
    def preprocess(self):
        """
        Call wesnoth --preprocess to get preprocessed WML which we
        can subsequently parse.
        
        If this is not called then the .parse method will assume the
        WML is already preprocessed.
        """
        output = "/tmp/wmlparser"
        if not os.path.exists(output): os.mkdir(output)
        p_option = "-p=" + self.defines if self.defines else "-p "
        p = subprocess.Popen([self.wesnoth_exe, p_option, self.path,
            output], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = p.communicate()
        self.preprocess = output + "/" + os.path.basename(self.path) +\
            ".plain"

    def parse_line_without_commands(self, line):
        """
        Once the .plain commands are handled WML lines are passed to
        this.
        """
        if not line: return
        quote = line.find('"')
        if quote >= 0:
            if self.in_string:
                self.temp_string += line[:quote]
                self.temp_string_node = StringNode(self.temp_string)
                if self.translatable:
                    self.temp_string_node.textdomain = self.textdomain
                    self.translatable = False
                self.temp_string = ""
                if not self.temp_key_node:
                    raise WMLError(self, "Unexpected string value.")
                self.temp_key_node.value.append(self.temp_string_node)
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
        if self.temp_key_node == None:
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
            self.parent_node[-1].data.append(node)
            self.parent_node.append(node)
        self.parse_outside_strings(line[end + 1:])

    def handle_attribute(self, line):
        assign = line.find("=")
        if assign >= 0:
            self.temp_key_node = AttributeNode(line[:assign])
            self.parent_node[-1].data.append(self.temp_key_node)
            self.parse_outside_strings(line[assign + 1:])
        else:
            self.temp_key_node = AttributeNode(line)
            self.parent_node[-1].data.append(self.temp_key_node)

    def handle_value(self, segment, is_first):
        # Finish assignment on newline, except if there is a
        # plus sign before the newline.
        if segment[-1] == "\n":
            segment = segment.rstrip()
            if segment:
                self.temp_string += segment
                self.temp_string_node = StringNode(self.temp_string)
                self.temp_string = ""
                self.temp_key_node.value.append(self.temp_string_node)
                self.temp_key_node = None
            elif is_first:
                self.temp_key_node = None
        else:
            self.temp_string += segment
            self.temp_string_node = StringNode(self.temp_string)
            self.temp_string = ""
            self.temp_key_node.value.append(self.temp_string_node)

    def parse(self):
        """
        Parse preprocessed WML into a tree of tags and attributes.
        """
        
        # parsing state
        self.temp_string = ""
        self.temp_string_node = None
        self.temp_key_node = None
        self.in_string = False
        self.last_wml_line = "?"
        self.parser_line = 0
        self.textdomain = "wesnoth"
        self.translatable = False
        self.parent_node = [RootNode()]

        command_marker_byte = chr(254)
        
        input = self.preprocess
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
        
        print(self.parent_node[0].debug())
                    
    def handle_command(self, com):
        if com.startswith("line "):
            self.last_wml_line = com[5:]
        elif com.startswith("textdomain "):
            self.textdomain = com[11:]
        else:
            raise WMLError(self, "Unknown parser command: " + com);

if __name__ == "__main__":
    opt = optparse.OptionParser()
    opt.add_option("-i", "--input")
    opt.add_option("-w", "--wesnoth")
    opt.add_option("-d", "--defines")
    options, args = opt.parse_args()
    
    if not options.input:
        sys.stderr.write("No input given.\n")
        sys.exit(1)
    
    if not options.wesnoth or not os.path.exists(options.wesnoth):
        sys.stderr.write("Wesnoth executable not found.\n")
        sys.exit(1)

    p = Parser(options.input, options.wesnoth, options.defines)
    p.preprocess()

    try:
        p.parse()
    except WMLError as e:
        print("WMLError:")
        print("    " + str(e.line) + " " + p.preprocess)
        print("    " + e.wml_line)
        print("    " + e.message)

