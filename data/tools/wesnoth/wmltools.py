"""
wmltools.py -- Python routines for working with a Battle For Wesnoth WML tree

"""

import sys, os, re, sre_constants, md5, glob

resource_extensions = ("png", "jpg", "ogg", "wav", "map", "mask")
image_reference = r"[A-Za-z0-9{}.][A-Za-z0-9_/+{}.-]*\.(png|jpg)(?=(~.*)?)"

def string_strip(value):
    "String-strip the value"
    if value.startswith('"'):
        value = value[1:]
        if value.endswith('"'):
            value = value[:-1]
    if value.startswith("'"):
        value = value[1:]
        if value.endswith("'"):
            value = value[:-1]
    return value

def attr_strip(value):
    "Strip away an (optional) translation mark and string quotes."
    value = value.strip()
    if value.startswith('_'):
        value = value[1:]
    value = value.strip()
    return string_strip(value)

def parse_attribute(str):
    "Parse a WML key-value pair from a line."
    if '=' not in str:
        return None
    where = str.find("=")
    leader = str[:where]
    after = str[where+1:]
    after = after.lstrip()
    if "#" in after:
        where = after.find("#")
        while after[where-1] in (" ", "\t"):
            where -= 1
        value = after[:where+1]
        comment = after[where:]
    else:
        value = after.rstrip()
        comment = ""
    # Return four fields: stripped key, part of line before value,
    # value, trailing whitespace and comment.
    return (leader.strip(), leader+"=", string_strip(value), comment)

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
        for (dir, tree) in zip(self.dirpath, self.forest):
            for filename in tree:
                yield (dir, filename)

def iswml(filename):
    "Is the specified filename WML?"
    return filename.endswith(".cfg")

def isresource(filename):
    "Is the specified name a resource?"
    (root, ext) = os.path.splitext(filename)
    return ext and ext[1:] in resource_extensions

def formaltype(f):
    # Deduce the expected type of the formal
    if f in ("SIDE", "X", "Y", "RED", "GREEN", "BLUE", "TURN", "RADIUS") or f.endswith("NUMBER") or f.endswith("AMOUNT") or f.endswith("_X") or f.endswith("_Y"):
        ftype = "numeric"
    elif f in ("POSITION",):
        ftype = "position"
    elif f.endswith("_SPAN"):
        ftype = "span"
    elif f in ("RANGE",):
        ftype = "range"
    elif f in ("ALIGN",):
        ftype = "alignment"
    elif f in ("TERRAIN",):
        ftype = "terrain_code"
    elif f in ("NAME", "VAR", "IMAGESTEM", "ID") or f.endswith("_NAME"):
        ftype = "name"
    elif f in ("STRING", "TYPE", "TEXT"):
        ftype = "string"
    elif f in ("ID_STRING", "NAME_STRING", "DESCRIPTION"):
        ftype = "optional_string"
    elif f.endswith("IMAGE") or f == "PROFILE":
        ftype = "image"
    elif f in ("MUSIC",) or f.endswith("SOUND"):
        ftype = "sound"
    elif f in ("FILTER",):
        ftype = "filter"
    elif f.endswith("_WML"):
        ftype = "wml"
    elif f.endswith("VALUE"):
        ftype = "any"
    else:
        ftype = None
    return ftype

def actualtype(a):
    if a is None:
        return None
    # Deduce the type of the actual
    if a.isdigit() or a.startswith("-") and a[1:].isdigit():
        atype = "numeric"
    elif re.match(r"[0-9]+,[0-9]+\Z", a):
        atype = "position"
    elif re.match(r"([0-9]+\-[0-9]+,?|[0-9]+,?)+\Z", a):
        atype = "span"
    elif a in ("melee", "ranged"):
        atype = "range"
    elif a in ("lawful", "neutral", "chaotic"):
        atype = "alignment"
    elif a.startswith("{") and a.endswith("}") or a.startswith("$"):
        atype = None	# Can't tell -- it's a macro expansion
    elif re.match(image_reference, a):
        atype = "image"
    elif re.match(r"[A-Z][a-z]+\^[A-Z][a-z\\|/]+\Z", a):
        atype = "terrain_code"
    elif a.endswith(".wav") or a.endswith(".ogg"):
        atype = "sound"
    elif a.startswith('"') and a.endswith('"') or a.startswith("_"):
        atype = "stringliteral"
    elif "=" in a:
        atype = "filter"
    elif re.match(r"[A-Z][a-z][a-z]?\Z", a):
        atype = "shortname"
    elif a == "":
        atype = "empty"
    elif not ' ' in a:
        atype = "name"
    else:
        atype = "string"
    return atype

def argmatch(formals, actuals):
    if len(formals) != len(actuals):
        return False
    for (f, a) in zip(formals, actuals):
        ftype = formaltype(f)
        atype = actualtype(a)
        # Here's the compatibility logic.  First, we catch the situations
        # in which a more restricted actual type matches a more general
        # formal one.  Then we have a fallback rule checking for type
        # equality or wildcarding.
        if ftype == "any":
            pass
        elif atype in ("filter", "empty") and ftype == "wml":
            pass
        elif atype in ("numeric", "position") and ftype == "span":
            pass
        elif atype in ("shortname", "name", "stringliteral") and ftype == "string":
            pass
        elif atype in ("shortname", "name", "string", "stringliteral", "empty") and ftype == "optional_string":
            pass
        elif atype in ("shortname",) and ftype == "terrain_code":
            pass
        elif atype != ftype and ftype is not None and atype is not None:
            return False
    return True

class Reference:
    "Describes a location by file and line."
    def __init__(self, namespace, filename, lineno=None, docstring=None, args=None):
        self.namespace = namespace
        self.filename = filename
        self.lineno = lineno
        self.docstring = docstring
        self.args = args
        self.references = {}
        self.undef = None
    def append(self, fn, n, a=None):
        if fn not in self.references:
            self.references[fn] = []
        self.references[fn].append((n, a))
    def dump_references(self):
        for (file, refs) in self.references.items():
            print "    %s: %s" % (file, `map(lambda x: x[0], refs)`[1:-1])
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
    def mismatches(self):
        copy = Reference(self.namespace, self.filename, self.lineno, self.docstring, self.args)
        copy.undef = self.undef
        for filename in self.references:
            mis = filter(lambda (ln, a): a is not None and not argmatch(self.args, a), self.references[filename])
            if mis:
                copy.references[filename] = mis
        return copy
    def __str__(self):
        if self.lineno:
            return '"%s", line %d' % (self.filename, self.lineno)
        else:
            return self.filename
    __repr__ = __str__

class CrossRef:
    macro_reference = re.compile(r"\{([A-Z_][A-Za-z0-9_:]*)(?!\.)\b")
    file_reference =  re.compile(r"[A-Za-z0-9{}.][A-Za-z0-9_/+{}.-]*\.(" + "|".join(resource_extensions) + ")(?=(~.*)?)")
    tag_parse = re.compile("\s*([a-z_]+)\s*=(.*)") 
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
        if self.exports(defn.namespace):
            # Macros and resources in subtrees with export=yes are global
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
        self.filelist = Forest(dirpath, exclude)
        self.dirpath = filter(lambda x: not re.search(exclude, x), dirpath)
        self.xref = {}
        self.fileref = {}
        self.noxref = False
        self.properties = {}
        self.unit_ids = {}
        if warnlevel >=2:
            print "*** Beginning definition-gathering pass..."
        for (namespace, filename) in self.filelist.generator():
            if warnlevel > 1:
                print filename + ":"
            if isresource(filename):
                self.fileref[filename] = Reference(namespace, filename)
            elif iswml(filename):
                # It's a WML file, scan for macro definitions
                dfp = open(filename)
                state = "outside"
                latch_unit = in_base_unit = in_theme = False
                for (n, line) in enumerate(dfp):
                    if warnlevel > 1:
                        print `line`[1:-1]
                    if line.strip().startswith("#textdomain"):
                        continue
                    m = re.search("# *wmlscope: warnlevel ([0-9]*)", line)
                    if m:
                        warnlevel = int(m.group(1))
                        print '"%s", line %d: warnlevel set to %d (definition-gathering pass)' \
                             % (filename, n+1, warnlevel)
                        continue
                    m = re.search("# *wmlscope: set *([^=]*)=(.*)", line)
                    if m:
                        prop = m.group(1).strip()
                        value = m.group(2).strip()
                        if namespace not in self.properties:
                            self.properties[namespace] = {}
                        self.properties[namespace][prop] = value
                    if line.strip().startswith("#define"):
                        tokens = line.split()
                        name = tokens[1]
                        here = Reference(namespace, filename, n+1, line, args=tokens[2:])
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
                                  % (Reference(namespace, filename, n+1), name)
                    if state == 'outside':
                        if '[unit_type]' in line:
                            latch_unit = True
                        elif '[/unit_type]' in line:
                            latch_unit = False
                        elif '[base_unit]' in line:
                            in_base_unit = True
                        elif '[/base_unit]' in line:
                            in_base__unit = True
                        elif '[theme]' in line:
                            in_theme = True
                        elif '[/theme]' in line:
                            in_base__unit = True
                        elif latch_unit and not in_base_unit and not in_theme and "id" in line:
                            m = CrossRef.tag_parse.search(line)
                            if m and m.group(1) == "id":
                                uid = m.group(2)
                                if uid not in self.unit_ids:
                                    self.unit_ids[uid] = []
                                self.unit_ids[uid].append(Reference(namespace, filename, n+1))
                                latch_unit= False
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
        if warnlevel >=2:
            print "*** Beginning reference-gathering pass..."
        for (ns, fn) in self.filelist.generator():
            if iswml(fn):
                rfp = open(fn)
                attack_name = None
                beneath = 0
                for (n, line) in enumerate(rfp):
                    if line.strip().startswith("#define"):
                        formals = line.strip().split()[2:]
                    elif line.startswith("#enddef"):
                        formals = []
                    comment = ""
                    if '#' in line:
                        m = re.search("# *wmlscope: warnlevel ([0-9]*)", line)
                        if m:
                            warnlevel = int(m.group(1))
                            print '"%s", line %d: warnlevel set to %d (reference-gathering pass)' \
                                 % (fn, n+1, warnlevel)
                            continue
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
                        if warnlevel >=2:
                            print '"%s", line %d: seeking definition of %s' \
                                  % (fn, n+1, name)
                        if name in formals:
                            continue
                        elif name in self.xref:
                            # Count the number of actual arguments.
                            # Set args to None if the call doesn't
                            # close on this line
                            brackdepth = parendepth = 0
                            instring = False
                            args = []
                            arg = ""
                            for i in range(match.start(0), len(line)):
                                if instring:
                                    if line[i] == '"':
                                        instring = False
                                    else:
                                        arg += line[i]
                                elif line[i] == '"':
                                    instring = not instring
                                elif line[i] == "{":
                                    if brackdepth > 0:
                                        arg += line[i]
                                    brackdepth += 1
                                elif line[i] == "}":
                                    brackdepth -= 1
                                    if brackdepth == 0:
                                        if not line[i-1].isspace():
                                            args.append(arg)
                                            arg = ""
                                        break
                                    else:
                                        arg += line[i]
                                elif line[i] == "(":
                                    parendepth += 1
                                elif line[i] == ")":
                                    parendepth -= 1
                                elif not line[i-1].isspace() and \
                                     line[i].isspace() and \
                                     brackdepth == 1 and \
                                     parendepth == 0:
                                    args.append(arg)
                                    arg = ""
                                elif not line[i].isspace():
                                    arg += line[i]
                            if brackdepth > 0 or parendepth > 0:
                                args = None
                            else:
                                args.pop(0)
                            #if args:
                            #    print '"%s", line %d: args of %s is %s' \
                            #          % (fn, n+1, name, args)
                            # Figure out which macros might resolve this
                            for defn in self.xref[name]:
                                if self.visible_from(defn, fn, n+1):
                                    candidates += 1
                                    defn.append(fn, n+1, args)
                            if candidates > 1:
                                print "%s: more than one definition of %s is visible here." % (Reference(ns, fn, n), name)
                        if candidates == 0:
                            self.unresolved.append((name,Reference(ns,fn,n+1,args=args)))
                    # Find references to resource files
                    for match in re.finditer(CrossRef.file_reference, line):
                        name = match.group(0)
                        # Catches maps that look like macro names.
                        if (name.endswith(".map") or name.endswith(".mask")) and name[0] == '{':
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
                                print "%s: more than one definition of %s is visible here (%s)." % (Reference(ns,fn, n), name, ", ".join(candidates))
                        if not key:
                            self.missing.append((name, Reference(ns,fn,n+1)))
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
                                    print "%s: more than one definition of %s is visible here (%s)." % (Reference(ns,fn, n), name, ", ".join(candidates))
                            if not key:
                                self.missing.append((default_icon, Reference(ns,fn,n+1)))
                        elif line.strip().startswith("[/"):
                            beneath -= 1
                        elif line.strip().startswith("["):
                            beneath += 1
                rfp.close()
        # Check whether each namespace has a defined export property
        for namespace in self.dirpath:
            if namespace not in self.properties or "export" not in self.properties[namespace]:
                print "warning: %s has no export property" % namespace
    def exports(self, namespace):
        return namespace in self.properties and self.properties[namespace].get("export") == "yes"
    def subtract(self, filelist):

        "Transplant file references in files from filelist to a new CrossRef."
        smallref = CrossRef()
        for filename in self.fileref:
            for (referrer, referlines) in self.fileref[filename].references.items():
                if referrer in filelist:
                    if filename not in smallref.fileref:
                        smallref.fileref[filename] = Reference(None, filename)
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
