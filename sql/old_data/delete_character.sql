
    DELETE FROM chrCertificates WHERE characterID=90000003;
DELETE FROM chrCharacters WHERE characterID=90000003;
DELETE FROM chrEmployment WHERE characterID=90000003;
DELETE FROM chrSkillHistory WHERE characterID=90000003;
DELETE FROM chrSkillQueue WHERE characterID=90000003;
DELETE FROM crpApplications WHERE characterID=90000003;
DELETE FROM crpShares WHERE shareholderID=90000003;
    DELETE FROM chrCharacterAttributes WHERE charID = 90000003;
DELETE FROM chrPausedSkillQueue WHERE characterID = 90000003;
    DELETE FROM entity_attributes WHERE itemID IN (SELECT itemID FROM entity WHERE ownerID = 90000003);

    DELETE FROM entity WHERE ownerID = 90000003;
DELETE FROM jnlCharacters WHERE ownerID=90000003;

    DELETE FROM avatar_colors WHERE charID = 90000003;

DELETE FROM avatar_modifiers WHERE charID = 90000003;
DELETE FROM avatar_sculpts WHERE charID = 90000003;
DELETE FROM avatars WHERE charID = 90000003;
