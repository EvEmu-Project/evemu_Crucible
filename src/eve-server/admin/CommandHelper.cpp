#include "admin/CommandHelper.h"


// finds the nth noneq argument
// this function is 1 indexed meaning n = 1 would return the FIRST occurence
// if not found returns 0
// this function returns the index of the args
int cmd_find_nth_noneq(const Seperator &args, int n) {
    int c = 0;
    for (int i = 0; i < args.argCount(); i++) {
        std::string arg = args.arg(i);
        size_t pos = arg.find("=");

        // Didn't find = in string
        if (pos == std::string::npos) {
            c++;
            if (c == n) {
                return i;
            }
        }
    }
    return -1;
}

// name should be like "name="
std::string cmd_parse_eq_arg(const Seperator &args, const char *name) {
    for (int i = 1; i < args.argCount(); i++) {
        std::string arg = args.arg(i);
        size_t pos = arg.find(name);
        codelog(COMMAND__ERROR, "cmd_parse_eq_arg: %d name '%s' arg '%s' pos: '%d'", 
                i, name, arg.c_str(), pos);
        if (pos == std::string::npos) {
            continue;
        }
        if (strlen(name) >= arg.size()) {
            continue;
        }
        codelog(COMMAND__ERROR, "    returning: '%s'", arg.c_str() + strlen(name));
        return std::string(arg.c_str() + strlen(name));
    }

    return std::string();
}
