; Languages without translated licence
!macro STD_LANGUAGE Lg
  !insertmacro      MUI_LANGUAGE "${Lg}"
  LicenseLangString MUILicense   ${LANG_${Lg}}  "COPYING"
!macroend

; first language is the default language
  !insertmacro STD_LANGUAGE "English"
  !insertmacro STD_LANGUAGE "Afrikaans"
  !insertmacro STD_LANGUAGE "Albanian"
  !insertmacro STD_LANGUAGE "Arabic"
  !insertmacro STD_LANGUAGE "Belarusian"
  !insertmacro STD_LANGUAGE "Bosnian"
  !insertmacro STD_LANGUAGE "Breton"
  !insertmacro STD_LANGUAGE "Bulgarian"
  !insertmacro STD_LANGUAGE "Catalan"
  !insertmacro STD_LANGUAGE "Croatian"
  !insertmacro STD_LANGUAGE "Danish"
  !insertmacro STD_LANGUAGE "Dutch"
  !insertmacro STD_LANGUAGE "Estonian"
  !insertmacro STD_LANGUAGE "Farsi"
  !insertmacro STD_LANGUAGE "Finnish"
  !insertmacro STD_LANGUAGE "Greek"
  !insertmacro STD_LANGUAGE "Hebrew"
  !insertmacro STD_LANGUAGE "Icelandic"
  !insertmacro STD_LANGUAGE "Indonesian"
  !insertmacro STD_LANGUAGE "Irish"
  !insertmacro STD_LANGUAGE "Japanese"
  !insertmacro STD_LANGUAGE "Korean"
  !insertmacro STD_LANGUAGE "Kurdish"
  !insertmacro STD_LANGUAGE "Latvian"
  !insertmacro STD_LANGUAGE "Lithuanian"
  !insertmacro STD_LANGUAGE "Luxembourgish"
  !insertmacro STD_LANGUAGE "Macedonian"
  !insertmacro STD_LANGUAGE "Malay"
  !insertmacro STD_LANGUAGE "Mongolian"
  !insertmacro STD_LANGUAGE "Norwegian"
  !insertmacro STD_LANGUAGE "NorwegianNynorsk"
  !insertmacro STD_LANGUAGE "Romanian"
  !insertmacro STD_LANGUAGE "Serbian"
  !insertmacro STD_LANGUAGE "SerbianLatin"
  !insertmacro STD_LANGUAGE "Slovak"
  !insertmacro STD_LANGUAGE "Slovenian"
  !insertmacro STD_LANGUAGE "Spanish"
  !insertmacro STD_LANGUAGE "SpanishInternational"
  !insertmacro STD_LANGUAGE "Swedish"
  !insertmacro STD_LANGUAGE "Thai"
  !insertmacro STD_LANGUAGE "Turkish"
  !insertmacro STD_LANGUAGE "Ukrainian"
  !insertmacro STD_LANGUAGE "Uzbek"

; Translations
  !insertmacro		MUI_LANGUAGE	"Czech"
  LicenseLangString	MUILicense		${LANG_CZECH}			"packaging\windows\gpl-3.cs.txt"

  !insertmacro		MUI_LANGUAGE	"French"
  LicenseLangString	MUILicense		${LANG_FRENCH}			"packaging\windows\gpl-3.fr.txt"

  !insertmacro		MUI_LANGUAGE	"Galician"
  LicenseLangString	MUILicense		${LANG_GALICIAN}		"packaging\windows\gpl-2.gl.txt"

  !insertmacro		MUI_LANGUAGE	"German"
  LicenseLangString	MUILicense		${LANG_GERMAN}			"packaging\windows\gpl-2.de.txt"

  !insertmacro		MUI_LANGUAGE	"Hungarian"
  LicenseLangString	MUILicense		${LANG_HUNGARIAN}		"packaging\windows\gpl-3.hu.txt"

  !insertmacro		MUI_LANGUAGE	"Italian"
  LicenseLangString	MUILicense		${LANG_ITALIAN}			"packaging\windows\gpl-2.it.txt"

  !insertmacro		MUI_LANGUAGE	"Polish"
  LicenseLangString	MUILicense		${LANG_POLISH}			"packaging\windows\gpl-2.pl.txt"

  !insertmacro		MUI_LANGUAGE	"Portuguese"
  LicenseLangString	MUILicense		${LANG_PORTUGUESE}		"packaging\windows\gpl-2.pt.txt"

  !insertmacro		MUI_LANGUAGE	"PortugueseBR"
  LicenseLangString	MUILicense		${LANG_PORTUGUESEBR}	"packaging\windows\gpl-2.pt-br.txt"

  !insertmacro		MUI_LANGUAGE	"Russian"
  LicenseLangString	MUILicense		${LANG_RUSSIAN}			"packaging\windows\gpl-2.ru.txt"

  !insertmacro		MUI_LANGUAGE	"SimpChinese"
  LicenseLangString	MUILicense		${LANG_SIMPCHINESE}		"packaging\windows\gpl-2.zh-cn.txt"

  !insertmacro		MUI_LANGUAGE	"TradChinese"
  LicenseLangString	MUILicense		${LANG_TRADCHINESE}		"packaging\windows\gpl-2.zh-tw.txt"
