#!/bin/bash

set -Eeuo pipefail

: "${MARIADB_HOST:?MARIADB_HOST must be set}"
: "${MARIADB_DATABASE:?MARIADB_DATABASE must be set}"
: "${MARIADB_PASSWORD:?MARIADB_PASSWORD must be set}"
: "${MARIADB_USER:?MARIADB_USER must be set}"
: "${MARIADB_PORT:?MARIADB_PORT must be set}"

DB_WAIT_ATTEMPTS=60
DB_WAIT_SECONDS=1
CONFIG_PATH="${EVEMU_CONFIG_PATH:-/app/etc/eve-server.xml}"

fail() {
    printf 'db_init: %s\n' "$1" >&2
    exit 1
}

[[ "$MARIADB_HOST" =~ ^[A-Za-z0-9_.:-]+$ ]] || fail "invalid database host"
[[ "$MARIADB_DATABASE" =~ ^[A-Za-z0-9_]+$ ]] || fail "invalid database name"
[[ "$MARIADB_USER" =~ ^[A-Za-z0-9_]+$ ]] || fail "invalid database user"
[[ "$MARIADB_PORT" =~ ^[0-9]+$ ]] || fail "invalid database port"
(( MARIADB_PORT > 0 && MARIADB_PORT <= 65535 )) || fail "invalid database port"
[[ "$MARIADB_PASSWORD" != *$'\r'* && "$MARIADB_PASSWORD" != *$'\n'* ]] || \
    fail "database password contains a line break"

wait_container() {
    local attempt=0
    while ! MYSQL_PWD="$MARIADB_PASSWORD" mysql \
        --skip-auto-rehash \
        --host="$MARIADB_HOST" \
        --port="$MARIADB_PORT" \
        --user="$MARIADB_USER" \
        --database="$MARIADB_DATABASE" \
        --execute='SHOW TABLES;' >/dev/null 2>&1; do
        (( attempt += 1 ))
        if (( attempt >= DB_WAIT_ATTEMPTS )); then
            fail "database did not become ready"
        fi
        printf .
        sleep "$DB_WAIT_SECONDS"
    done
}

echo "Waiting for DB to start..."
wait_container

[[ -f "$CONFIG_PATH" ]] || fail "effective server configuration is missing"

escape_sed_replacement() {
    printf '%s' "$1" | sed 's/[\\&|]/\\&/g'
}

config_temp="$(mktemp "${CONFIG_PATH}.tmp.XXXXXX")"
trap 'rm -f "$config_temp"' EXIT
sed \
    -e "s|database_host|$(escape_sed_replacement "$MARIADB_HOST")|g" \
    -e "s|database_username|$(escape_sed_replacement "$MARIADB_USER")|g" \
    -e "s|database_password|$(escape_sed_replacement "$MARIADB_PASSWORD")|g" \
    -e "s|database_name|$(escape_sed_replacement "$MARIADB_DATABASE")|g" \
    -e "s|database_port|$(escape_sed_replacement "$MARIADB_PORT")|g" \
    "$CONFIG_PATH" > "$config_temp"
chmod 600 "$config_temp"
mv -f "$config_temp" "$CONFIG_PATH"
trap - EXIT

cd /src/sql
umask 077
cat >/src/sql/evedb.yaml <<EOF
base-dir: /src/sql/base
db-database: $MARIADB_DATABASE
db-host: $MARIADB_HOST
db-pass: $MARIADB_PASSWORD
db-port: $MARIADB_PORT
db-user: $MARIADB_USER
log-level: Info
migrations-dir: /src/sql/migrations
dungeons-dir: /src/sql/dungeons
EOF

echo "Running EVEDBTool..."
/src/sql/evedbtool install

if [[ "$SEED_MARKET" == "TRUE" ]]; then
    IFS=',' read -r -a array <<< "$SEED_REGIONS"
    [[ "$SEED_SATURATION" =~ ^[0-9]+$ ]] || fail "invalid seed saturation"
    (( SEED_SATURATION <= 100 )) || fail "invalid seed saturation"
    echo "seed-saturation: $SEED_SATURATION" >> /src/sql/evedb.yaml
    echo "seed-regions: " >> /src/sql/evedb.yaml
    for region in "${array[@]}"; do
        [[ "$region" =~ ^[A-Za-z0-9][A-Za-z0-9 _-]*$ ]] || \
            fail "invalid seed region"
        echo "- $region" >> /src/sql/evedb.yaml
    done
    /src/sql/evedbtool seed
fi

echo "Loading all dungeons using EVEDBTool..."
/src/sql/evedbtool dungeon apply
