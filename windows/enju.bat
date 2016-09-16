@ECHO OFF

REM ################################################################################
REM
REM  enju.bat
REM
REM  batch script to run enju.exe.
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

IF DEFINED ENJU_TAGGER (
	IF NOT %ENJU_TAGGER%.==. SET DEFAULT_ENJU_TAGGER_OPT=-t %ENJU_TAGGER%
)

IF DEFINED ENJU_MORPH (
	IF NOT %ENJU_MORPH%.==. SET DEFAULT_ENJU_MORPH_OPT=-m %ENJU_MORPH%
)

REM SET BIN_DIR=%BASE_DIR%\bin
SET BIN_DIR=%~dp0\bin
CD /D %BIN_DIR%

SET COMMAND=enju.exe

IF DEFINED DEFAULT_ENJU_TAGGER_OPT SET COMMAND=%COMMAND% %DEFAULT_ENJU_TAGGER_OPT%
IF DEFINED DEFAULT_ENJU_MORPH_OPT SET COMMAND=%COMMAND% %DEFAULT_ENJU_MORPH_OPT%

CMD /c %COMMAND% %*

CD /D %CURRENT_DIR%

ENDLOCAL
@ECHO ON

