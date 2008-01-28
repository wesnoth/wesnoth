"""
wmltools.py -- Python routines for working with a Battle For Wesnoth WML tree

"""

import sys, os, re, sre_constants, md5, glob

resource_extensions = ("png", "jpg", "ogg", "wav", "map")

class Forest:
    "Return an iterable directory forest object."
    def __init__(self, dirpath, exclude=None):
        "Get the names of all files under dirpath, ignoring .svn directories."
        self.forest = []
        self.dirpath = dirpath
        for dir in dirpath:
            subtree = []
            if os.path.isdir(dir):	# So we skip .cfgs in a UMC mirror
                os.path.walk(dir,
                         lambda arg, dir, names: subtree.extend(map(lambda x: os.path.normpath(os.path.join(dir, x)), names)),
                         None)
            self.forest.append(subtree)
        for i in range(len(self.forest)):
            self.forest[i] = filter(lambda x: ".svn" not in x, self.forest[i])
            self.forest[i] = filter(lambda x: not os.path.isdir(x), self.forest[i])
            if exclude:
                self.forest[i] = filter(lambda x: not re.search(exclude, x), self.forest[i])
            self.forest[i] = filter(lambda x: not x.endswith("-bak"), self.forest[i])
        # Compute cliques (will be used later for visibility checks) 
        self.clique = {}
        counter = 0
        for tree in self.forest:
            for filename in tree:
                self.clique[filename] = counter
            counter += 1
    def parent(self, filename):
        "Return the directory root that caused this path to be included."
        return self.dirpath[self.clique[filename]]
    def neighbors(self, fn1, fn2):
        "Are two files from the same tree?"
        return self.clique[fn1] == self.clique[fn2]
    def flatten(self):
        "Return a flattened list of all files in the forest."
        allfiles = []
        for tree in self.forest:
            allfiles += tree
        return allfiles
    def generator(self):
        "Return a generator that walks through all files."
        for tree in self.forest:
            for filename in tree:
                yield filename

def iswml(filename):
    "Is the specified filename WML?"
    return filename.endswith(".cfg")

def isresource(filename):
    "Is the specified name a resource?"
    (root, ext) = os.path.splitext(filename)
    return ext and ext[1:] in resource_extensions

class Reference:
    "Describes a location by file and line."
    def __init__(self, filename, lineno=None, docstring=None):
        self.filename = filename
        self.lineno = lineno
        self.references = {}
        self.docstring = docstring
        self.undef = None
    def append(self, fn, n):
        if fn not in self.references:
            self.references[fn] = []
        self.references[fn].append(n)
    def dump_references(self):
        for (file, linenumbers) in self.references.items():
            print "    %s: %s" % (file, `linenumbers`[1:-1])
    def __cmp__(self, other):
        "Compare two documentation objects for place in the sort order."
        # Major sort by file, minor by line number.  This presumes that the
        # files correspond to coherent topics and gives us control of the
        # sequence.
        byfile = cmp(self.filename, other.filename)
        if byfile:
            return byfile
        else:
            return cmp(self.lineno, other.lineno)
    def __str__(self):
        if self.lineno:
            return '"%s", line %d' % (self.filename, self.lineno)
        else:
            return self.filename
    __repr__ = __str__

class CrossRef:
    macro_reference = re.compile(r"\{([A-Z_][A-Z0-9_:]*[A-Za-z0-9_])\b")
    file_reference =  re.compile(r"[A-Za-z0-9{}.][A-Za-z0-9_/+{}.-]*\.(" + "|".join(resource_extensions) + ")")
    def mark_matching_resources(self, pattern, fn, n):
        "Mark all definitions matching a specified pattern with a reference."
        pattern = pattern.replace("+", r"\+")
        try:
            pattern = re.compile(os.sep + pattern + "$")
        except sre_constants.error:
            print >>sys.stderr, "macroscope: confused by %s" % pattern
            return None
        key = None
        for trial in self.fileref:
            if pattern.search(trial) and self.visible_from(trial, fn, n):
                key = trial
                self.fileref[key].append(fn, n)
        return key
    def visible_from(self, defn, fn, n):
        "Is specified definition visible from the specified file and line?"
        if type(defn) == type(""):
            defn = self.fileref[defn]
        if defn.undef != None:
            # Local macros are only visible in the file where they were defined
            return defn.filename == fn and n >= defn.lineno and n <= defn.undef
        elif defn.filename in self.filelist.forest[0]:
            # Macros and resources in the first subtree are visible everywhere.
            return True
        elif not self.filelist.neighbors(defn.filename, fn):
            # Otherwise, must be in the same subtree.
            return False
        else:
            # If the two files are in the same subtree, assume visibility.
            # This doesn't match the actual preprocessor semantics.
            # It means any macro without an undef is visible anywhere in the
            # same argument directory.
            #
            # We can't do better than this without a lot of hairy graph-
            # coloring logic to simulate include path interpretation.
            # If that logic ever gets built, it will go here.
            return True
    def __init__(self, dirpath=[], exclude="", warnlevel=0):
        "Build cross-reference object from the specified filelist."
        self.dirpath = dirpath
        self.filelist = Forest(dirpath, exclude)
        self.xref = {}
        self.fileref = {}
        self.noxref = False
        for filename in self.filelist.generator():
            if warnlevel > 1:
                print filename + ":"
            if isresource(filename):
                self.fileref[filename] = Reference(filename)
            elif iswml(filename):
                # It's a WML file, scan for macro definitions
                dfp = open(filename)
                state = "outside"
                for (n, line) in enumerate(dfp):
                    if warnlevel > 1:
                        print `line`[1:-1]
                    if line.strip().startswith("#textdomain"):
                        continue
                    elif line.strip().startswith("#define"):
                        tokens = line.split()
                        name = tokens[1]
                        here = Reference(filename, n+1, line)
                        here.hash = md5.new()
                        here.docstring = line.lstrip()[8:]	# Strip off #define_
                        state = "macro_header"
                        continue
                    elif state != 'outside' and line.strip().endswith("#enddef"):
                        here.hash.update(line)
                        here.hash = here.hash.digest()
                        if name in self.xref:
                            for defn in self.xref[name]:
                                if not self.visible_from(defn, filename, n+1):
                                    continue
                                elif defn.hash != here.hash:
                                    print >>sys.stderr, \
                                            "%s: overrides different %s definition at %s" \
                                            % (here, name, defn)
                                elif warnlevel > 0:
                                    print >>sys.stderr, \
                                            "%s: duplicates %s definition at %s" \
                                            % (here, name, defn)
                        if name not in self.xref:
                            self.xref[name] = []
                        self.xref[name].append(here)
                        state = "outside"
                    elif state == "macro_header" and line.strip() and line.strip()[0] != "#":
                        state = "macro_body"
                    if state == "macro_header":
                        here.docstring += line.lstrip()[1:]
                    if state in ("macro_header", "macro_body"):
                        here.hash.update(line)
                    elif line.strip().startswith("#undef"):
                        tokens = line.split()
                        name = tokens[1]
                        if name in self.xref and self.xref[name]:
                            self.xref[name][-1].undef = n+1
                        else:
                            print "%s: unbalanced #undef on %s" \
                                  % (Reference(filename, n+1), name)
                dfp.close()
            elif filename.endswith(".def"):
                # It's a list of names to be considered defined
                self.noxref = True
                dfp = open(filename)
                for line in dfp:
                    self.xref[line.strip()] = True
                dfp.close()            
        # Next, decorate definitions with all references from the filelist.
        self.unresolved = []
        self.missing = []
        formals = []
        for fn in self.filelist.generator():
            if iswml(fn):
                rfp = open(fn)
                attack_name = None
                beneath = 0
                for (n, line) in enumerate(rfp):
                    if line.startswith("#define"):
                        formals = line.split()[2:]
                    elif line.startswith("#enddef"):
                        formals = []
                    comment = ""
                    if '#' in line:
                        fields = line.split('#')
                        line = fields[0]
                        if len(fields) > 1:
                            comment = fields[1]
                    if not line:
                        continue
                    # Find references to macros
                    for match in re.finditer(CrossRef.macro_reference, line):
                        name = match.group(1)
                        candidates = 0
                        if name in formals:
                            continue
                        elif name in self.xref:
                            for defn in self.xref[name]:
                                if self.visible_from(defn, fn, n+1):
                                    candidates += 1
                                    defn.append(fn, n+1)
                            if candidates > 1:
                                print "%s: more than one definition of %s is visible here." % (Reference(fn, n), name)
                        if candidates == 0:
                            self.unresolved.append((name, Reference(fn,n+1)))
                    # Find references to resource files
                    for match in re.finditer(CrossRef.file_reference, line):
                        name = match.group(0)
                        # Catches maps that look like macro names.
                        if name.endswith(".map") and name[0] == '{':
                            name = name[1:]
                        key = None
                        # If name is already in our resource list, it's easy.
                        if name in self.fileref and self.visible_from(name, fn, n):
                            self.fileref[trial].append(fn, n+1)
                            continue
                        # If the name contains subtitutable parts, count
                        # it as a reference to everything the substitutions
                        # could potentially match.
                        elif '{' in name:
                            pattern = re.sub(r"\{[^}]*\}", '.*', name)
                            key = self.mark_matching_resources(pattern, fn,n+1)
                            if key:
                                self.fileref[key].append(fn, n+1)
                        else:
                            candidates = []
                            for trial in self.fileref:
                                if trial.endswith(os.sep + name) and self.visible_from(trial, fn, n):
                                    key = trial
                                    self.fileref[trial].append(fn, n+1)
                                    candidates.append(trial)
                            if len(candidates) > 1:
                                print "%s: more than one definition of %s is visible here (%s)." % (Reference(fn, n), name, ", ".join(candidates))
                        if not key:
                            self.missing.append((name, Reference(fn,n+1)))
                    # Notice implicit references through attacks
                    if state == "outside":
                        if "[attack]" in line:
                            beneath = 0
                            attack_name = default_icon = None
                            have_icon = False
                        elif "name=" in line and not "no-icon" in comment:
                            attack_name = line[line.find("name=")+5:].strip()
                            default_icon = os.path.join("attacks", attack_name  + ".png")
                        elif "icon=" in line and beneath == 0:
                            have_icon = True
                        elif "[/attack]" in line:
                            if attack_name and not have_icon:
                                candidates = []
                                key = None
                                for trial in self.fileref:
                                    if trial.endswith(os.sep + default_icon) and self.visible_from(trial, fn, n):
                                        key = trial
                                        self.fileref[trial].append(fn, n+1)
                                        candidates.append(trial)
                                if len(candidates) > 1:
                                    print "%s: more than one definition of %s is visible here (%s)." % (Reference(fn, n), name, ", ".join(candidates))
                            if not key:
                                self.missing.append((default_icon, Reference(fn,n+1)))
                        elif line.strip().startswith("[/"):
                            beneath -= 1
                        elif line.strip().startswith("["):
                            beneath += 1
                rfp.close()
    def subtract(self, filelist):
        "Transplant file references in files from filelist to a new CrossRef."
        smallref = CrossRef()
        for filename in self.fileref:
            for (referrer, referlines) in self.fileref[filename].references.items():
                if referrer in filelist:
                    if filename not in smallref.fileref:
                        smallref.fileref[filename] = Reference(filename)
                    smallref.fileref[filename].references[referrer] = referlines
                    del self.fileref[filename].references[referrer]
        return smallref
    def refcount(self, name):
        "Return a reference count for the specified resource."
        try:
            return len(self.fileref[name].references.keys())
        except KeyError:
            return 0

## Namespace management
#
# This is the only part of the code that actually knows about the
# shape of the data tree.

def scopelist():
    "Return a list of (separate) package scopes, core first."
    return ["data/core"] + glob.glob("data/campaigns/*")

def is_namespace(name):
    "Is the name either a valid campaign name or core?"
    return name in map(os.path.basename, scopelist())

def namespace_directory(name):
    "Go from namespace to directory."
    if name == "core":
        return "data/core/"
    else:
        return "data/campaigns/" + name + "/"

def directory_namespace(path):
    "Go from directory to namespace."
    if path.startswith("data/core/"):
        return "core"
    elif path.startswith("data/campaigns/"):
        return path.split("/")[2]
    else:
        return None

def namespace_member(path, namespace):
    "Is a path in a specified namespace?"
    ns = directory_namespace(path)
    return ns != None and ns == namespace

def resolve_unit_cfg(namespace, resource):
    "Get the location of a specified unit in a specified scope."
    loc = namespace_directory(namespace) + "units/" + resource
    if not loc.endswith(".cfg"):
        loc += ".cfg"
    return loc

def resolve_unit_image(namespace, subdir, resource):
    "Construct a plausible location for given resource in specified namespace."
    return os.path.join(namespace_directory(namespace), "images/units", subdir, resource)

# And this is for code that does syntax transformation
baseindent = "    "

## Version-control hooks begin here.
#
# Change these if we move away from Subversion

if sys.platform.startswith("win"):
    mv = "rename"
    rm = "del"
else:
    mv = "mv"
    rm = "rm"

vcdir = ".svn"

def vcmove(src, dst):
    "Move a file under version control. Only applied to unmodified files."
    (dir, base) = os.path.split(src)
    if os.path.exists(os.path.join(dir, ".svn")):
        return "svn mv %s %s" % (src, dst)
    else:
        return mv + " " + src + " " + dst

def vcunmove(src, dst):
    "Revert the result of a previous move (before commit)."
    (dir, base) = os.path.split(src)
    if os.path.exists(os.path.join(dir, ".svn")):
        return "svn revert %s" % dst	# Revert the add at the destination
        return rm + " " + dst		# Remove the moved copy
        return "svn revert %s" % src	# Revert the deletion
    else:
        return mv + " " + dst + " " + src

def vcdelete(src):
    "Delete a file under version control."
    (dir, base) = os.path.split(src)
    if os.path.exists(os.path.join(dir, ".svn")):
        return "svn rm %s" % src
    else:
        return rm + " " + src

def vcundelete(src):
    "Revert the result of a previous delete (before commit)."
    (dir, base) = os.path.split(src)
    if os.path.exists(os.path.join(dir, ".svn")):
        return "svn revert %s" % src	# Revert the deletion
    else:
        return "echo 'can't undelete %s, not under version control'" % src

#
## Version-control hooks end here

# wmltools.py ends here
