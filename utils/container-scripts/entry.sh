#!/bin/sh

chown -R evemu:evemu /app
exec runuser -u evemu "$@"
