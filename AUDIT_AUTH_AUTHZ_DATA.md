# Authentication, Authorization, and Data Audit

All paths in this document are relative to the audited project root.

## AUTH-001: Plaintext and Weak Password Handling

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-server/ServiceDB.cpp:76-78` selects both `password` and `hash`.
- `ServiceDB.cpp:89-118` auto-creates and returns plaintext password data.
- `ServiceDB.cpp:150-162` inserts plaintext password and hash together and
  logs the plaintext password on failure.
- `ServiceDB.cpp:169-173` writes plaintext passwords on update.
- `src/eve-server/Client.cpp:2429-2439` uses `strcmp` for password and hash
  comparisons and logs when plain-password authentication is used.
- The password update inside the hash branch is unreachable because it is
  nested under the branch where the plain password is empty.
- `src/eve-common/auth/PasswordModule.cpp:103-128` uses a salted-by-username
  SHA-1 construction with only 1,000 iterations, not a password KDF.

Impact:

Database, SQL error, debug, or backup exposure compromises every account. The
weak hash is inexpensive to brute force, and ordinary `strcmp` comparisons do
not provide constant-time behavior.

Required fix:

- Stop storing and logging plaintext passwords.
- Introduce a maintained password KDF with a versioned record format and a
  per-account random salt.
- Support a controlled dual-read migration only long enough to rehash accounts
  after successful login; never recreate plaintext storage.
- Use exact-length constant-time verification.
- Remove plain-password authentication once compatible clients are migrated.
- Rotate all deployed database and account credentials after the change.

Compatibility:

Keep the account table usable during migration, but make the new KDF record the
only newly written format. Define a one-time migration and recovery procedure
before deleting old columns.

Tests:

- Correct and incorrect passwords for each migration state.
- Hash version upgrade after successful login.
- Exact-length and prefix/suffix mismatch cases.
- Database failure without credential logging.

## AUTH-002: Placebo Crypto and Missing Transport Authentication

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-server/Client.cpp:2376-2397` accepts only the `placebo` crypto path.
- The accepted path queues `OK CC` without establishing authenticated
  encryption.
- `Client.cpp:2400-2469` sends authentication material over that connection.

Impact:

Network observers or active intermediaries can capture credentials, alter
authentication traffic, or tamper with service calls.

Required fix:

- Use an authenticated encrypted transport or an equivalent authenticated
  challenge protocol with replay protection.
- Bind the account challenge to the negotiated session and server identity.
- Reject downgrade to placebo outside an explicit local development mode.
- Do not log challenge, password, or hash payloads.

Compatibility:

Negotiate the new mode explicitly and retain the existing service and packet
interfaces after transport setup. A development-only placebo flag must default
off and be impossible to enable through an untrusted client request.

## AUTHZ-001: Automatic Development Accounts

Severity: Critical

Status: Confirmed

Evidence:

- `utils/config/eve-server.xml:13-22` sets `autoAccountRole` to the documented
  `ROLE_DEV` value.
- `src/eve-server/ServiceDB.cpp:89-105` creates an unknown account using that
  configured role and the supplied credentials.

Impact:

Anyone who can reach the login service can create a development-privileged
account when this default is active.

Required fix:

- Set the production default to zero and fail closed if the value is invalid.
- Require an explicit, local-only development configuration for auto-account
  creation.
- Never allow auto-created accounts to receive administrative or developer
  roles.
- Add startup logging that reports the safe mode without exposing credentials.

## AUTHZ-002: Character Ownership and Lifecycle IDOR

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-server/character/CharUnboundMgrService.cpp:38-56` registers the
  service with the default access level.
- `CharUnboundMgrService.cpp:63-66` returns selection data for any valid ID.
- `CharUnboundMgrService.cpp:124-139` selects any valid character without
  checking its account against `call.client->GetUserID()`.
- `CharUnboundMgrService.cpp:73-76` deletes any supplied character ID.
- `src/eve-server/Client.cpp:1904-1926` initializes a session from the ID
  without an account ownership check.
- `src/eve-server/character/CharacterDB.cpp:129-167` deletes the character
  and related data using only the supplied ID.
- The prepare and cancel methods do check account ownership at
  `CharacterDB.cpp:195-218`, but direct deletion does not require a prepared
  deletion or that account check.

Impact:

An authenticated account can select another account's character, destroy its
data, and potentially operate with the victim's assets and session state.

Required fix:

- Require account ownership for selection, information, preparation, cancel,
  and deletion.
- Require a valid deletion preparation timestamp before destructive deletion.
- Verify the character is not online and perform deletion transactionally.
- Return a generic failure without revealing whether another account's ID
  exists.

## AUTHZ-003: Corporation Wallet and Journal Authorization

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-server/account/AccountService.cpp:52-72` exposes wallet methods at
  character access level, not a corporation role level.
- `AccountService.cpp:124-152` returns corporation wallet balances without a
  corporation-role check.
- `AccountService.cpp:157-175` reads a corporation journal for any member
  session without checking the required wallet role.
- `AccountService.cpp:247-293` calls `TransferFunds` from the current
  corporation without validating `fromAcctKey` permissions.
- `AccountService.cpp:531-573` changes the corporation balance and journals
  the transaction.

Impact:

Any character with a valid corporation session may read sensitive wallet data
or withdraw corporation funds, subject to the supplied account key.

Required fix:

- Define a single role-to-wallet-operation policy.
- Check division access and withdrawal authority immediately before each read
  or mutation.
- Validate account keys against the corporation session and reject invalid
  keys.
- Record the authorized actor, target corporation, division, and result in a
  redacted audit event.

## AUTHZ-004: Developer, QA, and Dungeon Services

Severity: High

Status: Confirmed

Evidence:

- `src/eve-server/services/Service.h:68-74` defaults services to
  `eAccessLevel_None`.
- `src/eve-server/services/ServiceManager.cpp:58-78` checks session state
  fields but does not check account roles.
- `src/eve-server/eve-server.cpp:685-715` registers `DungeonService`,
  `DevToolsProviderService`, and QA services in the active server.
- `src/eve-server/admin/DevToolsProviderService.cpp:31-53` returns the
  development loader file without a role check.
- `src/eve-server/dungeon/DungeonService.cpp:51-77,95-167` exposes dungeon
  editing and mutation methods without an administrative role check.
- `src/eve-server/system/KeeperService.cpp:50-79` creates a shared editor
  instance for any client with the required system session state.
- `eve-server.cpp:714-715,730` also registers encounter, action, and network
  QA service names in the live service registry.

Impact:

Regular clients can discover development behavior, read server-side files, or
reach editor and QA code. Dungeon mutation can affect shared server state.

Required fix:

- Add role checks at the service boundary and repeat authorization in
  destructive handlers.
- Do not register QA/editor services in a production build by default.
- Scope bound editor instances to an authorized client and editor session.
- Fail closed when the configured role mask is missing or malformed.

## DATA-001: Mail IDOR and Missing Ownership Filters

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-server/mail/MailMgrService.cpp:163-207` accepts arbitrary message
  IDs for deletion, read state changes, and body retrieval.
- `src/eve-server/mail/MailDB.cpp:151-164` retrieves a body by message ID only.
- `MailDB.cpp:368-380,485-531` changes message state by ID only.
- `MailDB.cpp:246-274,443-579` builds multi-message updates without recipient
  or sender ownership predicates.
- `src/eve-server/chat/LSCService.cpp:1108-1112` explicitly leaves message
  read authorization as a TODO and accepts caller-supplied reader and message
  IDs.

Impact:

An authenticated character can read, mark, label, move, or delete another
character's mail by guessing message IDs. Bulk calls amplify the impact.

Required fix:

- Resolve message access through the current character's recipient, sender,
  list, or permitted corporation relationship.
- Include the ownership predicate in every read and write query.
- Validate and cap every submitted ID list before query construction.
- Use a transaction for multi-row state changes and return only affected rows
  visible to the caller.

## DATA-002: Mail and Chat SQL Injection

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-server/mail/MailDB.cpp:97-103` inserts `title` without escaping.
- `MailDB.cpp:304-365` inserts and updates label names without escaping.
- `src/eve-server/chat/LSCDB.cpp:88-115` inserts channel display name, MOTD,
  comparison key, and password without escaping.
- `LSCDB.cpp:145-154,222-225,365-368` interpolates user-controlled channel
  names or comparison keys into equality or regular-expression queries.

Impact:

Authenticated users can alter query meaning, read data, or execute unintended
database writes through mail, labels, or custom chat channel fields.

Required fix:

- Use parameterized database operations for values.
- If the database wrapper cannot bind values, use its connection-aware escape
  function consistently and validate maximum lengths first.
- Treat regular-expression searches as a separate input language and either
  escape regex metacharacters or replace them with exact matching.
- Keep table and column identifiers in internal allowlists only.

## CHAT-001: Missing Chat Authorization and Input Limits

Severity: High

Status: Confirmed

Evidence:

- `src/eve-server/chat/LSCService.cpp:273-342` joins channels without checking
  passwords or access-control policy.
- `LSCService.cpp:360-409` sends to any in-memory channel without checking
  membership, moderation, or channel access.
- `LSCService.cpp:528-645` configures any channel without verifying its owner;
  a missing old password can set a new one.
- `LSCService.cpp:764-791` destroys any channel and documents that ownership
  validation is not implemented.
- `LSCService.cpp:795-826` returns any channel's member list.
- `LSCService.cpp:403-405` can route a client-supplied dot command to the
  slash service.

Impact:

Clients can join private channels, read membership, change configuration,
destroy channels, post to channels they do not belong to, or reach slash
commands through chat input.

Required fix:

- Centralize channel authorization for join, read, write, configure, invite,
  moderate, and destroy operations.
- Require the owner or an explicit moderator role for configuration and
  destruction.
- Verify passwords with a bounded, non-logged comparison.
- Remove the chat-to-admin command path or gate it with the admin policy.
- Cap channel names, MOTDs, passwords, message sizes, and recipient lists.

## AUTH-003: Migration and Login Policy Gap

Severity: Medium

Status: Validation required

The traced login path has no visible per-account or per-address failure
throttle, and the password hash upgrade path is unreachable as written. Before
changing the protocol, define account lockout, IP rate limiting, recovery, and
hash migration behavior. Do not add permanent lockout behavior without an
operator recovery path.

## Data Verification Tests

- Cross-account character select, prepare, cancel, and delete attempts.
- Corporation member access with and without each wallet role.
- Mail access using a valid ID belonging to another character.
- Labels, titles, channel names, MOTDs, and passwords containing quotes and
  database metacharacters.
- Private channel join, configure, destroy, membership, and message attempts.
- Logs inspected for password, hash, message body, and raw credential values.
