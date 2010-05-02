#!/usr/bin/env python
# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
# $Id$ 

"""
   Copyright (C) 2007 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
"""   
"""
The wiki grabber is a tool to convert wiki comment formatting[1] into a text
page which can be used in the wiki.
The program has no runtime paremeters, all data is hardcoded in the __main__
section below.

[1] http://wesnoth.org/wiki/Wiki_grabber
"""
import operator, os, re, sys

if __name__ == "__main__":
    # contains all output generated:
    # - key filename 
    # - value node list
    #
    # every node is a list with 2 items 
    # - first the sorting order
    # - second the actual data
    file_map = {}

    # contains all macros:
    # - key macro name 
    # - value macro contents
    macro_map = {}

    # default directory to dump the output in with trailing /.
    output_directory = "/tmp/"

    # default directory to find the source files in, no trailing /.
    src_directory = "../src/gui"

    # current file being processed
    current_file = ""

    # current block being processed
    current_block = ""

    def reindent( data):
        """Converts the raw input to an easier to use format.
        
        Lines starting with 8 spaces are concatenated with the previous line.
        The start of line ' *' are removed and if another space exists it's also
        removed."""

        # concatenate 
        data = re.sub(r'\n \*(?: ){8,}', " ", data)

        # strip 
        data = re.sub(" \*(?: |)", "", data)

        return data

    def get_value(data, key):
        """Extracts data from a key value pair, a key must start at the start of a line."""

        key1 = "^" + key
        sep = "(?: )*=(?: )"
        value = "(.*)$"
        res = re.compile("^" + key + " *= *(.*)$", re.M).search(data)

        if(res != None):
            res = res.group(1)

        return res

    def process_header(data):
        """Processes the header."""

        page = get_value(data, "@page")
        order = get_value(data, "@order")
        if(order == None):
            order = 10000

        return [page, order]

    def debug_dump(data, res):
        """Show the data the regex retrieved from a match.
        
        data is the raw data the regex tried to match.
        res is the result of the regex.findall.
        """
        sys.stderr.write("data : " + data)
        for i in range(len(res)):
            for j in range(len(res[i])):
                 sys.stderr.write("Line " + str(i) + " match " + str(j) + " : " + res[i][j] + "\n")

    def format(data):
        """Formats the data for the wiki.
        
        @* -> \n* needed in a list.
        @- -> \n needed to add text after a list."""
        data = re.sub(r'@\*', "\n*", data)
        data = re.sub(r'@\-', "\n", data)
        return data

    def create_config_table(data):
        """Creates a table for data in a config table.

        A config table is a table with info about WML configuration key value pairs.
        """
    
        # matches a line like
        # x1 (f_unsigned = 0)             The x coordinate of the startpoint.
        # x1 (f_unsigned)                 The x coordinate of the startpoint.
        variable = "(?:[a-z]|[A-Z])(?:[a-z]|[A-Z]|[0-9]|_)*"
        regex = re.compile(" *(" + variable +  ") \((" + variable + ") *(?:(?:= *(.*?))|)\) +(.*)\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!key\n!type\n!default\n!description\n"
        for i in range(len(res)):
            result += "|-\n"
            result += "| " + res[i][0] + "\n"
            result += "| [[GUIVariable#" + res[i][1] + "|" + res[i][1] + "]]\n"
            if(res[i][2] == ""):
                result += "| mandatory\n"
            else:
                result += "| " + res[i][2] + "\n"
            result += "| " + format(res[i][3]) + "\n"
        result += "|}"

        return result

    def create_formula_table(data):
        """Creates a table for data in a formula table.

        A formula table is a table with info about which function parameters 
        are available for processing in WML formulas.
        """

        #matches a line like
        #width unsigned                  The width of the canvas.
        variable = "(?:[a-z]|[A-Z])(?:[a-z]|[A-Z]|[0-9]|_)*"
        regex = re.compile(" *(" + variable +  ") (" + variable + ") +(.*)\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!Variable\n!type\n!description\n"
        for i in range(len(res)):
            result += "|-\n"
            result += "| " + res[i][0] + "\n"
            result += "| " + res[i][1] + "\n"
            result += "| " + format(res[i][2]) + "\n"
        result += "|}"

        return result

    def create_variable_types_table(data):
        """Creates a table for the variable types."""

        #matches a line like
        # int                             Signed number (whole numbers).
        variable = "(?:[a-z]|[A-Z])(?:[a-z]|[A-Z]|[0-9]|_)*"
        regex = re.compile(" *(" + variable +  ") +(.*)\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!Variable\n!description\n"
        for i in range(len(res)):
            result += "|-\n"
            result += "| <span id=\"" + res[i][0] + "\">" + res[i][0] + "</span>\n"
            result += "| " + format(res[i][1]) + "\n"
        result += "|}"

        return result
    
    def create_widget_overview_table(data):
        """Creates a table for all available widgets."""
        #matches a line like
        # Button                        A push button.
        variable = "(?:[a-z]|[A-Z])(?:[a-z]|[A-Z]|[0-9]|_)*"
        regex = re.compile(" *(" + variable +  ") +(.*)\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!Section\n!Description\n"
        for i in range(len(res)):
            result += "|-\n"
            result += "| " + "<span id=\"" + res[i][0].lower() + "\">" 
            result += re.sub(r'_', ' ', res[i][0])
            result += "</span>"
            result += " ([[GUIWidgetDefinitionWML#" 
            result += res[i][0]
            result += "|definition]]"
            result += ", [[GUIWidgetInstanceWML#" 
            result += res[i][0]
            result += "|instantiation]])\n"
            result += "| " + format(res[i][1]) + "\n"
        result += "|}"

        return result

    def create_window_overview_table(data):
        """Creates a table for all available windows."""
        #matches a line like
        # Addon_connect                 The dialog to connect to the addon server
        variable = "(?:[a-z]|[A-Z])(?:[a-z]|[A-Z]|[0-9]|_)*"
        regex = re.compile(" *(" + variable +  ") +(.*)\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!Section\n!Description\n"
        for i in range(len(res)):
            result += "|-\n"
            result += "| " + re.sub(r'_', ' ', res[i][0])
            result += " ([[GUIWindowDefinitionWML#" 
            result += res[i][0]
            result += "|definition]])\n"
            result += "| " + format(res[i][1]) + "\n"
        result += "|}"

        return result

    def create_container_grid(data):
        """Creates a table for a grid."""
        # matches a line like:
        # indention_marker id widget_list ret_val description
        #
        # idention_marker = -* every - sign means a level deeper, these levels
        # are to convey ownership rules eg to mark that certain widgets are in a
        # listbox, this makes it clear that the elements displayed are for every
        # row in the listbox. Can't think of a reason to have multiple levels
        # but hey let's prepare this silly parser.
        #
        # id is the id of the widget. If the widget is mandatory it's between
        # parens else between square brackets. The id might be empty in that
        # case only the brackets/parens are needed.
        #
        # widget_list is a comma separated list of the widgets allowed for this
        # item. The value control means every non container widget. The value
        # container means every container widget. The value all means all
        # widgets. The value is between parens.
        #
        # ret_val is the return value of the widget, either a number, a named
        # value, or nothing. The value is between parens.
        #
        # Description is the description for the control, this is all the text
        # after the ret_val field.
        idention = r"(-*)"
        id = r"(?:(\[)|\()(.*)(?(2)\]|\))"
        widget_list = r"\((.+)\)"
        ret_val = r"\((.*)\)"
        description = r"(.+)"

        regex = re.compile(" *" + idention + " *" + id + " *" + widget_list + " * " + ret_val + " *" + description + "\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!ID (return value)\n!Type\n!Mandatory\n!Description\n"
        for i in range(len(res)):
            result += "|-\n| " + "&nbsp;" * len(res[i][0]) * 8 + res[i][2] + " "

            if(res[i][4] == ""):
                result += "\n"
            else:
                result += "(" + res[i][4] + ")\n"

            result += "| " + "[[GUIToolkitWML#" + res[i][3] + "|" + res[i][3] + "]]\n"

            if(res[i][1] == ""):
                result += "|yes\n"
            else:
                result += "|no\n"

            result += "| " + re.sub(r'@\*', "\n*", res[i][5]) + "\n"
        result += "|}"

        return result

    def create_container_table(data):
        """Creates a table for a container."""
        print "The container table is deprecated, use the grid instead.\n"

        #matches a line like
        # name [widget_list] ret_val description
        # or
        # name (widget_list) ret_val description
        # The square bracket version is an optional widget, the round version is a mandatory widget.
        # The retval is optional.


        # A widget_list looks like:
        # widget ([ widget)*

        # A widget looks like
        # widget_name | {widget_class}

        # the widgets links to the widget instance location.
        # the {widget_class} links to the list with know widget classes.

        # FIXME not everything is implemented yet.

        # NOTE unlike other variables are these allowed to start with an underscore.
        variable = "(?:[a-z]|[A-Z]|[0-9]|_)+|"
        widget_type = " \((.*?)\)"
        retval = " ?(?:\((.*)\)|)"
        
        regex = re.compile(" *(\[)?(" +variable + ")(?(1)])" + widget_type + retval +" *(.*)\n")
        res = regex.findall(data)

        # empty table
        if(len(res) == 0):
            sys.stderr.write("Empty table:\n" + data + "\n")
            return "Empty table."

        result = '{| border="1"'
        result += "\n!ID (return value)\n!Type\n!Mandatory\n!Description\n"
        for i in range(len(res)):
            result += "|-\n"
            if(res[i][1] == ""):
                result += "|"
            else:
                result += "| " + res[i][1] + " "

            if(res[i][3] == ""):
                result += "\n"
            else:
                result += "(" + res[i][3] + ")\n"

            result += "| " + res[i][2] + "\n"

            if(res[i][0] == ""):
                result += "|no\n"
            else:
                result += "|yes\n"

            result += "| " + re.sub(r'@\*', "\n*", res[i][4]) + "\n"
        result += "|}"

        return result

    def create_table(table) :
        """Wrapper for creating tables."""

        type = table.group(1)
        if(type == "config"):
            return create_config_table(table.group(2) + "\n")
        elif(type == "formula"):
            return create_formula_table(table.group(2) + "\n")
        elif(type == "variable_types"):
            return create_variable_types_table(table.group(2) + "\n")
        elif(type == "widget_overview"):
            return create_widget_overview_table(table.group(2) + "\n")
        elif(type == "window_overview"):
            return create_window_overview_table(table.group(2) + "\n")
        elif(type == "grid"):
            return create_container_grid(table.group(2) + "\n")
        elif(type == "container"):
            return create_container_table(table.group(2) + "\n")
        else:
            sys.stderr.write("Unknown table definition '" + type + "'.\n")
            return "Unknown table definition '" + type + "'."

    def process_body(data):
        """Process the body.
        
        The body needs to be stripped of known markup values.
        """

        table_regex = re.compile("^@start_table *= *(.*?)\n(.*?)\n@end_table.*?$", re.M | re.S)
        data = table_regex.sub(lambda match: create_table(match), data)
        return data

    def process(data):
        """Processes a wiki block."""

        data = replace_macros(data)

        res = re.compile("(.*?)\n\n(.*)", re.S).findall(data);
        if(res != None and len(res) != 0):
            header = process_header(res[0][0])
            body = process_body(res[0][1])
        else:
            print "File: "  + current_file
            print "Block:\n"  + current_block
            print "\n\nInvalid wiki block, discarded."
            return

        if(header[0] == None):
            print "File: "  + current_file
            print "Block:\n"  + current_block
            print "\n\nNo page defined, dropped."
            return

        if(file_map.has_key(header[0]) == False):
            file_map[header[0]] = []

        file_map[header[0]].append([header[1], body])

    def create_output():
        """Generates the output"""

        for file, data_list in file_map.iteritems():
            data_list.sort(key=operator.itemgetter(0))
            fd = open(output_directory + file, "w")
            for i in range(len(data_list)):
                fd.write(data_list[i][1])
            fd.close()
    
    def process_file(name):
        """Processes all wiki blocks (if any) of a file."""

        global current_file
        current_file = name
        file = open(name, "r")
        data = file.read()
        file.close()

        regex = re.compile("(/\*WIKI($.*?)^ \*/)", re.M | re.S)
        res = regex.findall(data)
        if(res != None):
            for i in range(len(res)):
                global current_block
                current_block = res[i][0]
                section = reindent(res[i][1])
                wiki_info = process(section)

    def process_directory(dir):
        """Processes a directory.

        Recurses into the sub directory, ignoring hidden directories.
        Processes every .[c|h]pp file.
        """

        items = os.listdir(dir)

        for item in items:

            # Ignore hidden directories.
            if(item.startswith(".")):
                continue

            if(os.path.isdir(dir + "/" + item)):
                process_directory(dir + "/" + item)
            elif(item.endswith(".cpp") or item.endswith(".hpp")):
                process_file(dir + "/" + item)

    ##### ##### ##### MACRO PROCESSING ##### ##### #####

    def instanciate_macro(macro):
        """Replaces a macro text by the marco value."""

        global macro_map

        if(macro_map.has_key(macro.group(1)) == False):
            print "Macro '" + macro.group(1) + "' is not defined."
            return macro.group(0)

        return macro_map[macro.group(1)]


    def replace_macros(data):
        """Replaces all macros found in the data.

        If a macro isn't defined, the text is left as is."""

        macro_regex = re.compile("@macro *= *(\w+)", re.M | re.S)
        data = macro_regex.sub(lambda match: instanciate_macro(match), data)
        return data

    def create_macro(macro):
        """Wrapper for creating macros."""

        global macro_map

        if(macro_map.has_key(macro.group(1))):
            print "Macro '" + macro.group(1) + "' is being redefined."

        macro_map[macro.group(1)] = macro.group(2)

    def process_file_macros(name):
        """Processes all wiki macro blocks (if any) of a file."""

        global current_file
        current_file = name
        file = open(name, "r")
        data = file.read()
        file.close()

        regex = re.compile("(/\*WIKI_MACRO($.*?)^ \*/)", re.M | re.S)
        res = regex.findall(data)
        if(res != None):
            for i in range(len(res)):
                global current_block
                current_block = res[i][0]
                section = reindent(res[i][1])
                macro_regex = re.compile("^@start_macro *= *(.*?)\n(.*?)\n@end_macro.*?$", re.M | re.S)
                macro_regex.sub(lambda match: create_macro(match), section)


    def process_directory_macros(dir):
        """Processes a directory looking for macros.

        Recurses into the sub directory, ignoring hidden directories.
        Processes every .[c|h]pp file.
        """

        items = os.listdir(dir)

        for item in items:

            # Ignore hidden directories.
            if(item.startswith(".")):
                continue

            if(os.path.isdir(dir + "/" + item)):
                process_directory_macros(dir + "/" + item)
            elif(item.endswith(".cpp") or item.endswith(".hpp")):
                process_file_macros(dir + "/" + item)

    ##### ##### ##### MAIN ##### ##### #####


    process_directory_macros(src_directory)
    process_directory(src_directory)

    create_output()

