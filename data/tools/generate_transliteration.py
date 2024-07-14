#! /usr/bin/env python3

"""Generate the long lists of [substitute] tags needed for the [hotkey_transliteration]
tag. If that feature is going to be merged then it will need a proper UI, but for
prototyping there's this script. Either run it yourself or cut&paste, someone else will
probably post the output.

The output needs to be in a .cfg file that Wesnoth reads, for example it can be pasted
into data/core/hotkeys.cfg."""

def substitute(src, dst):
	"""Takes two single-letter strings, returns a [substitute] tag.

	If either src or dst is purely whitespace, the empty string will be returned instead,
	so a space can be used in one of the strings that's passed to zip.
	"""
	# Currently doesn't handle any strings that need escaping
	wml_template = "\t[substitute]\n\t\tsrc={src}\n\t\tdst={dst}\n\t[/substitute]".replace("\t", "    ")
	src = src.strip()
	dst = dst.strip()
	if src and dst:
		return wml_template.format(src=src, dst=dst)
	return "";

if __name__ == '__main__':
	latin = "abcdefghijklmnopqrstuvwxyz"

	# For Greek I've left out the semicolon that's on the latin 'Q' key.
	# There's a historical connection between Greek and Latin, with many of the
	# letters having a standard and direct transliteration.  The docs on
	# Wikipedia suggest that there's also a well-agreed convention about how
	# the keyboard layout corresponds for other letters.
	greek = "αβψδεφγηιξκλμνοπ ρστθωςχυζ"
	print("[hotkey_transliteration]")
	print('    id="greek_to_latin"')
	print('    # po: Handling for hotkeys so that a Greek keyboard layout can trigger hotkeys bound to ASCII')
	print('    name=_ "Greek letters to Latin letters"')
	for src,dst in (zip(greek, latin)):
		print(substitute(src, dst))
	print("[/hotkey_transliteration]")

	# Cyrillic letters are used by many languages, with many different key layouts.
	#
	# This is a mapping to Qwerty - the list of non-Latin letters are in the order
	# of physical keys corresponding to a-z on a Qwerty keyboard.
	#
	# It would be possible to do a single mapping that covers both Russian and
	# Ukrainian; while they have a few different letters, every letter that's
	# in both layouts is on the same physical key in both layouts.
	#
	# This should also handle "ґ", but that key doesn't have a Latin letter on
	# a Qwerty layout. Also, IBus and Wikipedia disagree about which physical
	# key it's on.
	russian = "фисвуапршолдьтщзйкыегмцчня"
	ukrainian = "фисвуапршолдьтщзйкіегмцчня"

	print("[hotkey_transliteration]")
	print('    id="russian_to_qwerty"')
	print('    # po: a hotkey transliteration based on two specific keyboard layouts')
	print('    name=_ "Russian keyboard to Qwerty"')
	for src,dst in (zip(russian, latin)):
		print(substitute(src, dst))
	for src,dst in (zip(ukrainian, latin)):
		if not src in russian:
			print(substitute(src, dst))
	print("[/hotkey_transliteration]")
