Option Explicit

Dim SourceDirectory, DLLDirectory, PythonDirectory, GetTextDirectory
Dim DestinationDirectory
Dim PoFile, MoFile, ExecuteString, domains, languages, domain, language
Dim fso, shell

'on error resume next

'Initialization
SourceDirectory = "C:\Entwicklung\Wesnoth\wesnoth-1.3.19\"
DLLDirectory = "C:\Entwicklung\Wesnoth\DLL\mingw\"
PythonDirectory = "C:\Entwicklung\Wesnoth\DLL\python\"
DestinationDirectory = "C:\Entwicklung\Wesnoth\Release 1.3.19\"
GetTextDirectory = "D:\Programme\GnuWin32\bin\"
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")

'Copy files
If fso.FolderExists(DestinationDirectory) = False Then
	fso.CreateFolder(DestinationDirectory)
End If

Call fso.DeleteFile(DestinationDirectory + "*.*")
Call fso.CopyFile(SourceDirectory + "Copying", DestinationDirectory)
Call fso.CopyFile(SourceDirectory + "Copyright", DestinationDirectory)
Call fso.CopyFile(SourceDirectory + "editor.exe", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "editor.ico", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "freetype6.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "fribidi.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "gettextlib.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "gettextpo.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "gettextsrc.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "jpeg.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "iconv.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "libintl3.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "libogg-0.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "libpng12-0.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "libvorbis-0.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "libvorbisfile-3.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "mingwm10.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "msvcp60.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "python24.dll", DestinationDirectory)
Call fso.CopyFile(SourceDirectory + "Readme", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "SDL.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "SDL_image.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "SDL_mixer.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "SDL_net.dll", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "SDL_ttf.dll", DestinationDirectory)
Call fso.CopyFile(SourceDirectory + "wesnoth.exe", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "wesnoth.ico", DestinationDirectory)
Call fso.CopyFile(SourceDirectory + "wesnothd.exe", DestinationDirectory)
Call fso.CopyFile(DLLDirectory + "zlib1.dll", DestinationDirectory)
Call fso.CopyFolder(PythonDirectory + "lib", DestinationDirectory)

Call fso.CopyFolder(SourceDirectory + "data", DestinationDirectory)
Call fso.CopyFolder(SourceDirectory + "fonts", DestinationDirectory)
Call fso.CopyFolder(SourceDirectory + "images", DestinationDirectory)
Call fso.CopyFolder(SourceDirectory + "sounds", DestinationDirectory)

If fso.FolderExists(DestinationDirectory + "manual_html") = False Then
	Call fso.CreateFolder(DestinationDirectory + "manual_html")
End If
Call fso.CopyFile(SourceDirectory + "doc\manual\manual.en.html", DestinationDirectory + "\manual_html\")
Call fso.CopyFile(SourceDirectory + "doc\manual\manual.cs.html", DestinationDirectory + "\manual_html\")
Call fso.CopyFolder(SourceDirectory + "doc\manual\images", DestinationDirectory + "\manual_html\")
Call fso.CopyFolder(SourceDirectory + "doc\manual\styles", DestinationDirectory + "\manual_html\")

If fso.FolderExists(DestinationDirectory + "po") = False Then
	Call fso.CreateFolder(DestinationDirectory + "po")
End If

domains = Array("wesnoth", "wesnoth-aoi", "wesnoth-did", "wesnoth-editor", "wesnoth-ei", "wesnoth-httt", "wesnoth-l", "wesnoth-lib", "wesnoth-multiplayer", "wesnoth-nr", "wesnoth-sof", "wesnoth-sotbe", "wesnoth-tb", "wesnoth-thot", "wesnoth-trow", "wesnoth-tsg", "wesnoth-tutorial", "wesnoth-units", "wesnoth-utbs")
languages = Array("af", "bg", "ca", "ca_ES@valencia", "cs", "da", "de", "el", "en_GB", "eo", "es", "et", "eu", "fi", "fr", "gl_ES", "he", "hu", "id", "it", "ja", "ko", "la", "nb_NO", "nl", "pl", "pt", "pt_BR", "ro", "ru", "sk", "sl", "sr", "sr@latin", "sv", "tl", "tr", "zh_CN")

For Each domain In domains
	For Each language In languages
		If fso.FolderExists(DestinationDirectory + "po\" + language) = False Then
			fso.CreateFolder(DestinationDirectory + "po\" + language)
		End If
		If fso.FolderExists(DestinationDirectory + "po\" + language + "\LC_MESSAGES") = False Then
			fso.CreateFolder(DestinationDirectory + "po\" + language + "\LC_MESSAGES")
		End If
	
		PoFile = SourceDirectory + "po\" + domain + "\" + language + ".po"
		MoFile = DestinationDirectory + "po\" + language + "\LC_MESSAGES\" + domain + ".mo"
		shell.Run """" + GetTextDirectory + "msgfmt.exe"" " + """" + PoFile + """ -o """ + MoFile + """", 1, True
	Next
Next

Set shell = Nothing
Set fso = Nothing

MsgBox "Fertig"

