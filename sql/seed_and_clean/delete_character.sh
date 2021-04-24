#!/bin/bash
db_host=127.0.0.1
db_name=evemu
db_user=evemu
db_pass=evemu

char_id=$1

if [[ $# -eq 0 ]] ; then
    echo 'Usage: ./delete_character.sh <character id>'
    exit 0
fi

echo "Deleting Character ${char_id}"

echo " \
DELETE FROM chrCertificates WHERE characterID=${char_id}; \
DELETE FROM chrCharacters WHERE characterID=${char_id}; \
DELETE FROM chrEmployment WHERE characterID=${char_id}; \
DELETE FROM chrSkillHistory WHERE characterID=${char_id}; \
DELETE FROM chrSkillQueue WHERE characterID=${char_id}; \
DELETE FROM crpApplications WHERE characterID=${char_id}; \
DELETE FROM crpShares WHERE shareholderID=${char_id}; \
DELETE FROM chrCharacterAttributes WHERE charID = ${char_id}; \
DELETE FROM chrPausedSkillQueue WHERE characterID = ${char_id}; \
DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM entity WHERE ownerID = ${char_id}); \
DELETE FROM entity WHERE ownerID = ${char_id}; \
DELETE FROM jnlCharacters WHERE ownerID=${char_id}; \
DELETE FROM avatar_colors WHERE charID = ${char_id}; \
DELETE FROM avatar_modifiers WHERE charID = ${char_id}; \
DELETE FROM avatar_sculpts WHERE charID = ${char_id}; \
DELETE FROM avatars WHERE charID = ${char_id}; \
" | mysql -h ${db_host} -D${db_name} -u${db_user} -p${db_pass}
