#!/bin/sh

OUTPUT_FILE=ofic-update.sql

if [ -e $OUTPUT_FILE ]; then
	rm $OUTPUT_FILE
fi
cat *.sql > _merge_temp_
mv _merge_temp_ $OUTPUT_FILE

echo File $OUTPUT_FILE has been successfully created.

