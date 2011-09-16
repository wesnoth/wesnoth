
# Assorted Pretty printers for wesnoth data structures in gdb 

import gdb
import re
import itertools

from wesnoth_type_tools import strip_type

#Printer for n_interned::t_interned 
class T_InternedPrinter(object) :
    """Print a t_interned_token<T>"""
    def __init__(self, val) :
        self.val = val

    def to_string(self) :
        #Returns either a string or an other convertible to string
        return self.val['iter_']['first']
    
    def display_hint(self) :
        #one of 'string' 'array' 'map'
        return 'string'

#Printer for n_token::t_token 
class T_TokenPrinter(object) :
    """Print a t_token"""
    def __init__(self, val) :
        self.val = val

    def to_string(self) :
        # Get the type.
        type = self.val.type
    
        # Get the type name.    
        type = strip_type(self.val)

        #Return the underlying base class
        baseclass = type.fields()[0].type
       
        return self.val.cast(baseclass)
    
    def display_hint(self) :
        #one of 'string' 'array' 'map'
        return 'string'

class TstringPrinter(object) :
    """Print a t_string"""
    def __init__(self, val) :
        self.val = val

    def to_string(self) :
        # Get the type.
        type = self.val.type
    
        # Get the type name.    
        type = strip_type(self.val)

        #Return the underlying base class
        baseclass = type.fields()[0].type
       
        shared = self.val.cast(baseclass)['val_']
        if shared == 0 :
            return 'NULL'
        return shared.dereference()['val']['value_']
    
    def display_hint(self) :
        #one of 'string' 'array' 'map'
        return 'string'

class AttributeValuePrinter(object) :
    """Print an attribute_value"""
    def __init__(self, val) :
        self.val = val

    def to_string(self) :
        # Get the type.
        type = self.val.type
    
        # Get the type name.    
        type = strip_type(self.val)
       
        attr = self.val.cast(type)
        attr_type = attr['type_']
       
        if attr_type == 0:
            return ""
        elif attr_type == 1 :
            return 'true' if attr['bool_value_'] else 'false'
        elif attr_type == 2 :
            return 'int ' + ('%s' % attr['int_value_'])
        elif attr_type == 3 :
            return 'double ' + ('%s' % attr['double_value_'])
        elif attr_type == 4 :
            return 'token ' + ('%s' % attr['token_value_'])
        else :
            return 't_string ' + ('%s' % attr['t_string_value_'])
        
        return "attribute pretty printer found an unknown type"
    
    def display_hint(self) :
        #one of 'string' 'array' 'map'
        return 'string'


#This will be brittle as it depends on the underlying boost implementation, but its better than nothing
#With much help from http://stackoverflow.com/questions/2804641/pretty-printing-boostunordered-map-on-gdb
class BoostUnorderedMapPrinter(object) :
    """ Print a boost unordered map"""

    class _iterator:
        def __init__ (self, val):
            self.buckets = val['table_']['buckets_']
            if self.buckets != 0:
                t_key = val.type.template_argument(0)
                t_val = val.type.template_argument(1)
                self.bucket_count = val['table_']['bucket_count_']
                self.current_bucket = 0
                pair = "std::pair<%s const, %s>" % (t_key, t_val)
                self.pair_pointer = gdb.lookup_type(pair).pointer()
                self.base_pointer = gdb.lookup_type("boost::unordered_detail::value_base< %s >" % pair).pointer()
                self.node_pointer = gdb.lookup_type("boost::unordered_detail::hash_node<std::allocator< %s >, boost::unordered_detail::ungrouped>" % pair).pointer()
                self.node = self.buckets[self.current_bucket]['next_']

        def __iter__(self):
            return self

        def next(self):
            if self.buckets == 0 :
                raise StopIteration
            while not self.node:
                self.current_bucket = self.current_bucket + 1
                if self.current_bucket >= self.bucket_count:
                    raise StopIteration
                self.node = self.buckets[self.current_bucket]['next_']

            iterator = self.node.cast(self.node_pointer).cast(self.base_pointer).cast(self.pair_pointer).dereference()   
            self.node = self.node['next_']

            return ('%s' % iterator['first']), iterator['second']

    def __init__(self, val):
        self.val = val
        self.buckets = val['table_']['buckets_']

    def children(self):
        return self._iterator(self.val)

    def to_string(self):
        ret = "boost::unordered_map"
        if self.buckets == 0 :
            ret += " EMPTY"
        return ret

    def display_hint(self):
        return 'string'


class BoostUnorderedMapIteratorPrinter(object):
    def __init__ (self, val):
        pair = val.type.template_argument(0).template_argument(0)
        t_key = pair.template_argument(0)
        t_val = pair.template_argument(1)
        self.node = val['base_']['node_']
        self.pair_pointer = pair.pointer()
        self.base_pointer = gdb.lookup_type("boost::unordered_detail::value_base< %s >" % pair).pointer()
        self.node_pointer = gdb.lookup_type("boost::unordered_detail::hash_node<std::allocator< %s >, boost::unordered_detail::ungrouped>" % pair).pointer()
        self.bucket_pointer = gdb.lookup_type("boost::unordered_detail::hash_bucket<std::allocator< %s > >" % pair).pointer()

    def to_string(self):
        if not self.node:
            return 'NULL'
        iterator = self.node.cast(self.node_pointer).cast(self.base_pointer).cast(self.pair_pointer).dereference()   
        return iterator['second']

    def display_hint(self):
        return 'string'
    

class ConfigPrinter(object) :
    """Print a config"""
    def __init__(self, val) :
        self.val = val

    def to_string(self) :
        return "config"
    def children(self) :
        #yield "invalid",  self.val['invalid']
        yield "values", self.val['values']
        yield "children", self.val['children']
        yield "ordered_children", self.val['ordered_children']
           
    def display_hint(self) :
        #one of 'string' 'array' 'map'
        return 'string'



# register the pretty-printers
def add_printers(pretty_printers_dict) :
    pretty_printers_dict[re.compile ('^n_interned::t_interned_token.*$')] = T_InternedPrinter
    pretty_printers_dict[re.compile ('^n_token::t_token$')] = T_TokenPrinter
    pretty_printers_dict[re.compile ('^t_string$')] = TstringPrinter
    pretty_printers_dict[re.compile ('^config::attribute_value$')] = AttributeValuePrinter
    pretty_printers_dict[re.compile ('^boost::unordered_map.*$')] = BoostUnorderedMapPrinter
    pretty_printers_dict[re.compile ('^boost::unordered_detail::hash_iterator\<std::allocator\<std::pair\<.*$')] =  BoostUnorderedMapIteratorPrinter
    pretty_printers_dict[re.compile ('^config$')] = ConfigPrinter
    
    return pretty_printers_dict

