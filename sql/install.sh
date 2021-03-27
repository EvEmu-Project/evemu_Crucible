#!/bin/bash

host="127.0.0.1"    #Database Host
port="3306"     #Database Port
user="evemu"     #Database Username
pass="evemu"     #Database Password

database="evemu"   #Database name

# Check if mysql client is available or not
if ! command -v mysql &> /dev/null
then
    echo "Client not found, please install the latest MariaDB client"
    exit
fi

tables=(`find tables/. -name "*.sql.gz"`)
for i in ${!tables[*]}
do
    echo " >> Installing.. ($(($i+1))/${#tables[@]})  ${tables[$i]}"
    zcat ${tables[$i]} | mysql -h ${host} --user=${user} --port=${port} --password=${pass} ${database}

done
