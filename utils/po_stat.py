import sys
import os
import glob
import subprocess
import re


class Stats:
    def __init__(self):
        self._stats = [0, 0, 0]

    def parse(self, line):
        m = re.search(r'(\d+)\s*translated', line)
        if m:
            self._stats[0] = int(m.group(1))

        m = re.search(r'(\d+)\s*fuzzy', line)
        if m:
            self._stats[1] = int(m.group(1))

        m = re.search(r'(\d+)\s*untranslated', line)
        if m:
            self._stats[2] = int(m.group(1))

    def __iadd__(self, other):
        for i in range(3):
            self._stats[i] += other._stats[i]
        return self

    def __add__(self, other):
        s = Stats()
        s.stats = list(self._stats)
        s += other
        return s

    def translated_percent(self):
        s = sum(self._stats)
        return self._stats[0] * 100 // s if s else 0


if __name__ == '__main__':

    if len(sys.argv) == 1:
        print('Usage: {} <locale_name>. */<locale_name>.po files must exist.')
        exit(0)

    locale_name = sys.argv[1]
    po_files = glob.glob('*/{}.po'.format(locale_name))
    if not po_files:
        print('{} should be run from po subdirectory'.format(sys.argv[0]), file=sys.stderr)
        exit(1)

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

        except subprocess.CalledProcessError as e:
            print('yo {}'.format(e.returncode))
            break

    print('{}={}'.format(
        locale_name,
        total.translated_percent()
    ))
