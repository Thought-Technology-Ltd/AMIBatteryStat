echo off

REM %cd% refers to the current working directory (variable)
REM %~dp0 refers to the full path to the batch file's directory (static)
REM %~dpnx0 refers to the full path to the batch directory and file name (static).

SET BUILDCFG=Release

set mainVer=1-13-0-2
set stimVer=1-0-19-0
set pkgVer=1.13.0.2
set prodType=Extended-SendTouchPadEvents

REM set EncoderFileLoc="C:\Users\cfall\Desktop\GitHub\AMI-Device\Main_MCU\06_Behaviour\src\Common"
REM set EncoderFileName="Encoder.c"
REM notepad %EncoderFileLoc%\%EncoderFileName%
REM pause

REM Build binary form Atollic ######################################
call do_build_LF.bat
REM ################################################################

echo Main Version 	= %mainVer%
echo STIM Version 	= %stimVer%
echo PKG Version  	= %pkgVer%
echo PROD Type		= %prodType%
echo Main Bin		= %mainBinLoc%

set mainBinLoc="C:\Users\cfall\Desktop\GitHub\AMI-Device\Main_MCU\Release"
REM copy binary from Main MCU project release folder 
copy /y %mainBinLoc%\AMI-Device.bin Release\AMI-Device-%mainVer%-%prodType%.bin
copy /y Release\AMI-Device-%mainVer%-%prodType%.bin Release\AMI-Device-%mainVer%.bin

REM Make sure all tools have been compiled and both main and stim binaries are in the build folder.

REM convert and compress images
call convert.bat %BUILDCFG%

REM convert images to palette, overwrites previous
call convert_pl.bat %BUILDCFG%

REM only convert images, overwrites previous
call convert_uc.bat %BUILDCFG%

if %errorlevel% neq 0 exit /b %errorlevel%
cd %BUILDCFG%
set pcApp=..\application\TT_AMI_Updater

packager.exe -m AMI-Device-%mainVer%.bin -s Stim_%stimVer%.bin -a assets -p 21483644 -f 1 -o output.pak
if %errorlevel% neq 0 exit /b %errorlevel%
pause
builder.exe -i output.pak -v %pkgVer% -o %pcApp%\package
if %errorlevel% neq 0 exit /b %errorlevel%

cd ..

ECHO Building updater...

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
if %errorlevel% neq 0 exit /b %errorlevel%

devenv all.sln /rebuild "%BUILDCFG%|x86" /Project TT_AMI_Updater
if %errorlevel% neq 0 exit /b %errorlevel%
copy /y .\%BUILDCFG%\TT_AMI_Updater.exe Myonix-Updater-v%pkgVer%-%prodType%.exe
if %errorlevel% neq 0 exit /b %errorlevel%

ECHO Updater built: TT_AMI_Updater.%pkgVer%.exe
pause