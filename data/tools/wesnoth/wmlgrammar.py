"""
wmlgrammar -- parses a given schema into a more usable form
"""
import re

REQUIRED = 1
OPTIONAL = 2
REPEATED = 3
FORBIDDEN = 4

class Grammar(object):
    def __init__(self, schema):
        schema = schema.get_first("schema")
        self.datatypes = {
            "boolean" : re.compile("^(yes|no|true|false|on|off)$"),
            #"character" : re.compile("^.$"),
            "float" : re.compile("^(\\+|-)?[0-9]+(\.[0-9]*)?$"),
            "integer" : re.compile("^(\\+|-)?[0-9]+$"),
            "string" : re.compile(".*"),
            "tstring" : re.compile(".*"),
        }
        self.elements = {}
        self.categories = {}
        for type in schema.get_all_text():
            match = parse_match(type.data)
            self.datatypes.update( { type.name : match } )
        for element in schema.get_all_subs():
            node = Node(element, self.datatypes)
            self.elements.update( { node.name : node } )
        for element in [el for el in self.elements.values() if el.parent]:
            element.inherit(self.elements[element.parent])
        # categories
        for element in [el for el in self.elements.values() if el.category]:
            if element.category not in self.categories:
                self.categories[element.category] = []
            self.categories[element.category].append( element )

    def get_element(self, name):
        return self.elements[name]

    def get_datatype(self, name):
        return self.datatypes[name]

    def get_category(self, name):
        if name not in self.categories: return []
        return self.categories[name]

class Node(object):
    def __init__(self, schema, datatypes):
        self.name = schema.name
        self.elements = set([])
        self.ext_elements = [] #Ugh, do we really want to do this?
        self.attributes = set([])
        self.parent = None
        self.description = None
        self.category = None
        for item in schema.get_all_text():
            if item.name[0] == '_':
                self.elements.add( Element(item) )
            else:
                self.attributes.add( Attribute(item, datatypes) )
        for item in schema.get_all_subs():
            if item.name == "element":
                print "[element] found in schema, not parsing yet"
                #self.ext_elements...
            elif item.name == "description":
                self.description = item.get_text("text")
                self.category = item.get_text("category")
            else:
                raise Exception( "Unknown element [%s] encountered in grammar for [%s]" % (item.name, self.name) )
        if ':' in self.name:
            self.name, self.parent = self.name.split(':',1)
    def inherit(self, other):
        assert self.parent == other.name
        self.elements.update( other.elements )
        self.attributes.update( other.attributes )
        self.parent = None
    def get_attributes(self):
        return self.attributes
    def get_elements(self):
        return self.elements


class Element(object):
    def __init__(self, schema):
        first, second = schema.data.split(" ",1)
        self.name = schema.name[1:]
        self.freq = parse_frequency(first)
        self.subname = second
    def match(self, name):
        return self.name == name
    def __hash__(self):
        return hash(self.name)
    def __cmp__(self, other):
        return (isinstance(other, type(self)) or isinstance(self, type(other))) and cmp(self.name, other.name)

class ExtElement(Element):
    def __init__(self, schema):
        self.re = parse_match( schema.get_text("match").data )
        self.freq = parse_frequency( schema.get_text("freq").data )
        self.subname = schema.get_text("name").data
    def match(self, name):
        return bool(self.re.match(name))

class Attribute(object):
    def __init__(self, schema, datatypes):
        first, second = schema.data.split(" ",1)
        if not second in datatypes:
            raise Exception( "Unknown datatype '%s'" % (second,) )
        self.name = schema.name
        self.freq = parse_frequency(first)
        self.type = second
        self.re = datatypes[second]
    def match(self, name):
        return self.name == name
    def validate(self, value):
        return bool(self.re.match(value))
    def __hash__(self):
        return hash(self.name)
    def __cmp__(self, other):
        return (isinstance(other, type(self)) or isinstance(self, type(other))) and cmp(self.name, other.name)

def parse_frequency(string):
    if string == "required":
        return REQUIRED
    elif string == "optional":
        return OPTIONAL
    elif string == "repeated":
        return REPEATED
    elif string == "forbidden":
        return FORBIDDEN
    else:
        raise Exception( "Unknown frequency '%s'" % (string,) )

def parse_match(string):
    (matchtype, matchtext) = string.split(" ",1)
    if matchtype == "re":
        match = re.compile(matchtext)
    elif matchtype == "enum":
        match = re.compile( "^(" + matchtext.replace(',','|') + ")$" )
    else:
        raise Exception( "Unknown datatype encountered in %s=\"%s\": '%s'" % (type.name, type.data, matchtype) )
    return match
# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
