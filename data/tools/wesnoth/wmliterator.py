"""
wmliterator.py -- Python routines for navigating a Battle For Wesnoth WML tree
Author: Sapient (Patrick Parker), 2007
"""
import sys, re, copy
keyPattern = re.compile('(\w+)(,\s?\w+)*\s*=')
keySplit = re.compile(r'[=,\s]')
tagPattern = re.compile(r'(^|(?<![\w\|\}]))(\[.*?\])')
macroOpenPattern = re.compile(r'(\{[^\s\}]*)')
macroClosePattern = re.compile(r'\}')

silenceErrors = {}

def wmlfind(element, wmlItor):
    """Find a simple element from traversing a WML iterator"""
    for itor in wmlItor.copy():
        if element == itor.element:
            return itor
    return None

def wmlfindin(element, scopeElement, wmlItor):
    """Find an element inside a particular type of scope element"""
    if not scopeElement:
        return wmlfind(element, wmlItor)
    for itor in wmlItor.copy():
        if element == itor.element\
        and itor.scopes and scopeElement == itor.scopes[-1].element:
            return itor
    return None

def parseQuotes(lines, fname, lineno):
    """Return the line or multiline text if a quote spans multiple lines"""
    text = lines[lineno]
    span = 1
    begincomment = text.find('#')
    if begincomment < 0:
        begincomment = None
    beginquote = text[:begincomment].find('"')
    while beginquote >= 0:
        endquote = -1
        beginofend = beginquote+1
        while endquote < 0:
            endquote = text.find('"', beginofend)
            if endquote < 0:
                if lineno + span >= len(lines):
                    printError(fname, 'reached EOF due to unterminated string at line', lineno+1)
                    return text, span
                text += lines[lineno + span]
                span += 1
                beginofend = text.rfind('\n', beginofend, len(text)-1)
        begincomment = text.find('#', endquote+1)
        if begincomment < 0:
            begincomment = None
        beginquote = text[:begincomment].find('"', endquote+1)
    return text, span

def isDirective(elem):
    "Identify things that shouldn't be indented."
    if isinstance(elem, WmlIterator):
        elem = elem.element
    for prefix in ("#ifdef", "#else", "#endif", "#define", "#enddef"):
        if elem.startswith(prefix):
            return True
    return False

def isCloser(elem):
    "Are we looking at a closing tag?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    return elem.startswith("[/")

def isOpener(elem):
    "Are we looking at an opening tag?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    return elem.startswith("[") and not isCloser(elem)

def closeScope(scopes, closerElement, fname, lineno):
    """Close the most recently opened scope. Return false if not enough scopes.
    note: directives close all the way back to the last open directive
    non-directives cannot close a directive and will no-op in that case."""
    try:
        if isDirective(closerElement):
            while not isDirective(scopes.pop()):
                pass
        elif not isDirective(scopes[-1]):
            closed = scopes.pop()
            elem = closed
            if isinstance(closed, WmlIterator):
                elem = closed.element
            if ((isOpener(elem) and closerElement != '[/'+elem[1:]
                 and '+'+closerElement != elem[1]+'[/'+elem[2:])
            or (elem.startswith('{') and closerElement.find('macro')<0)):
                printError(fname, 'reached', closerElement, 'at line', lineno+1, 'before closing scope', elem, '(%d)'%lineno)
                scopes.append(closed) # to reduce additional errors (hopefully)
        return True
    except IndexError:
        return False

def printError(fname, *misc):
    """Print error associated with a given file; avoid printing duplicates"""
    if fname:
        silenceValue = ' '.join(map(str, misc))
        if fname not in silenceErrors:
            print >>sys.stderr, fname
            silenceErrors[fname] = set()
        elif silenceValue in silenceErrors[fname]:
            return # do not print a duplicate error for this file
        silenceErrors[fname].add(silenceValue)
    print >>sys.stderr, 'wmliterator:',
    for item in misc:
        print >>sys.stderr, item,
    print >>sys.stderr #terminate line
        
def printScopeError(elementType, fname, lineno):
    """Print out warning if a scope was unable to close"""
    printError(fname, 'attempt to close empty scope at', elementType, 'line', lineno+1)

def parseElements(text, fname, lineno, scopes):
    """Remove any closed scopes, return a tuple of element names
    and list of new unclosed scopes    
Element Types:
    tags: one of "[tag_name]" or "[/tag_name]"
        [tag_name] - opens a scope
        [/tag_name] - closes a scope
    keys: either "key=" or "key1,key2=" (multi-assignment)
        key= - does not affect the scope
        (key1=, key2=) - multi-assignment returns multiple elements
    directives: one of "#ifdef", "#else", "#endif", "#define", "#enddef"
        #ifdef - opens a scope
        #else - closes a scope, also opens a new scope
        #endif - closes a scope
        #define - opens a scope
        #enddef - closes a scope
    macro calls: "{MACRO_NAME}"
        {MACRO_NAME - opens a scope
        } - closes a scope (not an element)
    """
    closeMacroType = 'end of macro'
    elements = [] #(elementType, sortPos, scopeDelta)
    # first remove any quoted strings
    beginquote = text.find('"')
    while beginquote >= 0:
        endquote = text.find('"', beginquote+1)
        if endquote < 0:
            text = text[:beginquote]
            beginquote = -1 #terminate loop
        else:
            text = text[:beginquote] + text[endquote+1:]
            beginquote = text.find('"')
    # next remove any comments
    text = text.lstrip()
    commentSearch = 1
    if text.startswith('#ifdef'):
        return (['#ifdef'],)*2
    elif text.startswith('#else'):
        if not closeScope(scopes, '#else', fname, lineno):
            printScopeError('#else', fname, lineno)
        return (['#else'],)*2
    elif text.startswith('#endif'):
        if not closeScope(scopes, '#endif', fname, lineno):
            printScopeError('#endif', fname, lineno)            
        return ['#endif'], []
    elif text.startswith('#define'):
        return (['#define'],)*2
    elif text.find('#enddef') >= 0:
        elements.append(('#enddef', text.find('#enddef'), -1))
    else:
        commentSearch = 0
    begincomment = text.find('#', commentSearch)
    if begincomment >= 0:
        text = text[:begincomment]
    #now find elements in a loop
    for m in tagPattern.finditer(text):
        delta = 1
        if isCloser(m.group(2)):
            delta = -1
        elements.append((m.group(2), m.start(), delta))
    for m in keyPattern.finditer(text):
        for i, k in enumerate(keySplit.split(m.group(0))):
            if k:
                elements.append((k+'=', m.start()+i, 0))
    for m in macroOpenPattern.finditer(text):
        elements.append((m.group(1)+'}', m.start(), 1))
    for m in macroClosePattern.finditer(text):
        elements.append((closeMacroType, m.start(), -1))
    #sort by start position
    elements.sort(key=lambda x:x[1])
    resultElements = []
    openedScopes = []
    for elem, sortPos, scopeDelta in elements:
        while scopeDelta < 0:
            if not(closeScope(openedScopes, elem, fname, lineno)\
            or closeScope(scopes, elem, fname, lineno)):
                printScopeError(elem, fname, lineno)
            scopeDelta += 1
        while scopeDelta > 0:
            openedScopes.append(elem)
            scopeDelta -= 1
        if elem != closeMacroType:
            resultElements.append(elem)
    return resultElements, openedScopes

class WmlIterator(object):
    """Return an iterable WML navigation object.
    note: if changes are made to lines while iterating, this may produce
    unexpected results. In such case, seek() to the linenumber of a
    scope behind where changes were made."""
    def __init__(self, lines, fname=None, begin=-1, endScope=None):
        "Initialize a new WmlIterator"
        self.lines = lines
        self.fname = fname
        self.reset()
        self.seek(begin)

    def __iter__(self):
        """The magic iterator method"""
        return self

    def __cmp__(self, other):
        """Compare two iterators"""
        return cmp((self.fname, self.lineno, self.element),
                   (other.fname, other.lineno, other.element))

    def reset(self):
        """Reset any line tracking information to defaults"""
        self.lineno = -1
        self.scopes = []
        self.nextScopes = []
        self.text = ""
        self.span = 1
        self.element = ""
        return self
        
    def seek(self, lineno, clearEnd=True):
        """Move the iterator to a specific line number"""
        if clearEnd:
            self.endScope = None
        if lineno < self.lineno:
            for scope in reversed(self.scopes):
                # if moving backwards, try to re-use a scope iterator
                if scope.lineno <= lineno:
                    # copy the scope iterator's state to self
                    self.__dict__ = dict(scope.__dict__)
                    self.scopes = scope.scopes[:]
                    self.nextScopes = scope.nextScopes[:]
                    break
            else:
                # moving backwards past all scopes forces a reset
                self.reset()
        while self.lineno + self.span - 1 < lineno:
            self.next()
        return self

    def hasNext(self):
        """Some loops may wish to check this method instead of calling next()
        and handling StopIteration... note: inaccurate for ScopeIterators"""
        return len(self.lines) > self.lineno + self.span

    def copy(self):
        """Return a copy of this iterator"""
        itor = copy.copy(self)
        itor.scopes = self.scopes[:]
        itor.nextScopes = self.nextScopes[:]
        return itor

    def __str__(self):
        """Return a pretty string representation"""
        if self.lineno == -1:
            return 'beginning of file'
        loc = ' at line ' + str(self.lineno+1)
        if self.element:
            return str(self.element) + loc
        if self.text.strip():
            return 'text' + loc
        return 'whitespace' + loc

    def __repr__(self):
        """Return a very basic string representation"""
        return 'WmlIterator<' + repr(self.element) +', line %d>'%(self.lineno+1)

    def next(self):
        """Move the iterator to the next line number
        note: May raise StopIteration"""
        if not self.hasNext():
            if self.scopes:
                printError(self.fname, "reached EOF with open scopes", self.scopes)
            raise StopIteration
        self.lineno = self.lineno + self.span
        self.text, self.span = parseQuotes(self.lines, self.fname, self.lineno)
        self.scopes.extend(self.nextScopes)
        self.element, nextScopes = parseElements(self.text, self.fname, self.lineno, self.scopes)
        self.nextScopes = []
        for elem in nextScopes:
            assert elem
            copyItor = self.copy()
            copyItor.element = elem
            copyItor.scopes = self.scopes[:]
            self.nextScopes.append(copyItor)
            copyItor.nextScopes = self.nextScopes[:]
        if(len(self.element) == 1):
            # currently we only wish to handle simple single assignment syntax
            self.element = self.element[0]
        if self.endScope is not None and not self.scopes.count(self.endScope):
            raise StopIteration
        return self

    def iterScope(self):
        """Return an iterator for the current scope"""
        if not self.scopes:
            return WmlIterator(self.lines, self.fname)
        scopeItor = self.scopes[-1].copy()
        scopeItor.endScope = self.scopes[-1]
        return scopeItor

if __name__ == '__main__':
    """Perform a test run on a file or directory"""
    import os, glob
    didSomething = False
    print 'Current directory is', os.getcwd()
    flist = glob.glob(os.path.join(os.getcwd(), raw_input('Which file(s) would you like to test?\n')))
    while flist:
        fname = flist.pop()
        if os.path.isdir(fname):
            flist += glob.glob(fname + os.path.sep + '*')
            continue
        if not os.path.isfile(fname) or os.path.splitext(fname)[1] != '.cfg':
            continue
        print 'Reading', fname+'...'
        didSomething = True
        f = open(fname)
        itor = WmlIterator(f.readlines())
        for i in itor:
            pass
        f.close()
        print itor.lineno + itor.span, 'lines read.'
    if not didSomething:
        print 'That is not a valid .cfg file'
    if os.name == 'nt' and os.path.splitext(__file__)[0].endswith('wmliterator') and not sys.argv[1:]:
        os.system('pause')

# wmliterator.py ends here
