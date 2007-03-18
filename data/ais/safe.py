"""An attempt at creating a safe_exec for python.

This file is public domain and is not suited for any serious purpose.
This code is not guaranteed to work. Use at your own risk!
Beware!  Trust no one!

Please e-mail philhassey@yahoo.com if you find any security holes.

Known limitations:
    - Safe doesn't have any testing for timeouts/DoS.  One-liners
        like these will lock up the system: "while 1: pass", "234234**234234"
    - Lots of (likely) safe builtins and safe AST Nodes are not allowed.
        I suppose you can add them to the whitelist if you want them.  I
        trimmed it down as much as I thought I could get away with and still
        have useful python code.
    - Might not work with future versions of python - this is made with
        python 2.4 in mind.  _STR_NOT_BEGIN might have to be extended
        in the future with more magic variable prefixes.  Or you can
        switch to conservative mode, but then even variables like "my_var" 
        won't work, which is sort of a nuisance.
    - If you get data back from a safe_exec, don't call any functions
        or methods - they might not be safe with __builtin__ restored
        to its normal state.  Work with them again via an additional safe_exec.
    - The "context" sent to the functions is not tested at all.  If you 
        pass in a dangerous function {'myfile':file} the code will be able
        to call it.
"""

# Built-in Objects
# http://docs.python.org/lib/builtin.html

# AST Nodes - compiler
# http://docs.python.org/lib/module-compiler.ast.html

# Types and members - inspection
# http://docs.python.org/lib/inspect-types.html
# The standard type heirarchy
# http://docs.python.org/ref/types.html

# Based loosely on - Restricted "safe" eval - by Babar K. Zafar
# (it isn't very safe, but it got me started)
# http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/496746

# Securing Python: Controlling the abilities of the interpreter
# (or - why even trying this is likely to end in tears)
# http://us.pycon.org/common/talkdata/PyCon2007/062/PyCon_2007.pdf

# Changes
# 2007-03-13: added test for unicode strings that contain __, etc
# 2007-03-09: renamed safe_eval to safe_exec, since that's what it is.
# 2007-03-09: use "exec code in context" , because of test_misc_recursive_fnc
# 2007-03-09: Removed 'type' from _BUILTIN_OK - see test_misc_type_escape
# 2007-03-08: Cleaned up the destroy / restore mechanism, added more tests
# 2007-03-08: Fixed how contexts work.
# 2007-03-07: Added test for global node
# 2007-03-07: Added test for SyntaxError
# 2007-03-07: Fixed an issue where the context wasn't being reset (added test) 
# 2007-03-07: Added unittest for dir()
# 2007-03-07: Removed 'isinstance', 'issubclass' from builtins whitelist
# 2007-03-07: Removed 'EmptyNode', 'Global' from AST whitelist
# 2007-03-07: Added import __builtin__; s/__builtins__/__builtin__ 

import compiler
import __builtin__

class SafeException(Exception):
    """Base class for Safe Exceptions"""
    def __init__(self,*value):
        self.value = str(value)
    def __str__(self):
        return self.value
class CheckNodeException(SafeException):
    """AST Node class is not in the whitelist."""
    pass
class CheckStrException(SafeException):
    """A string in the AST looks insecure."""
    pass
class RunBuiltinException(SafeException):
    """During the run a non-whitelisted builtin was called."""
    pass

_NODE_CLASS_OK = [
    'Add', 'And', 'AssAttr', 'AssList', 'AssName', 'AssTuple',
    'Assert', 'Assign','AugAssign', 'Bitand', 'Bitor', 'Bitxor', 'Break',
    'CallFunc', 'Class', 'Compare', 'Const', 'Continue',
    'Dict', 'Discard', 'Div', 'Ellipsis', 'Expression', 'FloorDiv',
    'For', 'Function', 'Getattr', 'If', 'Keyword',
    'LeftShift', 'List', 'ListComp', 'ListCompFor', 'ListCompIf', 'Mod',
    'Module', 'Mul', 'Name', 'Node', 'Not', 'Or', 'Pass', 'Power',
    'Print', 'Printnl', 'Return', 'RightShift', 'Slice', 'Sliceobj',
    'Stmt', 'Sub', 'Subscript', 'Tuple', 'UnaryAdd', 'UnarySub', 'While',
    ]
_NODE_ATTR_OK = []
_STR_OK = ['__init__']
_STR_NOT_CONTAIN = ['__']
_STR_NOT_BEGIN = ['im_','func_','tb_','f_','co_',]

## conservative settings
#_NODE_ATTR_OK = ['flags']
#_STR_NOT_CONTAIN = ['_']
#_STR_NOT_BEGIN = []

def _check_node(node):
    if node.__class__.__name__ not in _NODE_CLASS_OK:
        raise CheckNodeException(node.lineno,node.__class__.__name__)
    for k,v in node.__dict__.items():
        if k in _NODE_ATTR_OK: continue
        if v in _STR_OK: continue
        if type(v) not in [str,unicode]: continue
        for s in _STR_NOT_CONTAIN:
            if s in v: raise CheckStrException(node.lineno,k,v)
        for s in _STR_NOT_BEGIN:
            if v[:len(s)] == s: raise CheckStrException(node.lineno,k,v)
    for child in node.getChildNodes():
        _check_node(child)

def _check_ast(code):
    ast = compiler.parse(code)
    _check_node(ast)

# r = [v for v in dir(__builtin__) if v[0] != '_' and v[0] == v[0].upper()] ; r.sort() ; print r
_BUILTIN_OK = [
    '__debug__','quit','exit',
    
    'ArithmeticError', 'AssertionError', 'AttributeError', 'DeprecationWarning', 'EOFError', 'Ellipsis', 'EnvironmentError', 'Exception', 'False', 'FloatingPointError', 'FutureWarning', 'IOError', 'ImportError', 'IndentationError', 'IndexError', 'KeyError', 'KeyboardInterrupt', 'LookupError', 'MemoryError', 'NameError', 'None', 'NotImplemented', 'NotImplementedError', 'OSError', 'OverflowError', 'OverflowWarning', 'PendingDeprecationWarning', 'ReferenceError', 'RuntimeError', 'RuntimeWarning', 'StandardError', 'StopIteration', 'SyntaxError', 'SyntaxWarning', 'SystemError', 'SystemExit', 'TabError', 'True', 'TypeError', 'UnboundLocalError', 'UnicodeDecodeError', 'UnicodeEncodeError', 'UnicodeError', 'UnicodeTranslateError', 'UserWarning', 'ValueError', 'Warning', 'ZeroDivisionError',
    
    'abs', 'bool', 'cmp', 'complex', 'dict', 'divmod', 'filter', 'float', 'frozenset', 'hex', 'int', 'len', 'list', 'long', 'map', 'max', 'min', 'object', 'oct', 'pow', 'range', 'reduce', 'repr', 'round', 'set', 'slice', 'str', 'sum', 'tuple',  'xrange', 'zip',
    'isinstance', 'issubclass']
    
#this is zope's list...
    #in ['False', 'None', 'True', 'abs', 'basestring', 'bool', 'callable',
             #'chr', 'cmp', 'complex', 'divmod', 'float', 'hash',
             #'hex', 'id', 'int', 'isinstance', 'issubclass', 'len',
             #'long', 'oct', 'ord', 'pow', 'range', 'repr', 'round',
             #'str', 'tuple', 'unichr', 'unicode', 'xrange', 'zip']:
    
    
_BUILTIN_STR = [
    'copyright','credits','license','__name__','__doc__',
    ]

def _builtin_fnc(k):
    def fnc(*vargs,**kargs):
        raise RunBuiltinException(k)
    return fnc
_builtin_globals = None
_builtin_globals_r = None
def _builtin_init():
    global _builtin_globals, _builtin_globals_r
    if _builtin_globals != None: return
    _builtin_globals_r = __builtin__.__dict__.copy()
    r = _builtin_globals = {}
    for k in __builtin__.__dict__.keys():
        v = None
        if k in _BUILTIN_OK: v = __builtin__.__dict__[k]
        elif k in _BUILTIN_STR: v = ''
        else: v = _builtin_fnc(k)
        r[k] = v
def _builtin_destroy():
    _builtin_init()
    for k,v in _builtin_globals.items():
        __builtin__.__dict__[k] = v
def _builtin_restore():
    for k,v in _builtin_globals_r.items():
        __builtin__.__dict__[k] = v
    
def safe_check(code):
    """Check the code to be safe."""
    return _check_ast(code)

def safe_run(code,context=None):
    """Exec code with only safe builtins on."""
    if context == None: context = {}
    
    _builtin_destroy()
    try:
        #exec code in _builtin_globals,context
        context['__builtins__'] = _builtin_globals
        exec code in context
        _builtin_restore()
    except:
        _builtin_restore()
        raise

def safe_exec(code,context = None):
    """Check the code to be safe, then run it with only safe builtins on."""
    safe_check(code)
    safe_run(code,context)
    
if __name__ == '__main__':
    import unittest
    
    class TestSafe(unittest.TestCase):
        def test_check_node_import(self):
            self.assertRaises(CheckNodeException,safe_exec,"import os")
        def test_check_node_from(self):
            self.assertRaises(CheckNodeException,safe_exec,"from os import *")
        def test_check_node_exec(self):
            self.assertRaises(CheckNodeException,safe_exec,"exec 'None'")
        def test_check_node_raise(self):
            self.assertRaises(CheckNodeException,safe_exec,"raise Exception")
        def test_check_node_global(self):
            self.assertRaises(CheckNodeException,safe_exec,"global abs")
        
        def test_check_str_x(self):
            self.assertRaises(CheckStrException,safe_exec,"x__ = 1")
        def test_check_str_str(self):
            self.assertRaises(CheckStrException,safe_exec,"x = '__'")
        def test_check_str_class(self):
            self.assertRaises(CheckStrException,safe_exec,"None.__class__")
        def test_check_str_func_globals(self):
            self.assertRaises(CheckStrException,safe_exec,"def x(): pass; x.func_globals")
        def test_check_str_init(self):
            safe_exec("def __init__(self): pass")
        def test_check_str_subclasses(self):
            self.assertRaises(CheckStrException,safe_exec,"object.__subclasses__")
        def test_check_str_properties(self):
            code = """
class X(object):
    def __get__(self,k,t=None):
        1/0
"""
            self.assertRaises(CheckStrException,safe_exec,code)
        def test_check_str_unicode(self):
            self.assertRaises(CheckStrException,safe_exec,"u'__'")
        
        def test_run_builtin_open(self):
            self.assertRaises(RunBuiltinException,safe_exec,"open('test.txt','w')")
        def test_run_builtin_getattr(self):
            self.assertRaises(RunBuiltinException,safe_exec,"getattr(None,'x')")
        def test_run_builtin_abs(self):
            safe_exec("abs(-1)")
        def test_run_builtin_open_fnc(self):
            def test():
                f = open('test.txt','w')
            self.assertRaises(RunBuiltinException,safe_exec,"test()",{'test':test})
        def test_run_builtin_open_context(self):
            #this demonstrates how python jumps into some mystical
            #restricted mode at this point .. causing this to throw
            #an IOError.  a bit strange, if you ask me.
            self.assertRaises(IOError,safe_exec,"test('test.txt','w')",{'test':open})
        def test_run_builtin_type_context(self):
            #however, even though this is also a very dangerous function
            #python's mystical restricted mode doesn't throw anything.
            safe_exec("test(1)",{'test':type})
        def test_run_builtin_dir(self):
            self.assertRaises(RunBuiltinException,safe_exec,"dir(None)")
        
        def test_run_exeception_div(self):
            self.assertRaises(ZeroDivisionError,safe_exec,"1/0")
        def test_run_exeception_i(self):
            self.assertRaises(ValueError,safe_exec,"(-1)**0.5")
        
        def test_misc_callback(self):
            self.value = None
            def test(): self.value = 1
            safe_exec("test()", {'test':test})
            self.assertEqual(self.value, 1)
        def test_misc_safe(self):
            self.value = None
            def test(v): self.value = v
            code = """
class Test:
    def __init__(self,value):
        self.x = value
        self.y = 4
    def run(self):
        for n in xrange(0,34):
            self.x += n
            self.y *= n
        return self.x+self.y
b = Test(value)
r = b.run()
test(r)
"""
            safe_exec(code,{'value':3,'test':test})
            self.assertEqual(self.value, 564)
            
        def test_misc_context_reset(self):
            #test that local contact is reset
            safe_exec("abs = None")
            safe_exec("abs(-1)")
            safe_run("abs = None")
            safe_run("abs(-1)")
            
        def test_misc_syntax_error(self):
            self.assertRaises(SyntaxError,safe_exec,"/")
            
        def test_misc_context_switch(self):
            self.value = None
            def test(v): self.value = v
            safe_exec("""
def test2():
    open('test.txt','w')
test(test2)
""",{'test':test})
            self.assertRaises(RunBuiltinException,safe_exec,"test()",{'test':self.value})
        
        def test_misc_context_junk(self):
            #test that stuff isn't being added into *my* context
            #except what i want in it..
            c = {}
            safe_exec("b=1",c)
            self.assertEqual(c['b'],1)
            
        def test_misc_context_later(self):
            #honestly, i'd rec that people don't do this, but
            #at least we've got it covered ...
            c = {}
            safe_exec("def test(): open('test.txt','w')",c)
            self.assertRaises(RunBuiltinException,c['test'])
        
        #def test_misc_test(self):
            #code = "".join(open('test.py').readlines())
            #safe_check(code)
            
        def test_misc_builtin_globals_write(self):
            #check that a user can't modify the special _builtin_globals stuff
            safe_exec("abs = None")
            self.assertNotEqual(_builtin_globals['abs'],None)
            
        #def test_misc_builtin_globals_used(self):
            ##check that the same builtin globals are always used
            #c1,c2 = {},{}
            #safe_exec("def test(): pass",c1)
            #safe_exec("def test(): pass",c2)
            #self.assertEqual(c1['test'].func_globals,c2['test'].func_globals)
            #self.assertEqual(c1['test'].func_globals,_builtin_globals)
        
        def test_misc_builtin_globals_used(self):
            #check that the same builtin globals are always used
            c = {}
            safe_exec("def test1(): pass",c)
            safe_exec("def test2(): pass",c)
            self.assertEqual(c['test1'].func_globals,c['test2'].func_globals)
            self.assertEqual(c['test1'].func_globals['__builtins__'],_builtin_globals)
            self.assertEqual(c['__builtins__'],_builtin_globals)
            
        def test_misc_type_escape(self):
            #tests that 'type' isn't allowed anymore
            #with type defined, you could create magical classes like this: 
            code = """
def delmethod(self): 1/0
foo=type('Foo', (object,), {'_' + '_del_' + '_':delmethod})()
foo.error
"""
            try:
                self.assertRaises(RunBuiltinException,safe_exec,code)
            finally:
                pass
            
        def test_misc_recursive_fnc(self):
            code = "def test():test()\ntest()"
            self.assertRaises(RuntimeError,safe_exec,code)
            

    unittest.main()

    #safe_exec('print locals()')
