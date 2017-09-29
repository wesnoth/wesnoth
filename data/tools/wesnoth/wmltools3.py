"""
wmltools.py -- Python routines for working with a Battle For Wesnoth WML tree

"""

from functools import total_ordering
import collections, codecs
import sys, os, re, sre_constants, hashlib, glob, gzip
import string

map_extensions   = ("map", "mask")
image_extensions = ("png", "jpg", "jpeg")
sound_extensions = ("ogg", "wav")
vc_directories = (".git", ".svn")
l10n_directories = ("l10n",)
resource_extensions = map_extensions + image_extensions + sound_extensions
image_reference = r"[A-Za-z0-9{}.][A-Za-z0-9_/+{}.\-\[\]~\*,]*\.(png|jpe?g)(?=(~.*)?)"

class Substitution(object):
    __slots__ = ["sub", "start", "end"]
    def __init__(self, sub, start, end):
        self.sub = sub
        self.start = int(start)
        self.end = int(end)
    def __repr__(self):
        return "<Class Substitution, sub={}, start={:d}, end={:d}>".format(self.sub, self.start, self.end)

def expand_square_braces(path):
    """Expands the square brackets notation to normal file names.
Yields the expanded file names, or the original path itself
if no expansion could be performed"""
    if "[" not in path: # clearly, no expansion can be done in this case
        yield path
    else:
        substitutions = [] # a matrix that will hold all the substitutions to be performed

        for i, match in enumerate(re.finditer(r"\[(.*?)\]", path)): # stop on the first closed square braces, to allow multiple substitutions
            substitutions.append([]) # a new list which will host all the substitutions for the current expansion
            for token in match.group(1).split(","):
                match_mult = re.match("(.+)\*(\d+)", token) # repeat syntax, eg. [melee*3]
                if match_mult:
                    substitutions[i].extend([Substitution(match_mult.group(1), match.start(0), match.end(0))] *
                                         int(match_mult.group(2)))
                    continue
                match_range = re.match("(\d+)~(\d+)", token) # range syntax, eg [1~4]
                if match_range:
                    before, after = int(match_range.group(1)), int(match_range.group(2))
                    incr = 1 if before <= after else -1 # to allow iterating in reversed order, eg. [4~1]
                    substitutions[i].extend([Substitution(str(n), match.start(0), match.end(0)) for n in range(before, after + incr, incr)])
                    continue
                substitutions[i].append(Substitution(token, match.start(0), match.end(0))) # no operator found

        # the purpose is to have all the subs "increasing" simultaneously
        # in other words, if a string has two square braces blocks, like [1~3] and [melee*2,ranged]
        # we want to expand them as (1,melee), (2,melee), (3,ranged)
        # for this reason we need to transpose the matrix
        substitutions = zip(*substitutions)

        for sub_array in substitutions:
            new_string = path
            for sub in reversed(sub_array): # to avoid creating "holes" in the strings
                yield new_string[:sub.start] + sub.sub + new_string[sub.end:] # these are the expanded strings

def is_root(dirname):
    "Is the specified path the filesystem root?"
    return dirname == os.sep or (os.sep == '\\' and dirname.endswith(':\\'))

def pop_to_top(whoami):
    "Pop upward to the top-level directory."
    upwards = os.getcwd().split(os.sep)
    upwards.reverse()
    for pathpart in upwards:
        # Loose match because people have things like git trees.
        if os.path.basename(pathpart).find("wesnoth") > -1:
            break
        else:
            os.chdir("..")
    else:
        print(whoami + ": must be run from within a Battle "
                         "for Wesnoth source tree.", file=sys.stderr)
        sys.exit(1)

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

def comma_split(csstring, list=None, strip="r"):
    "Split a comma-separated string, and append the entries to a list if specified."
    vallist = [x.lstrip() for x in csstring.split(",") if x.lstrip()]
    # strip=: utils::split will remove trailing whitespace from items in comma-
    # separated lists but the wml-tags.lua split function only removes leading
    # whitespace. So two flags are offered to change default behavior: one to
    # lstrip() only, the other to warn about trailing whitespace.
    if 'w' in strip:
        for item in vallist:
            if re.search('\s$', item):
                print('Trailing whitespace may be problematic: "%s" in "%s"' % (item, csstring))
    if 'l' not in strip:
        vallist = [x.rstrip() for x in vallist]
    if list is not None:
        list.extend(vallist)
    else:
        return vallist

def parse_attribute(line):
    "Parse a WML key-value pair from a line."
    if '=' not in line or line.find("#") > -1 and line.find("#") < line.find("="):
        return None
    where = line.find("=")
    leader = line[:where]
    after = line[where+1:]
    after = after.lstrip()
    if re.search("\s#", after):
        where = len(re.split("\s+#", after)[0])
        value = after[:where]
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
        "Get the names of all files under dirpath, ignoring version-control directories."
        self.forest = []
        self.dirpath = dirpath
        roots = ["campaigns", "add-ons"]
        for directory in dirpath:
            subtree = []
            rooted = False
            if os.path.isdir(directory): # So we skip .cfgs in a UMC mirror
                oldmain = os.path.join(os.path.dirname(directory), os.path.basename(directory) + '.cfg')
                if os.path.isfile(oldmain):
                    subtree.append(oldmain)
                base = os.path.basename(os.path.dirname(os.path.abspath(directory)))
                if base in roots or base == "core":
                    rooted = True
                for root, dirs, files in os.walk(directory):
                    dirs.sort()
                    dirlist = [x for x in dirs]
                    # Split out individual campaigns/add-ons into their own subtrees
                    if not rooted:
                        if os.path.basename(root) == "core":
                            rooted = True
                        elif os.path.basename(root) in roots:
                            for subdir in dirlist:
                                if subdir + '.cfg' in files:
                                    files.remove(subdir + '.cfg')
                                dirs.remove(subdir)
                                dirpath.append(os.path.join(root, subdir))
                            rooted = True
                        elif "_info.cfg" in files or "info.cfg" in files:
                            rooted = True
                            roots.append(os.path.basename(os.path.dirname(os.path.abspath(root))))
                        else:
                            stop = min(len(dirs), 5)
                            count = 0
                            for subdir in dirlist[:stop]:
                                if os.path.isfile(os.path.join(root, subdir, '_info.cfg')):
                                    count += 1
                                elif os.path.isfile(os.path.join(root, subdir, 'info.cfg')):
                                    if os.path.isfile(os.path.join(root, subdir, 'COPYING.txt')):
                                        count += 1
                            if count >= (stop // 2):
                                roots.append(os.path.basename(root))
                                for subdir in dirlist:
                                    if subdir + '.cfg' in files:
                                        files.remove(subdir + '.cfg')
                                    dirs.remove(subdir)
                                    dirpath.append(os.path.join(root, subdir))
                    subtree.extend([os.path.normpath(os.path.join(root, x)) for x in files])
            # Always look at _main.cfg first
            maincfgs = [elem for elem in subtree if elem.endswith("_main.cfg")]
            rest = [elem for elem in subtree if not elem.endswith("_main.cfg")]
            subtree = sorted(maincfgs) + sorted(rest)
            self.forest.append(subtree)
        for i in self.forest:
            # Ignore version-control subdirectories and Emacs tempfiles
            for dirkind in vc_directories + l10n_directories:
                i = [x for x in i if dirkind not in x]
            i = [x for x in i if '.#' not in x]
            i = [x for x in i if not os.path.isdir(x)]
            if exclude:
                i = [x for x in i if not re.search(exclude, x)]
            i = [x for x in i if not x.endswith("-bak")]
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
        for (directory, tree) in zip(self.dirpath, self.forest):
            for filename in tree:
                yield (directory, filename)

def iswml(filename):
    "Is the specified filename WML?"
    return filename.endswith(".cfg")

def issave(filename):
    "Is the specified filename a WML save? (Detects compressed saves too.)"
    if isresource(filename):
        return False
    if filename.endswith(".gz"):
        with gzip.open(filename) as content:
            firstline = content.readline()
    else:
        try:
            with codecs.open(filename, "r", "utf8") as content:
                firstline = content.readline()
        except UnicodeDecodeError:
            # our saves are in UTF-8, so this file shouldn't be one
            return False
    return firstline.startswith("label=")

def isresource(filename):
    "Is the specified name a resource?"
    (root, ext) = os.path.splitext(filename)
    return ext and ext[1:] in resource_extensions

def parse_macroref(start, line):
    brackdepth = parendepth = 0
    instring = False
    args = []
    arg = ""
    for i in range(start, len(line)):
        if instring:
            if line[i] == '"':
                instring = False
            arg += line[i]
        elif line[i] == '"':
            instring = not instring
            arg += line[i]
        elif line[i] == "{":
            if brackdepth > 0:
                arg += line[i]
            brackdepth += 1
        elif line[i] == "}":
            brackdepth -= 1
            if brackdepth == 0:
                if not line[i-1].isspace():
                    arg = arg.strip()
                    if arg.startswith('"') and arg.endswith('"'):
                        arg = arg[1:-1].strip()
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
            arg = arg.strip()
            if arg.startswith('"') and arg.endswith('"'):
                arg = arg[1:-1].strip()
            args.append(arg)
            arg = ""
        elif not line[i].isspace() or parendepth > 0:
            arg += line[i]
    return (args, brackdepth, parendepth)

def formaltype(f):
    # Deduce the expected type of the formal
    if f.startswith("_"):
        f = f[1:]
    if f == "SIDE" or f.endswith("_SIDE") or re.match("SIDE[0-9]", f):
        ftype = "side"
    elif f in ("SIDE", "X", "Y", "RED", "GREEN", "BLUE", "TURN", "PROB", "LAYER", "TIME", "DURATION") or f.endswith("NUMBER") or f.endswith("AMOUNT") or f.endswith("COST") or f.endswith("RADIUS") or f.endswith("_X") or f.endswith("_Y") or f.endswith("_INCREMENT") or f.endswith("_FACTOR") or f.endswith("_TIME") or f.endswith("_SIZE"):
        ftype = "numeric"
    elif f.endswith("PERCENTAGE"):
        ftype = "percentage"
    elif f in ("POSITION",) or f.endswith("_POSITION") or f == "BASE":
        ftype = "position"
    elif f.endswith("_SPAN"):
        ftype = "span"
    elif f == "SIDES" or f.endswith("_SIDES"):
        ftype = "alliance"
    elif f in ("RANGE",):
        ftype = "range"
    elif f in ("ALIGN",):
        ftype = "alignment"
    elif f in ("TYPES"):
        ftype = "types"
    elif f.startswith("ADJACENT") or f.startswith("TERRAINLIST") or f == "RESTRICTING":
        ftype = "terrain_pattern"
    elif f.startswith("TERRAIN") or f.endswith("TERRAIN"):
        ftype = "terrain_code"
    elif f in ("NAME", "NAMESPACE", "VAR", "IMAGESTEM", "ID", "FLAG", "BUILDER") or f.endswith("_NAME") or f.endswith("_ID") or f.endswith("_VAR") or f.endswith("_OVERLAY"):
        ftype = "name"
    elif f in ("ID_STRING", "NAME_STRING", "DESCRIPTION", "IPF"):
        ftype = "optional_string"
    elif f in ("STRING", "TYPE", "TEXT") or f.endswith("_STRING") or f.endswith("_TYPE") or f.endswith("_TEXT"):
        ftype = "string"
    elif f.endswith("IMAGE") or f == "PROFILE":
        ftype = "image"
    elif f.endswith("MUSIC",) or f.endswith("SOUND"):
        ftype = "sound"
    elif f.endswith("FILTER",):
        ftype = "filter"
    elif f == "WML" or f.endswith("_WML"):
        ftype = "wml"
    elif f in ("AFFIX", "POSTFIX", "ROTATION") or f.endswith("AFFIX"):
        ftype = "affix"
    # The regexp case avoids complaints about some wacky terrain macros.
    elif f.endswith("VALUE") or re.match("[ARS][0-9]", f):
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
    elif re.match(r"0\.[0-9]+\Z", a):
        atype = "percentage"
    elif re.match(r"-?[0-9]+,-?[0-9]+\Z", a):
        atype = "position"
    elif re.match(r"([0-9]+\-[0-9]+,?|[0-9]+,?)+\Z", a):
        atype = "span"
    elif a in ("melee", "ranged"):
        atype = "range"
    elif a in ("lawful", "neutral", "chaotic", "liminal"):
        atype = "alignment"
    elif a.startswith("{") or a.endswith("}") or a.startswith("$"):
        atype = None # Can't tell -- it's a macro expansion
    elif re.match(image_reference, a) or a == "unit_image":
        atype = "image"
    elif re.match(r"(\*|[A-Z][a-z]+)\^([A-Z][a-z\\|/]+\Z)?", a):
        atype = "terrain_code"
    elif a.endswith(".wav") or a.endswith(".ogg"):
        atype = "sound"
    elif a.startswith('"') and a.endswith('"') or (a.startswith("_") and a[1] not in string.ascii_lowercase):
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
        # Here's the compatibility logic.  First, we catch the situations
        # in which a more restricted actual type matches a more general
        # formal one.  Then we have a fallback rule checking for type
        # equality or wildcarding.
        ftype = formaltype(f)
        atype = actualtype(a)
        if ftype == "any":
            pass
        elif (atype == "numeric" or a == "global") and ftype == "side":
            pass
        elif atype in ("filter", "empty") and ftype == "wml":
            pass
        elif atype in ("numeric", "position") and ftype == "span":
            pass
        elif atype in ("shortname", "name", "empty", "stringliteral") and ftype == "affix":
            pass
        elif atype in ("shortname", "name", "stringliteral") and ftype == "string":
            pass
        elif atype in ("shortname", "name", "string", "stringliteral", "empty") and ftype == "optional_string":
            pass
        elif atype in ("shortname",) and ftype == "terrain_code":
            pass
        elif atype in ("numeric", "position", "span", "empty") and ftype == "alliance":
            pass
        elif atype in ("terrain_code", "shortname", "name") and ftype == "terrain_pattern":
            pass
        elif atype in ("string", "shortname", "name") and ftype == "types":
            pass
        elif atype in ("numeric", "percentage") and ftype == "percentage":
            pass
        elif atype == "range" and ftype == "name":
            pass
        elif atype != ftype and ftype is not None and atype is not None:
            return False
    return True

# the total_ordering decorator from functools allows to define only two comparison
# methods, and Python generates the remaining methods
# it comes with a speed penalty, but the alternative is defining six methods by hand...
@total_ordering
class Reference:
    "Describes a location by file and line."
    def __init__(self, namespace, filename, lineno=None, docstring=None, args=None):
        self.namespace = namespace
        self.filename = filename
        self.lineno = lineno
        self.docstring = docstring
        self.args = args
        self.references = collections.defaultdict(list)
        self.undef = None

    def append(self, fn, n, a=None):
        self.references[fn].append((n, a))

    def dump_references(self):
        "Dump all known references to this definition."
        for (file, refs) in self.references.items():
            print("    %s: %s" % (file, repr([x[0] for x in refs])[1:-1]))

    def __eq__(self, other):
        return self.filename == other.filename and self.lineno == other.lineno

    def __gt__(self, other):
        # Major sort by file, minor by line number.  This presumes that the
        # files correspond to coherent topics and gives us control of the
        # sequence.
        if self.filename == other.filename:
            return self.lineno > other.lineno
        else:
            return self.filename > other.filename

    def mismatches(self):
        copy = Reference(self.namespace, self.filename, self.lineno, self.docstring, self.args)
        copy.undef = self.undef
        for filename in self.references:
            mis = [(ln,a) for (ln,a) in self.references[filename] if a is not None and not argmatch(self.args, a)]
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
    file_reference = re.compile(r"[A-Za-z0-9{}.][A-Za-z0-9_/+{}.@\-\[\],~\*]*(" + "|".join(resource_extensions) + ")(?=(~.*)?)")
    tag_parse = re.compile("\s*([a-z_]+)\s*=(.*)")
    def mark_matching_resources(self, pattern, fn, n):
        "Mark all definitions matching a specified pattern with a reference."
        pattern = pattern.replace("+", r"\+")
        pattern = os.sep + pattern + "$"
        if os.sep == "\\":
            pattern = pattern.replace("\\", "\\\\")
        try:
            pattern = re.compile(pattern)
        except sre_constants.error:
            print("wmlscope: confused by %s" % pattern, file=sys.stderr)
            return None
        key = None
        for trial in self.fileref:
            if pattern.search(trial) and self.visible_from(trial, fn, n):
                key = trial
                self.fileref[key].append(fn, n)
        return key
    def visible_from(self, defn, fn, n):
        "Is specified definition visible from the specified file and line?"
        if isinstance(defn, str):
            defn = self.fileref[defn]
        if defn.undef is not None:
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
    def scan_for_definitions(self, namespace, filename):
        ignoreflag = False
        conditionalsflag = False
        with codecs.open(filename, "r", "utf8") as dfp:
            state = "outside"
            latch_unit = in_base_unit = in_theme = False
            for (n, line) in enumerate(dfp):
                if self.warnlevel > 1:
                    print(repr(line)[1:-1])
                if line.strip().startswith("#textdomain"):
                    continue
                m = re.search("# *wmlscope: warnlevel ([0-9]*)", line)
                if m:
                    self.warnlevel = int(m.group(1))
                    print('"%s", line %d: warnlevel set to %d (definition-gathering pass)' \
                         % (filename, n+1, self.warnlevel))
                    continue
                m = re.search("# *wmlscope: set *([^=]*)=(.*)", line)
                if m:
                    prop = m.group(1).strip()
                    value = m.group(2).strip()
                    if namespace not in self.properties:
                        self.properties[namespace] = {}
                    self.properties[namespace][prop] = value
                m = re.search("# *wmlscope: prune (.*)", line)
                if m:
                    name = m.group(1)
                    if self.warnlevel >= 2:
                        print('"%s", line %d: pruning definitions of %s' \
                              % (filename, n+1, name ))
                    if name not in self.xref:
                        print("wmlscope: can't prune undefined macro %s" % name, file=sys.stderr)
                    else:
                        self.xref[name] = self.xref[name][:1]
                    continue
                if "# wmlscope: start conditionals" in line:
                    if self.warnlevel > 1:
                        print('"%s", line %d: starting conditionals' \
                              % (filename, n+1))
                    conditionalsflag = True
                elif "# wmlscope: stop conditionals" in line:
                    if self.warnlevel > 1:
                        print('"%s", line %d: stopping conditionals' \
                              % (filename, n+1))
                    conditionalsflag = False
                if "# wmlscope: start ignoring" in line:
                    if self.warnlevel > 1:
                        print('"%s", line %d: starting ignoring (definition pass)' \
                              % (filename, n+1))
                    ignoreflag = True
                elif "# wmlscope: stop ignoring" in line:
                    if self.warnlevel > 1:
                        print('"%s", line %d: stopping ignoring (definition pass)' \
                              % (filename, n+1))
                    ignoreflag = False
                elif ignoreflag:
                    continue
                if line.strip().startswith("#define"):
                    tokens = line.split()
                    if len(tokens) < 2:
                        print('"%s", line %d: malformed #define' \
                              % (filename, n+1), file=sys.stderr)
                    else:
                        name = tokens[1]
                        here = Reference(namespace, filename, n+1, line, args=tokens[2:])
                        here.hash = hashlib.md5()
                        here.docstring = line.lstrip()[8:] # Strip off #define_
                        state = "macro_header"
                    continue
                elif state != 'outside' and line.strip().endswith("#enddef"):
                    here.hash.update(line.encode("utf8"))
                    here.hash = here.hash.digest()
                    if name in self.xref:
                        for defn in self.xref[name]:
                            if not self.visible_from(defn, filename, n+1):
                                continue
                            elif conditionalsflag:
                                continue
                            elif defn.hash != here.hash:
                                print("%s: overrides different %s definition at %s" \
                                        % (here, name, defn), file=sys.stderr)
                            elif self.warnlevel > 0:
                                print("%s: duplicates %s definition at %s" \
                                        % (here, name, defn), file=sys.stderr)
                    if name not in self.xref:
                        self.xref[name] = []
                    self.xref[name].append(here)
                    state = "outside"
                elif state == "macro_header" and line.strip() and line.strip()[0] != "#":
                    state = "macro_body"
                if state == "macro_header":
                    # Ignore macro header commends that are pragmas
                    if "wmlscope" not in line and "wmllint:" not in line:
                        here.docstring += line.lstrip()[1:]
                if state in ("macro_header", "macro_body"):
                    here.hash.update(line.encode("utf8"))
                elif line.strip().startswith("#undef"):
                    tokens = line.split()
                    name = tokens[1]
                    if name in self.xref and self.xref[name]:
                        self.xref[name][-1].undef = n+1
                    else:
                        print("%s: unbalanced #undef on %s" \
                              % (Reference(namespace, filename, n+1), name))
                if state == 'outside':
                    if '[unit_type]' in line:
                        latch_unit = True
                    elif '[/unit_type]' in line:
                        latch_unit = False
                    elif '[base_unit]' in line:
                        in_base_unit = True
                    elif '[/base_unit]' in line:
                        in_base_unit = False
                    elif '[theme]' in line:
                        in_theme = True
                    elif '[/theme]' in line:
                        in_theme = False
                    elif latch_unit and not in_base_unit and not in_theme and "id" in line:
                        m = CrossRef.tag_parse.search(line)
                        if m and m.group(1) == "id":
                            uid = m.group(2)
                            if uid not in self.unit_ids:
                                self.unit_ids[uid] = []
                            self.unit_ids[uid].append(Reference(namespace, filename, n+1))
                            latch_unit= False
    def __init__(self, dirpath=[], exclude="", warnlevel=0, progress=False):
        "Build cross-reference object from the specified filelist."
        self.filelist = Forest(dirpath, exclude)
        self.dirpath = [x for x in dirpath if not re.search(exclude, x)]
        self.warnlevel = warnlevel
        self.xref = {}
        self.fileref = {}
        self.noxref = False
        self.properties = {}
        self.unit_ids = {}
        all_in = []
        if self.warnlevel >=2 or progress:
            print("*** Beginning definition-gathering pass...")
        for (namespace, filename) in self.filelist.generator():
            all_in.append((namespace, filename))
            if self.warnlevel > 1:
                print(filename + ":")
            if progress:
                print(filename)
            if isresource(filename):
                self.fileref[filename] = Reference(namespace, filename)
            elif iswml(filename):
                # It's a WML file, scan for macro definitions
                self.scan_for_definitions(namespace, filename)
            elif filename.endswith(".def"):
                # It's a list of names to be considered defined
                self.noxref = True
                with codecs.open(filename, "r", "utf8") as dfp:
                    for line in dfp:
                        self.xref[line.strip()] = True
        # Next, decorate definitions with all references from the filelist.
        self.unresolved = []
        self.missing = []
        formals = []
        state = "outside"
        if self.warnlevel >=2 or progress:
            print("*** Beginning reference-gathering pass...")
        for (ns, fn) in all_in:
            if progress:
                print(filename)
            if iswml(fn):
                with codecs.open(fn, "r", "utf8") as rfp:
                    attack_name = None
                    beneath = 0
                    ignoreflag = False
                    for (n, line) in enumerate(rfp):
                        if line.strip().startswith("#define"):
                            formals = line.strip().split()[2:]
                        elif line.startswith("#enddef"):
                            formals = []
                        comment = ""
                        if '#' in line:
                            if "# wmlscope: start ignoring" in line:
                                if self.warnlevel > 1:
                                    print('"%s", line %d: starting ignoring (reference pass)' \
                                          % (fn, n+1))
                                ignoreflag = True
                            elif "# wmlscope: stop ignoring" in line:
                                if self.warnlevel > 1:
                                    print('"%s", line %d: stopping ignoring (reference pass)' \
                                          % (fn, n+1))
                                ignoreflag = False
                            m = re.search("# *wmlscope: self.warnlevel ([0-9]*)", line)
                            if m:
                                self.warnlevel = int(m.group(1))
                                print('"%s", line %d: self.warnlevel set to %d (reference-gathering pass)' \
                                     % (fn, n+1, self.warnlevel))
                                continue
                            fields = line.split('#')
                            line = fields[0]
                            if len(fields) > 1:
                                comment = fields[1]
                        if ignoreflag or not line:
                            continue
                        # Find references to macros
                        for match in re.finditer(CrossRef.macro_reference, line):
                            name = match.group(1)
                            candidates = []
                            if self.warnlevel >=2:
                                print('"%s", line %d: seeking definition of %s' \
                                      % (fn, n+1, name))
                            if name in formals:
                                continue
                            elif name in self.xref:
                                # Count the number of actual arguments.
                                # Set args to None if the call doesn't
                                # close on this line
                                (args, brackdepth, parendepth) = parse_macroref(match.start(0), line)
                                if brackdepth > 0 or parendepth > 0:
                                    args = None
                                else:
                                    args.pop(0)
                                #if args:
                                #    print('"%s", line %d: args of %s is %s' \
                                #          % (fn, n+1, name, args))
                                # Figure out which macros might resolve this
                                for defn in self.xref[name]:
                                    if self.visible_from(defn, fn, n+1):
                                        defn.append(fn, n+1, args)
                                        candidates.append(str(defn))
                                if len(candidates) > 1:
                                    print("%s: more than one definition of %s is visible here (%s)." % (Reference(ns, fn, n), name, "; ".join(candidates)))
                            if len(candidates) == 0:
                                self.unresolved.append((name,Reference(ns,fn,n+1)))
                        # Don't be fooled by HTML image references in help strings.
                        if "<img>" in line:
                            continue
                        # Find references to resource files
                        for match in re.finditer(CrossRef.file_reference, line):
                            for name in expand_square_braces(match.group(0)):
                                # Catches maps that look like macro names.
                                if (name.endswith(".map") or name.endswith(".mask")) and name[0] == '{':
                                    name = name[1:]
                                if os.sep == "\\":
                                    name = name.replace("/", "\\")
                                key = None
                                # If name is already in our resource list, it's easy.
                                if name in self.fileref and self.visible_from(name, fn, n):
                                    self.fileref[name].append(fn, n+1)
                                    continue
                                # If the name contains substitutable parts, count
                                # it as a reference to everything the substitutions
                                # could potentially match.
                                elif '{' in name or '@' in name:
                                    pattern = re.sub(r"(\{[^}]*\}|@R0|@V)", '.*', name)
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
                                        print("%s: more than one resource matching %s is visible here (%s)." % (Reference(ns,fn, n), name, ", ".join(candidates)))
                                if not key:
                                    self.missing.append((name, Reference(ns,fn,n+1)))
                        # Notice implicit references through attacks
                        if state == "outside":
                            have_icon = False
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
                                        print("%s: more than one definition of %s is visible here (%s)." % (Reference(ns,fn, n), name, ", ".join(candidates)))
                                if not key:
                                    self.missing.append((default_icon, Reference(ns,fn,n+1)))
                            elif line.strip().startswith("[/"):
                                beneath -= 1
                            elif line.strip().startswith("["):
                                beneath += 1
        # Check whether each namespace has a defined export property
        for namespace in self.dirpath:
            if namespace not in self.properties or "export" not in self.properties[namespace]:
                print("warning: %s has no export property" % namespace)
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
            return len(self.fileref[name].references)
        except KeyError:
            return 0

#
# String translations from po files.  The advantage of this code is that it
# does not require the gettext binary message catalogs to have been compiled.
# The disadvantage is that it eats lots of core!
#


class TranslationError(Exception):
    def __init__(self, textdomain, isocode):
        self.isocode = isocode
        self.textdomain = textdomain
    def __str__(self):
        return "No translations found for %s/%s.\n" % (
            self.textdomain, self.isocode)

class Translation(dict):
    "Parses a po file to create a translation dictionary."
    def __init__(self, textdomain, isocode, topdir=""):
        self.textdomain = textdomain
        self.isocode = isocode
        self.gettext = {}
        if self.isocode != "C":
            isocode2 = isocode[:isocode.rfind("_")]
            for code in [isocode, isocode2]:
                fn = "po/%s/%s.po" % (textdomain, code)
                if topdir: fn = os.path.join(topdir, fn)
                try:
                    f = file(fn)
                    break
                except IOError:
                    pass
            else:
                raise TranslationError(textdomain, self.isocode)

            expect = False
            fuzzy = "#, fuzzy\n"
            gettext = f.read().decode("utf8")
            matches = re.compile("""(msgid|msgstr)((\s*".*?")+)""").finditer(gettext)
            msgid = ""
            for match in matches:
                text = "".join(re.compile('"(.*?)"').findall(match.group(2)))
                if match.group(1) == "msgid":
                    msgid = text.replace("\\n", "\n")
                    expect = gettext[match.start(1) - len(fuzzy):match.start(1)] != fuzzy
                elif expect:
                    self.gettext[msgid] = text.replace("\\n", "\n")
    def get(self, key, dflt):
        if self.isocode == "C":
            if key:
                return key[key.find("^") + 1:]
            return "?"
        else:
            t = self.gettext.get(key, dflt)
            if not t:
                if key:
                    return key[key.find("^") + 1:]
                return "?"
            return t
    def __getitem__(self, key):
        if self.isocode == "C":
            return key
        else:
            return self.gettext[key]
    def __contains__(self, key):
        if self.isocode == "C":
            return True
        else:
            return key in self.gettext

class Translations:
    "Wraps around Translation to support multiple languages and domains."
    def __init__(self, topdir = ""):
        self.translations = {}
        self.topdir = topdir
    def get(self, textdomain, isocode, key, default):
        t = (textdomain, isocode)
        if not t in self.translations:
            try:
                self.translations[t] = Translation(textdomain, isocode, self.topdir)
            except TranslationError as e:
                print(str(e), file=sys.stderr)
                self.translations[t] = Translation(textdomain, "C", self.topdir)
        result = self.translations[t].get(key, default)
        return result

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
    return ns is not None and ns == namespace

def resolve_unit_cfg(namespace, utype, resource=None):
    "Get the location of a specified unit in a specified scope."
    if resource:
        resource = os.path.join(utype, resource)
    else:
        resource = utype
    loc = namespace_directory(namespace) + "units/" + resource
    if not loc.endswith(".cfg"):
        loc += ".cfg"
    return loc

def resolve_unit_image(namespace, subdir, resource):
    "Construct a plausible location for given resource in specified namespace."
    return os.path.join(namespace_directory(namespace), "images/units", subdir, resource)

# And this is for code that does syntax transformation
baseindent = "    "

# a constant to detect version control directories
vcdir = ".git"

# wmltools.py ends here
