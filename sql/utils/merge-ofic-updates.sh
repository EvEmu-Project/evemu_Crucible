#!/bin/sh

SQL_ROOT=..

INPUT_FILE=$SQL_ROOT/ofic-updates/*.sql
OUTPUT_FILE=$SQL_ROOT/ofic-updates.sql
TEMP_FILE=_merge_temp_

if [ -e $OUTPUT_FILE ]; then
	rm $OUTPUT_FILE
fi
cat $INPUT_FILE > $TEMP_FILE
mv $TEMP_FILE $OUTPUT_FILE

echo File $OUTPUT_FILE has been successfully created.

