/*
_____________________________________________________________________________

                       File Functions Header v3.3
_____________________________________________________________________________

 2006 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)

 Copy "FileFunc.nsh" to NSIS include directory
 Usually "C:\Program Files\NSIS\Include"

 Usage in script:
 1. !include "FileFunc.nsh"
 2. !insertmacro FileFunction
 3. [Section|Function]
      ${FileFunction} "Param1" "Param2" "..." $var
    [SectionEnd|FunctionEnd]


 FileFunction=[Locate|GetSize|DriveSpace|GetDrives|GetTime|GetFileAttributes|
               GetFileVersion|GetExeName|GetExePath|GetParameters|GetOptions|
               GetOptionsS|GetRoot|GetParent|GetFileName|GetBaseName|GetFileExt|
               BannerTrimPath|DirState|RefreshShellIcons]

 un.FileFunction=[un.Locate|un.GetSize|un.DriveSpace|un.GetDrives|un.GetTime|
                  un.GetFileAttributes|un.GetFileVersion|un.GetExeName|
                  un.GetExePath|un.GetParameters|un.GetOptions|un.GetOptionsS|
                  un.GetRoot|un.GetParent|un.GetFileName|un.GetBaseName|
                  un.GetFileExt|un.BannerTrimPath|un.DirState|un.RefreshShellIcons]

____________________________________________________________________________

                         Locate v1.8
____________________________________________________________________________


Find files, directories and empty directories with mask and size options.


Syntax:
${Locate} "[Path]" "[Options]" "Function"

"[Path]"      ; Disk or Directory
              ;
"[Options]"   ; /L=[FD|F|D|DE|FDE]
              ;     /L=FD    - Locate Files and Directories (default)
              ;     /L=F     - Locate Files only
              ;     /L=D     - Locate Directories only
              ;     /L=DE    - Locate Empty Directories only
              ;     /L=FDE   - Locate Files and Empty Directories
              ; /M=[mask]
              ;     /M=*.*         - Locate all (default)
              ;     /M=*.doc       - Locate Work.doc, 1.doc ...
              ;     /M=Pho*        - Locate PHOTOS, phone.txt ...
              ;     /M=win???.exe  - Locate winamp.exe, winver.exe ...
              ;     /M=winamp.exe  - Locate winamp.exe only
              ; /S=No:No[B|K|M|G]
              ;     /S=      - Don't locate file size (faster) (default)
              ;     /S=0:0B  - Locate only files of 0 Bytes exactly
              ;     /S=5:9K  - Locate only files of 5 to 9 Kilobytes
              ;     /S=:10M  - Locate only files of 10 Megabyte or less
              ;     /S=1G    - Locate only files of 1 Gigabyte or more
              ; /G=[1|0]
              ;     /G=1     - Locate with subdirectories (default)
              ;     /G=0     - Locate without subdirectories
              ; /B=[0|1]
              ;     /B=0     - Banner isn't used (default)
              ;     /B=1     - Banner is used. Callback when function
              ;                start to search in new directory
"Function"    ; Callback function when found

Function "Function"
	; $R9    "path\name"
	; $R8    "path"
	; $R7    "name"
	; $R6    "size"  ($R6="" if directory, $R6="0" if file with /S=)

	; $R0-$R5  are not used (save data in them).
	; ...


	Push $var    ; If $var="StopLocate" Then exit from function
FunctionEnd


Note:
- Error flag if disk or directory isn't exist
- Error flag if syntax error



Example (Find one file):
Section
	${Locate} "C:\ftp" "/L=F /M=RPC DCOM.rar /S=1K" "Example1"
	; 'RPC DCOM.rar' file in 'C:\ftp' with size 1 Kb or more

	IfErrors 0 +2
	MessageBox MB_OK "Error" IDOK +2
	MessageBox MB_OK "$$R0=$R0"
SectionEnd

Function Example1
	StrCpy $R0 $R9
	; $R0="C:\ftp\files\RPC DCOM.rar"

	MessageBox MB_YESNO '$R0$\n$\nFind next?' IDYES +2
	StrCpy $0 StopLocate

	Push $0
FunctionEnd



Example (Write founded in text file):
Section
	GetTempFileName $R0
	FileOpen $R1 $R0 w
	${Locate} "C:\ftp" "/S=:2M /G=0" "Example2"
	; folders and all files with size 2 Mb or less
	; don't scan subdirectories
	FileClose $R1

	IfErrors 0 +2
	MessageBox MB_OK "Error" IDOK +2
	Exec '"notepad.exe" "$R0"'
SectionEnd

Function Example2
	StrCmp $R6 '' 0 +3
	FileWrite $R1 "Directory=$R9$\r$\n"
	goto +2
	FileWrite $R1 "File=$R9  Size=$R6 Mb$\r$\n"

	Push $0
FunctionEnd



Example (Write founded in INI file):
Section
	GetTempFileName $R0
	${Locate} "C:\ftp" "/L=F /S=0K" "Example3"
	; all files in 'C:\ftp' with size detect in Kb

	IfErrors 0 +2
	MessageBox MB_OK "Error" IDOK +2
	Exec '"notepad.exe" "$R0"'
SectionEnd

Function Example3
	WriteINIStr $R0 "$R8" "$R7" "$R6 Kb"

	Push $0
FunctionEnd



Example (Delete empty directories):
Section
	StrCpy $R2 0
	StrCpy $R3 0

	loop:
	StrCpy $R1 0
	${Locate} "C:\ftp" "/L=DE" "Example4"
	IntOp $R3 $R3 + 1
	IntOp $R2 $R2 + $R1
	StrCmp $R0 StopLocate +2
	StrCmp $R1 0 0 loop

	IfErrors 0 +2
	MessageBox MB_OK 'error' IDOK +2
	MessageBox MB_OK '$R2 directories were removed$\n$R3 loops'
SectionEnd

Function Example4
	MessageBox MB_YESNOCANCEL 'Delete empty "$R9"?' IDNO end IDCANCEL cancel
	RMDir $R9
	IntOp $R1 $R1 + 1
	goto end

	cancel:
	StrCpy $R0 StopLocate

	end:
	Push $R0
FunctionEnd



Example (Move all files into one folder):
Section
	StrCpy $R0 "C:\ftp"   ;Directory move from
	StrCpy $R1 "C:\ftp2"  ;Directory move into

	StrCpy $R2 0
	StrCpy $R3 0
	${Locate} "$R0" "/L=F" "Example5"

	IfErrors 0 +2
	MessageBox MB_OK 'error' IDOK +4
	StrCmp $R3 0 0 +2
	MessageBox MB_OK '$R2 files were moved' IDOK +2
	MessageBox MB_OK '$R2 files were moved$\n$R3 files were NOT moved'
SectionEnd

Function Example5
	StrCmp $R8 $R1 +6
	IfFileExists '$R1\$R7' +4
	Rename $R9 '$R1\$R7'
	IntOp $R2 $R2 + 1
	goto +2
	IntOp $R3 $R3 + 1

	Push $0
FunctionEnd



Example (Copy files with log):
Section
	StrCpy $R0 "C:\ftp"   ;Directory copy from
	StrCpy $R1 "C:\ftp2"  ;Directory copy into
	StrLen $R2 $R0

	GetTempFileName $0
	FileOpen $R3 $0 w
	${Locate} "$R0" "/L=FDE" "Example6"
	FileClose $R3

	IfErrors 0 +2
	MessageBox MB_OK 'error'

	Exec '"notepad.exe" "$0"'     ;view log
SectionEnd

Function Example6
	StrCpy $1 $R8 '' $R2

	StrCmp $R6 '' 0 +3
	CreateDirectory '$R1$1\$R7'
	goto end
	CreateDirectory '$R1$1'
	CopyFiles /SILENT $R9 '$R1$1'

	IfFileExists '$R1$1\$R7' 0 +3
	FileWrite $R3 "-old:$R9  -new:$R1$1\$R7  -success$\r$\n"
	goto +2
	FileWrite $R3 "-old:$R9  -new:$R1$1\$R7  -failed$\r$\n"

	end:
	Push $0
FunctionEnd



Example (Recreate directory structure):
Section
	StrCpy $R0 "C:\ftp"     ;Directory structure from
	StrCpy $R1 "C:\ftp2"    ;Directory structure into
	StrLen $R2 $R0

	${Locate} "$R0" "/L=D" "Example7"

	IfErrors 0 +2
	MessageBox MB_OK 'error'
SectionEnd

Function Example7
	StrCpy $1 $R9 '' $R2
	CreateDirectory '$R1$1'

	Push $0
FunctionEnd



Example (Locate with banner - "NxS" plugin required):
Section
	nxs::Show /NOUNLOAD `$(^Name) Setup` /top `Setup searching something$\r$\nPlease wait... If you can..` /h 1 /can 1 /end
	${Locate} "C:\WINDOWS" "/L=F /M=*.inf /B=1" "Example8"
	nxs::Destroy
SectionEnd

Function Example8
	StrCmp $R0 $R8 abortcheck
	StrCpy $R0 $R8
	nxs::Update /NOUNLOAD /sub "$R8" /pos 78 /end

	abortcheck:
	nxs::HasUserAborted /NOUNLOAD
	Pop $0
	StrCmp $0 1 0 +2
	StrCpy $0 StopLocate

	StrCmp $R9 '' end
	;...

	end:
	Push $0
FunctionEnd

____________________________________________________________________________

                         GetSize v2.0
____________________________________________________________________________

Thanks KiCHiK (Function "FindFiles")


Features:
1. Find the size of a file, files mask or directory.
2. Find the sum of the files, directories and subdirectories.


Syntax:
${GetSize} "[Path]" "[Options]" $var1 $var2 $var3

"[Path]"      ; Disk or Directory
              ;
"[Options]"   ; /M=[mask]
              ;     /M=*.*         - Find all (default)
              ;     /M=*.doc       - Find Work.doc, 1.doc ...
              ;     /M=Pho*        - Find PHOTOS, phone.txt ...
              ;     /M=win???.exe  - Find winamp.exe, winver.exe ...
              ;     /M=winamp.exe  - Find winamp.exe only
              ; /S=No:No[B|K|M|G]
              ;     /S=      - Don't find file size (faster) (default)
              ;     /S=0:0B  - Find only files of 0 Bytes exactly
              ;     /S=5:9K  - Find only files of 5 to 9 Kilobytes
              ;     /S=:10M  - Find only files of 10 Megabyte or less
              ;     /S=1G    - Find only files of 1 Gigabyte or more
              ; /G=[1|0]
              ;     /G=1     - Find with subdirectories (default)
              ;     /G=0     - Find without subdirectories
              ;
$var1         ; Result1: Size
$var2         ; Result2: Sum of files
$var3         ; Result3: Sum of directories


Note:
- Error flag if disk or directory isn't exist
- Error flag if syntax error



Example (1):
Section
	; Find file size "C:\WINDOWS\Explorer.exe" in kilobytes

	${GetSize} "C:\WINDOWS" "/M=Explorer.exe /S=0K /G=0" $0 $1 $2
	; $0="220" Kb
	; $1="1"   files
	; $2=""    directories

	IfErrors 0 +2
	MessageBox MB_OK "Error"
SectionEnd

Example (2):
Section
	; Find folder size "C:\Installs\Reanimator\Drivers" in megabytes

	${GetSize} "C:\Installs\Reanimator\Drivers" "/S=0M" $0 $1 $2
	; $0="132" Mb
	; $1="555" files
	; $2="55"  directories

	IfErrors 0 +2
	MessageBox MB_OK "Error"
SectionEnd

Example (3):
Section
	; Find sum of files and folders "C:\WINDOWS" (no subfolders)

	${GetSize} "C:\WINDOWS" "/G=0" $0 $1 $2
	; $0=""    size
	; $1="253" files
	; $2="46"  directories

	IfErrors 0 +2
	MessageBox MB_OK "Error"
SectionEnd

____________________________________________________________________________

                         DriveSpace v1.2
____________________________________________________________________________

Thanks sunjammer (Function "CheckSpaceFree")


Get total, occupied or free space of the drive.


Syntax:
${GetSize} "[Drive]" "[Options]" $var

"[Drive]"     ; Disk to check
              ;     
"[Options]"   ; /D=[T|O|F]
              ;     /D=T  - Total space (default)
              ;     /D=O  - Occupied space
              ;     /D=F  - Free space
              ; /S=[B|K|M|G]
              ;     /S=B  - size in Bytes (default)
              ;     /S=K  - size in Kilobytes
              ;     /S=M  - size in Megabytes
              ;     /S=G  - size in Gigabytes
              ;
$var          ; Result: Size


Note:
- Error flag if disk isn't exist or not ready
- Error flag if syntax error



Example:
Section
	${DriveSpace} "C:\" "/D=F /S=M" $R0
	; $R0="2530"   megabytes free on drive C:
SectionEnd

____________________________________________________________________________

                         GetDrives v1.5
____________________________________________________________________________

Thanks deguix (Based on his idea of Function "DetectDrives")


Find all available drives in the system.


Syntax:
${GetDrives} "[Option]" "Function"

"[Option]"      ; [FDD+HDD+CDROM+NET+RAM]
                ;   FDD    Floppy Disk Drives
                ;   HDD    Hard Disk Drives 
                ;   CDROM  CD-ROM Drives
                ;   NET    Network Drives
                ;   RAM    RAM Disk Drives
                ;
                ; [ALL]
                ;   Find all drives by letter (default)
                ;
"Function"      ; Callback function when found

Function "Function"
	; $9    "drive letter"  (a:\ c:\ ...)
	; $8    "drive type"    (FDD HDD ...)


	; $R0-$R9  are not used (save data in them).
	; ...


	Push $var    ; If $var="StopGetDrives" Then exit from function
FunctionEnd



Example1:
Section
	${GetDrives} "FDD+CDROM" "Example1"
SectionEnd

Function Example1
	MessageBox MB_OK "$9  ($8 Drive)"

	Push $0
FunctionEnd



Example2:
Section
	${GetDrives} "ALL" "Example2"
SectionEnd

Function Example2
	MessageBox MB_OK "$9  ($8 Drive)"

	Push $0
FunctionEnd



Example3 (Get type of drive):
Section
	StrCpy $R0 "D:\"      ;Drive letter
	StrCpy $R1 "invalid"

	${GetDrives} "ALL" "Example3"

	MessageBox MB_OK "Type of drive $R0 is $R1"
SectionEnd

Function Example3
	StrCmp $9 $R0 0 +3
	StrCpy $R1 $8
	StrCpy $0 StopGetDrives

	Push $0
FunctionEnd

____________________________________________________________________________

                         GetTime v1.5
____________________________________________________________________________

Thanks Takhir (Script "StatTest") and deguix (Function "FileModifiedDate")


Features:
1. Get local or system time.
2. Get file time (access, creation and modification).


Syntax:
${GetTime} "[File]" "[Option]" $var1 $var2 $var3 $var4 $var5 $var6 $var7

"[File]"        ; Ignored if "L" or "LS"
                ;
"[Option]"      ; [Options]
                ;   L   Local time
                ;   A   last Access file time
                ;   C   Creation file time
                ;   M   Modification file time
                ;   LS  System time (UTC)
                ;   AS  last Access file time (UTC)
                ;   CS  Creation file time (UTC)
                ;   MS  Modification file time (UTC)
                ;
$var1           ; Result1: day
$var2           ; Result2: month
$var3           ; Result3: year
$var4           ; Result4: day of week name
$var5           ; Result5: hour
$var6           ; Result6: minute
$var7           ; Result7: seconds


Note:
- Error flag if file isn't exist
- Error flag if syntax error



Example (Get local time):
Section
	${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
	; $0="01"      day
	; $1="04"      month
	; $2="2005"    year
	; $3="Friday"  day of week name
	; $4="16"      hour
	; $5="05"      minute
	; $6="50"      seconds

	MessageBox MB_OK 'Date=$0/$1/$2 ($3)$\nTime=$4:$5:$6'
SectionEnd


Example (Get file time):
Section
	${GetTime} "$WINDIR\Explorer.exe" "C" $0 $1 $2 $3 $4 $5 $6
	; $0="12"       day
	; $1="10"       month
	; $2="2004"     year
	; $3="Tuesday"  day of week name
	; $4="2"        hour
	; $5="32"       minute
	; $6="03"       seconds

	IfErrors 0 +2
	MessageBox MB_OK "Error" IDOK +2
	MessageBox MB_OK 'Date=$0/$1/$2 ($3)$\nTime=$4:$5:$6'
SectionEnd


Example (Get system time):
Section
	${GetTime} "" "LS" $0 $1 $2 $3 $4 $5 $6
	; $0="01"      day
	; $1="04"      month
	; $2="2005"    year
	; $3="Friday"  day of week name
	; $4="11"      hour
	; $5="05"      minute
	; $6="50"      seconds

	MessageBox MB_OK 'Date=$0/$1/$2 ($3)$\nTime=$4:$5:$6'
SectionEnd


Example (Convert time to 12-hour format AM/PM):
Section
	${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6

	StrCmp $4 0 0 +3
	StrCpy $4 12
	goto +3
	StrCmp $4 12 +5
	IntCmp $4 12 0 0 +3
	StrCpy $7 AM
	goto +3
	IntOp $4 $4 - 12
	StrCpy $7 PM

	MessageBox MB_OK 'Date=$0/$1/$2 ($3)$\nTime=$4:$5:$6 $7'
SectionEnd

____________________________________________________________________________

                         GetFileAttributes v1.2
____________________________________________________________________________


Get attributes of file or directory.


Syntax:
${GetFileAttributes} "[File]" "[Attributes]" $var

"[File]"          ; File or directory
                  ;
"[Attributes]"    ; "ALL"  (default)
                  ;  -all attributes of file combined with "|" to output
                  ;
                  ; "READONLY|HIDDEN|SYSTEM|DIRECTORY|ARCHIVE|
                  ; DEVICE|NORMAL|TEMPORARY|SPARSE_FILE|REPARSE_POINT|
                  ; COMPRESSED|OFFLINE|NOT_CONTENT_INDEXED|ENCRYPTED"
                  ;  -file must have specified attributes
                  ;
$var              ; Result:
                  ;    $var=attr1|attr2|... (if used "ALL")
                  ;    $var=1   file has specified attributes
                  ;    $var=0   file has no specified attributes


Note:
- Error flag if file isn't exist



Example1:
Section
	${GetFileAttributes} "C:\MSDOS.SYS" "ALL" $R0
	; $R0=READONLY|HIDDEN|SYSTEM|ARCHIVE
SectionEnd

Example2:
Section
	${GetFileAttributes} "C:\MSDOS.SYS" "SYSTEM|HIDDEN" $R0
	; $R0=1
SectionEnd

Example3:
Section
	${GetFileAttributes} "C:\MSDOS.SYS" "NORMAL" $R0
	; $R0=0
SectionEnd

____________________________________________________________________________

                         GetFileVersion
____________________________________________________________________________

Thanks KiCHiK (Based on his example for command "GetDLLVersion")


Get version information from executable file.


Syntax:
${GetFileVersion} "[Executable]" $var

"[Executable]"      ; Executable file (*.exe *.dll ...)
$var                ; Result: Version number


Note:
- Error flag if file isn't exist
- Error flag if file isn't contain version information



Example:
Section
	${GetFileVersion} "C:\ftp\program.exe" $R0
	; $R0="1.1.0.12"
SectionEnd

____________________________________________________________________________

                         GetExeName
____________________________________________________________________________


Get installer filename (with valid case for Windows 98/Me).


Syntax:
${GetExeName} $var


Example:
Section
	${GetExeName} $R0
	; $R0="C:\ftp\program.exe"
SectionEnd

____________________________________________________________________________

                         GetExePath
____________________________________________________________________________


Get installer pathname ($EXEDIR with valid case for Windows 98/Me).


Syntax:
${GetExePath} $var


Example:
Section
	${GetExePath} $R0
	; $R0="C:\ftp"
SectionEnd

____________________________________________________________________________

                         GetParameters
____________________________________________________________________________

Thanks sunjammer (Based on his Function "GetParameters")


Get command line parameters.


Syntax:
${GetParameters} $var


Example:
Section
	${GetParameters} $R0
	; $R0="[parameters]"
SectionEnd

____________________________________________________________________________

                         GetOptions
____________________________________________________________________________


Get options from command line parameters.


Syntax:
${GetOptions} "[Parameters]" "[Option]" $var

"[Parameters]"     ; command line parameters
                   ;
"[Option]"         ; option name
                   ;
$var               ; Result: option string


Note:
- Error flag if option not found
- First option symbol it is delimiter



Example1:
Section
	${GetOptions} "/S /T" "/T"  $R0

	IfErrors 0 +2
	MessageBox MB_OK "Not found" IDOK +2
	MessageBox MB_OK "Found"
SectionEnd

Example2:
Section
	${GetOptions} "-INSTDIR=C:\Program Files\Common Files -SILENT=yes" "-INSTDIR="  $R0
	;$R0=C:\Program Files\Common Files
SectionEnd

Example3:
Section
	${GetOptions} '/SILENT=yes /INSTDIR="C:/Program Files/Common Files" /ADMIN=password' "/INSTDIR="  $R0
	;$R0=C:/Program Files/Common Files
SectionEnd

Example4:
Section
	${GetOptions} `-SILENT=yes -INSTDIR='"C:/Program Files/Common Files"' -ADMIN=password` "-INSTDIR="  $R0
	;$R0="C:/Program Files/Common Files"
SectionEnd

____________________________________________________________________________

                         GetOptionsS
____________________________________________________________________________

Same as GetOptions, but case sensitive.

____________________________________________________________________________

                         GetRoot (not like in NSIS CVS)
____________________________________________________________________________

Thanks KiCHiK (Based on his Function "GetRoot")


Get root directory.


Syntax:
${GetRoot} "[FullPath]" $var


Example1:
Section
	${GetRoot} "C:\Program Files\NSIS" $R0
	; $R0="C:"
SectionEnd

Example2:
Section
	${GetRoot} "\\SuperPimp\NSIS\Source\exehead\Ui.c" $R0
	; $R0="\\SuperPimp\NSIS"
SectionEnd

Example3:
Section
	${GetRoot} "NSIS\Source\exehead\Ui.c" $R0
	; $R0="NSIS"
SectionEnd

____________________________________________________________________________

                         GetParent
____________________________________________________________________________

Thanks sunjammer (Based on his Function "GetParent")


Get parent directory.


Syntax:
${GetParent} "[PathString]" $var


Example:
Section
	${GetParent} "C:\Program Files\Winamp\uninstwa.exe" $R0
	; $R0="C:\Program Files\Winamp"
SectionEnd

____________________________________________________________________________

                         GetFileName
____________________________________________________________________________

Thanks KiCHiK (Based on his Function "GetFileName")


Get last part from directory path.


Syntax:
${GetFileName} "[PathString]" $var


Example:
Section
	${GetFileName} "C:\Program Files\Winamp\uninstwa.exe" $R0
	; $R0="uninstwa.exe"
SectionEnd

____________________________________________________________________________

                         GetBaseName
____________________________________________________________________________

Thanks comperio (Based on his idea of Function "GetBaseName")


Get file name without extension.


Syntax:
${GetBaseName} "[FileString]" $var


Example:
Section
	${GetBaseName} "C:\ftp\program.exe" $R0
	; $R0="program"
SectionEnd

____________________________________________________________________________

                         GetFileExt
____________________________________________________________________________

Written by opher


Get extention of file.


Syntax:
${GetFileExt} "[FileString]" $var


Example:
Section
	${GetFileExt} "C:\ftp\program.exe" $R0
	; $R0="exe"
SectionEnd

____________________________________________________________________________

                         BannerTrimPath
____________________________________________________________________________


Trim string path for banner.


Syntax:
${BannerTrimPath} "[PathString]" "[Option]" $var

"[PathString]"    ;
                  ;
"[Option]"        ; [Length][A|B|C|D]
                  ;
                  ; Length  -Maximum string length
                  ;   A     -Trim center path (default)
                  ;           (C:\root\...\third path) 
                  ;           If A mode not possible Then will be used B mode
                  ;   B     -Trim right path
                  ;           (C:\root\second path\...)
                  ;           If B mode not possible Then will be used C mode
                  ;   C     -Trim right string
                  ;           (C:\root\second path\third p...)
                  ;   D     -Trim right string + filename
                  ;           (C:\root\second p...\third path)
                  ;           If D mode not possible Then will be used C mode
                  ;
$var              ; Result:  Trimmed path


Example:
Section
	${BannerTrimPath} "C:\Server\Documents\Terminal\license.htm" "34A" $R0
	;$R0=C:\Server\...\Terminal\license.htm
SectionEnd



Example (Banner plugin):
!include "WinMessages.nsh"
!include "FileFunc.nsh"
!insertmacro Locate

Section
	Banner::show /NOUNLOAD "Starting..."
	Banner::getWindow /NOUNLOAD
	Pop $R1
	${Locate} "$WINDIR" "/L=F /M=*.* /B=1" "LocateCallback"
	Banner::destroy
SectionEnd

Function LocateCallback
	StrCmp $R0 $R8 code
	StrCpy $R0 $R8
	${BannerTrimPath} "$R8" "38B" $R8
	GetDlgItem $1 $R1 1030
	SendMessage $1 ${WM_SETTEXT} 0 "STR:$R8"

	code:
	StrCmp $R9 '' end
	;...

	end:
	Push $0
FunctionEnd



Example (nxs plugin):
!include "FileFunc.nsh"
!insertmacro Locate

Section
	nxs::Show /NOUNLOAD `$(^Name) Setup`\
	  /top `Setup searching something$\nPlease wait$\nIf you can...`\
	  /h 1 /can 1 /end
	${Locate} "$WINDIR" "/L=F /M=*.* /B=1" "LocateCallback"
	nxs::Destroy
SectionEnd

Function LocateCallback
	StrCmp $R0 $R8 abortcheck
	StrCpy $R0 $R8
	${BannerTrimPath} "$R8" "55A" $R8
	nxs::Update /NOUNLOAD /sub "$R8" /pos 78 /end

	abortcheck:
	nxs::HasUserAborted /NOUNLOAD
	Pop $0
	StrCmp $0 1 0 +2
	StrCpy $0 StopLocate

	StrCmp $R9 '' end
	;...

	end:
	Push $0
FunctionEnd

____________________________________________________________________________

                         DirState
____________________________________________________________________________


Check directory full, empty or not exist.


Syntax:
${DirState} "[path]" $var

"[path]"      ; Directory
$var          ; Result:
              ;    $var=0  (empty)
              ;    $var=1  (full)
              ;    $var=-1 (directory not found)


Example:
Section
	${DirState} "$TEMP" $R0
	; $R0="1"  directory is full
SectionEnd

____________________________________________________________________________

                         RefreshShellIcons
____________________________________________________________________________

Written by jerome tremblay


After changing file associations, you can call this function
to refresh the shell immediately.


Syntax:
${RefreshShellIcons}


Example:
Section
	WriteRegStr HKCR "Winamp.File\DefaultIcon" "" "$PROGRAMFILES\Winamp\WINAMP.EXE,2"

	${RefreshShellIcons}
SectionEnd
*/


;_____________________________________________________________________________
;
;                         Macros
;_____________________________________________________________________________
;
; Change log window verbosity (default: 3=no script)
;
; Example:
; !include "FileFunc.nsh"
; !insertmacro Locate
; ${FILEFUNC_VERBOSE} 4   # all verbosity
; !insertmacro VersionCompare
; ${FILEFUNC_VERBOSE} 3   # no script

!ifndef FILEFUNC_INCLUDED
!define FILEFUNC_INCLUDED

!verbose push
!verbose 3
!ifndef _FILEFUNC_VERBOSE
	!define _FILEFUNC_VERBOSE 3
!endif
!verbose ${_FILEFUNC_VERBOSE}
!define FILEFUNC_VERBOSE `!insertmacro FILEFUNC_VERBOSE`
!define _FILEFUNC_UN
!define _FILEFUNC_S
!verbose pop

!macro FILEFUNC_VERBOSE _VERBOSE
	!verbose push
	!verbose 3
	!undef _FILEFUNC_VERBOSE
	!define _FILEFUNC_VERBOSE ${_VERBOSE}
	!verbose pop
!macroend


# Install. Case insensitive. #

!macro LocateCall _PATH _OPTIONS _FUNC
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push $0
	Push `${_PATH}`
	Push `${_OPTIONS}`
	GetFunctionAddress $0 `${_FUNC}`
	Push `$0`
	Call Locate
	Pop $0
	!verbose pop
!macroend

!macro GetSizeCall _PATH _OPTIONS _RESULT1 _RESULT2 _RESULT3
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Push `${_OPTIONS}`
	Call GetSize
	Pop ${_RESULT1}
	Pop ${_RESULT2}
	Pop ${_RESULT3}
	!verbose pop
!macroend

!macro DriveSpaceCall _DRIVE _OPTIONS _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_DRIVE}`
	Push `${_OPTIONS}`
	Call DriveSpace
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetDrivesCall _DRV _FUNC
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push $0
	Push `${_DRV}`
	GetFunctionAddress $0 `${_FUNC}`
	Push `$0`
	Call GetDrives
	Pop $0
	!verbose pop
!macroend

!macro GetTimeCall _FILE _OPTION _RESULT1 _RESULT2 _RESULT3 _RESULT4 _RESULT5 _RESULT6 _RESULT7
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILE}`
	Push `${_OPTION}`
	Call GetTime
	Pop ${_RESULT1}
	Pop ${_RESULT2}
	Pop ${_RESULT3}
	Pop ${_RESULT4}
	Pop ${_RESULT5}
	Pop ${_RESULT6}
	Pop ${_RESULT7}
	!verbose pop
!macroend

!macro GetFileAttributesCall _PATH _ATTR _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Push `${_ATTR}`
	Call GetFileAttributes
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetFileVersionCall _FILE _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILE}`
	Call GetFileVersion
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetExeNameCall _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call GetExeName
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetExePathCall _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call GetExePath
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetParametersCall _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call GetParameters
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetOptionsCall _PARAMETERS _OPTION _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PARAMETERS}`
	Push `${_OPTION}`
	Call GetOptions
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetRootCall _FULLPATH _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FULLPATH}`
	Call GetRoot
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetParentCall _PATHSTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATHSTRING}`
	Call GetParent
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetFileNameCall _PATHSTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATHSTRING}`
	Call GetFileName
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetBaseNameCall _FILESTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILESTRING}`
	Call GetBaseName
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetFileExtCall _FILESTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILESTRING}`
	Call GetFileExt
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro BannerTrimPathCall _PATH _LENGHT _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Push `${_LENGHT}`
	Call BannerTrimPath
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro DirStateCall _PATH _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Call DirState
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro RefreshShellIconsCall
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call RefreshShellIcons
	!verbose pop
!macroend

!macro Locate
	!ifndef ${_FILEFUNC_UN}Locate
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}Locate `!insertmacro ${_FILEFUNC_UN}LocateCall`

		Function ${_FILEFUNC_UN}Locate
			Exch $2
			Exch
			Exch $1
			Exch
			Exch 2
			Exch $0
			Exch 2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $7
			Push $8
			Push $9
			Push $R6
			Push $R7
			Push $R8
			Push $R9
			ClearErrors

			StrCpy $3 ''
			StrCpy $4 ''
			StrCpy $5 ''
			StrCpy $6 ''
			StrCpy $7 ''
			StrCpy $8 0
			StrCpy $R7 ''

			StrCpy $R9 $0 1 -1
			StrCmp $R9 '\' 0 +3
			StrCpy $0 $0 -1
			goto -3
			IfFileExists '$0\*.*' 0 error

			option:
			StrCpy $R9 $1 1
			StrCpy $1 $1 '' 1
			StrCmp $R9 ' ' -2
			StrCmp $R9 '' sizeset
			StrCmp $R9 '/' 0 -4
			StrCpy $9 -1
			IntOp $9 $9 + 1
			StrCpy $R9 $1 1 $9
			StrCmp $R9 '' +2
			StrCmp $R9 '/' 0 -3
			StrCpy $R8 $1 $9
			StrCpy $R8 $R8 '' 2
			StrCpy $R9 $R8 '' -1
			StrCmp $R9 ' ' 0 +3
			StrCpy $R8 $R8 -1
			goto -3
			StrCpy $R9 $1 2
			StrCpy $1 $1 '' $9

			StrCmp $R9 'L=' 0 mask
			StrCpy $3 $R8
			StrCmp $3 '' +6
			StrCmp $3 'FD' +5
			StrCmp $3 'F' +4
			StrCmp $3 'D' +3
			StrCmp $3 'DE' +2
			StrCmp $3 'FDE' 0 error
			goto option

			mask:
			StrCmp $R9 'M=' 0 size
			StrCpy $4 $R8
			goto option

			size:
			StrCmp $R9 'S=' 0 gotosubdir
			StrCpy $6 $R8
			goto option

			gotosubdir:
			StrCmp $R9 'G=' 0 banner
			StrCpy $7 $R8
			StrCmp $7 '' +3
			StrCmp $7 '1' +2
			StrCmp $7 '0' 0 error
			goto option

			banner:
			StrCmp $R9 'B=' 0 error
			StrCpy $R7 $R8
			StrCmp $R7 '' +3
			StrCmp $R7 '1' +2
			StrCmp $R7 '0' 0 error
			goto option

			sizeset:
			StrCmp $6 '' default
			StrCpy $9 0
			StrCpy $R9 $6 1 $9
			StrCmp $R9 '' +4
			StrCmp $R9 ':' +3
			IntOp $9 $9 + 1
			goto -4
			StrCpy $5 $6 $9
			IntOp $9 $9 + 1
			StrCpy $1 $6 1 -1
			StrCpy $6 $6 -1 $9
			StrCmp $5 '' +2
			IntOp $5 $5 + 0
			StrCmp $6 '' +2
			IntOp $6 $6 + 0

			StrCmp $1 'B' 0 +3
			StrCpy $1 1
			goto default
			StrCmp $1 'K' 0 +3
			StrCpy $1 1024
			goto default
			StrCmp $1 'M' 0 +3
			StrCpy $1 1048576
			goto default
			StrCmp $1 'G' 0 error
			StrCpy $1 1073741824

			default:
			StrCmp $3 '' 0 +2
			StrCpy $3 'FD'
			StrCmp $4 '' 0 +2
			StrCpy $4 '*.*'
			StrCmp $7 '' 0 +2
			StrCpy $7 '1'
			StrCmp $R7 '' 0 +2
			StrCpy $R7 '0'
			StrCpy $7 'G$7B$R7'

			StrCpy $8 1
			Push $0
			SetDetailsPrint textonly

			nextdir:
			IntOp $8 $8 - 1
			Pop $R8

			StrCpy $9 $7 2 2
			StrCmp $9 'B0' +3
			GetLabelAddress $9 findfirst
			goto call
			DetailPrint 'Search in: $R8'

			findfirst:
			FindFirst $0 $R7 '$R8\$4'
			IfErrors subdir
			StrCmp $R7 '.' 0 dir
			FindNext $0 $R7
			StrCmp $R7 '..' 0 dir
			FindNext $0 $R7
			IfErrors 0 dir
			FindClose $0
			goto subdir

			dir:
			IfFileExists '$R8\$R7\*.*' 0 file
			StrCpy $R6 ''
			StrCmp $3 'DE' +4
			StrCmp $3 'FDE' +3
			StrCmp $3 'FD' precall
			StrCmp $3 'F' findnext precall
			FindFirst $9 $R9 '$R8\$R7\*.*'
			StrCmp $R9 '.' 0 +4
			FindNext $9 $R9
			StrCmp $R9 '..' 0 +2
			FindNext $9 $R9
			FindClose $9
			IfErrors precall findnext

			file:
			StrCmp $3 'FDE' +3
			StrCmp $3 'FD' +2
			StrCmp $3 'F' 0 findnext
			StrCpy $R6 0
			StrCmp $5$6 '' precall
			FileOpen $9 '$R8\$R7' r
			IfErrors +3
			FileSeek $9 0 END $R6
			FileClose $9
			System::Int64Op $R6 / $1
			Pop $R6
			StrCmp $5 '' +2
			IntCmp $R6 $5 0 findnext
			StrCmp $6 '' +2
			IntCmp $R6 $6 0 0 findnext

			precall:
			StrCpy $9 0
			StrCpy $R9 '$R8\$R7'

			call:
			Push $0
			Push $1
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $7
			Push $8
			Push $9
			Push $R7
			Push $R8
			StrCmp $9 0 +4
			StrCpy $R6 ''
			StrCpy $R7 ''
			StrCpy $R9 ''
			Call $2
			Pop $R9
			Pop $R8
			Pop $R7
			Pop $9
			Pop $8
			Pop $7
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Pop $0
			IfErrors error

			StrCmp $R9 'StopLocate' clearstack
			goto $9

			findnext:
			FindNext $0 $R7
			IfErrors 0 dir
			FindClose $0

			subdir:
			StrCpy $9 $7 2
			StrCmp $9 'G0' end
			FindFirst $0 $R7 '$R8\*.*'
			StrCmp $R7 '.' 0 pushdir
			FindNext $0 $R7
			StrCmp $R7 '..' 0 pushdir
			FindNext $0 $R7
			IfErrors 0 pushdir
			FindClose $0
			goto +7

			pushdir:
			IfFileExists '$R8\$R7\*.*' 0 +3
			Push '$R8\$R7'
			IntOp $8 $8 + 1
			FindNext $0 $R7
			IfErrors 0 -4
			FindClose $0
			StrCmp $8 0 end nextdir

			error:
			SetErrors

			clearstack:
			StrCmp $8 0 end
			IntOp $8 $8 - 1
			Pop $R8
			goto clearstack

			end:
			SetDetailsPrint both
			Pop $R9
			Pop $R8
			Pop $R7
			Pop $R6
			Pop $9
			Pop $8
			Pop $7
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Pop $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetSize
	!ifndef ${_FILEFUNC_UN}GetSize
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetSize `!insertmacro ${_FILEFUNC_UN}GetSizeCall`

		Function ${_FILEFUNC_UN}GetSize
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $7
			Push $8
			Push $9
			Push $R3
			Push $R4
			Push $R5
			Push $R6
			Push $R7
			Push $R8
			Push $R9
			ClearErrors

			StrCpy $R9 $0 1 -1
			StrCmp $R9 '\' 0 +3
			StrCpy $0 $0 -1
			goto -3
			IfFileExists '$0\*.*' 0 error

			StrCpy $3 ''
			StrCpy $4 ''
			StrCpy $5 ''
			StrCpy $6 ''
			StrCpy $8 0
			StrCpy $R3 ''
			StrCpy $R4 ''
			StrCpy $R5 ''

			option:
			StrCpy $R9 $1 1
			StrCpy $1 $1 '' 1
			StrCmp $R9 ' ' -2
			StrCmp $R9 '' sizeset
			StrCmp $R9 '/' 0 -4

			StrCpy $9 -1
			IntOp $9 $9 + 1
			StrCpy $R9 $1 1 $9
			StrCmp $R9 '' +2
			StrCmp $R9 '/' 0 -3
			StrCpy $8 $1 $9
			StrCpy $8 $8 '' 2
			StrCpy $R9 $8 '' -1
			StrCmp $R9 ' ' 0 +3
			StrCpy $8 $8 -1
			goto -3
			StrCpy $R9 $1 2
			StrCpy $1 $1 '' $9

			StrCmp $R9 'M=' 0 size
			StrCpy $4 $8
			goto option

			size:
			StrCmp $R9 'S=' 0 gotosubdir
			StrCpy $6 $8
			goto option

			gotosubdir:
			StrCmp $R9 'G=' 0 error
			StrCpy $7 $8
			StrCmp $7 '' +3
			StrCmp $7 '1' +2
			StrCmp $7 '0' 0 error
			goto option

			sizeset:
			StrCmp $6 '' default
			StrCpy $9 0
			StrCpy $R9 $6 1 $9
			StrCmp $R9 '' +4
			StrCmp $R9 ':' +3
			IntOp $9 $9 + 1
			goto -4
			StrCpy $5 $6 $9
			IntOp $9 $9 + 1
			StrCpy $1 $6 1 -1
			StrCpy $6 $6 -1 $9
			StrCmp $5 '' +2
			IntOp $5 $5 + 0
			StrCmp $6 '' +2
			IntOp $6 $6 + 0

			StrCmp $1 'B' 0 +4
			StrCpy $1 1
			StrCpy $2 bytes
			goto default
			StrCmp $1 'K' 0 +4
			StrCpy $1 1024
			StrCpy $2 Kb
			goto default
			StrCmp $1 'M' 0 +4
			StrCpy $1 1048576
			StrCpy $2 Mb
			goto default
			StrCmp $1 'G' 0 error
			StrCpy $1 1073741824
			StrCpy $2 Gb

			default:
			StrCmp $4 '' 0 +2
			StrCpy $4 '*.*'
			StrCmp $7 '' 0 +2
			StrCpy $7 '1'

			StrCpy $8 1
			Push $0
			SetDetailsPrint textonly

			nextdir:
			IntOp $8 $8 - 1
			Pop $R8
			FindFirst $0 $R7 '$R8\$4'
			IfErrors show
			StrCmp $R7 '.' 0 dir
			FindNext $0 $R7
			StrCmp $R7 '..' 0 dir
			FindNext $0 $R7
			IfErrors 0 dir
			FindClose $0
			goto show

			dir:
			IfFileExists '$R8\$R7\*.*' 0 file
			IntOp $R5 $R5 + 1
			goto findnext

			file:
			StrCpy $R6 0
			StrCmp $5$6 '' 0 +3
			IntOp $R4 $R4 + 1
			goto findnext
			FileOpen $9 '$R8\$R7' r
			IfErrors +3
			FileSeek $9 0 END $R6
			FileClose $9
			StrCmp $5 '' +2
			IntCmp $R6 $5 0 findnext
			StrCmp $6 '' +2
			IntCmp $R6 $6 0 0 findnext
			IntOp $R4 $R4 + 1
			System::Int64Op /NOUNLOAD $R3 + $R6
			Pop $R3

			findnext:
			FindNext $0 $R7
			IfErrors 0 dir
			FindClose $0

			show:
			StrCmp $5$6 '' nosize
			System::Int64Op /NOUNLOAD $R3 / $1
			Pop $9
			DetailPrint 'Size:$9 $2  Files:$R4  Folders:$R5'
			goto subdir
			nosize:
			DetailPrint 'Files:$R4  Folders:$R5'

			subdir:
			StrCmp $7 0 preend
			FindFirst $0 $R7 '$R8\*.*'
			StrCmp $R7 '.' 0 pushdir
			FindNext $0 $R7
			StrCmp $R7 '..' 0 pushdir
			FindNext $0 $R7
			IfErrors 0 pushdir
			FindClose $0
			goto +7

			pushdir:
			IfFileExists '$R8\$R7\*.*' 0 +3
			Push '$R8\$R7'
			IntOp $8 $8 + 1
			FindNext $0 $R7
			IfErrors 0 -4
			FindClose $0
			StrCmp $8 0 0 nextdir

			preend:
			StrCmp $R3 '' nosizeend
			System::Int64Op $R3 / $1
			Pop $R3
			nosizeend:
			StrCpy $2 $R4
			StrCpy $1 $R5
			StrCpy $0 $R3
			goto end

			error:
			SetErrors
			StrCpy $0 ''
			StrCpy $1 ''
			StrCpy $2 ''

			end:
			SetDetailsPrint both
			Pop $R9
			Pop $R8
			Pop $R7
			Pop $R6
			Pop $R5
			Pop $R4
			Pop $R3
			Pop $9
			Pop $8
			Pop $7
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Exch $2
			Exch
			Exch $1
			Exch 2
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro DriveSpace
	!ifndef ${_FILEFUNC_UN}DriveSpace
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}DriveSpace `!insertmacro ${_FILEFUNC_UN}DriveSpaceCall`

		Function ${_FILEFUNC_UN}DriveSpace
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			ClearErrors

			StrCpy $2 $0 1 -1
			StrCmp $2 '\' 0 +3
			StrCpy $0 $0 -1
			goto -3
			IfFileExists '$0\NUL' 0 error

			StrCpy $5 ''
			StrCpy $6 ''

			option:
			StrCpy $2 $1 1
			StrCpy $1 $1 '' 1
			StrCmp $2 ' ' -2
			StrCmp $2 '' default
			StrCmp $2 '/' 0 -4
			StrCpy $3 -1
			IntOp $3 $3 + 1
			StrCpy $2 $1 1 $3
			StrCmp $2 '' +2
			StrCmp $2 '/' 0 -3
			StrCpy $4 $1 $3
			StrCpy $4 $4 '' 2
			StrCpy $2 $4 1 -1
			StrCmp $2 ' ' 0 +3
			StrCpy $4 $4 -1
			goto -3
			StrCpy $2 $1 2
			StrCpy $1 $1 '' $3

			StrCmp $2 'D=' 0 unit
			StrCpy $5 $4
			StrCmp $5 '' +4
			StrCmp $5 'T' +3
			StrCmp $5 'O' +2
			StrCmp $5 'F' 0 error
			goto option

			unit:
			StrCmp $2 'S=' 0 error
			StrCpy $6 $4
			goto option

			default:
			StrCmp $5 '' 0 +2
			StrCpy $5 'T'
			StrCmp $6 '' 0 +3
			StrCpy $6 '1'
			goto getspace

			StrCmp $6 'B' 0 +3
			StrCpy $6 1
			goto getspace
			StrCmp $6 'K' 0 +3
			StrCpy $6 1024
			goto getspace
			StrCmp $6 'M' 0 +3
			StrCpy $6 1048576
			goto getspace
			StrCmp $6 'G' 0 error
			StrCpy $6 1073741824

			getspace:
			System::Call /NOUNLOAD 'kernel32::GetDiskFreeSpaceExA(t, *l, *l, *l)i(r0,.r2,.r3,.)'

			StrCmp $5 T 0 +3
			StrCpy $0 $3
			goto getsize
			StrCmp $5 O 0 +4
			System::Int64Op /NOUNLOAD $3 - $2
			Pop $0
			goto getsize
			StrCmp $5 F 0 +2
			StrCpy $0 $2

			getsize:
			System::Int64Op $0 / $6
			Pop $0
			goto end

			error:
			SetErrors
			StrCpy $0 ''

			end:
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetDrives
	!ifndef ${_FILEFUNC_UN}GetDrives
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetDrives `!insertmacro ${_FILEFUNC_UN}GetDrivesCall`

		Function ${_FILEFUNC_UN}GetDrives
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $8
			Push $9

			System::Alloc /NOUNLOAD 1024
			Pop $2
			System::Call /NOUNLOAD 'kernel32::GetLogicalDriveStringsA(i,i) i(1024, r2)'

			StrCmp $0 ALL drivestring
			StrCmp $0 '' 0 typeset
			StrCpy $0 ALL
			goto drivestring

			typeset:
			StrCpy $6 -1
			IntOp $6 $6 + 1
			StrCpy $8 $0 1 $6
			StrCmp $8$0 '' enumex
			StrCmp $8 '' +2
			StrCmp $8 '+' 0 -4
			StrCpy $8 $0 $6
			IntOp $6 $6 + 1
			StrCpy $0 $0 '' $6

			StrCmp $8 'FDD' 0 +3
			StrCpy $6 2
			goto drivestring
			StrCmp $8 'HDD' 0 +3
			StrCpy $6 3
			goto drivestring
			StrCmp $8 'NET' 0 +3
			StrCpy $6 4
			goto drivestring
			StrCmp $8 'CDROM' 0 +3
			StrCpy $6 5
			goto drivestring
			StrCmp $8 'RAM' 0 typeset
			StrCpy $6 6

			drivestring:
			StrCpy $3 $2

			enumok:
			System::Call /NOUNLOAD 'kernel32::lstrlenA(t) i(i r3) .r4'
			StrCmp $4$0 '0ALL' enumex
			StrCmp $4 0 typeset
			System::Call /NOUNLOAD 'kernel32::GetDriveTypeA(t) i(i r3) .r5'

			StrCmp $0 ALL +2
			StrCmp $5 $6 letter enumnext
			StrCmp $5 2 0 +3
			StrCpy $8 FDD
			goto letter
			StrCmp $5 3 0 +3
			StrCpy $8 HDD
			goto letter
			StrCmp $5 4 0 +3
			StrCpy $8 NET
			goto letter
			StrCmp $5 5 0 +3
			StrCpy $8 CDROM
			goto letter
			StrCmp $5 6 0 enumex
			StrCpy $8 RAM

			letter:
			System::Call /NOUNLOAD '*$3(&t1024 .r9)'

			Push $0
			Push $1
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $8
			Call $1
			Pop $9
			Pop $8
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Pop $0
			StrCmp $9 'StopGetDrives' enumex

			enumnext:
			IntOp $3 $3 + $4
			IntOp $3 $3 + 1
			goto enumok

			enumex:
			System::Free $2

			Pop $9
			Pop $8
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Pop $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetTime
	!ifndef ${_FILEFUNC_UN}GetTime
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetTime `!insertmacro ${_FILEFUNC_UN}GetTimeCall`

		Function ${_FILEFUNC_UN}GetTime
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $7
			ClearErrors

			StrCmp $1 'L' gettime
			StrCmp $1 'A' getfile
			StrCmp $1 'C' getfile
			StrCmp $1 'M' getfile
			StrCmp $1 'LS' gettime
			StrCmp $1 'AS' getfile
			StrCmp $1 'CS' getfile
			StrCmp $1 'MS' getfile
			goto error

			getfile:
			IfFileExists $0 0 error
			System::Call /NOUNLOAD '*(i,l,l,l,i,i,i,i,&t260,&t14) i .r6'
			System::Call /NOUNLOAD 'kernel32::FindFirstFileA(t,i)i(r0,r6) .r2'
			System::Call /NOUNLOAD 'kernel32::FindClose(i)i(r2)'

			gettime:
			System::Call /NOUNLOAD '*(&i2,&i2,&i2,&i2,&i2,&i2,&i2,&i2) i .r7'
			StrCmp $1 'L' 0 systemtime
			System::Call /NOUNLOAD 'kernel32::GetLocalTime(i)i(r7)'
			goto convert
			systemtime:
			StrCmp $1 'LS' 0 filetime
			System::Call /NOUNLOAD 'kernel32::GetSystemTime(i)i(r7)'
			goto convert

			filetime:
			System::Call /NOUNLOAD '*$6(i,l,l,l,i,i,i,i,&t260,&t14)i(,.r4,.r3,.r2)'
			System::Free /NOUNLOAD $6
			StrCmp $1 'A' 0 +3
			StrCpy $2 $3
			goto tolocal
			StrCmp $1 'C' 0 +3
			StrCpy $2 $4
			goto tolocal
			StrCmp $1 'M' tolocal

			StrCmp $1 'AS' tosystem
			StrCmp $1 'CS' 0 +3
			StrCpy $3 $4
			goto tosystem
			StrCmp $1 'MS' 0 +3
			StrCpy $3 $2
			goto tosystem

			tolocal:
			System::Call /NOUNLOAD 'kernel32::FileTimeToLocalFileTime(*l,*l)i(r2,.r3)'
			tosystem:
			System::Call /NOUNLOAD 'kernel32::FileTimeToSystemTime(*l,i)i(r3,r7)'

			convert:
			System::Call /NOUNLOAD '*$7(&i2,&i2,&i2,&i2,&i2,&i2,&i2,&i2)i(.r5,.r6,.r4,.r0,.r3,.r2,.r1,)'
			System::Free $7

			IntCmp $0 9 0 0 +2
			StrCpy $0 '0$0'
			IntCmp $1 9 0 0 +2
			StrCpy $1 '0$1'
			IntCmp $2 9 0 0 +2
			StrCpy $2 '0$2'
			IntCmp $6 9 0 0 +2
			StrCpy $6 '0$6'

			StrCmp $4 0 0 +3
			StrCpy $4 Sunday
			goto end
			StrCmp $4 1 0 +3
			StrCpy $4 Monday
			goto end
			StrCmp $4 2 0 +3
			StrCpy $4 Tuesday
			goto end
			StrCmp $4 3 0 +3
			StrCpy $4 Wednesday
			goto end
			StrCmp $4 4 0 +3
			StrCpy $4 Thursday
			goto end
			StrCmp $4 5 0 +3
			StrCpy $4 Friday
			goto end
			StrCmp $4 6 0 error
			StrCpy $4 Saturday
			goto end

			error:
			SetErrors
			StrCpy $0 ''
			StrCpy $1 ''
			StrCpy $2 ''
			StrCpy $3 ''
			StrCpy $4 ''
			StrCpy $5 ''
			StrCpy $6 ''

			end:
			Pop $7
			Exch $6
			Exch
			Exch $5
			Exch 2
			Exch $4
			Exch 3
			Exch $3
			Exch 4
			Exch $2
			Exch 5
			Exch $1
			Exch 6
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetFileAttributes
	!ifndef ${_FILEFUNC_UN}GetFileAttributes
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetFileAttributes `!insertmacro ${_FILEFUNC_UN}GetFileAttributesCall`

		Function ${_FILEFUNC_UN}GetFileAttributes
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4
			Push $5

			System::Call 'kernel32::GetFileAttributes(t r0)i .r2'
			StrCmp $2 -1 error
			StrCpy $3 ''

			IntOp $0 $2 & 0x4000
			IntCmp $0 0 +2
			StrCpy $3 'ENCRYPTED|'

			IntOp $0 $2 & 0x2000
			IntCmp $0 0 +2
			StrCpy $3 'NOT_CONTENT_INDEXED|$3'

			IntOp $0 $2 & 0x1000
			IntCmp $0 0 +2
			StrCpy $3 'OFFLINE|$3'

			IntOp $0 $2 & 0x0800
			IntCmp $0 0 +2
			StrCpy $3 'COMPRESSED|$3'

			IntOp $0 $2 & 0x0400
			IntCmp $0 0 +2
			StrCpy $3 'REPARSE_POINT|$3'

			IntOp $0 $2 & 0x0200
			IntCmp $0 0 +2
			StrCpy $3 'SPARSE_FILE|$3'

			IntOp $0 $2 & 0x0100
			IntCmp $0 0 +2
			StrCpy $3 'TEMPORARY|$3'

			IntOp $0 $2 & 0x0080
			IntCmp $0 0 +2
			StrCpy $3 'NORMAL|$3'

			IntOp $0 $2 & 0x0040
			IntCmp $0 0 +2
			StrCpy $3 'DEVICE|$3'

			IntOp $0 $2 & 0x0020
			IntCmp $0 0 +2
			StrCpy $3 'ARCHIVE|$3'

			IntOp $0 $2 & 0x0010
			IntCmp $0 0 +2
			StrCpy $3 'DIRECTORY|$3'

			IntOp $0 $2 & 0x0004
			IntCmp $0 0 +2
			StrCpy $3 'SYSTEM|$3'

			IntOp $0 $2 & 0x0002
			IntCmp $0 0 +2
			StrCpy $3 'HIDDEN|$3'

			IntOp $0 $2 & 0x0001
			IntCmp $0 0 +2
			StrCpy $3 'READONLY|$3'

			StrCpy $0 $3 -1
			StrCmp $1 '' end
			StrCmp $1 'ALL' end

			attrcmp:
			StrCpy $5 0
			IntOp $5 $5 + 1
			StrCpy $4 $1 1 $5
			StrCmp $4 '' +2
			StrCmp $4 '|'  0 -3
			StrCpy $2 $1 $5
			IntOp $5 $5 + 1
			StrCpy $1 $1 '' $5
			StrLen $3 $2
			StrCpy $5 -1
			IntOp $5 $5 + 1
			StrCpy $4 $0 $3 $5
			StrCmp $4 '' notfound
			StrCmp $4 $2 0 -3
			StrCmp $1 '' 0 attrcmp
			StrCpy $0 1
			goto end

			notfound:
			StrCpy $0 0
			goto end

			error:
			SetErrors
			StrCpy $0 ''

			end:
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetFileVersion
	!ifndef ${_FILEFUNC_UN}GetFileVersion
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetFileVersion `!insertmacro ${_FILEFUNC_UN}GetFileVersionCall`

		Function ${_FILEFUNC_UN}GetFileVersion
			Exch $0
			Push $1
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			ClearErrors

			GetDllVersion '$0' $1 $2
			IfErrors error
			IntOp $3 $1 / 0x00010000
			IntOp $4 $1 & 0x0000FFFF
			IntOp $5 $2 / 0x00010000
			IntOp $6 $2 & 0x0000FFFF
			StrCpy $0 '$3.$4.$5.$6'
			goto end

			error:
			SetErrors
			StrCpy $0 ''

			end:
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetExeName
	!ifndef ${_FILEFUNC_UN}GetExeName
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetExeName `!insertmacro ${_FILEFUNC_UN}GetExeNameCall`

		Function ${_FILEFUNC_UN}GetExeName
			Push $0
			Push $1
			Push $2
			System::Call /NOUNLOAD 'kernel32::GetModuleFileNameA(i 0, t .r0, i 1024)'
			System::Call 'kernel32::GetLongPathNameA(t r0, t .r1, i 1024)i .r2'
			StrCmp $2 error +2
			StrCpy $0 $1
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetExePath
	!ifndef ${_FILEFUNC_UN}GetExePath
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetExePath `!insertmacro ${_FILEFUNC_UN}GetExePathCall`

		Function ${_FILEFUNC_UN}GetExePath
			Push $0
			Push $1
			Push $2
			StrCpy $0 $EXEDIR
			System::Call 'kernel32::GetLongPathNameA(t r0, t .r1, i 1024)i .r2'
			StrCmp $2 error +2
			StrCpy $0 $1
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetParameters
	!ifndef ${_FILEFUNC_UN}GetParameters
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetParameters `!insertmacro ${_FILEFUNC_UN}GetParametersCall`

		Function ${_FILEFUNC_UN}GetParameters
			Push $0
			Push $1
			Push $2

			StrCpy $1 1
			StrCpy $0 $CMDLINE 1
			StrCmp $0 '"' 0 +3
			StrCpy $2 '"'
			goto +2
			StrCpy $2 ' '

			IntOp $1 $1 + 1
			StrCpy $0 $CMDLINE 1 $1
			StrCmp $0 $2 +2
			StrCmp $0 '' end -3

			IntOp $1 $1 + 1
			StrCpy $0 $CMDLINE 1 $1
			StrCmp $0 ' ' -2
			StrCpy $0 $CMDLINE '' $1

			StrCpy $1 $0 1 -1
			StrCmp $1 ' ' 0 +3
			StrCpy $0 $0 -1
			goto -3

			end:
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetOptions
	!ifndef ${_FILEFUNC_UN}GetOptions${_FILEFUNC_S}
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetOptions${_FILEFUNC_S} `!insertmacro ${_FILEFUNC_UN}GetOptions${_FILEFUNC_S}Call`

		Function ${_FILEFUNC_UN}GetOptions${_FILEFUNC_S}
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4
			Push $5
			Push $6
			Push $7
			ClearErrors

			StrCpy $2 $1 '' 1
			StrCpy $1 $1 1
			StrLen $3 $2
			StrCpy $7 0

			begin:
			StrCpy $4 -1
			StrCpy $6 ''

			quote:
			IntOp $4 $4 + 1
			StrCpy $5 $0 1 $4
			StrCmp${_FILEFUNC_S} $5$7 '0' notfound
			StrCmp${_FILEFUNC_S} $5 '' trimright
			StrCmp${_FILEFUNC_S} $5 '"' 0 +7
			StrCmp${_FILEFUNC_S} $6 '' 0 +3
			StrCpy $6 '"'
			goto quote
			StrCmp${_FILEFUNC_S} $6 '"' 0 +3
			StrCpy $6 ''
			goto quote
			StrCmp${_FILEFUNC_S} $5 `'` 0 +7
			StrCmp${_FILEFUNC_S} $6 `` 0 +3
			StrCpy $6 `'`
			goto quote
			StrCmp${_FILEFUNC_S} $6 `'` 0 +3
			StrCpy $6 ``
			goto quote
			StrCmp${_FILEFUNC_S} $5 '`' 0 +7
			StrCmp${_FILEFUNC_S} $6 '' 0 +3
			StrCpy $6 '`'
			goto quote
			StrCmp${_FILEFUNC_S} $6 '`' 0 +3
			StrCpy $6 ''
			goto quote
			StrCmp${_FILEFUNC_S} $6 '"' quote
			StrCmp${_FILEFUNC_S} $6 `'` quote
			StrCmp${_FILEFUNC_S} $6 '`' quote
			StrCmp${_FILEFUNC_S} $5 $1 0 quote
			StrCmp${_FILEFUNC_S} $7 0 trimleft trimright

			trimleft:
			IntOp $4 $4 + 1
			StrCpy $5 $0 $3 $4
			StrCmp${_FILEFUNC_S} $5 '' notfound
			StrCmp${_FILEFUNC_S} $5 $2 0 quote
			IntOp $4 $4 + $3
			StrCpy $0 $0 '' $4
			StrCpy $4 $0 1
			StrCmp${_FILEFUNC_S} $4 ' ' 0 +3
			StrCpy $0 $0 '' 1
			goto -3
			StrCpy $7 1
			goto begin

			trimright:
			StrCpy $0 $0 $4
			StrCpy $4 $0 1 -1
			StrCmp${_FILEFUNC_S} $4 ' ' 0 +3
			StrCpy $0 $0 -1
			goto -3
			StrCpy $3 $0 1
			StrCpy $4 $0 1 -1
			StrCmp${_FILEFUNC_S} $3 $4 0 end
			StrCmp${_FILEFUNC_S} $3 '"' +3
			StrCmp${_FILEFUNC_S} $3 `'` +2
			StrCmp${_FILEFUNC_S} $3 '`' 0 end
			StrCpy $0 $0 -1 1
			goto end

			notfound:
			SetErrors
			StrCpy $0 ''

			end:
			Pop $7
			Pop $6
			Pop $5
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend


!macro GetRoot
	!ifndef ${_FILEFUNC_UN}GetRoot
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetRoot `!insertmacro ${_FILEFUNC_UN}GetRootCall`

		Function ${_FILEFUNC_UN}GetRoot
			Exch $0
			Push $1
			Push $2
			Push $3

			StrCpy $1 $0 2
			StrCmp $1 '\\' UNC
			StrCpy $1 0
			IntOp $1 $1 + 1
			StrCpy $2 $0 1 $1
			StrCmp $2 '' end
			StrCmp $2 '\' 0 -3
			StrCpy $0 $0 $1
			goto end

			UNC:
			StrCpy $2 2
			StrCpy $3 ''
			StrCpy $1 $0 1 $2
			StrCmp $1 '\' empty

			loop:
			IntOp $2 $2 + 1
			StrCpy $1 $0 1 $2
			StrCmp $1$3 '' empty
			StrCmp $1 '' +5
			StrCmp $1 '\' 0 loop
			StrCmp $3 '1' +3
			StrCpy $3 '1'
			goto loop
			StrCpy $0 $0 $2
			StrCpy $2 $0 1 -1
			StrCmp $2 '\' empty end

			empty:
			StrCpy $0 ''

			end:
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetParent
	!ifndef ${_FILEFUNC_UN}GetParent
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetParent `!insertmacro ${_FILEFUNC_UN}GetParentCall`

		Function ${_FILEFUNC_UN}GetParent
			Exch $0
			Push $1
			Push $2

			StrCpy $2 $0 1 -1
			StrCmp $2 '\' 0 +3
			StrCpy $0 $0 -1
			goto -3

			StrCpy $1 0
			IntOp $1 $1 - 1
			StrCpy $2 $0 1 $1
			StrCmp $2 '\' +2
			StrCmp $2 '' 0 -3
			StrCpy $0 $0 $1

			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetFileName
	!ifndef ${_FILEFUNC_UN}GetFileName
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetFileName `!insertmacro ${_FILEFUNC_UN}GetFileNameCall`

		Function ${_FILEFUNC_UN}GetFileName
			Exch $0
			Push $1
			Push $2

			StrCpy $2 $0 1 -1
			StrCmp $2 '\' 0 +3
			StrCpy $0 $0 -1
			goto -3

			StrCpy $1 0
			IntOp $1 $1 - 1
			StrCpy $2 $0 1 $1
			StrCmp $2 '' end
			StrCmp $2 '\' 0 -3
			IntOp $1 $1 + 1
			StrCpy $0 $0 '' $1

			end:
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetBaseName
	!ifndef ${_FILEFUNC_UN}GetBaseName
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetBaseName `!insertmacro ${_FILEFUNC_UN}GetBaseNameCall`

		Function ${_FILEFUNC_UN}GetBaseName
			Exch $0
			Push $1
			Push $2
			Push $3

			StrCpy $1 0
			StrCpy $3 ''

			loop:
			IntOp $1 $1 - 1
			StrCpy $2 $0 1 $1
			StrCmp $2 '' trimpath
			StrCmp $2 '\' trimpath
			StrCmp $3 'noext' loop
			StrCmp $2 '.' 0 loop
			StrCpy $0 $0 $1
			StrCpy $3 'noext'
			StrCpy $1 0
			goto loop

			trimpath:
			StrCmp $1 -1 empty
			IntOp $1 $1 + 1
			StrCpy $0 $0 '' $1
			goto end

			empty:
			StrCpy $0 ''

			end:
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro GetFileExt
	!ifndef ${_FILEFUNC_UN}GetFileExt
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}GetFileExt `!insertmacro ${_FILEFUNC_UN}GetFileExtCall`

		Function ${_FILEFUNC_UN}GetFileExt
			Exch $0
			Push $1
			Push $2

			StrCpy $1 0

			loop:
			IntOp $1 $1 - 1
			StrCpy $2 $0 1 $1
			StrCmp $2 '' empty
			StrCmp $2 '\' empty
			StrCmp $2 '.' 0 loop

			StrCmp $1 -1 empty
			IntOp $1 $1 + 1
			StrCpy $0 $0 '' $1
			goto end

			empty:
			StrCpy $0 ''

			end:
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro BannerTrimPath
	!ifndef ${_FILEFUNC_UN}BannerTrimPath
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}BannerTrimPath `!insertmacro ${_FILEFUNC_UN}BannerTrimPathCall`

		Function ${_FILEFUNC_UN}BannerTrimPath
			Exch $1
			Exch
			Exch $0
			Exch
			Push $2
			Push $3
			Push $4

			StrCpy $3 $1 1 -1
			IntOp $1 $1 + 0
			StrLen $2 $0
			IntCmp $2 $1 end end
			IntOp $1 $1 - 3
			IntCmp $1 0 empty empty
			StrCmp $3 'A' A-trim
			StrCmp $3 'B' B-trim
			StrCmp $3 'C' C-trim
			StrCmp $3 'D' D-trim

			A-trim:
			StrCpy $3 $0 1 1
			StrCpy $2 0
			StrCmp $3 ':' 0 +2
			IntOp $2 $2 + 2

			loopleft:
			IntOp $2 $2 + 1
			StrCpy $3 $0 1 $2
			StrCmp $2 $1 C-trim
			StrCmp $3 '\' 0 loopleft
			StrCpy $3 $0 $2
			IntOp $2 $2 - $1
			IntCmp $2 0 B-trim 0 B-trim

			loopright:
			IntOp $2 $2 + 1
			StrCpy $4 $0 1 $2
			StrCmp $2 0 B-trim
			StrCmp $4 '\' 0 loopright
			StrCpy $4 $0 '' $2
			StrCpy $0 '$3\...$4'
			goto end

			B-trim:
			StrCpy $2 $1
			IntOp $2 $2 - 1
			StrCmp $2 -1 C-trim
			StrCpy $3 $0 1 $2
			StrCmp $3 '\' 0 -3
			StrCpy $0 $0 $2
			StrCpy $0 '$0\...'
			goto end

			C-trim:
			StrCpy $0 $0 $1
			StrCpy $0 '$0...'
			goto end

			D-trim:
			StrCpy $3 -1
			IntOp $3 $3 - 1
			StrCmp $3 -$2 C-trim
			StrCpy $4 $0 1 $3
			StrCmp $4 '\' 0 -3
			StrCpy $4 $0 '' $3
			IntOp $3 $1 + $3
			IntCmp $3 2 C-trim C-trim
			StrCpy $0 $0 $3
			StrCpy $0 '$0...$4'
			goto end

			empty:
			StrCpy $0 ''

			end:
			Pop $4
			Pop $3
			Pop $2
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro DirState
	!ifndef ${_FILEFUNC_UN}DirState
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}DirState `!insertmacro ${_FILEFUNC_UN}DirStateCall`

		Function ${_FILEFUNC_UN}DirState
			Exch $0
			Push $1
			ClearErrors

			FindFirst $1 $0 '$0\*.*'
			IfErrors 0 +3
			StrCpy $0 -1
			goto end
			StrCmp $0 '.' 0 +4
			FindNext $1 $0
			StrCmp $0 '..' 0 +2
			FindNext $1 $0
			FindClose $1
			IfErrors 0 +3
			StrCpy $0 0
			goto end
			StrCpy $0 1

			end:
			Pop $1
			Exch $0
		FunctionEnd

		!verbose pop
	!endif
!macroend

!macro RefreshShellIcons
	!ifndef ${_FILEFUNC_UN}RefreshShellIcons
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!define ${_FILEFUNC_UN}RefreshShellIcons `!insertmacro ${_FILEFUNC_UN}RefreshShellIconsCall`

		Function ${_FILEFUNC_UN}RefreshShellIcons
			System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
		FunctionEnd

		!verbose pop
	!endif
!macroend


# Uninstall. Case insensitive. #

!macro un.LocateCall _PATH _OPTIONS _FUNC
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push $0
	Push `${_PATH}`
	Push `${_OPTIONS}`
	GetFunctionAddress $0 `${_FUNC}`
	Push `$0`
	Call un.Locate
	Pop $0
	!verbose pop
!macroend

!macro un.GetSizeCall _PATH _OPTIONS _RESULT1 _RESULT2 _RESULT3
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Push `${_OPTIONS}`
	Call un.GetSize
	Pop ${_RESULT1}
	Pop ${_RESULT2}
	Pop ${_RESULT3}
	!verbose pop
!macroend

!macro un.DriveSpaceCall _DRIVE _OPTIONS _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_DRIVE}`
	Push `${_OPTIONS}`
	Call un.DriveSpace
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetDrivesCall _DRV _FUNC
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push $0
	Push `${_DRV}`
	GetFunctionAddress $0 `${_FUNC}`
	Push `$0`
	Call un.GetDrives
	Pop $0
	!verbose pop
!macroend

!macro un.GetTimeCall _FILE _OPTION _RESULT1 _RESULT2 _RESULT3 _RESULT4 _RESULT5 _RESULT6 _RESULT7
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILE}`
	Push `${_OPTION}`
	Call un.GetTime
	Pop ${_RESULT1}
	Pop ${_RESULT2}
	Pop ${_RESULT3}
	Pop ${_RESULT4}
	Pop ${_RESULT5}
	Pop ${_RESULT6}
	Pop ${_RESULT7}
	!verbose pop
!macroend

!macro un.GetFileAttributesCall _PATH _ATTR _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Push `${_ATTR}`
	Call un.GetFileAttributes
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetFileVersionCall _FILE _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILE}`
	Call un.GetFileVersion
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetExeNameCall _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call un.GetExeName
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetExePathCall _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call un.GetExePath
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetParametersCall _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call un.GetParameters
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetOptionsCall _PARAMETERS _OPTION _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PARAMETERS}`
	Push `${_OPTION}`
	Call un.GetOptions
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetRootCall _FULLPATH _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FULLPATH}`
	Call un.GetRoot
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetParentCall _PATHSTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATHSTRING}`
	Call un.GetParent
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetFileNameCall _PATHSTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATHSTRING}`
	Call un.GetFileName
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetBaseNameCall _FILESTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILESTRING}`
	Call un.GetBaseName
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetFileExtCall _FILESTRING _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_FILESTRING}`
	Call un.GetFileExt
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.BannerTrimPathCall _PATH _LENGHT _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Push `${_LENGHT}`
	Call un.BannerTrimPath
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.DirStateCall _PATH _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PATH}`
	Call un.DirState
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.RefreshShellIconsCall
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Call un.RefreshShellIcons
	!verbose pop
!macroend

!macro un.Locate
	!ifndef un.Locate
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro Locate

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetSize
	!ifndef un.GetSize
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetSize

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.DriveSpace
	!ifndef un.DriveSpace
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro DriveSpace

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetDrives
	!ifndef un.GetDrives
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetDrives

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetTime
	!ifndef un.GetTime
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetTime

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetFileAttributes
	!ifndef un.GetFileAttributes
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetFileAttributes

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetFileVersion
	!ifndef un.GetFileVersion
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetFileVersion

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetExeName
	!ifndef un.GetExeName
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetExeName

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetExePath
	!ifndef un.GetExePath
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetExePath

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetParameters
	!ifndef un.GetParameters
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetParameters

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetOptions
	!ifndef un.GetOptions
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetOptions

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetRoot
	!ifndef un.GetRoot
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetRoot

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetParent
	!ifndef un.GetParent
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetParent

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetFileName
	!ifndef un.GetFileName
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetFileName

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetBaseName
	!ifndef un.GetBaseName
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetBaseName

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.GetFileExt
	!ifndef un.GetFileExt
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro GetFileExt

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.BannerTrimPath
	!ifndef un.BannerTrimPath
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro BannerTrimPath

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.DirState
	!ifndef un.DirState
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro DirState

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend

!macro un.RefreshShellIcons
	!ifndef un.RefreshShellIcons
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`

		!insertmacro RefreshShellIcons

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!verbose pop
	!endif
!macroend


# Install. Case sensitive. #

!macro GetOptionsSCall _PARAMETERS _OPTION _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PARAMETERS}`
	Push `${_OPTION}`
	Call GetOptionsS
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro GetOptionsS
	!ifndef GetOptionsS
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_S
		!define _FILEFUNC_S `S`

		!insertmacro GetOptions

		!undef _FILEFUNC_S
		!define _FILEFUNC_S
		!verbose pop
	!endif
!macroend


# Uninstall. Case sensitive. #

!macro un.GetOptionsSCall _PARAMETERS _OPTION _RESULT
	!verbose push
	!verbose ${_FILEFUNC_VERBOSE}
	Push `${_PARAMETERS}`
	Push `${_OPTION}`
	Call un.GetOptionsS
	Pop ${_RESULT}
	!verbose pop
!macroend

!macro un.GetOptionsS
	!ifndef un.GetOptionsS
		!verbose push
		!verbose ${_FILEFUNC_VERBOSE}
		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN `un.`
		!undef _FILEFUNC_S
		!define _FILEFUNC_S `S`

		!insertmacro GetOptions

		!undef _FILEFUNC_UN
		!define _FILEFUNC_UN
		!undef _FILEFUNC_S
		!define _FILEFUNC_S
		!verbose pop
	!endif
!macroend

!endif
