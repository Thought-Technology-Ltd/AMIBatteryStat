@echo off

REM   Build Command: headless.bat
REM
REM   Abstract:		Performs automated builds of TrueSTUDIO(R) C/C++ projects from the command line.
REM
REM   Environment:	Atollic TrueSTUDIO(R) version 3.1.0 or greater.
REM		
REM   Distribution: The file is distributed "as is", without any warranty of any kind.
REM
REM   Example usage: 
REM   1. Performs a rebuild on build configuration "config" for project "proj" in workspace "MyWorkspace".
REM      Command: headless.bat -data C:\MyWorkspace -build proj/config
REM   
REM	  2. Import all projects from a directory and perform the build in a temporary workspace. This procedure is useful when 
REM		 performing batch-builds, where TrueSTUDIO C/C++ projects are checked out from a source code repository. 	
REM      Command: headless.bat -data C:\TempWorkspace -importAll C:\projectFolder -build all
REM
REM   Output from the build process will appear on the command console and could be redirected to a file (headless.bat options > C:\build.log).
REM   Note! Failure in one sub build process results in exitcode=1 for the complete build process. 
REM
REM	  Options (case sensitive):
REM   -data       {[uri:/]/path/to/workspace}
REM   -import     {[uri:/]/path/to/project}
REM   -importAll  {[uri:/]/path/to/projectTreeURI} Import all projects under URI
REM   -build      {project_name_reg_ex{/config_reg_ex} | all}
REM   -cleanBuild {project_name_reg_ex{/config_reg_ex} | all}
REM   -I          {include_path} additional include_path to add to tools
REM   -include    {include_file} additional include_file to pass to tools
REM   -D          {prepoc_define} addition preprocessor defines to pass to the tools
REM   -E          {var=value} replace/add value to environment variable when running all tools
REM   -Ea         {var=value} append value to environment variable when running all tools
REM   -Ep         {var=value} prepend value to environment variable when running all tools
REM   -Er         {var} remove/unset the given environment variable

set workspacepath=C:\Users\cfall\Atollic\TrueSTUDIO
set workspacename=STM32_workspace_9.0
set project=AMI-Device
set config=Release
set applicationpath=C:\Program Files (x86)\Atollic\TrueSTUDIO for STM32 9.0.1\ide

echo Workspace = %workspacename%
echo Project   = %project%
echo Config	   = %config%

cd %applicationpath%
REM CMSIS
REM call headless.bat -data %workspacepath%\%workspacename% -build CMSIS/%config%
call headless.bat -data %workspacepath%\%workspacename% -cleanBuild CMSIS/%config%

REM AMI-Device
call headless.bat -data %workspacepath%\%workspacename% -cleanBuild %project%/%config%
REM call headless.bat -data %workspacepath%\%workspacename% -build %project%/%config%

cd %~dp0