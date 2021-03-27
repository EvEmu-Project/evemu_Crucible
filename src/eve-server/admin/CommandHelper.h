#ifndef __COMMAND_HELPER_H__
#define __COMMAND_HELPER_H__

#include "ConsoleCommands.h"

int cmd_find_nth_noneq(const Seperator &args, int n);
std::string cmd_parse_eq_arg(const Seperator &args, const char *name);

#endif
