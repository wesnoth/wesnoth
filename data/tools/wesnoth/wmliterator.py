"""
wmliterator.py -- Python routines for navigating a Battle For Wesnoth WML tree

"""

def parseQuotes(lines, lineno):
    """return the line or multiline text if a quote spans multiple lines"""
    text = lines[lineno]
    span = 1
    begincomment = text.find('#')
    if begincomment < 0:
        begincomment = None
    beginquote = text[:begincomment].find('"')
    while beginquote > 0:
        endquote = -1
        beginofend = beginquote+1
        while endquote < 0:
            endquote = text.find('"', beginofend)
            if endquote < 0:
                # todo: check for end of lines
                text += '\n' + lines[lineno + span]
                span += 1
                beginofend = 0
        begincomment = text.find('#', endquote+1)
        if begincomment < 0:
            begincomment = None
        beginquote = text[:begincomment].find('"', endquote+1)
    return text, span

def parseElements(line, scope):
    """remove any closed scopes, return a tuple of element names
    and list of opened scopes"""
    raise NotImplemented
    return elements, openedScopes
    
class WmlIterator:
    """Return an iterable WML navigation object.
    note: if changes are made to lines while iterating, this may produce
    unexpected results."""
    def __init__(self, lines, begin=-1, end=None):
        "Initialize a new WmlIterator"
        self.lines = lines
        if end is None:
            self.end = len(lines)
        else:
            self.end = end
        self.seek(begin)

    def seek(self, lineno):
        """Move the iterator to a specific line number"""
        if lineno < 0 or lineno < self.lineno:
            # moving backwards or to the beginning forces a reset
            self.lineno = -1
            self.scope = []
            self.nextScope = []
            self.text = ""
            self.span = 1
            self.element = ""
        while self.lineno + self.span - 1 < lineno:
            self.next()

    def hasNext(self):
        """some loops may wish to check this method instead of calling next()
        and handling StopIteration"""
        return self.end > self.lineno + self.span

    def next(self):
        """Move the iterator to the next line number
        note: May raise StopIteration"""
        if not self.hasNext():
            raise StopIteration
        self.lineno = self.lineno + self.span
        self.text, self.span = parseQuotes(self.lines, self.lineno)
        self.scope.extend(self.nextScope)
        self.element, self.nextScope = parseElements(self.lines[self.lineno],
                                                     self.scope)
        if(len(self.element) == 1):
            # currently we only wish to handle simple single assignment syntax
            self.element = self.element[0]

# wmliterator.py ends here
