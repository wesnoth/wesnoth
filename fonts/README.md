The TrueType/OpenType [OS/2][] table [fsType][] field of these fonts should be
set to `0x0` to allow installable embeddeding, as is typically done with Wesnoth
and allowed by the fonts' licenses.  However, the Droid Sans and Oldania ADF Std
fonts originally set this field to `0x8`, only allowing only editable embedding.

When updating the Droid Sans and Oldania ADF Std fonts or adding new fonts,
please check them with the `utils/font-embedding-check.pl` script, and if
that program prints anything (e.g. `edit only`), compile and run the [program
`utils/font-embedding-fix.c`][embed] on the font files to fix the embedding
permissions field.

[OS/2]: https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6OS2.html
[fsType]: https://learn.microsoft.com/en-us/typography/opentype/otspec140/os2#fst
[embed]: http://carnage-melon.tom7.org/embed/
