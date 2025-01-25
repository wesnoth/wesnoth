#!/usr/bin/env python3
# encoding: utf-8

import sys
import os
import glob
import subprocess
import re


verbose = False
update_cfg = False
textdomains = []


class Stats:
    def __init__(self):
        self.stats = [0, 0, 0]

    def parse(self, line):
        m = re.search(r'(\d+)\s*translated', line)
        if m:
            self.stats[0] = int(m.group(1))

        m = re.search(r'(\d+)\s*fuzzy', line)
        if m:
            self.stats[1] = int(m.group(1))

        m = re.search(r'(\d+)\s*untranslated', line)
        if m:
            self.stats[2] = int(m.group(1))

    def __iadd__(self, other):
        for i in range(3):
            self.stats[i] += other.stats[i]
        return self

    def __add__(self, other):
        s = Stats()
        s.stats = list(self.stats)
        s += other
        return s

    def translated_percent(self):
        s = sum(self.stats)
        return self.stats[0] * 100 // s if s else 0


def usage(file=sys.stdout):
    file.write(
        'Usage: {} [--verbose] [--update-cfg] [--help]\n'
        'Calculates the translated% (non-fuzzy) of Wesnoth\'s po files, prints it to stdout.\n'
        ' --update-cfg: writes the result into data/languages/*.cfg\n'
        ' --verbose: prints extra status messages\n'
        ' --textdomains <comma-separated list>: use only these textdomains.\n'
        ' --help: prints this text\n'
        'po/*/<locale_name>.po files must exist.'.format(sys.argv[0]))


def do_stats(po_files):
    env = dict(os.environ)
    env['LC_MESSAGES'] = 'C'
    env.pop('LC_ALL', None)

    total = Stats()

    for po_file in po_files:
        command_args = ['msgfmt', '-o', '/dev/null', '--statistics', po_file]

        try:
            # print('running {}'.format(' '.join(command_args)))
            byte_output = subprocess.check_output(command_args, env=env, stderr=subprocess.STDOUT)

            # print('output: {}'.format(output))
            str_output = byte_output.decode("utf-8")

            current = Stats()
            current.parse(str_output)
            total += current
            if verbose:
                print('File {}: {} strings translated'.format(po_file, current.translated_percent()))

        except subprocess.CalledProcessError as e:
            print('yo {}'.format(e.returncode))
            break

    return total.translated_percent()


def write_percentage_wml(wml_file, percent):
    text = ''
    with open(wml_file) as f:
        text = f.read()

    percent_line = 'percent={}'.format(percent)
    if 'percent=' in text:
        text = re.sub(r'percent=\d+', percent_line, text)
    else:
        text = re.sub(r'\[/locale\]', '    {}\n[/locale]'.format(percent_line), text)

    with open(wml_file, 'w') as f:
        f.write(text)


def get_wml_cfg(wml_dir, locale):
    match = re.match(r'([a-z]+)(_([A-Z0-9]+))?(@([a-z]+))?', locale)
    if not match:
        return None

    lang, script, region = match.group(1, 3, 5)
    candidates = [
        '{}.cfg'.format(lang),
    ]

    if region:
        if script:
            candidates.append('{}_{}@{}.cfg'.format(lang, script, region))
        else:
            candidates.append('{}@{}.cfg'.format(lang, region))
            candidates.append('{}_{}@{}.cfg'.format(lang, lang.upper(), region))
            candidates.append('{}_*@{}.cfg'.format(lang, region))
    else:
        if script:
            candidates.append('{}_{}.cfg'.format(lang, script))
            candidates.append('{}_{}@*.cfg'.format(lang, script))
        else:
            candidates.append('{}_{}.cfg'.format(lang, lang.upper()))
            candidates.append('{}_*.cfg'.format(lang))
            candidates.append('{}@*.cfg'.format(lang))
            candidates.append('{}_*@*.cfg'.format(lang))

    candidate_files = {
        file: glob.glob(os.path.join(wml_dir, file))
        for file in candidates
    }

    for current_index, current in enumerate(candidates):
        for previous in range(0, current_index-1):
            # Remove more specialized candidates from more general one's list
            for file in candidate_files[current]:
                if file in candidate_files[candidates[previous]]:
                    candidate_files[candidates[previous]].remove(file)

    for current in candidates:
        files = candidate_files[current]
        if files:
            if len(files) > 1:
                print('Warning: too many candidates for {}: {}'.format(locale, files))
            return files[0]

    return None


def get_pos(wesnoth_dir, locale_name):
    po_files = []
    if not textdomains:
        po_mask = '*/{}.po'.format(locale_name)
        pos_mask = os.path.join(wesnoth_dir, 'po', po_mask)

        po_files = glob.glob(pos_mask)
        if not po_files:
            sys.stderr.write('Error: Cannot find {}\n'.format(pos_mask))
            exit(1)
    else:
        for d in textdomains:
            po_file = os.path.join(wesnoth_dir, 'po', d, locale_name + '.po')
            if os.path.isfile(po_file):
                po_files.append(po_file)
            else:
                sys.stderr.write('Warning: cannot find {}\n'.format(po_file))

    return po_files


def stat_one_locale(wesnoth_dir, locale_name):
    wml_dir = os.path.join(wesnoth_dir, 'data/languages')

    po_files = get_pos(wesnoth_dir, locale_name)

    if not os.path.isdir(wml_dir):
        sys.stderr.write('File {} not found\n'.format(wml_dir))
        exit(1)

    percent = do_stats(po_files)
    print('{} {}'.format(locale_name, percent))

    if not update_cfg:
        return

    wml_cfg = get_wml_cfg(wml_dir, locale_name)
    if not wml_cfg:
        exit(1)

    try:
        write_percentage_wml(wml_cfg, percent)
    except IOError as whoopsie:
        sys.stderr.write(whoopsie)


if __name__ == '__main__':

    for arg in sys.argv:
        if arg == '--verbose':
            verbose = True
        elif arg == '--help':
            usage()
        elif arg == '--update-cfg':
            update_cfg = True
        elif arg.startswith('--textdomains='):
            arg = arg[14:]
            # Is supposed to be:
            # wesnoth,wesnoth-editor,wesnoth-help,wesnoth-lib,wesnoth-multiplayer,wesnoth-tsg,wesnoth-units
            textdomains = re.split(r'[;, ]', arg)

    wesnoth_dir = os.path.realpath(os.path.join(os.path.dirname(sys.argv[0]), '..'))
    with open(os.path.join(wesnoth_dir, 'po/LINGUAS')) as f:
        da_langs = f.read().split()

    for lang in da_langs:
        stat_one_locale(wesnoth_dir, lang)
