"""
Extract translatable strings for checking tools, such as check_typography.

Strings are extracted straight from the source with the same extractors that
pot-update uses (wmlxgettext for WML/Lua, xgettext for C++), so no build,
CMake configure or committed .pot file is needed.

Typical use, from a tool in data/tools:

    from wesnoth import gettext_strings

    try:
        pots = gettext_strings.extract_sources(args.paths, args.domain)
    except gettext_strings.NotACheckout as error:
        parser.error(str(error))
    for pot_lines in pots:
        for references, msgid in gettext_strings.read_pot(pot_lines):
            ...
"""

import ast
import glob
import os
import re
import subprocess
import sys

# data/tools, where wmlxgettext lives
TOOLS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

XGETTEXT_KEYWORDS = ["_", "N_", "sgettext", "vgettext", "VGETTEXT", "_n:1,2", "N_n:1,2",
                     "sngettext:1,2", "vngettext:1,2", "VNGETTEXT:1,2"]


class NotACheckout(Exception):
    """Mainline strings were asked for outside the root of a Wesnoth checkout."""


def mainline_domains():
    """Normal text domains listed in po/CMakeLists.txt, or None outside a repository checkout."""
    try:
        with open("po/CMakeLists.txt", encoding="utf-8") as cmake_file:
            domains_match = re.search(r"set\(NORMAL_DOMAINS(.*?)\)", cmake_file.read(), re.S)
    except OSError:
        return None
    return domains_match.group(1).split() if domains_match else None


def run_extractor(command):
    """Run an extractor that writes a .pot to stdout, return its lines."""
    result = subprocess.run(command, check=True, stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL, encoding="utf-8")
    return result.stdout.splitlines()


def extract_wml(paths, domains, skip=()):
    """Extract strings from the .cfg and .lua files in (or at) paths, skipping files whose path starts with skip."""
    files = []
    for path in paths:
        if os.path.isfile(path):
            files.append(path)
        else:
            files += glob.glob(os.path.join(path, "**", "*.cfg"), recursive=True)
            files += glob.glob(os.path.join(path, "**", "*.lua"), recursive=True)
    files = sorted(path for path in files if path.endswith((".cfg", ".lua")) and not path.startswith(skip))
    if not files:
        return []
    command = [sys.executable, os.path.join(TOOLS_DIR, "wmlxgettext"),
               "--no-text-colors", "-o", "-", *files]
    if domains:
        command += ["--domain", *domains]
    return run_extractor(command)


def extract_cpp(skip=()):
    """Extract strings from mainline C++, skipping files whose path starts with skip.

    Python sources (data/tools) are deliberately left out: they are tools for
    content authors rather than player-visible text.
    """
    cpp_files = sorted(glob.glob("src/**/*.cpp", recursive=True) + glob.glob("src/**/*.hpp", recursive=True))
    cpp_files = [path for path in cpp_files if not path.startswith(skip)]
    return run_extractor(["xgettext", "--force-po", "--from-code=UTF-8", "--sort-by-file",
                          *("--keyword=" + keyword for keyword in XGETTEXT_KEYWORDS), "-o", "-", *cpp_files])


def extract_sources(paths, domains, cpp_skip=()):
    """Return the .pot content, as lists of lines, for the strings a checking tool was asked about.

    With no paths, that is mainline: WML/Lua under data (except data/test) in the given domains, or the
    domains listed in po/CMakeLists.txt, plus all C++ except files whose path starts with cpp_skip.
    Otherwise, paths are .pot files to read and directories or .cfg/.lua files to extract from.
    Raises NotACheckout if mainline is asked for outside the root of a Wesnoth checkout.
    """
    if not paths:
        domains = domains or mainline_domains()
        if domains is None:
            raise NotACheckout("no PATH given, and this is not the root of a Wesnoth checkout")
        return [extract_wml(["data"], domains, skip=("data/test/",)), extract_cpp(skip=cpp_skip)]
    pots = []
    for path in paths:
        if path.endswith(".pot"):
            with open(path, encoding="utf-8") as pot_file:
                pots.append(pot_file.read().splitlines())
    pots.append(extract_wml([path for path in paths if not path.endswith(".pot")], domains))
    return pots


def read_pot(lines):
    """Yield (references, msgid) for every msgid/msgid_plural in .pot content."""
    references, field, strings = [], None, {}

    def flush():
        for key in ("msgid", "msgid_plural"):
            if strings.get(key):
                yield references, strings[key]

    for line in lines:
        line = line.strip()
        if line.startswith("#:"):
            if strings:
                yield from flush()
                references, strings = [], {}
            references.extend(line[2:].split())
        elif line.startswith("#"):
            continue
        elif not line:
            yield from flush()
            references, field, strings = [], None, {}
        elif line.startswith('"'):
            if field:
                strings[field] += ast.literal_eval(line)
        else:
            key, _, value = line.partition(" ")
            field = key if key in ("msgid", "msgid_plural") else None
            if field:
                strings[field] = ast.literal_eval(value)
    yield from flush()


def running_on_github():
    return os.environ.get("GITHUB_ACTIONS") == "true"


def github_annotation(level, reference, title, message):
    """A GitHub Actions annotation ("error" or "warning") for a "path:line" reference, or None if it has no line."""
    if ":" not in reference:
        return None
    path, line = reference.rsplit(":", 1)
    return "::{} file={},line={},title={}::{}".format(level, path, line, title, message)
