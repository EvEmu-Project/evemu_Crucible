-- Repair paperdoll rows that reference missing client bulk data.
-- Take and validate a database backup before applying this migration.
-- +migrate Up
DELETE s
FROM avatar_sculpts AS s
LEFT JOIN paperdollSculptingLocations AS l
    ON l.sculptLocationID = s.sculptLocationID
WHERE l.sculptLocationID IS NULL;

DELETE c
FROM avatar_colors AS c
LEFT JOIN paperdollColors AS p
    ON p.colorID = c.colorID
LEFT JOIN paperdollColorNames AS a
    ON a.colorNameID = c.colorNameA
LEFT JOIN paperdollColorNames AS bc
    ON bc.colorNameID = c.colorNameBC
WHERE p.colorID IS NULL
   OR a.colorNameID IS NULL
   OR (c.colorNameBC <> 0 AND bc.colorNameID IS NULL);

UPDATE chrPortraitData AS p
LEFT JOIN chrBackgrounds AS b
    ON b.backgroundID = p.backgroundID
LEFT JOIN chrLights AS l
    ON l.lightID = p.lightID
LEFT JOIN paperdollColorNames AS c
    ON c.colorNameID = p.lightColorID
SET p.backgroundID = CASE
        WHEN b.backgroundID IS NULL
        THEN (SELECT MIN(backgroundID) FROM chrBackgrounds)
        ELSE p.backgroundID
    END,
    p.lightID = CASE
        WHEN l.lightID IS NULL
        THEN (SELECT MIN(lightID) FROM chrLights)
        ELSE p.lightID
    END,
    p.lightColorID = CASE
        WHEN c.colorNameID IS NULL
        THEN (SELECT MIN(colorNameID) FROM paperdollColorNames)
        ELSE p.lightColorID
    END
WHERE b.backgroundID IS NULL
   OR l.lightID IS NULL
   OR c.colorNameID IS NULL;

-- +migrate Down
-- This cleanup is irreversible; restore the pre-migration backup if needed.
