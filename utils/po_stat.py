#!/usr/bin/env python
# encoding: utf-8

import sys
import os
import glob
import subprocess
import re


verbose = False


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
        'Usage: {} <locale_name> <wml config file to add percent field> [--verbose]\n'
        ' */<locale_name>.po files must exist.'.format(sys.argv[0]))


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
    match = re.match(r'([a-z]+)(_([A-Z]+))?(@([a-z]+))?', locale)
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

    for current in range(len(candidates)):
        for previous in range(0, current-1):
            # Remove more specialized candidates from more general one's list
            for file in candidate_files[candidates[current]]:
                if file in candidate_files[candidates[previous]]:
                    candidate_files[candidates[previous]].remove(file)

    for current in range(len(candidates)):
        files = candidate_files[candidates[current]]
        if files:
            if len(files) > 1:
                print('Warning: too many candidates for {}: {}'.format(locale, files))
            return files[0]

    return None


if __name__ == '__main__':

    if len(sys.argv) < 3:
        usage()
        exit(0)

    locale_name = sys.argv[1]

    po_mask = '*/{}.po'.format(locale_name)
    pos_mask = os.path.join(os.path.dirname(sys.argv[0]), '../po', po_mask)

    po_files = glob.glob(pos_mask)
    if not po_files:
        sys.stderr.write('Cannot find {}'.format(pos_mask))
        usage(file=sys.stderr)
        exit(1)

    wml_dir = sys.argv[2]

    if len(sys.argv) >= 4 and sys.argv[3] == '--verbose':
        verbose = True

    if not os.path.isdir(wml_dir):
        sys.stderr.write('File {} not found '.format(wml_dir))
        usage(file=sys.stderr)
        exit(1)

    percent = do_stats(po_files)
    print('{}: {}%'.format(locale_name, percent))
    wml_cfg = get_wml_cfg(wml_dir, locale_name)

    if not wml_cfg:
        exit(1)

    try:
        write_percentage_wml(wml_cfg, percent)
        # print('{} saved!'.format(wml_cfg))
    except IOError as whoopsie:
        sys.stderr.write(whoopsie)
