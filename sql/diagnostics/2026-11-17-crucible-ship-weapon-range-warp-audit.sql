-- Crucible 2011 read-only audit: enumerate ALL hulls + weapon/module dogma.
-- Attribute 600 is itself fitted warp speed in AU/s. No UPDATE/INSERT/DELETE.
-- Export and compare against an independently pinned 2011-2012 SDE build.
--
-- 1. Per-hull ship warp speed: includes all ship groups, faction/T2/capitals.
SELECT t.typeID, t.typeName, g.groupID, g.groupName, t.published,
       COALESCE(w.valueFloat, w.valueInt) AS ship_warp_speed_au_per_second,
       CASE
         WHEN w.typeID IS NULL THEN 'MISSING_STATIC_WARP_ATTRIBUTE'
         WHEN COALESCE(w.valueFloat, w.valueInt) IS NULL
           OR COALESCE(w.valueFloat, w.valueInt) <= 0.1 THEN 'INVALID_WARP_ATTRIBUTE'
         WHEN COALESCE(w.valueFloat, w.valueInt) > 20 THEN 'CHECK_HISTORICAL_OUTLIER'
         ELSE 'COMPARE_TO_CRUCIBLE_SDE'
       END AS verification_status
FROM invTypes AS t
JOIN invGroups AS g ON g.groupID = t.groupID
LEFT JOIN dgmTypeAttributes AS w
    ON w.typeID = t.typeID AND w.attributeID = 600
WHERE g.categoryID = 6
ORDER BY g.groupName, t.typeName, t.typeID;

-- 2. Missing, zero or suspicious entries among published hulls.
SELECT t.typeID, t.typeName, g.groupName,
       COALESCE(w.valueFloat, w.valueInt) AS ship_warp_speed_au_per_second
FROM invTypes AS t
JOIN invGroups AS g ON g.groupID = t.groupID
LEFT JOIN dgmTypeAttributes AS w
    ON w.typeID = t.typeID AND w.attributeID = 600
WHERE g.categoryID = 6 AND t.published = 1
  AND (w.typeID IS NULL OR COALESCE(w.valueFloat, w.valueInt) <= 0.1
       OR COALESCE(w.valueFloat, w.valueInt) > 20)
ORDER BY g.groupName, t.typeName;

-- 3. Crucible 2011 official patch note: Logistics group -> 3.75 AU/s.
SELECT t.typeID, t.typeName, g.groupName,
       COALESCE(w.valueFloat, w.valueInt) AS ship_warp_speed_au_per_second
FROM invTypes AS t
JOIN invGroups AS g ON g.groupID = t.groupID
LEFT JOIN dgmTypeAttributes AS w
    ON w.typeID = t.typeID AND w.attributeID = 600
WHERE g.categoryID = 6 AND t.published = 1 AND g.groupName = 'Logistics'
ORDER BY t.typeName;

-- 4. All weapon, mining laser and tractor module base ranges.
-- attribute 54=optimal/max range metres; 158=turret falloff metres;
-- attribute 1045=tractor pulling velocity metres/second.
-- Effective in-space values vary with skills, ammo, modules, hull bonuses.
SELECT t.typeID, t.typeName, g.groupID, g.groupName, t.published,
       COALESCE(o.valueFloat, o.valueInt) AS optimal_range_metres,
       COALESCE(f.valueFloat, f.valueInt) AS accuracy_falloff_metres,
       COALESCE(v.valueFloat, v.valueInt) AS tractor_velocity_metres_per_second
FROM invTypes AS t
JOIN invGroups AS g ON g.groupID = t.groupID
LEFT JOIN dgmTypeAttributes AS o
    ON o.typeID = t.typeID AND o.attributeID = 54
LEFT JOIN dgmTypeAttributes AS f
    ON f.typeID = t.typeID AND f.attributeID = 158
LEFT JOIN dgmTypeAttributes AS v
    ON v.typeID = t.typeID AND v.attributeID = 1045
WHERE g.categoryID = 7
  AND g.groupName REGEXP 'Weapon|Laser|Mining|Strip|Tractor|Harvester'
ORDER BY g.groupName, t.typeName, t.typeID;

-- 5. All missile/rocket/torpedo charge ranges (UNBONUSED).
-- attribute 37=max velocity m/s; 281=flight time milliseconds.
SELECT t.typeID, t.typeName, g.groupName,
       COALESCE(v.valueFloat, v.valueInt) AS missile_velocity_m_per_second,
       COALESCE(ft.valueFloat, ft.valueInt) AS missile_flight_time_ms,
       ROUND(COALESCE(v.valueFloat, v.valueInt) *
             COALESCE(ft.valueFloat, ft.valueInt) / 1000.0, 0) AS ideal_range_metres
FROM invTypes AS t
JOIN invGroups AS g ON g.groupID = t.groupID
LEFT JOIN dgmTypeAttributes AS v
    ON v.typeID = t.typeID AND v.attributeID = 37
LEFT JOIN dgmTypeAttributes AS ft
    ON ft.typeID = t.typeID AND ft.attributeID = 281
WHERE g.categoryID = 8
  AND g.groupName REGEXP 'Missile|Rocket|Torpedo'
ORDER BY g.groupName, t.typeName, t.typeID;
