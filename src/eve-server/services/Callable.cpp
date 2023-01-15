#include "Callable.h"
#include "Client.h"

PyCallArgs::PyCallArgs(Client* c, PyTuple* tup, PyDict* dict) :
        client(c),
        tuple(tup)
{
    // PyIncRef( tup );
    for (PyDict::const_iterator cur = dict->begin(); cur != dict->end(); cur++) {
        if (!cur->first->IsString()) {
            _log(SERVICE__ERROR, "Non-string key in call named arguments. Skipping.");
            cur->first->Dump(SERVICE__ERROR, "    ");
            continue;
        }
        PyIncRef(cur->second);
        byname[cur->first->AsString()->content()] = cur->second;
    }
}

PyCallArgs::~PyCallArgs() {
    PySafeDecRef(tuple);
    for (auto cur : byname)
        PySafeDecRef(cur.second);
}

void PyCallArgs::Dump(LogType type) const {
    if (!is_log_enabled(type))
        return;

    _log(type, "  Call Arguments:");
    tuple->Dump(type, "    ");
    if (!byname.empty()) {
        _log(type, " Named Arguments:");
        for (auto cur : byname) {
            _log(type, "  %s", cur.first.c_str());
            cur.second->Dump(type, "    ");
        }
    }
}

/* PyResult */
PyResult::PyResult() : ssResult(nullptr), ssNamedResult(nullptr) {}
PyResult::PyResult(PyRep* result)
        : ssResult(result != nullptr ? result : PyStatic.NewNone()),
          ssNamedResult(nullptr)
{}
PyResult::PyResult(PyRep* result, PyDict* namedResult)
        : ssResult(result != nullptr ? result : PyStatic.NewNone()),
          ssNamedResult(namedResult)
{}

PyResult::PyResult(const PyResult& oth) : ssResult(nullptr), ssNamedResult(nullptr) { *this = oth; }
PyResult::~PyResult() { PySafeDecRef(ssResult); PySafeDecRef(ssNamedResult); }

PyResult& PyResult::operator=(const PyResult& oth)
{
    PySafeDecRef(ssResult);
    if (oth.ssResult != nullptr) {
        ssResult = oth.ssResult;
    }
    else {
        ssResult = PyStatic.NewNone();
    }
    PySafeIncRef(ssResult);

    PySafeDecRef(ssNamedResult);
    ssNamedResult = oth.ssNamedResult;
    PySafeIncRef(ssNamedResult);

    return *this;
}
