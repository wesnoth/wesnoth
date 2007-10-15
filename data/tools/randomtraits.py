"""
randomtraits.py -- Python routine for upconverting v1.3-1.3.7 saved replays
                   to v1.3.8+ format. Backups are saved to /backup
Author: Sapient (Patrick Parker), 2007
"""
from wesnoth.wmliterator import *

if __name__ == '__main__':
    """Perform a random_traits=no conversion on a saved replay"""
    import os, glob, re
    versionPattern = re.compile(r'\s*version="1.3.([0-9]+).*')
    didSomething = False
    print 'Current directory is', os.getcwd()
    flist = glob.glob(os.path.join(os.getcwd(), raw_input('What is the path to the saved replay?\n')))
    while flist:
        fname = flist.pop()
        if os.path.isdir(fname):
            flist += glob.glob(fname + os.path.sep + '*')
            continue
        if not os.path.isfile(fname) or os.path.splitext(fname)[1]:
            continue
        print 'Reading', fname+'...'
        f = open(fname, 'U')
        itor = wmlfind('version=', WmlIterator(f.readlines()))
        if itor is None:
            f.close()
            continue
        didSomething = True
        changed = 0
        elems = set()
        match = versionPattern.match(itor.text)
        if (match is not None and int(match.group(1)) < 8)\
        or ('"test"' in itor.text and 'yes'.startswith(raw_input('Warning: %s\n\tversion is "test", convert? '%fname).lower())):
            itor.reset()
            i = wmlfind('[unit]', itor)
            while i is not None:
                elems.add(i.lineno)
                i = wmlfind('[unit]', i)
            i = wmlfindin('random_traits=', '[unit]', itor)
            while i is not None:
                elems.remove(i.scopes[-1].lineno)
                i = wmlfindin('random_traits=', '[unit]', i)
            for elem in sorted(elems):
                changed += 1
                itor.lines = itor.lines[:elem+changed] + ['random_traits="no"\n'] + itor.lines[elem+changed:]
        f.close()
        if changed:
            backup = os.path.join(os.path.dirname(fname), 'backup', os.path.basename(fname))
            print 'Creating backup', backup
            os.renames(fname, backup)
            f = open(fname, "w")
            f.write("".join(itor.lines))
            f.close()
            print 'Converted', fname
            print '\t(%d line(s) changed)'%changed
    if not didSomething:
        print 'That is not a valid v1.3-v1.3.7 replay savefile'
    if os.name == 'nt' and os.path.splitext(__file__)[0].endswith('randomtraits') and not sys.argv[1:]:
        os.system('pause')

# randomtraits.py ends here
