# Protocol and Memory-Safety Audit

All paths in this document are relative to the audited project root.

## NET-001: Unbounded Stream Buffering

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-core/network/StreamPacketizer.cpp:35-59` appends received bytes to
  `mBuffer` and retains incomplete frames indefinitely.
- A declared frame length is not rejected while only the four-byte header is
  available.
- `src/eve-common/network/EVETCPConnection.cpp:85-103` applies
  `PACKET_SIZE_LIMIT` only after `StreamPacketizer` has copied and queued the
  complete frame.

Impact:

An unauthenticated peer can declare a valid-looking large frame and send it
slowly. Each connection retains the partial frame, allowing memory exhaustion
before the post-buffer size check runs. Multiple queued frames can also grow
without a per-connection or global limit.

Required fix:

- Parse the four-byte length before appending the body.
- Reject zero, invalid, and oversized lengths immediately.
- Cap incomplete bytes, queued frames, and total queued bytes per connection.
- Disconnect on a limit violation instead of continuing to buffer.
- Use one named protocol limit for inbound framing and unmarshalling.

Compatibility:

Valid frames at or below the existing limit must retain their current wire
format and processing order.

Tests:

- Header-only input with a large length.
- Slow partial input at the limit.
- Zero-length input.
- Multiple frames that exceed the queue cap.

## NET-002: Unchecked Marshal Reads and Counts

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-common/marshal/EVEUnmarshal.h:75-100` advances iterators without
  checking remaining bytes.
- `src/eve-common/marshal/EVEUnmarshal.cpp:134-158` reads the header and save
  count before proving that the input has the required minimum length.
- `EVEUnmarshal.cpp:181-189` allocates the object store from an untrusted
  `saveCount`.
- `EVEUnmarshal.cpp:328-440` allocates strings, buffers, tuples, lists, and
  dictionaries from untrusted lengths and counts.

Impact:

Truncated or malicious marshal data can cause out-of-bounds iterator access,
assertion failures, huge allocations, long parse loops, or process termination.
`streamLength - sizeof(...)` can also underflow for short input.

Required fix:

- Make the reader carry a bounded end iterator and remaining-byte count.
- Check every scalar read, extended length, and container count.
- Reject lengths that exceed both remaining bytes and named per-type limits.
- Bound nesting depth and saved-object count.
- Clean up partially built containers on every parse error.

Compatibility:

The decoder may remain wire-compatible for valid packets. Invalid packets
should fail closed and close or quarantine the connection.

Tests:

- Every opcode with truncated input at each byte boundary.
- Maximum valid strings, buffers, lists, tuples, and dictionaries.
- Counts larger than remaining input.
- Deep nesting and saved-object references.

## NET-003: Unbounded Inflate and Empty Input

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-core/utils/Deflate.cpp:35-38` reads `data[0]` without checking for
  an empty buffer.
- `Deflate.cpp:81-106` doubles the output allocation until zlib succeeds and
  has no maximum output size or iteration bound.
- `src/eve-common/marshal/EVEUnmarshal.cpp:46-55` calls `IsDeflated` before
  unmarshal validation.

Impact:

Empty input can crash. A small compressed payload can force unbounded output
allocation, creating a compression-bomb denial of service.

Required fix:

- Return an error for empty input before inspecting the first byte.
- Use a named maximum inflated packet size.
- Stop when the output limit is reached or zlib makes no progress.
- Reject malformed compressed input without retrying indefinitely.

Tests:

- Empty input.
- Truncated zlib headers.
- A valid payload exactly at the output limit.
- A payload that requires more output than the limit.

## NET-004: Malformed Call and Session Crashes

Severity: High

Status: Confirmed

Evidence:

- `src/eve-common/python/PyPacket.cpp:738-744` converts a named-argument
  `None` into a null `arg_dict`.
- `src/eve-server/services/Callable.cpp:4-18` dereferences `dict` without a
  null check.
- `src/eve-server/Client.cpp:2666-2670` passes the untrusted dictionary into
  `PyCallArgs`.
- `Client.cpp:212-227` can log `m_char->name()` after a failed dispatch even
  when no character has been selected.
- The same null-character assumption appears in notification error paths.
- `src/eve-server/chat/LSCService.cpp:403-409` calls `message.at(0)` without
  rejecting an empty message.

Impact:

Malformed or pre-character packets can terminate a client-processing thread or
the server process, depending on the surrounding exception boundary.

Required fix:

- Treat a null named-argument dictionary as an empty dictionary.
- Reject null or invalid call tuples before constructing `PyCallArgs`.
- Use a safe account/session label in error logging before character selection.
- Return a protocol error for empty chat messages instead of indexing them.
- Reset `m_canThrow` with an exception-safe guard around dispatch.

Tests:

- Calls with named arguments set to `None`.
- Calls received before character selection.
- Failed dispatch with no character object.
- Empty chat messages and exceptions from service handlers.

## MEM-001: Packet and Python Representation Ownership

Severity: Critical

Status: Confirmed leak; ownership repair requires focused tests.

Evidence:

- `src/eve-server/Client.cpp:212-227` sets the popped packet pointer to null
  inside the loop, then calls `SafeDelete(p)` after the loop. The processed
  packet is therefore not deleted.
- `src/eve-common/python/PyRep.h:584-648` has an empty tuple destructor.
- `PyRep.h:655-727` has a list destructor that does not release items.
- `PyRep.h:735-834` has a dictionary destructor whose item cleanup is
  commented out.
- `src/eve-common/python/PyPacket.cpp:552-563` comments out named-argument
  cleanup in `PyCallStream`.
- `PyPacket.cpp:597-759` transfers raw nested pointers while leaving the
  ownership cleanup commented out.
- `PyPacket.cpp:154-160` and `:217-230` contain error returns that do not
  consistently release the consumed root object.

Impact:

Every malformed or successfully processed packet can leak a packet object or
an entire representation graph. An attacker can turn this into a long-lived
memory exhaustion condition. The inconsistent raw-pointer and refcount model
also creates use-after-free and double-decrement risk when cleanup is added in
only one layer.

Required fix:

- Document one ownership rule for every `PyRep` container operation.
- Repair container destructors and `SetItem`/`AddItem` ownership together.
- Make packet and call-stream decoding use explicit ownership transfer.
- Use scoped cleanup for partially decoded trees.
- Do not mix a destructor repair with unrelated wire-format changes.

Tests:

- Repeated valid packet processing under LeakSanitizer or an equivalent tool.
- Repeated malformed packet processing.
- Clone, encode, decode, and destroy every representation container.
- Exception paths during nested call decoding.

## MEM-002: Log Buffer Overflow

Severity: Critical

Status: Confirmed

Evidence:

- `src/eve-core/log/logsys.cpp:86-115` allocates a fixed 0x400-byte buffer.
- `vsnprintf` returns the required length, but that length is added to
  `log_msg_index` even when the output was truncated.
- The newline and terminator are then written using the over-advanced index.
- `logsys.cpp:173-180` uses unbounded `%[]` fields in `sscanf` for a 512-byte
  line buffer.

Impact:

Long attacker-controlled strings reaching a log call can write past the log
  allocation. A malformed log configuration line can overflow stack buffers.

Required fix:

- Use a dynamically sized formatter or clamp the index to the actual bytes
  written before appending the suffix.
- Add field widths to every `sscanf` conversion.
- Bound log message length and truncate with an explicit marker.
- Keep secrets and raw client payloads out of logs.

Tests:

- Log strings of zero, exact-limit, and over-limit lengths.
- A long client name, chat message, and SQL error.
- Configuration fields at and beyond their maximum lengths.

## NET-005: Connection and Queue Exhaustion

Severity: High

Status: Confirmed resource risk.

Evidence:

- `src/eve-core/network/TCPConnection.cpp:192-204` starts a new loop thread
  for each connection.
- `src/eve-server/eve-server.cpp:899-911` accepts connections and adds a new
  `Client` without a visible concurrent connection cap.
- The packetizer has no pending frame count or byte cap.

Impact:

Connection floods can exhaust threads, descriptors, client objects, or queued
  memory before authentication.

Required fix:

- Enforce global and per-address connection limits before client allocation.
- Bound pending bytes and packets.
- Add idle, handshake, and authentication timeouts.
- Prefer the existing thread pool or a bounded connection worker model.

Tests:

- Connection flood at the configured limit.
- Slow handshake timeout.
- Queue pressure with authenticated and unauthenticated clients.

## IMG-001: Image HTTP Resource and Parsing Risks

Severity: High

Status: Confirmed

Evidence:

- `src/eve-server/imageserver/ImageServerConnection.cpp:45-49` uses
  `async_read_until` without a header-size or read-time limit.
- `ImageServerConnection.cpp:90-101` parses identifiers with `atoi`, which
  accepts malformed and overflowing input without validation.
- `ImageServer.cpp:115-148` trusts `ftell`, allocates the full file, and has
  no maximum file size or read-result checks.
- `ImageServer.cpp:48-50` indexes an empty configured base path.
- `ImageServer.cpp:65-113` accepts image data without a size or format limit
  and does not check `fopen` or `fwrite` results.
- `ImageServerListener.cpp:46-65` accepts unlimited concurrent HTTP clients.

Impact:

Slow clients can retain HTTP buffers. Malformed paths can produce incorrect
IDs, and oversized or empty files can exhaust memory or trigger undefined
behavior. Upload data can also consume unbounded memory before persistence.

Required fix:

- Bound HTTP headers, request time, and concurrent image connections.
- Parse decimal IDs with strict range checks and reject trailing data.
- Bound image file and upload sizes, validate image format, and check all file
  operations.
- Reject an empty image directory during configuration validation.
- Use HTTPS or a configured internal URL for fallback redirects.

Tests:

- Header slowloris and oversized headers.
- Negative, overflowing, and trailing-character IDs.
- Empty, missing, and oversized image files.
- Oversized upload data and concurrent request limits.

## LOG-001: Credential and PII Logging

Severity: High

Status: Confirmed

Evidence:

- `src/eve-server/Client.cpp:2429-2439` logs the account name and client ID
  while reporting plain or hashed authentication behavior.
- `src/eve-server/ServiceDB.cpp:161` includes the plaintext password in an
  account-creation error log.
- `src/eve-server/chat/LSCService.cpp:1129-1134` logs message subject and body
  content.
- `src/eve-server/Client.cpp:2881-2889` can log chat content.
- Multiple production paths enable raw packet or call dumps through the
  existing logging system.

Impact:

Logs, backups, and support bundles can expose credentials, account data,
message content, and raw client payloads. The current text logger also makes
redaction and structured retention controls difficult.

Required fix:

- Remove credentials and message content from all production log calls.
- Replace raw packet dumps with bounded, redacted diagnostic events.
- Use structured, leveled records with retention and access controls.
- Add tests that assert sensitive values are absent from captured logs.

## Logging Remediation

The logging findings overlap authentication and chat data. The final fix must
remove plaintext passwords from `Client.cpp:2429-2439` and
`ServiceDB.cpp:150-172`, remove message content logging from
`LSCService.cpp:1129-1134`, and replace raw client dumps in production with
bounded, redacted structured events.
