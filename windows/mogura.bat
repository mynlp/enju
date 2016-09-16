@ECHO OFF

REM ################################################################################
REM
REM  mogura.bat
REM
REM  batch script to run mogura.exe.
REM
REM ################################################################################

SETLOCAL

SET CURRENT_DIR=%CD%

REM SET BASE_DIR=%~dp0
SET BASE_DIR=..

REM set environment variables
SET ENJU_WIN=1

IF NOT DEFINED ENJU_PREFIX (
	SET ENJU_PREFIX=%BASE_DIR%
)

IF NOT DEFINED ENJU_DIR (
	SET ENJU_DIR=%BASE_DIR%\lib\enju
)

IF NOT DEFINED LILFES_PATH (
	SET LILFES_PATH=%BASE_DIR%\share\liblilfes
) ELSE (
	SET LILFES_PATH=%BASE_DIR%\share\liblilfes;%LILFES_PATH%
)

REM SET BIN_DIR=%BASE_DIR%\bin
SET BIN_DIR=%~dp0\bin
CD /D %BIN_DIR%

SET COMMAND=mogura.exe

CMD /c %COMMAND% %*

CD /D %CURRENT_DIR%

ENDLOCAL
@ECHO ON

