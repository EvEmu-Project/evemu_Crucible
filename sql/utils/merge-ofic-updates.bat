@echo off

set SQL_ROOT=..

set INPUT_FILE=%SQL_ROOT%\ofic-updates\*.sql
set OUTPUT_FILE=%SQL_ROOT%\ofic-updates.sql
set TEMP_FILE=_merge_temp_

if exist %OUTPUT_FILE% del %OUTPUT_FILE%
type %INPUT_FILE% > %TEMP_FILE%
move %TEMP_FILE% %OUTPUT_FILE%

echo File %OUTPUT_FILE% has been successfully created.
