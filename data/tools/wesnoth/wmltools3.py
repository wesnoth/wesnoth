#!/usr/bin/env python3

"""
wmltools.py -- Python routines for working with a Battle For Wesnoth WML tree

"""

from functools import total_ordering
import collections
import sys, os, re, sre_constants, hashlib, glob, gzip
import string
import enum

# Extensions
# Ordering is important, see default extensions below
map_extensions   = ("map", "mask")
wml_extensions   = ("cfg",)
image_extensions = ("png", "jpg", "jpeg", "webp")
sound_extensions = ("ogg", "wav")

# Default extensions
default_map_extension  = "." + map_extensions[0] # ".map" at the moment
default_mask_extension = "." + map_extensions[1] # ".mask" at the moment
default_wml_extension  = "." + wml_extensions[0] # ".cfg" at the moment
resource_extensions = map_extensions + image_extensions + sound_extensions
image_reference = r"[A-Za-z0-9{}.][A-Za-z0-9_/+{}.\-\[\]~\*,]*\.(png|jpe?g|webp)(?=(~.*)?)"

# Directories
l10n_directories = ("l10n",)
vc_directories   = (".git", ".svn")

# Misc files and extensions
misc_files_extensions = ("-bak", ".DS_Store", "Thumbs.db") # These files and extensions should be included in the `default_blacklist` in filesystem.hpp.

EQUALS = '='
QUOTE = '"'
OPEN_BRACE = '{'
CLOSE_BRACE = '}'
OPEN_PARENS = '('
CLOSE_PARENS = ')'

class Substitution(object):
    __slots__ = ["sub", "start", "end"]
    def __init__(self, sub, start, end):
        self.sub = sub
        self.start = int(start)
        self.end = int(end)
    def __repr__(self):
        return "<Class Substitution, sub={}, start={:d}, end={:d}>".format(self.sub, self.start, self.end)

def split_filenames(match):
    filename_group = match.group(1)
    if ',' not in filename_group:
        yield filename_group
    elif '[' not in filename_group:
        yield from filename_group.split(',')
    else:
        bracket_depth = 0
        start_ix = 0

        for ix, ch in enumerate(filename_group):
            if ch == '[':
                bracket_depth += 1
            elif ch == ']':
                bracket_depth -= 1
            elif ch == ',' and bracket_depth == 0:
                yield filename_group[start_ix:ix]
                start_ix = ix + 1
        yield filename_group[start_ix:]

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
                match_mult = re.match(r"(.+)\*(\d+)", token) # repeat syntax, eg. [melee*3]
                if match_mult:
                    substitutions[i].extend([Substitution(match_mult.group(1), match.start(0), match.end(0))] *
                                         int(match_mult.group(2)))
                    continue
                match_range = re.match(r"(\d+)~(\d+)", token) # range syntax, eg [1~4]
                if match_range:
                    before, after = int(match_range.group(1)), int(match_range.group(2))
                    # does one of the limits have leading zeros? If so, detect the length of the numbers used
                    if match_range.group(1).startswith("0") or match_range.group(2).startswith("0"):
                        leading_zeros = max(len(match_range.group(1)), len(match_range.group(2)))
                    else:
                        leading_zeros = 0
                    incr = 1 if before <= after else -1 # to allow iterating in reversed order, eg. [4~1]
                    # previously this code used a mere casting to str
                    # string formatting allows proper handling of leading zeros, if any
                    fmt_string = "{:0" + str(leading_zeros) + "d}"
                    substitutions[i].extend([Substitution(fmt_string.format(n), match.start(0), match.end(0)) for n in range(before, after + incr, incr)])
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
                new_string = new_string[:sub.start] + sub.sub + new_string[sub.end:] # these are the expanded strings
            yield new_string

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
            if re.search(r"\s$", item):
                print('Trailing whitespace may be problematic: "%s" in "%s"' % (item, csstring))
    if 'l' not in strip:
        vallist = [x.rstrip() for x in vallist]
    if list is not None:
        list.extend(vallist)
    else:
        return vallist

def parse_attribute(line):
    "Parse a WML key-value pair from a line."
    where = line.find("=")
    # Ignore lines with a # or " before the first =. They're likely line continuations that won't be parsed correctly.
    if '=' not in line or -1 < line.find("#") < where or -1 < line.find("\"") < where:
        return None
    leader = line[:where]
    after = line[where+1:]
    if re.search(r"\s#", after):
        where = len(re.split(r"\s+#", after)[0])
        value = after[:where].lstrip()
        comment = after[where:]
    else:
        value = after.strip()
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
                oldmain = os.path.join(os.path.dirname(directory), os.path.basename(directory) + default_wml_extension)
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
                                if subdir + default_wml_extension in files:
                                    files.remove(subdir + default_wml_extension)
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
                                    if subdir + default_wml_extension in files:
                                        files.remove(subdir + default_wml_extension)
                                    dirs.remove(subdir)
                                    dirpath.append(os.path.join(root, subdir))
                    subtree.extend([os.path.normpath(os.path.join(root, x)) for x in files])
            # Always look at _main.cfg first
            maincfgs = [elem for elem in subtree if elem.endswith("_main.cfg")]
            rest = [elem for elem in subtree if not elem.endswith("_main.cfg")]
            subtree = sorted(maincfgs) + sorted(rest)
            self.forest.append(subtree)
        for i in range(len(self.forest)):
            # Ignore version-control subdirectories, Emacs tempfiles, and other files and extensions
            for dirkind in vc_directories + l10n_directories:
                self.forest[i] = [x for x in self.forest[i] if dirkind not in x]
            self.forest[i] = [x for x in self.forest[i] if '.#' not in x]
            self.forest[i] = [x for x in self.forest[i] if not os.path.isdir(x)]
            if exclude:
                self.forest[i] = [x for x in self.forest[i] if not re.search(exclude, x)]
            for each in misc_files_extensions:
                self.forest[i] = [x for x in self.forest[i] if not x.endswith(each)]
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

    def __iter__(self):
        "Return a generator that walks through all files."
        for (directory, tree) in zip(self.dirpath, self.forest):
            for filename in tree:
                yield (directory, filename)

def ismap(filename):
    "Is this file a map?"
    return filename.split('.')[-1] in map_extensions

def iswml(filename):
    "Is the specified filename WML?"
    return filename.split('.')[-1] in wml_extensions

def issave(filename):
    "Is the specified filename a WML save? (Detects compressed saves too.)"
    if isresource(filename):
        return False
    if filename.endswith(".gz"):
        with gzip.open(filename) as content:
            firstline = content.readline()
            if not isinstance(firstline, str):
                # It's a compressed binary file
                return False
    else:
        try:
            with open(filename, "r", encoding="utf8") as content:
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
    def handle_argument(buffer):
        nonlocal args
        nonlocal optional_args

        opt_arg = ""

        arg = "".join(buffer)
        # arg may be empty, so arg[0] may be OOB.
        if arg[0:1].isspace():
            arg = arg[1:]

        # is this an optional argument?
        # argument names are usually made of uppercase letters, numbers and underscores
        # if they're optional, they're followed by an equal sign
        # stop matching on the first one, because the argument value might contain one too
        if re.match(r"^([A-Z0-9_]+?)=", arg):
            opt_arg, arg = arg.split("=", 1)
        if opt_arg:
            optional_args[opt_arg] = arg
            opt_arg = ""
        else:
            args.append(arg)
        buffer.clear()
        return True

    buffer = []
    args = []
    optional_args = {}

    depth = {
        EQUALS: 0,
        OPEN_BRACE: 0,
        OPEN_PARENS: 0,
        QUOTE: 0,
    }
    wrapper_stack = []
    prev_added_arg = False

    # close_token - Closes all active scopes, until the matching scope is found.
    # This is useful, for example, in {MACRO OPT_NAME=VAL}
    # In this example, close_token("}") will implicitly close the
    # optional argument scope.
    def close_token(token):
        while len(wrapper_stack) > 0:
            last_token = wrapper_stack.pop()
            depth[last_token] -= 1
            if last_token == token:
                break
        else:
            return False
        return True

    # close_if_token - Closes the current scope, if it matches the given token.
    # Atomic version of close_token, which provides expressivity.
    def close_if_token(token):
        if wrapper_stack[-1] == token:
            wrapper_stack.pop()
            depth[token] -= 1
            return True
        return False

    def open_token(token):
        wrapper_stack.append(token)
        depth[token] += 1

    for i in range(start, len(line)):
        added_arg = False
        if depth[QUOTE] > 0:
            # If EOL, line[i+1] may be OOB, but slice is valid.
            if line[i] == QUOTE and line[i+1:i+2] != QUOTE:
                close_token(QUOTE)
            buffer.append(line[i])
        elif line[i] == QUOTE:
            open_token(QUOTE)
            buffer.append(line[i])
        elif line[i] == OPEN_BRACE:
            open_token(OPEN_BRACE)
            buffer.append(line[i])
        elif line[i] == CLOSE_BRACE:
            if wrapper_stack[-1] != OPEN_PARENS:
                close_token(OPEN_BRACE)
            if depth[OPEN_BRACE] == 0:
                # Flush at end
                if not prev_added_arg and len(buffer) > 0:
                    added_arg = handle_argument(buffer)
                break
            else:
                buffer.append(line[i])
        elif line[i] == OPEN_PARENS:
            if wrapper_stack[-1] == OPEN_PARENS or wrapper_stack[-2:] == [OPEN_PARENS, EQUALS]:
                # Char in an argument
                buffer.append(line[i])
            else:
                if wrapper_stack[-1] == EQUALS or (wrapper_stack[-1] == OPEN_BRACE and \
                    not line[i-1].isspace()):
                    close_if_token(EQUALS)
                    if depth[OPEN_BRACE] == 1 and not prev_added_arg:
                        added_arg = handle_argument(buffer)
                open_token(OPEN_PARENS)
                if depth[OPEN_BRACE] != 1:
                    buffer.append(line[i])
        elif line[i] == CLOSE_PARENS:
            # Source has too many closing parens.
            quit = not close_token(OPEN_PARENS)
            if depth[OPEN_BRACE] != 1:
                buffer.append(line[i])
            elif not prev_added_arg or line[i-1] == OPEN_PARENS:
                # {MACRO arg1()} has two arguments
                added_arg = handle_argument(buffer)
            if quit:
                added_arg = handle_argument(buffer)
                break
        elif line[i] == EQUALS and re.match(r"^([A-Z0-9_]+?)", line[i-1]):
            open_token(EQUALS)
            buffer.append(line[i])
        elif line[i].isspace():
            if line[i-1].isspace():
                # Ignore consecutive spaces
                continue
            if not prev_added_arg and \
             depth[OPEN_BRACE] == 1 and \
             depth[OPEN_PARENS] == 0:
                close_if_token(EQUALS)
                added_arg = handle_argument(buffer)
            buffer.append(line[i])
        else:
            buffer.append(line[i])

        prev_added_arg = added_arg

    args.pop(0)
    return (args, optional_args, depth[OPEN_BRACE] > 0)

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
    elif re.match(r"(([0-9]+-)?[0-9]+,)*([0-9]+-)?[0-9]+\Z", a):
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

def argmatch(formals, optional_formals, actuals, optional_actuals):
    if optional_formals:
        for key in optional_actuals.keys():
            if key not in optional_formals:
                return False
    if len(formals) != len(actuals):
        return False
    opt_formals, opt_actuals = [], []
    for key, value in optional_actuals.items():
        opt_formals.append(key)
        opt_actuals.append(value)
    for (f, a) in zip(formals + opt_formals, actuals + opt_actuals):
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
    def __init__(self, namespace, filename, lineno=None, lineno_end=None, docstring=None, args=None,
                 optional_args=None, deprecated=False, deprecation_level=0, removal_version=None):
        self.namespace = namespace
        self.filename = filename
        self.lineno = lineno
        self.lineno_end = lineno_end
        self.docstring = docstring
        self.args = args
        self._raw_optional_args = optional_args
        self.optional_args = {}
        self.body = []
        self.deprecated = deprecated
        self.deprecation_level = deprecation_level
        self.removal_version = removal_version
        self.references = collections.defaultdict(list)
        self.undef = None

    def append(self, fn, n, args=None, optional_args=None):
        self.references[fn].append((n, args, optional_args))

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
        copy = Reference(self.namespace, self.filename, self.lineno, self.lineno_end, self.docstring, self.args, self._raw_optional_args)
        copy.undef = self.undef
        for filename in self.references:
            mis = [(ln,a,oa) for (ln,a,oa) in self.references[filename] if a is not None and not argmatch(self.args, self.optional_args, a, oa)]
            if mis:
                copy.references[filename] = mis
        return copy
    def __str__(self):
        if self.lineno:
            return '"%s", line %d' % (self.filename, self.lineno)
        else:
            return self.filename
    __repr__ = __str__

class States(enum.Enum):
    OUTSIDE = enum.auto()
    MACRO_HEADER = enum.auto()
    EXTERNAL_DOCSTRING = enum.auto()
    MACRO_OPTIONAL_ARGUMENT = enum.auto()
    MACRO_BODY = enum.auto()

class CrossRef:
    macro_reference = re.compile(r"\{([A-Z_][A-Za-z0-9_:]*)(?!\.)\b")
    file_reference = re.compile(r"([A-Za-z0-9{}.][A-Za-z0-9_/+{}.@\-\[\],~\*]*?\.(" + "|".join(resource_extensions) + r"))((~[A-Z]+\(.*\))*)(:([0-9]+|\[[0-9,*~]*\]))?")
    tag_parse = re.compile(r"\s*([a-z_]+)\s*=(.*)")
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
        elif defn.filename != fn and not self.filelist.neighbors(defn.filename, fn):
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
        temp_docstrings = {}
        current_docstring = None
        try:
            with open(filename, "r", encoding="utf8") as dfp:
                state = States.OUTSIDE
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
                            here = Reference(namespace, filename, n+1, line, None, args=tokens[2:], optional_args=[])
                            here.hash = hashlib.md5()
                            here.docstring = line.lstrip()[8:] # Strip off #define_
                            current_docstring = None
                            if name in temp_docstrings:
                                here.docstring += temp_docstrings[name]
                                del temp_docstrings[name]
                            state = States.MACRO_HEADER
                        continue
                    if state in (States.OUTSIDE, States.EXTERNAL_DOCSTRING):
                        # allow starting new docstrings even one after another
                        m = re.match(r"\s*# wmlscope: docstring (\w+)", line)
                        if m:
                            current_docstring = m.group(1)
                            # what if someone tries to define a docstring twice for the same macro?
                            # In this case, warn and overwrite the old one
                            if current_docstring in temp_docstrings:
                                print("Redefining a docstring for macro {} at {}, line {}".format(m.group(1),
                                                                                                  filename,
                                                                                                  n+1),
                                      file=sys.stderr)
                            temp_docstrings[current_docstring] = ""
                            state = States.EXTERNAL_DOCSTRING
                            continue
                    if state == States.EXTERNAL_DOCSTRING:
                        # stop collecting the docstring on the first non-comment line (even if it's empty)
                        # if the line starts with a #define or another docstring directive
                        # it'll be handled in the blocks above
                        if line.lstrip().startswith("#"):
                            temp_docstrings[current_docstring] += line.lstrip()[1:]
                        else:
                            current_docstring = None
                            state = States.OUTSIDE
                    elif state != States.OUTSIDE and line.strip().endswith("#enddef"):
                        end_def_index = line.index("#enddef")
                        here.body.append(line[0:end_def_index])
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

                        here.lineno_end = n+1
                        self.xref[name].append(here)
                        state = States.OUTSIDE
                    elif state == States.MACRO_HEADER and line.strip():
                        if line.strip().startswith("#arg"):
                            state = States.MACRO_OPTIONAL_ARGUMENT
                            here._raw_optional_args.append([line.strip().split()[1],""])
                        elif line.strip()[0] != "#":
                            state = States.MACRO_BODY
                    elif state == States.MACRO_OPTIONAL_ARGUMENT and not "#endarg" in line:
                        here._raw_optional_args[-1][1] += line
                    elif state == States.MACRO_OPTIONAL_ARGUMENT:
                        end_arg_index = line.index("#endarg")
                        here._raw_optional_args[-1][1] += line[0:end_arg_index]
                        here.optional_args = dict(here._raw_optional_args)
                        state = States.MACRO_HEADER
                        continue
                    if state == States.MACRO_HEADER:
                        # Ignore macro header commends that are pragmas
                        if ("wmlscope" in line) or ("wmllint:" in line):
                            continue
                        # handle deprecated macros
                        if "deprecated" in line:
                            # There are four levels of macro deprecation (1, 2, 3 and 4)
                            # Sometimes they have a version number in which they're
                            # scheduled for removal and sometimes they don't have it
                            # This regex seems to match every deprecated macro in mainline
                            # in version 1.15.6
                            m = re.match(r"\s*#\s?deprecated\s(1|2|3|4)\s?([0-9.]*)\s?(.*)", line)
                            if m:
                                here.deprecated = True
                                # leave them as strings: they'll be used for HTML output
                                here.deprecation_level = m.group(1)
                                here.removal_version = m.group(2)
                                here.docstring += m.group(3)
                            else:
                                print("Deprecation line not matched found in {}, line {}".format(filename, n+1), file=sys.stderr)
                        else:
                            here.docstring += line.lstrip()[1:]
                    if state == States.MACRO_BODY:
                        here.body.append(line)
                    if state in (States.MACRO_HEADER, States.MACRO_OPTIONAL_ARGUMENT, States.MACRO_BODY):
                        here.hash.update(line.encode("utf8"))
                    elif line.strip().startswith("#undef"):
                        tokens = line.split()
                        name = tokens[1]
                        if name in self.xref and self.xref[name]:
                            self.xref[name][-1].undef = n+1
                        else:
                            print("%s: unbalanced #undef on %s" \
                                  % (Reference(namespace, filename, n+1), name))
                    if state == States.OUTSIDE:
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
            # handling of the file is over, but there are some external docstring still around
            # this happens if someone defined an external docstring *after* the macro it refers to
            # or to a macro which isn't even in the file
            # warn if that's the case
            if temp_docstrings: # non-empty dictionaries cast as True
                print("Docstrings defined after their macros or referring to a missing \
    macro found in {}: {}".format(filename,
                                  ", ".join(temp_docstrings.keys())),
                      file=sys.stderr)
        except UnicodeDecodeError as e:
            print('wmlscope: "{}" is not a valid UTF-8 file'.format(filename), file=sys.stderr)

    def __init__(self, dirpath=[], filelist=None, exclude="", warnlevel=0, progress=False):
        "Build cross-reference object from the specified filelist."
        if filelist is None:
            self.filelist = Forest(dirpath, exclude)
            self.dirpath = [x for x in dirpath if not re.search(exclude, x)]
        else:
            # All specified files share the same namespace
            self.filelist = [("src", filename) for filename in filelist]
            self.dirpath = ["src"]
            
        self.warnlevel = warnlevel
        self.xref = {}
        self.fileref = {}
        self.noxref = False
        self.properties = {}
        self.unit_ids = {}
        all_in = []
        if self.warnlevel >=2 or progress:
            print("*** Beginning definition-gathering pass...")
        for (namespace, filename) in self.filelist:
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
                with open(filename, "r", encoding="utf8") as dfp:
                    for line in dfp:
                        self.xref[line.strip()] = True
        # Next, decorate definitions with all references from the filelist.
        self.unresolved = []
        self.missing = []
        self.deprecated = []
        formals = []
        optional_formals = []
        state = States.OUTSIDE
        if self.warnlevel >=2 or progress:
            print("*** Beginning reference-gathering pass...")
        for (ns, fn) in all_in:
            if progress:
                print(fn)
            if iswml(fn):
                try:
                    with open(fn, "r", encoding="utf8") as rfp:
                        attack_name = None
                        have_icon = False
                        beneath = 0
                        ignoreflag = False
                        in_macro_definition = False
                        for (n, line) in enumerate(rfp):
                            if line.strip().startswith("#define"):
                                formals = line.strip().split()[2:]
                                in_macro_definition = True
                            elif line.startswith("#enddef"):
                                formals = []
                                optional_formals = []
                                in_macro_definition = False
                            elif in_macro_definition and line.startswith("#arg"):
                                optional_formals.append(line.strip().split()[1])
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
                                if name in formals or name in optional_formals:
                                    continue
                                elif name in self.xref:
                                    # Count the number of actual arguments.
                                    # Set args to None if the call doesn't
                                    # close on this line
                                    (args, optional_args, is_unfinished) = parse_macroref(match.start(0), line)
                                    if is_unfinished:
                                        args = None
                                        optional_args = None
                                    #if args:
                                    #    print('"%s", line %d: args of %s is %s' \
                                    #          % (fn, n+1, name, args))
                                    # Figure out which macros might resolve this
                                    for defn in self.xref[name]:
                                        if self.visible_from(defn, fn, n+1):
                                            defn.append(fn, n+1, args, optional_args)
                                            candidates.append(str(defn))
                                            if defn.deprecated:
                                                self.deprecated.append((name,Reference(ns,fn,n+1)))
                                    if len(candidates) > 1:
                                        print("%s: more than one definition of %s is visible here (%s)." % (Reference(ns, fn, n), name, "; ".join(candidates)))
                                if len(candidates) == 0:
                                    self.unresolved.append((name,Reference(ns,fn,n+1)))
                            # Don't be fooled by HTML image references in help strings.
                            if "<img>" in line:
                                continue
                            # Find references to resource files
                            for match in re.finditer(CrossRef.file_reference, line):
                                for pattern in split_filenames(match):
                                    for name in expand_square_braces(pattern):
                                        # Catches maps that look like macro names.
                                        if (ismap(name)):
                                            if name.startswith("{~"):
                                                name = name[2:]
                                            elif name.startswith("{"):
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
                                            pattern = re.sub(r"(\{[^}]*\}|@R[0-5]|@V)", '.*', name)
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
                            if state == States.OUTSIDE:
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
                except UnicodeDecodeError as e:
                    pass # to not have the invalid UTF-8 file warning printed twice
        # Check whether each namespace has a defined export property
        if self.warnlevel >= 1:
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
    if not loc.endswith(default_wml_extension):
        loc += default_wml_extension
    return loc

def resolve_unit_image(namespace, subdir, resource):
    "Construct a plausible location for given resource in specified namespace."
    return os.path.join(namespace_directory(namespace), "images/units", subdir, resource)

# And this is for code that does syntax transformation
baseindent = "    "

# a constant to detect version control directories
vcdir = ".git"

# wmltools.py ends here
