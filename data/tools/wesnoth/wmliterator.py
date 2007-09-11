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

def wmlfind(element, itor):
    """Find a simple element from traversing a WML iterator"""
    for i in itor:
        if element == itor.element:
            return itor
    return None

def wmlfindin(element, scopeElement, itor, memory={}):
    """Find an element inside a particular type of scope element
    return an iterator pointing to the element and an iterator to its scope"""
    if element is None:
        #pass element None to flush wmlfindin's cached memory
        for k in memory.keys():
            if k[0] == itor.fname and k[1] == scopeElement:
                del memory[k]
        return None
    #start the main loop... this is optimized to prevent iterator resets
    initLoop = True
    tracker, breaker = memory.get((itor.fname, scopeElement, itor.lineno), ([], itor.endScope))
    while True:
        try:
            if initLoop:
                itor.endScope, breaker = breaker, itor.endScope
                initLoop = False
            itor.next()
            if scopeElement == itor.element:
                restoreBreak = itor.endScope
                itor.endScope = (scopeElement, itor.lineno)
                tracker.append((len(itor.scopes), itor.copy(), restoreBreak))
                memory[(itor.fname, scopeElement, itor.lineno)] = (tracker, itor.endScope)
            if element == itor.element:
                for track in tracker:
                    if len(itor.scopes) == track[0]+1:
                        memory[(itor.fname, scopeElement, itor.lineno)] = (tracker, itor.endScope)
                        itor.endScope = breaker
                        return itor, track[1].copy()
        except StopIteration:
            if not tracker:
                return None
            track = tracker.pop()
            itor.endScope = track[2]
            del memory[(itor.fname, scopeElement, track[1].lineno)]

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

def isDirective(str):
    "Identify things that shouldn't be indented."
    for prefix in ("#ifdef", "#else", "#endif", "#define", "#enddef"):
        if str.startswith(prefix):
            return True
    return False

def isCloser(str):
    "Are we looking at a closing tag?"
    return str.startswith("[/")

def isOpener(str):
    "Are we looking at an opening tag?"
    return str.startswith("[") and not isCloser(str)

def closeScope(scopes, closerElement, fname, lineno):
    """Close the most recently opened scope. Return false if not enough scopes.
    note: directives close all the way back to the last open directive
    non-directives cannot close a directive and will no-op in that case."""
    try:
        if isDirective(closerElement):
            while not isDirective(scopes.pop()[0]):
                pass
        elif not isDirective(scopes[-1][0]):
            closed = scopes.pop()
            if ((isOpener(closed[0]) and closerElement != '[/'+closed[0][1:]
                 and '+'+closerElement != closed[0][1]+'[/'+closed[0][2:])
            or (closed[0].startswith('{') and closerElement.find('macro')<0)):
                printError(fname, 'reached', closerElement, 'at line', lineno+1, 'before closing scope', closed[0], '(%d)'%(closed[1]+1))
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
        return ['#ifdef'], [('#ifdef', lineno)]
    elif text.startswith('#else'):
        if not closeScope(scopes, '#else', fname, lineno):
            printScopeError('#else', fname, lineno)
        return ['#else'], [('#else', lineno)]
    elif text.startswith('#endif'):
        if not closeScope(scopes, '#endif', fname, lineno):
            printScopeError('#endif', fname, lineno)            
        return ['#endif'], []
    elif text.startswith('#define'):
        return ['#define'], [('#define', lineno)]
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
            openedScopes.append((elem, lineno))
            scopeDelta -= 1
        if elem != closeMacroType:
            resultElements.append(elem)
    return resultElements, openedScopes
    
class WmlIterator(object):
    """Return an iterable WML navigation object.
    note: if changes are made to lines while iterating, this may produce
    unexpected results."""
    def __init__(self, lines, fname=None, begin=-1, endScope=None):
        "Initialize a new WmlIterator"
        self.lines = lines
        self.fname = fname
        self.endScope = None
        self.end = len(lines)
        self.reset()
        self.seek(begin)
        self.endScope = endScope

    def __iter__(self):
        """The magic iterator method"""
        return self

    def reset(self):
        """Reset any line tracking information to defaults"""
        self.lineno = -1
        self.scopes = []
        self.nextScopes = []
        self.text = ""
        self.span = 1
        self.element = ""
        
    def seek(self, lineno):
        """Move the iterator to a specific line number"""
        if lineno < self.lineno:
        # moving backwards forces a reset
            self.reset()
        while self.lineno + self.span - 1 < lineno:
            self.next()

    def hasNext(self):
        """Some loops may wish to check this method instead of calling next()
        and handling StopIteration... note: inaccurate for ScopeIterators"""
        return self.end > self.lineno + self.span

    def copy(self):
        """Return a copy of this iterator"""
        itor = copy.copy(self)
        itor.scopes = self.scopes[:]
        itor.nextScopes = self.nextScopes[:]
        return itor

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
        self.element, self.nextScopes = parseElements(self.text, self.fname, self.lineno, self.scopes)
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
        return WmlIterator(self.lines, self.fname, self.scopes[-1][1], self.scopes[-1])

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
