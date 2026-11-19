#include "eve-test.h"
#include "EVE_Roles.h"
#include "EVE_CommandPermissions.h"
#include <cstdio>
#include <cstdlib>

int utils_CommandPermissionsTest(int argc, char* argv[])
{
    (void)argc; (void)argv;
    const bool passed =
        EvECommand::HasAnyRequiredRole(
            Acct::Role::DEV, Acct::Role::CONTENT, Acct::Role::ADMIN)
        && !EvECommand::HasRequiredRole(Acct::Role::DEV, Acct::Role::ADMIN)
        && EvECommand::HasAnyRequiredRole(
            Acct::Role::BOSS, Acct::Role::CONTENT, Acct::Role::ADMIN)
        && EvECommand::HasAnyRequiredRole(
            Acct::Role::CONTENT, Acct::Role::CONTENT, Acct::Role::ADMIN)
        && EvECommand::HasAnyRequiredRole(
            Acct::Role::ADMIN, Acct::Role::CONTENT, Acct::Role::ADMIN)
        && !EvECommand::HasAnyRequiredRole(
            Acct::Role::VIP, Acct::Role::CONTENT, Acct::Role::ADMIN)
        && !EvECommand::HasAnyRequiredRole(
            Acct::Role::STD, Acct::Role::CONTENT, Acct::Role::ADMIN)
        && !EvECommand::HasAnyRequiredRole(Acct::Role::DEV, Acct::Role::ADMIN)
        && !EvECommand::HasRequiredRole(
            Acct::Role::ADMIN, Acct::Role::ADMIN | Acct::Role::CONTENT)
        && EvECommand::HasRequiredRole(
            Acct::Role::BOSS, Acct::Role::ADMIN | Acct::Role::CONTENT);
    if (!passed) {
        ::printf("Command permission mask regression test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Command permission mask regression test passed.\n");
    return EXIT_SUCCESS;
}
