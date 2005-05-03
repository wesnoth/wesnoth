/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ gettext-runtime / gettext-runtime /
  tb
  s/ $/ gettext-runtime /
  :b
  s/^/# Packages using this file:/
}
