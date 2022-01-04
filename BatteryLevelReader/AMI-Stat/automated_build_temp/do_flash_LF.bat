@echo off

REM %cd% refers to the current working directory (variable)
REM %~dp0 refers to the full path to the batch file's directory (static)
REM %~dpnx0 refers to the full path to the batch directory and file name (static).

set STM32Prog_CLI_Path=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin
set PortType=SWD
set Freq=4000
set AP=0
set Origin=0x08000000
set debugBin="C:\Users\cfall\Desktop\GitHub\AMI-Device\Main_MCU\Debug\AMI-Device.bin"
set releaseBin="C:\Users\cfall\Desktop\GitHub\AMI-Device\Main_MCU\Release\AMI-Device.bin"

echo Port 		= %PortType%
echo Frequency 	= %Freq%
echo Address 	= %Origin%

REM connect, mass erase, flash and start
cd %STM32Prog_CLI_Path%
REM STM32_Programmer_CLI.exe -c port=%PortType% freq=%Freq% ap=%AP% -e all -w %releaseBin% %Origin% -s
STM32_Programmer_CLI.exe -c port=%PortType% freq=%Freq% ap=%AP% -e all -w %debugBin% %Origin% -s
pause

cd %~dp0