#!/usr/bin/env python3
"""
Check translatable strings against the Typography Style Guide.

https://wiki.wesnoth.org/Typography_Style_Guide
https://wiki.wesnoth.org/UI_Style_Guide#Typography_Summary
See also https://github.com/wesnoth/wesnoth/issues/7967

Strings are extracted straight from the source tree with the same extractors
that pot-update uses (wmlxgettext for WML/Lua, xgettext for C++), so no
build, CMake configure or committed .pot file is needed. Only the msgids
(the English source strings) are checked. It only reports; nothing is changed.

Usage, from the repository root:
    utils/CI/check_typography.py            # extract and check
    utils/CI/check_typography.py FILE.pot   # check existing .pot file(s)

Exits non-zero if any problem is found. When run under GitHub Actions, problems
are also emitted as annotations so they show up on the PR diff.

Intentional exceptions go in EXCEPTIONS below.
"""

import ast
import glob
import os
import re
import subprocess
import sys
import tempfile
from collections import defaultdict

# (rule, sub-string of the msgid) pairs that are deliberately exempt.
EXCEPTIONS = [
    # These list the characters that are forbidden in file names, so the
    # straight double quote is meant literally.
    ("straight-double-quote", '" * / : < > ? \\ | ~'),
    # Error and diagnostic messages, not held to the style guide.
    ("number-range", "(should be 1-4)"),                  # deprecation level
    ("straight-apostrophe", "is corrupt: '"),             # game_launcher.cpp, quote
    ("straight-apostrophe", "while reading the game: '"), # closed by a later string
    ("straight-apostrophe", "Mandatory widget "),         # GUI definition error
    # A command to be typed literally in the console.
    ("straight-double-quote", 'Use "help all"'),
]

# Mark-up whose contents are not prose, removed before the character checks.
PANGO_TAG = re.compile(r"<[^<>]*>")                    # <span color='red'>, </i>
HELP_ATTR = re.compile(r"\b\w+='[^']*'")               # <ref>text='Orcs' dst='..race_orc'</ref>
WML_FORMULA = re.compile(r"\$\((?:[^()]|\([^()]*\))*\)")  # $( $a - $b )
# Straight quotes delimiting a value rather than prose, typical of error and
# diagnostic messages: '$addon|', '$id', 'fixed_width', '..'
QUOTED_VALUE = re.compile(r"'(?:\$[\w.]+\|?|\w+_\w+|[^\w\s']+)'")

RULES = [
    # rule name, pattern, message
    ("backquote", re.compile(r"`"),
     "backquote (`) used; use ‘ ’ (U+2018, U+2019)"),
    ("straight-double-quote", re.compile(r'"'),
     "straight double quote used; use “ ” (U+201C, U+201D) or ‘ ’"),
    ("straight-apostrophe", re.compile(r"'"),
     "straight single quote/apostrophe used; use ’ (U+2019) or ‘ ’"),
    ("spaced-hyphen", re.compile(r"(?:^| )--? (?=\S)|\S --?(?: |$)", re.M),
     "hyphen used as a dash; use — (U+2014) or – (U+2013)"),
    # Damage×strikes ("7-4 fire", "19-1 ranged-cold") is written several ways
    # at present and is deliberately left out until that is agreed.
    ("number-range", re.compile(r"(?<![\w.#/-])\d+-\d+(?![\w-]|\.\d)"
                                r"(?!\s+(?:blade|pierce|impact|fire|cold|arcane|melee|ranged)\b)"),
     "hyphen between numbers; use – (U+2013) for ranges"),
    ("hyphen-minus-number", re.compile(r"(?:^|[\s(])-\d"),
     "hyphen-minus before a number; use − (U+2212)"),
    ("unicode-ellipsis", re.compile("…"),
     "ellipsis character (U+2026) used; the style guide uses three full stops"),
    ("double-space", re.compile(r"(?<=[^\d\s])[.!?]  +(?=\S)"),
     "two spaces after a full stop"),
]

# Not a typography rule, but a real bug of the same kind: sgettext/dsgettext
# strip everything up to the first '^' when no translation exists, so a caret
# that is not a "context^" marker eats the start of the English string.
# Context markers are short and single-line ("editor^", "race+female^");
# flag a first caret after a non-word character ("(^Qhh)"), or after a long or
# multi-line run of text ("... the Gg^Efm terrain").
STRAY_CARET = re.compile(r"^(?:[^^]*[^\w^]|[^^]*\n[^^]*|[^^]{61,})\^")


def domains():
    """Normal (WML/C++/Python) text domains, as listed in po/CMakeLists.txt."""
    with open("po/CMakeLists.txt", encoding="utf-8") as f:
        m = re.search(r"set\(NORMAL_DOMAINS(.*?)\)", f.read(), re.S)
    return m.group(1).split()


def extract(tmp):
    """Run the extractors over the tree, return a list of .pot paths."""
    wml = sorted(p for p in glob.glob("data/**/*.cfg", recursive=True)
                 + glob.glob("data/**/*.lua", recursive=True)
                 if not p.startswith("data/test/"))
    subprocess.run([sys.executable, "data/tools/wmlxgettext", "--no-text-colors",
                    "-o", os.path.join(tmp, "wml"), *wml, "--domain", *domains()],
                   check=True, stderr=subprocess.DEVNULL)

    # Python sources (data/tools) are left out: they are tools for content
    # authors rather than player-facing text.
    keywords = ["_", "N_", "sgettext", "vgettext", "VGETTEXT", "_n:1,2", "N_n:1,2",
                "sngettext:1,2", "vngettext:1,2", "VNGETTEXT:1,2"]
    xgettext = ["xgettext", "--force-po", "--from-code=UTF-8", "--sort-by-file",
                *("--keyword=" + k for k in keywords)]
    cpp = sorted(glob.glob("src/**/*.cpp", recursive=True) + glob.glob("src/**/*.hpp", recursive=True))
    subprocess.run([*xgettext, "-o", os.path.join(tmp, "cpp.pot"), *cpp],
                   check=True, stderr=subprocess.DEVNULL)
    return sorted(glob.glob(os.path.join(tmp, "**", "*.pot"), recursive=True))


def read_pot(path):
    """Yield (references, msgid) for every msgid/msgid_plural in a .pot file."""
    refs, field, strs = [], None, {}

    def flush():
        for key in ("msgid", "msgid_plural"):
            if strs.get(key):
                yield refs, strs[key]

    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith("#:"):
                if strs:
                    yield from flush()
                    refs, strs = [], {}
                refs.extend(line[2:].split())
            elif line.startswith("#"):
                continue
            elif not line:
                yield from flush()
                refs, field, strs = [], None, {}
            elif line.startswith('"'):
                if field:
                    strs[field] += ast.literal_eval(line)
            else:
                key, _, value = line.partition(" ")
                field = key if key in ("msgid", "msgid_plural") else None
                if field:
                    strs[field] = ast.literal_eval(value)
        yield from flush()


def excerpt(text, match, width=25):
    """A single-line excerpt of text around a match."""
    start, end = max(match.start() - width, 0), min(match.end() + width, len(text))
    snippet = text[start:end].replace("\n", "⏎")
    return ("…" if start else "") + snippet + ("…" if end < len(text) else "")


def check(msgid):
    """Return the list of (rule, message, excerpts) problems found in one msgid."""
    problems = []
    m = STRAY_CARET.search(msgid)
    if m:
        problems.append(("stray-caret", "'^' that is not a context marker; the text before it "
                         "is cut off when untranslated (add a context, eg 'editor^…')",
                         [excerpt(msgid, re.search(r"\^", msgid))]))
    # What the player sees: dsgettext drops everything up to the first '^'.
    text = msgid.split("^", 1)[1] if "^" in msgid else msgid
    prose = QUOTED_VALUE.sub("", WML_FORMULA.sub("", HELP_ATTR.sub("", PANGO_TAG.sub("", text))))
    for name, pattern, message in RULES:
        target = text if name == "double-space" else prose
        matches = list(pattern.finditer(target))
        if matches and not any(rule == name and s in msgid for rule, s in EXCEPTIONS):
            problems.append((name, message, [excerpt(target, x) for x in matches]))
    return problems


def main():
    with tempfile.TemporaryDirectory() as tmp:
        pots = sys.argv[1:] or extract(tmp)
        found = defaultdict(list)  # (rule, msgid) -> refs, merges duplicate msgids
        messages, excerpts = {}, {}
        for pot in pots:
            for refs, msgid in read_pot(pot):
                for rule, message, snippets in check(msgid):
                    found[(rule, msgid)].extend(refs)
                    messages[rule] = message
                    excerpts[(rule, msgid)] = snippets

    github = os.environ.get("GITHUB_ACTIONS") == "true"
    for (rule, msgid), refs in sorted(found.items(), key=lambda i: i[1][0] if i[1] else ""):
        refs = list(dict.fromkeys(refs)) or ["?"]
        print(f"{refs[0]}: {rule}: {messages[rule]}")
        for snippet in excerpts[(rule, msgid)]:
            print(f"    {snippet}")
        for ref in refs[1:]:
            print(f"    also at {ref}")
        if github:
            for ref in refs:
                if ":" in ref:
                    path, line = ref.rsplit(":", 1)
                    print(f"::error file={path},line={line},title=Typography ({rule})::{messages[rule]}")

    if found:
        sys.stdout.flush()
        print(f"\n{len(found)} typography problem(s) found. "
              "See https://wiki.wesnoth.org/Typography_Style_Guide", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
