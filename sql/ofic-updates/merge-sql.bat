@echo off

set OUTPUT_FILE=ofic-update.sql

if exist %OUTPUT_FILE% del %OUTPUT_FILE%
type *.sql > _merge_temp_
move _merge_temp_ %OUTPUT_FILE%

echo File %OUTPUT_FILE% has been successfully created.
