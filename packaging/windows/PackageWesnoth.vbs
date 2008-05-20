Option Explicit

Dim SourceDirectory, DLLDirectory, PythonDirectory, GetTextDirectory
Dim DestinationDirectory
Dim File, PoFile, MoFile, ExecuteString, domains, languages, domain, language
Dim fso, shell

'on error resume next

'Initialization
SourceDirectory = "C:\Entwicklung\Wesnoth\wesnoth-1.4.2\"
DLLDirectory = "C:\Entwicklung\Wesnoth\DLL\mingw\"
PythonDirectory = "C:\Entwicklung\Wesnoth\DLL\python\"
DestinationDirectory = "C:\Entwicklung\Wesnoth\Release 1.4.2\"
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
Call fso.CopyFile(SourceDirectory + "Readme", DestinationDirectory)

Call CopyFilesInDirectory(DLLDirectory + "bin", DestinationDirectory, fso)
Call CopyFilesInDirectory(SourceDirectory + "bin", DestinationDirectory, fso)
Call fso.CopyFolder(SourceDirectory + "data", DestinationDirectory)
Call fso.CopyFolder(SourceDirectory + "fonts", DestinationDirectory)
Call fso.CopyFolder(SourceDirectory + "images", DestinationDirectory)
Call fso.CopyFolder(PythonDirectory + "lib", DestinationDirectory)
Call fso.CopyFolder(SourceDirectory + "sounds", DestinationDirectory)
stop

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

domains = Array("wesnoth", "wesnoth-anl", "wesnoth-aoi", "wesnoth-did", "wesnoth-editor", "wesnoth-ei", "wesnoth-httt", "wesnoth-l", "wesnoth-lib", "wesnoth-multiplayer", "wesnoth-nr", "wesnoth-sof", "wesnoth-sotbe", "wesnoth-tb", "wesnoth-thot", "wesnoth-trow", "wesnoth-tsg", "wesnoth-tutorial", "wesnoth-units", "wesnoth-utbs")
Set File = fso.OpenTextFile(SourceDirectory + "po\wesnoth\LINGUAS", 1)
languages = split(File.ReadLine(), " ")

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

Sub CopyFilesInDirectory(strSourceDirectory, strDestinationDirectory, fso)
	Dim dirSource
	Dim file
	
	Set dirSource = fso.GetFolder(strSourceDirectory)
	
	For Each file in dirSource.Files
		Call fso.CopyFile(strSourceDirectory + "\" + file.Name, strDestinationDirectory)
	Next
	
	Set file = Nothing
	Set dirSource = Nothing
End Sub