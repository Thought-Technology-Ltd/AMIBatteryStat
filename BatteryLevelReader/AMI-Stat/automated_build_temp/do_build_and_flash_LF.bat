echo off

REM %cd% refers to the current working directory (variable)
REM %~dp0 refers to the full path to the batch file's directory (static)
REM %~dpnx0 refers to the full path to the batch directory and file name (static).

REM Build binary form Atollic ######################################
call do_build_LF.bat
REM ################################################################

REM Flash binary ###################################################
call do_flash_LF.bat
REM ################################################################