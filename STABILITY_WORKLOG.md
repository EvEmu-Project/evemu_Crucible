# EVEmu Stability Worklog

## 2026-11-13

### CI review — commit `f4585534db19971f564560709deedaad3bfbbaa9`

Reviewed both workflow executions for the same commit:

- Push run [43420481276](https://github.com/xrizonx/evemu_Crucible/actions/runs/43420481276)
- Pull request run [43420489840](https://github.com/xrizonx/evemu_Crucible/actions/runs/43420489840)

Results were consistent across both runs:
- Linux GCC: passed build, CTest, install, packaging, and artifact upload.
- Docker: image build passed.
- Windows 64-bit UCRT64 GCC: failed during compilation of `src/eve-core/utils/utils_time.cpp`.

### Confirmed Windows failure and fix

The compiler reported that `boost::gregorian` was not declared at the `GetTimeParts()` implementation, where the code constructs a `boost::gregorian::date` and reads its week/day fields.

The source relied on a transitive Boost include. Added the direct include:

```cpp
#include <boost/date_time/gregorian/gregorian.hpp>
```

This is a targeted include-dependency correction; date calculations and runtime behavior were not changed.

- Fix commit: `4e6c9ac0290d20ad29da45c6ef137fe3e0a9092a`
- Validation: pending a fresh Windows UCRT64 build.
- Linux and Docker were already green on the parent commit; rerun validation is still required for the new commit.

### PR and branch safeguards

- Work remains on `evemu-crucible-modernization`.
- PR #1 must remain open and draft.
- No merge, release, tag, issue closure, or default-branch changes were made.
### Follow-up CI review — current branch before BFD fix

The next push and pull-request runs for commit `a3450a888dc8684e67128dbf8d8e6d32c744c56d` confirmed:
- Linux GCC: passed build, CTest, install, packaging, and artifact upload.
- Docker: image build passed.
- Windows UCRT64: failed in `src/eve-core/utils/CallStack.cpp` because the installed modern BFD headers no longer declare the legacy `bfd_get_section_flags`, `bfd_get_section_vma`, and `bfd_get_section_size` accessors.

### Confirmed BFD compatibility fix

Updated the Windows MinGW BFD section inspection code to use the current section accessors:
- `bfd_section_flags(tsec)`
- `bfd_section_vma(tsec)`
- `bfd_section_size(tsec)`

This follows the current BFD API, which removed the older accessors that accepted an unnecessary `bfd*` argument. It preserves the section filtering, address range check, and symbol lookup behavior.

- Fix commit: `02764f59b4443839e4b822f629f90da63413c67a`
- Validation: pending new Linux, Windows UCRT64, and Docker workflow runs.
### Windows UCRT64 link-stage correction

The BFD accessor correction allowed compilation to progress further, but both CI runs for commit `70b1d0a0651101de08db822ebd2d0b7148d8eaa7` failed while linking `eve-xmlpktgen.exe`. The linker reported unresolved Winsock (`WSAStartup`, `WSACleanup`), DbgHelp (`SymInitialize`, `StackWalk64`, etc.), and BFD symbols.

Corrective changes:
- Removed the MinGW-only BFD stack-symbol implementation and use the already-present Windows DbgHelp symbol path instead. This removes the unnecessary BFD link dependency from stack tracing.
- Link `ws2_32` for Windows builds and `dbghelp` for non-MSVC Windows builds; MSVC retains `DbgHelp.lib`.
- Also fixed the early-return path after `SymInitialize` failure so the stack mutex is released.

Commits: `4c6f635e23177c2299c336a24c45bf5f64711834`, `86db8862da512b658ba568194e893485633d82b9`.

Validation is pending fresh CI on the updated branch. The PR was found unexpectedly closed during this check and was reopened immediately; it remains a draft and unmerged.


## 2026-11-13 — Windows UCRT64 Boost.Asio compatibility
- Rechecked the latest PR workflow: Linux GCC build, CTest, packaging, and Docker image build pass; Windows UCRT64 fails compiling `ImageServerConnection.cpp` because current Boost.Asio no longer exposes the legacy `const_buffers_1` type.
- Replaced the four static HTTP response buffer declarations/definitions with the supported `boost::asio::const_buffer` type. The response payloads and asynchronous write behavior are unchanged.
- Windows rebuild is required to verify this compatibility fix; no claim of a passing Windows build until CI completes.


### Windows UCRT64 linker correction — Mswsock

The uploaded Windows CI log shows the full 345-step compile completed and the failure occurs only when linking `eve-server.exe`. The remaining unresolved symbols are `AcceptEx` and `GetAcceptExSockaddrs`, used by Boost.Asio's Windows IOCP accept implementation. These extension functions are exported by the Windows Mswsock library.

- Updated the global Windows link dependencies from `ws2_32` to `ws2_32 mswsock`; this applies consistently to all Windows executable targets.
- No application code or socket behavior was changed.
- The same log contains repeated `std::iterator` deprecation warnings from `Buffer.h` and a GCC `-Wstringop-overflow` diagnostic around its generic `memmove`. These warnings are not the linker failure; they are recorded for a separate, behavior-preserving cleanup after the Windows link is verified. No warning suppression or test weakening was added.
- Validation: fresh CI required; Windows link success is not yet claimed.


### Windows UCRT64 runtime packaging correction

The latest Windows workflow now compiles all 345 translation units, links `eve-server.exe`, runs the configured CTest command, and installs the executable. The packaging step then failed because it hard-coded `C:\\msys64\\ucrt64\\bin\\objdump.exe`, which is not the executable location exposed by the hosted runner's MSYS2 setup.

Corrective workflow changes:
- Explicitly install the UCRT64 binutils package so `objdump.exe` is available.
- Resolve `objdump.exe` from the runner's command search path rather than assuming a fixed MSYS2 installation directory.
- Derive the runtime DLL search directory from the resolved executable path, keeping the dependency scan and recursive DLL-copy behavior intact.

- Fix commit: `5bc93bd8a6b525bc20e1679e3de1a0a3fdb23cb8`
- Validation: pending fresh Windows UCRT64 packaging run. A green Windows artifact is not yet claimed.
- Latest completed validation before this change: Linux GCC and Docker passed; Windows compile, link, test invocation, and install passed, but packaging failed.


### Green CI verification and warning review

Verified PR workflow run [43430216126](https://github.com/xrizonx/evemu_Crucible/actions/runs/43430216126) on commit `52f0a977d7120945d27f25960be8cf77af413fb7`:
- Linux GCC: build, CTest invocation, install, package, and artifact upload succeeded.
- Windows UCRT64: build, CTest invocation, install, runtime DLL packaging, and artifact upload succeeded.
- Docker image build succeeded.
- Both platform artifacts were uploaded successfully.
- Important qualification: CTest printed `No tests were found!!!` on both platforms. The CI green result therefore does not represent automated gameplay/test coverage.

### Confirmed null-pointer defects fixed after green run

The compiler warning review exposed two definite guard/handling defects:

1. `ModuleManager::RepairModule(GenericModule*, EvilNumber)` tested `pMod != nullptr` before logging that the module was not loaded and returning. This rejected valid modules and allowed null pointers to be dereferenced. Corrected the guard to `pMod == nullptr`; valid modules now reach `Repair(amount)`.
   - Commit: `ff6c88ebbb6e9b695e185990e452bd8601891990`

2. `Colony::LaunchCommodities()` tested for a failed container spawn, then dereferenced the null container and continued. Removed the invalid delete and now return safely when spawning fails and exceptions are not enabled; the existing exception path is retained.
   - Commit: `314ef3a8ddb76618b2e990c6f89b86173fbb42a2`

These are targeted null-safety fixes; fresh Linux, Windows, and Docker CI validation is required on the updated branch before claiming them verified.

### Remaining warning and quality backlog

The successful Linux build still reports format/type mismatches, ignored `vasprintf`/`fread` return values, possible null `this` paths, a non-null violation in `ServiceDB.cpp`, and unsafe non-literal format strings in admin/debug commands. Windows still reports deprecated `std::iterator` inheritance and a possible oversized `memmove` bound in `Buffer.h`. Both platforms report no registered CTest tests. GitHub Actions also warns that `actions/checkout@v4` and `actions/upload-artifact@v4` target deprecated Node.js 20 runtimes.

These are recorded for targeted review; no warning suppression or speculative gameplay changes were made.


### Follow-up warning cleanup after green baseline

Applied a small safety cleanup to five admin/debug reply formatting calls. They previously passed generated text as the `snprintf` format string; they now pass the explicit `"%s"` format and the generated text as data. This removes the `-Wformat-security` warnings at the reviewed call sites and prevents percent characters in reply text from being interpreted as format directives.

- `debug_commands.cpp`: commit `5420e4635910858ab2bc50488ad6af12c7896ac8`
- `DungeonCommands.cpp`: commit `e0db93b773a887b47776028d2dced8e7bca94756`
- `SystemCommands.cpp`: commit `da50679894796ed5a7409375342d5b1fb99e067d`

The null-safety fixes and formatting cleanup are committed on the modernization branch. CI validation is pending for the resulting branch head; the earlier all-green result applies to the parent commit only.


### Windows 11 runtime DLL packaging hardening

User reported that the distributed Windows ZIP was missing `libtinyxml-0.dll`, `libmariadb.dll`, and `libz-1.dll`, causing startup failures on Windows 11 systems without the MSYS2 development environment installed.

Updated the Windows packaging step:
- Resolve imported DLL dependencies recursively against the complete UCRT64 installation tree, rather than checking only the `bin` directory.
- Copy discovered runtime DLLs into the package root beside the executables so Windows loader lookup can find them.
- Explicitly verify that `libtinyxml-0.dll`, `libmariadb.dll`, and `libz-1.dll` are present in the staged package; fail packaging if any are absent instead of uploading an incomplete ZIP.

- Fix commit: `bf340ba09a34dbb3e8d17b9d738b821cb21b01db`
- Validation: fresh Windows packaging workflow pending. The fix is not yet claimed as verified until the build produces a ZIP containing all three DLLs.


### Windows runtime DLL placement correction and ServiceDB numeric formatting

A follow-up user test confirmed the runtime DLLs still were not beside `eve-server.exe` after extraction. The earlier scanner copied DLLs to the staging root, which is insufficient when installed executables live in a subdirectory. Packaging now discovers every staged executable directory, places recursively discovered dependencies beside the executables, and verifies the three reported libraries in each executable directory before creating the ZIP.

Also corrected a confirmed undefined-behavior defect in `ServiceDB::ProcessRealChange()`: it passed null pointers to `fcvt()` for required output parameters. Replaced this with a locale-independent fixed two-decimal `std::ostringstream` representation, avoiding the invalid pointers and the non-thread-safe conversion function.

- Changes committed together to avoid unnecessary CI cancellation.
- Validation remains pending; Windows ZIP contents and both platform builds must be checked before calling this complete.


### Windows packaging failure diagnosis and build-time improvements

The failed Windows run completed compilation and linking successfully (345/345 objects). Packaging then failed because the PowerShell step selected `objdump.exe` from `C:\mingw64` via the Windows PATH, while the workflow is configured for UCRT64. That made the DLL search root incorrect; it could not see the installed UCRT64 TinyXML DLL. The official MSYS2 UCRT64 TinyXML package provides `/ucrt64/bin/libtinyxml-0.dll` (package metadata checked against MSYS2).

Corrected packaging to run dependency scanning inside the UCRT64 MSYS2 shell and use `/ucrt64/bin/objdump.exe` and `/ucrt64/bin` explicitly. The recursive dependency scanner now copies imported DLLs into the same directory as `stage/bin/eve-server.exe` and verifies all three reported DLLs there before ZIP creation. Archive creation remains a separate PowerShell step.

Added ccache to Linux and Windows CI, with GitHub Actions cache storage keyed by platform, architecture, branch and CMake inputs. This should accelerate repeat builds by reusing unchanged compilation outputs; clean builds still compile all translation units. Compiler parallelism remains at two workers, matching the hosted runner's typical CPU allocation.

- Failure diagnosed from Windows job `131909122206`: compile/link succeeded; package failed because it searched `C:\mingw64` instead of the UCRT64 prefix.
- New workflow changes: packaging path correction and ccache.
- Validation: pending fresh Linux, Windows and Docker runs.


### Windows startup-crash diagnostic symbols

The user's Windows 11 executable terminates before `main()` during C++ static initialization. The supplied GDB trace reaches the compiler-generated initializer for the translation unit containing `PyException::PyException(PyRep*)`, but the packaged executable has no debugging symbols. GDB also reports GCC's Windows C++ exception code (`0x20474343`) as an unknown target exception, so continuing causes the process to exit without a usable throw-site stack.

Changed the Windows CI configuration from `Release` to `RelWithDebInfo`. This keeps optimized code while embedding debug information in the executable, allowing a fresh Windows ZIP to provide source-level symbols for the next GDB run. Linux remains a Release build. No runtime code has been changed based on the incomplete stack trace.

- Diagnostic change: Windows `RelWithDebInfo` build; symbols must be confirmed in the resulting executable with GDB before use.
- Validation: pending Windows CI run and a fresh user-side GDB trace.


### Windows startup crash root cause and targeted fix

A user-provided GDB stack trace now identifies the exact startup failure:

- `std::string::erase(this=0x5ffca0, __pos=7)` throws because the string is empty.
- The call originates from global static initialization at `src/eve-common/EVEVersion.h:50`.
- The failing expression truncated `GIT_COMMIT_HASH` using `.erase(7, std::string::npos)`. The Windows build supplied an empty value for this macro, so the operation threw before `main()`.
- The compiler emitted the global initializer in the translation unit containing `PyException`, which was initially a misleading location; the source-level stack identifies `EVEVersion.h` as the actual fault.

Replaced the positional erase with a bounds-safe substring operation and an explicit `"unknown"` fallback when the build hash is empty. This prevents version-string construction from throwing during static initialization. The underlying reason the Windows build supplied an empty hash remains worth checking in CMake metadata generation, but it is no longer fatal to startup.

- Fix commit: `61226c9845f6aa8981ad40edd94f1d4fcef4554b`
- Validation: pending fresh Windows CI build and user test of the newly packaged ZIP. This is a source-level diagnosis and targeted fix, not yet a verified runtime pass.


### Database utilities included in release artifacts

The Windows runtime testing exposed that the repository's SQL directory had only a shell downloader and no Windows database-tool executable in the packaged release. The existing `get_evedbtool.bat` was also downloading from an unrelated/incorrect release path.

Changes made:
- The Linux CI package now downloads the official EVEDBTool 0.0.6 x86_64 binary into `sql/evedbtool`.
- The Windows CI package now downloads the official EVEDBTool 0.0.6 Windows executable into `sql/evedbtool.exe`.
- The Windows server DLL bundling step explicitly excludes the standalone Go database utility, so it is not mistaken for a MinGW server executable.
- Added `sql/evedb.yaml.example` with a local-only password placeholder; the real `evedb.yaml` remains git-ignored.
- Replaced the broken Windows downloader with a pinned, checked download of the official 0.0.6 executable.
- Rewrote `sql/README.md` with Windows/Linux setup, status checks, backup guidance, and a manual SQL migration procedure.

Manual migration safety:
- Only each migration's `-- +migrate Up` section is executed in filename order; `Down` sections are rollback SQL.
- `evedbtool skip` is documented only after every pending migration has been manually applied and verified. It records migration versions but does not execute SQL.
- No database migrations are run automatically by the release workflow or server startup.

Validation status: source and workflow edits are committed; fresh CI and artifact inspection are still required. The packaged Windows binary, YAML config, and migration folder must be confirmed in the resulting ZIP before this is considered release-verified.
