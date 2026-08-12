#!/bin/bash

set -Eeuo pipefail

: "${MYSQL_USER:?MYSQL_USER must be set}"
: "${MYSQL_PASSWORD:?MYSQL_PASSWORD must be set}"
MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
MYSQL_DATABASE="${MYSQL_DATABASE:-evemu}"

if [[ $# -ne 1 || ! "$1" =~ ^[1-9][0-9]{0,9}$ ]]; then
    echo 'Usage: ./delete_character.sh <character id>'
    exit 2
fi

char_id="$1"
[[ "$MYSQL_HOST" =~ ^[A-Za-z0-9_.:-]+$ ]] || exit 2
[[ "$MYSQL_DATABASE" =~ ^[A-Za-z0-9_]+$ ]] || exit 2
[[ "$MYSQL_PORT" =~ ^[0-9]+$ ]] || exit 2
(( MYSQL_PORT > 0 && MYSQL_PORT <= 65535 )) || exit 2

printf 'Type DELETE %s to remove character %s: ' "$char_id" "$char_id"
read -r confirmation
[[ "$confirmation" == "DELETE $char_id" ]] || exit 1

MYSQL_PWD="$MYSQL_PASSWORD" mysql \
    --skip-auto-rehash \
    --host="$MYSQL_HOST" \
    --port="$MYSQL_PORT" \
    --database="$MYSQL_DATABASE" \
    --user="$MYSQL_USER" <<SQL
START TRANSACTION;
DELETE FROM chrCertificates WHERE characterID=${char_id};
DELETE FROM chrCharacters WHERE characterID=${char_id};
DELETE FROM chrEmployment WHERE characterID=${char_id};
DELETE FROM chrSkillHistory WHERE characterID=${char_id};
DELETE FROM chrSkillQueue WHERE characterID=${char_id};
DELETE FROM crpApplications WHERE characterID=${char_id};
DELETE FROM crpShares WHERE shareholderID=${char_id};
DELETE FROM chrCharacterAttributes WHERE charID=${char_id};
DELETE FROM chrPausedSkillQueue WHERE characterID=${char_id};
DELETE FROM entity_attributes
    WHERE itemID IN (SELECT itemID FROM entity WHERE ownerID=${char_id});
DELETE FROM entity WHERE ownerID=${char_id};
DELETE FROM jnlCharacters WHERE ownerID=${char_id};
DELETE FROM avatar_colors WHERE charID=${char_id};
DELETE FROM avatar_modifiers WHERE charID=${char_id};
DELETE FROM avatar_sculpts WHERE charID=${char_id};
DELETE FROM avatars WHERE charID=${char_id};
COMMIT;
SQL
