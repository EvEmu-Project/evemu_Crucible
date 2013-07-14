@echo off

rem Check all input parameters for validity
if "%1"=="" goto usage
if "%2"=="" goto usage
if "%3"=="" goto usage

mkdir db_import_logs

echo Merging OFIC and EVEmu updates:
rm ofic-updates.sql
rm evemu-updates.sql
call .\utils\merge-ofic-updates.bat
call .\utils\merge-evemu-updates.bat
echo DONE!

echo Creating database '%3'...
mysql -u%1 -p%2 -v -e "create database %3;"
echo DONE!
echo.

echo Inserting CCP Public Dump for Crucible 1.6 into database...
echo (this may take 10-20 minutes or more depending on your machine)
if "%4"=="log" goto ccp_insert_with_logging
mysql -u%1 -p%2 %3 < cru16-mysql5-v1.sql
goto ccp_insert_done
:ccp_insert_with_logging
mysql -u%1 -p%2 -v %3 < cru16-mysql5-v1.sql >> .\db_import_logs\cru16-mysql5-v1_db_import.log
:ccp_insert_done
echo DONE!
echo.

echo Inserting EVEmu Dynamic Updates into database...
if "%4"=="log" goto evemu_dynamic_insert_with_logging
mysql -u%1 -p%2 %3 < evemu_dynamic-dump.sql
goto evemu_dynamic_insert_done
:evemu_dynamic_insert_with_logging
mysql -u%1 -p%2 -v %3 < evemu_dynamic-dump.sql >> .\db_import_logs\evemu_dynamic-dump_db_import.log
:evemu_dynamic_insert_done
echo DONE!
echo.

echo Inserting EVEmu Static Updates into database...
if "%4"=="log" goto evemu_static_insert_with_logging
mysql -u%1 -p%2 %3 < evemu_static-dump.sql
goto evemu_static_insert_done
:evemu_static_insert_with_logging
mysql -u%1 -p%2 -v %3 < evemu_static-dump.sql >> .\db_import_logs\evemu_static-dump_db_import.log
:evemu_static_insert_done
echo DONE!
echo.

echo Inserting EVEmu OFIC Updates into database...
echo (this may take more than 20 minutes depending on your machine)
cd .\ofic-updates
if "%4"=="log" goto evemu_ofic_insert_with_logging
for /r %%i in (*) do mysql -u%1 -p%2 %3 < %%i
goto evemu_ofic_insert_done
:evemu_ofic_insert_with_logging
for /r %%i in (*) do mysql -u%1 -p%2 -v %3 < %%i >> "..\db_import_logs\ofic-updates_db_import.log"
:evemu_ofic_insert_done
cd ..
echo DONE!
echo.

rem echo Inserting EVEmu Updates into database...
rem cd .\evemu-updates
rem if "%4"=="log" goto evemu_updates_insert_with_logging
rem for /r %%i in (*) do mysql -u%1 -p%2 %3 < %%i
rem goto evemu_updates_insert_done
rem :evemu_updates_insert_with_logging
rem for /r %%i in (*) do mysql -u%1 -p%2 -v %3 < %%i >> "..\db_import_logs\evemu-updates_db_import.log"
rem :evemu_updates_insert_done
rem cd ..
rem echo DONE!
rem echo.

echo Priming Database...
if "%4"=="log" goto priming_db_with_logging
mysql -u%1 -p%2 %3 < prime_db.sql
goto priming_db_done
:priming_db_with_logging
mysql -u%1 -p%2 -v %3 < prime_db.sql >> .\db_import_logs\prime_db_db_import.log
:priming_db_done

echo Applying Live Updates...
if "%4"=="log" goto live_updates_insert_with_logging
mysql -u%1 -p%2 %3 < liveupdates.sql
goto live_updates_insert_done
:live_updates_insert_with_logging
mysql -u%1 -p%2 -v %3 < liveupdates.sql >> .\db_import_logs\liveupdates_db_import.log
:live_updates_insert_done
echo DONE!
echo.


goto done

:usage
echo Usage:
echo.
echo "evemu_build_database <mysql_username> <mysql_password> <database> [log]"
echo.
echo    where:
echo         "<mysql_username>" - the username you use to log into mysql
echo         "<mysql_password>" - the password you use to log into mysql
echo         "<database>"       - the name of the database to target for the build
echo         "[log]"            - just type "log" without quotes as 4th parameter if you want all mysql output logged to db_import_logs subdir
echo.
echo example:
echo     evemu_build_database root password evemu log
echo.

:done