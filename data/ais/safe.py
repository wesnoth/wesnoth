"""An attempt at creating a safe_exec for python.

This file is public domain and is not suited for any serious purpose.
This code is not guaranteed to work. Use at your own risk!
Beware!  Trust no one!

Please e-mail philhassey@yahoo.com if you find any security holes.
svn://www.imitationpickles.org/pysafe/trunk

See README.txt, NOTES.txt, CHANGES.txt for more details.
"""
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

_BUILTIN_OK = [
    '__debug__','quit','exit',
    'Warning',
    'None','True','False',
    'abs', 'bool', 'callable', 'cmp', 'complex', 'dict', 'divmod', 'filter',
    'float', 'frozenset', 'hex', 'int', 'isinstance', 'issubclass', 'len',
    'list', 'long', 'map', 'max', 'min', 'object', 'oct', 'pow', 'range',
    'repr', 'round', 'set', 'slice', 'str', 'sum', 'tuple',  'xrange', 'zip',
    ]

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

# If you want to disable safe python, use this instead:
#
# def safe_exec(code, context = None): exec code in context
def safe_exec(code, context = None):
    """Check the code to be safe, then run it with only safe builtins on."""
    safe_check(code)
    safe_run(code,context)
    
