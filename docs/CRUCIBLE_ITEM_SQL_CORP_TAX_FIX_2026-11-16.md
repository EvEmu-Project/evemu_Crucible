# EVEmu Crucible 0.8.7-Rizonx — item SQL #1064 and corporation-tax repair

Date: 2026-11-16. Independent PR against `master`; PR #26 (sovereignty,
asteroid belts and NPCs) remains separate.

## Verified code-level cause of the item database error

Windows server log:

```text
DBCore Query - #1064 in 'INSERT INTO entity (... customInfo)
VALUES (140000641, 3082, 90000000, 140000640, 32, 0, 1, 1,
0.000000, 0.000000, 0.000000, '' : ... syntax error near ''' at line 1
SaveItems - unable to save data
```

`DBcore::DoEscapeString(std::string&, const std::string&)` had:

```cpp
to.resize(len * 2);
mysql_real_escape_string(mysql, &to[0], from.c_str(), len);
to.resize(esc_len + 1);
```

The MySQL library writes a NUL terminator at `esc_len`. The final
`resize(esc_len+1)` **included that NUL in the C++ string's length**.
`ItemDB::SaveItems` streamed escaped `customInfo` using `operator<<`,
inserting an embedded NUL *before* the closing quote and parenthesis. The
printf-style `RunQuery(err, Inserts.str().c_str())` then stopped at the
embedded NUL, producing the exact incomplete query seen in the log.

**Fix:** Allocate `2*len+1` bytes for the MySQL C API even with empty
strings; resize the stored string to **`esc_len`**, excluding terminator.
Also use `RunQuery(err, "%s", assembled_sql.c_str())` and a separating
space before `ON DUPLICATE KEY UPDATE`; protect empty item/attribute
batches and remove a duplicated quantity update expression.

**Scope:** This fixes future persistence attempts only. The prior failed
`INSERT` did not necessarily save item `140000641`. Do not invent or
recreate missing modules/items from the log alone.

## Corporation tax audit and corrections

- `CorpRegistryBound::UpdateCorporation` required a `PyFloat*` tax
  argument and dereferenced it without validating encoding, null value or
  range. It now accepts numeric float/int/long, validates a finite fraction
  from **0.0 through 1.0** (e.g. 12.5% = 0.125), and checks the bound
  corporation is the character's own player corp.
- Only the CEO or a member holding the **Director** role may change
  corporation details. Previously no ownership/management check was present.
- `ServiceDB::ProcessRealChange` formatted with only two decimal places,
  so a tax such as `0.125` could persist as `0.13`. Save with six digits
  after the decimal, locale-independent.
- The old `UpdateCorporation` execution used a dynamically built SQL
  statement as a *printf format*, potentially corrupting text containing
  `%`. Execute the finished query with `"%s"`.
- The old update used an unsupported corporation multicast path, and then
  reused a tuple after the multicast decremented it. Use the existing
  `CorpNotify(CorpNews, OnCorporationChanged, corpid, ...)` path once.
- `Client::GetCorpTaxRate()` derives from a character's cached `CorpData`.
  Refresh the tax in the online members' cached corp data immediately after
  a successful database update; no relog should be required for bounty tax.

No database migrations, schema changes, direct tax updates, admin bypasses,
or data deletion are included.

## Recommended read-only before/after queries

```sql
-- Check whether the specific module/item survived the rejected bulk save.
SELECT itemID, typeID, itemName, ownerID, locationID, flag,
       quantity, singleton, customInfo
FROM entity
WHERE itemID = 140000641;

-- Inspect the affected Ferox inventory without changing it.
SELECT itemID, typeID, itemName, locationID, ownerID, flag, quantity
FROM entity
WHERE locationID = 140000640
ORDER BY itemID;

-- Check this character's current corporation, CEO and stored tax.
SELECT c.characterID, c.corporationID, corp.ceoID, corp.taxRate,
       corp.corporationName
FROM chrCharacters AS c
INNER JOIN crpCorporation AS corp ON corp.corporationID = c.corporationID
WHERE c.characterID = 90000000;
```

These are diagnostic **SELECT** statements only. Do **not** rerun the failed
`INSERT` line by hand; the server must own item persistence.

## In-game acceptance (after successful final-head CI)

- [ ] Start the new Windows build, matching its `BUILD-INFO.txt`.
- [ ] Save a ship/module with empty `customInfo` and with nonempty custom
  text, including literal apostrophes or percent signs; no #1064 errors.
- [ ] Restart; verify the saved item IDs, locations, flags and quantities.
- [ ] Change player corporation tax as CEO from e.g. 10% to 12.5%, confirm
  `crpCorporation.taxRate = 0.125` and corporation interface refreshes.
- [ ] Confirm an online member's subsequent NPC bounty uses the new rate
  without relogging.
- [ ] Confirm a Director can edit the rate and ordinary members cannot.
- [ ] Confirm foreign-corporation bound objects cannot change tax.
- [ ] Confirm out-of-range/NaN tax requests are rejected, and no accidental
  change occurs after bad input.
- [ ] Check successful Windows, Linux and Docker CI including
  `verify_station_crash_safety.py` source invariants.

**Important:** These are code fixes and regression guards, not yet
operator-confirmed in-game results.
