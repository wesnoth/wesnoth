"""
wmliterator.py -- Python routines for navigating a Battle For Wesnoth WML tree
Author: Sapient (Patrick Parker), 2007

Purpose:
 The WmlIterator class can be used to analyze and search the structure of WML
 files non-invasively (i.e. preserving existing line structure), and its main
 use is to determine when a transformation of deprecated content needs to take
 place. (I wrote it was because wmllint was trying to do a lot of things with
 regular expressions which really required a more algorithmic approach. Also,
 wmllint was often inconsistent with correct handling of comments and values
 inside strings.)

Limitations:
 The WmlIterator does not attempt to expand macros, only to recognize them as
 another level of nesting. Also, the handling of multiple assignment syntax
 is somewhat limited (for similar reasons). Adding seamless support for these
 would be ideal, but it presents a design challenge since the iteration is
 supposed to be non-invasive. Thus, the current behavior is considered good
 enough for now.
"""

import sys, re, copy
keyPattern = re.compile('(\w+)(,\s?\w+)*\s*=')
keySplit = re.compile(r'[=,\s]')
tagPattern = re.compile(r'(^|(?<![\w\|\}]))(\[/?\+?[a-z].*?\])')
macroOpenPattern = re.compile(r'(\{[^\s\}\{]*)')
macroClosePattern = re.compile(r'\}')
closeMacroType = 'end of macro'

silenceErrors = {}

def wmlfind(element, wmlItor):
    """Find a simple element from traversing a WML iterator"""
    for itor in wmlItor.copy():
        if element == itor.element:
            return itor
    return None

def wmlfindin(element, scopeElement, wmlItor):
    """Find an element inside a particular type of scope element"""
    for itor in wmlItor.copy():
        if element == itor.element:
            if itor.scopes:
                if scopeElement == itor.scopes[-1].element:
                    return itor
            elif not scopeElement:
                # allow searching in the empty scope
                return itor
    return None

def isDirective(elem):
    "Identify things that shouldn't be indented."
    if isinstance(elem, WmlIterator):
        elem = elem.element
    for prefix in ("#ifdef", "#ifndef", "#ifhave", "#ifnhave", "#ifver", "#ifnver", "#else", "#endif", "#define", "#enddef", "#undef"):
        if elem.startswith(prefix):
            return True
    return False

def isCloser(elem):
    "Are we looking at a closing tag?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    return type(elem) == type("") and elem.startswith("[/")

def isMacroCloser(elem):
    "Are we looking at a macro closer?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    return type(elem) == type("") and elem == closeMacroType

def isOpener(elem):
    "Are we looking at an opening tag?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    return type(elem) == type("") and elem.startswith("[") and not isCloser(elem)

def isMacroOpener(elem):
    "Are we looking at a macro opener?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    return type(elem) == type("") and elem.startswith("{")

def isAttribute(elem):
    "Are we looking at an attribute (or attribute tuple)?"
    if isinstance(elem, WmlIterator):
        elem = elem.element
    if type(elem) == type(()):
        elem = elem[0]
    return type(elem) == type("") and elem.endswith("=")

class WmlIterator(object):
    """Return an iterable WML navigation object.
    Initialize with a list of lines or a file; if the the line list is
    empty and the filename is specified, lines will be read from the file.

    Note: if changes are made to lines while iterating, this may produce
    unexpected results. In such case, seek() to the linenumber of a
    scope behind where changes were made.
Important Attributes:
    lines - this is an internal list of all the physical lines
    scopes - this is an internal list of all open scopes (as iterators)
             note: when retreiving an iterator from this list, always
             use a copy to perform seek() or next(), and not the original
    element - the wml tag, key, or macro name for this logical line
              (in complex cases, this may be a tuple of elements...
              see parseElements for list of possible values)
    text - the exact text of this logical line, as it appears in the
           original source, and ending with a newline
           note: the logical line also includes multi-line quoted strings
    span - the number of physical lines in this logical line:
           always 1, unless text contains a multi-line quoted string
    lineno - a zero-based line index marking where this text begins
    """
    def __init__(self, lines=None, filename=None, begin=-1):
        "Initialize a new WmlIterator."
        self.fname = filename
        if lines is None:
            lines = []
            if filename:
                try:
                    ifp = open(self.fname)
                    lines = ifp.readlines()
                    ifp.close()
                except Exception:
                    self.printError('error opening file')
        self.lines = lines
        self.reset()
        self.seek(begin)

    def parseQuotes(self, lines):
        """Return the line or multiline text if a quote spans multiple lines"""
        text = lines[self.lineno]
        span = 1
        begincomment = text.find('#')
        if begincomment < 0:
            begincomment = None
        beginquote = text[:begincomment].find('<<')
        while beginquote >= 0:
            endquote = -1
            beginofend = beginquote+2
            while endquote < 0:
                endquote = text.find('>>', beginofend)
                if endquote < 0:
                    if self.lineno + span >= len(lines):
                        self.printError('reached EOF due to unterminated string at line', self.lineno+1)
                        return text, span
                    beginofend = len(text)
                    text += lines[self.lineno + span]
                    span += 1
            begincomment = text.find('#', endquote+2)
            if begincomment < 0:
                begincomment = None
            beginquote = text[:begincomment].find('<<', endquote+2)
        beginquote = text[:begincomment].find('"')
        while beginquote >= 0:
            endquote = -1
            beginofend = beginquote+1
            while endquote < 0:
                endquote = text.find('"', beginofend)
                if endquote < 0:
                    if self.lineno + span >= len(lines):
                        self.printError('reached EOF due to unterminated string at line', self.lineno+1)
                        return text, span
                    beginofend = len(text)
                    text += lines[self.lineno + span]
                    span += 1
            begincomment = text.find('#', endquote+1)
            if begincomment < 0:
                begincomment = None
            beginquote = text[:begincomment].find('"', endquote+1)
        return text, span

    def closeScope(self, scopes, closerElement):
        """Close the most recently opened scope. Return false if not enough scopes.
        note: directives close all the way back to the last open directive
        non-directives cannot close a directive and will no-op in that case."""
        try:
            if isDirective(closerElement):
                while not isDirective(scopes.pop()):
                    pass
            elif (closerElement==closeMacroType):
                elem = ''
                while not elem.startswith('{'):
                    closed = scopes.pop()
                    elem = closed
                    if isinstance(closed, WmlIterator):
                        elem = closed.element
                    if isDirective(elem):
                        self.printScopeError(closerElement)
                        scopes.append(closed) # to reduce additional errors (hopefully)
                        return True
            elif not isDirective(scopes[-1]):
                closed = scopes.pop()
                elem = closed
                if isinstance(closed, WmlIterator):
                    elem = closed.element
                if (elem.startswith('{') and closerElement != closeMacroType):
                    scopes.append(closed)
                elif (isOpener(elem) and closerElement != '[/'+elem[1:]
                and '+'+closerElement != elem[1]+'[/'+elem[2:]):
                    self.printError('reached', closerElement, 'at line', self.lineno+1, 'before closing scope', elem)
                    scopes.append(closed) # to reduce additional errors (hopefully)
            return True
        except IndexError:
            return False

    def parseElements(self, text):
        """Remove any closed scopes, return a list of element names
        and list of new unclosed scopes
    Element Types:
        tags: one of "[tag_name]" or "[/tag_name]"
            [tag_name] - opens a scope
            [/tag_name] - closes a scope
        keys: either "key=" or ("key1=", "key2=") for multi-assignment
            key= - does not affect the scope
            key1,key2= - multi-assignment returns multiple elements
        directives: one of "#ifdef", "#ifndef", "#ifhave", "#ifnhave", "#ifver", "#ifnver", "#else", "#endif", "#define", "#enddef"
            #ifdef - opens a scope
            #ifndef - opens a scope
            #ifhave - opens a scope
            #ifnhave - opens a scope
            #ifver - opens a scope
            #ifnver - opens a scope
            #else - closes a scope, also opens a new scope
            #endif - closes a scope
            #define - opens a scope
            #enddef - closes a scope
        macro calls: "{MACRO_NAME}"
            {MACRO_NAME - opens a scope
            } - closes a scope
        """
        elements = [] #(elementType, sortPos, scopeDelta)
        # first remove any lua strings
        beginquote = text.find('<<')
        while beginquote >= 0:
            endquote = text.find('>>')
            if endquote < -1:
                text = text[:beginquote]
                beginquote = -1 #terminate loop
            else:
                text = text[:beginquote] + text[endquote+2:]
                beginquote = text.find('<<')
        # remove any quoted strings
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
        elif text.startswith('#ifndef'):
            return (['#ifndef'],)*2
        elif text.startswith('#ifhave'):
            return (['#ifhave'],)*2
        elif text.startswith('#ifnhave'):
            return (['#ifnhave'],)*2
        elif text.startswith('#ifver'):
            return (['#ifver'],)*2
        elif text.startswith('#ifnver'):
            return (['#ifnver'],)*2
        elif text.startswith('#else'):
            if not self.closeScope(self.scopes, '#else'):
                self.printScopeError('#else')
            return (['#else'],)*2
        elif text.startswith('#endif'):
            if not self.closeScope(self.scopes, '#endif'):
                self.printScopeError('#endif')
                return ['#endif'], []
        elif text.startswith('#define'):
            return (['#define'],)*2
        elif text.find('#enddef') >= 0:
            elements.append(('#enddef', text.find('#enddef'), -1))
        elif text.startswith('#po:') or text.startswith('# po:'):
            elements.append(("#po", 0, 0))
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
            elements.append((m.group(1), m.start(), 1))
        for m in macroClosePattern.finditer(text):
            elements.append((closeMacroType, m.start(), -1))
        #sort by start position
        elements.sort(key=lambda x:x[1])
        resultElements = []
        openedScopes = []
        for elem, sortPos, scopeDelta in elements:
            while scopeDelta < 0:
                if not(self.closeScope(openedScopes, elem)\
                        or self.closeScope(self.scopes, elem)):
                    self.printScopeError(elem)
                scopeDelta += 1
            while scopeDelta > 0:
                openedScopes.append(elem)
                scopeDelta -= 1
            resultElements.append(elem)
        return resultElements, openedScopes

    def printScopeError(self, elementType):
        """Print out warning if a scope was unable to close"""
        self.printError('attempt to close empty scope at', elementType, 'line', self.lineno+1)

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

    def ancestors(self):
        """Return a list of tags enclosing this location, outermost first."""
        return tuple(map(lambda x: x.element, self.scopes))

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
                self.printError("reached EOF with open scopes", self.scopes)
            raise StopIteration
        self.lineno = self.lineno + self.span
        self.text, self.span = self.parseQuotes(self.lines)
        self.scopes.extend(self.nextScopes)
        self.element, nextScopes = self.parseElements(self.text)
        self.nextScopes = []
        for elem in nextScopes:
	    # remember scopes by storing a copy of the iterator
            copyItor = self.copy()
            copyItor.element = elem
            self.nextScopes.append(copyItor)
            copyItor.nextScopes.append(copyItor)
        if(len(self.element) == 1):
            # currently we only wish to handle simple single assignment syntax
            self.element = self.element[0]
        if self.endScope is not None and not self.scopes.count(self.endScope):
            raise StopIteration
        return self

    def isOpener(self):
        return isOpener(self)

    def isCloser(self):
        return isCloser(self)

    def isMacroOpener(self):
        return isMacroOpener(self)

    def isMacroCloser(self):
        return isMacroCloser(self)

    def isAttribute(self):
        return isAttribute(self)

    def iterScope(self):
        """Return an iterator for the current scope"""
        if not self.scopes:
            return WmlIterator(self.lines, self.fname)
        scopeItor = self.scopes[-1].copy()
        scopeItor.endScope = self.scopes[-1]
        return scopeItor

    def printError(nav, *misc):
        """Print error associated with a given file; avoid printing duplicates"""
        if nav.fname:
            silenceValue = ' '.join(map(str, misc))
            if nav.fname not in silenceErrors:
                print >>sys.stderr, nav.fname
                silenceErrors[nav.fname] = set()
            elif silenceValue in silenceErrors[nav.fname]:
                return # do not print a duplicate error for this file
            silenceErrors[nav.fname].add(silenceValue)
        print >>sys.stderr, 'wmliterator:',
        for item in misc:
            print >>sys.stderr, item,
        print >>sys.stderr #terminate line

if __name__ == '__main__':
    """Perform a test run on a file or directory"""
    import os, glob
    didSomething = False
    flist = sys.argv[1:]
    if not flist:
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
