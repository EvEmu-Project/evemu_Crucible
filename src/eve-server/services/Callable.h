//
// Created by games on 15/01/2023.
//

#ifndef EVEMU_CALLABLE_H
#define EVEMU_CALLABLE_H

#include <map>
#include <optional>

#include "eve-server.h"

class Client;

enum AccessLevel {
    eAccessLevel_None = 0,
    eAccessLevel_Location = 1,
    eAccessLevel_LocationPreferred = 2,
    eAccessLevel_SolarSystem = 3,
    eAccessLevel_SolarSystem2 = 4,
    eAccessLevel_Station = 5,
    eAccessLevel_Character = 6,
    eAccessLevel_Corporation = 7,
    eAccessLevel_User = 8
};

class method_not_found {
public:
    method_not_found() {}
};

class PyCallArgs
{
public:
    PyCallArgs(Client* c, PyTuple* tup, PyDict* dict);
    ~PyCallArgs();

    void Dump(LogType type) const;

    Client* const client;    //we do not own this
    PyTuple* tuple;        //we own this, but it may be taken
    std::map<std::string, PyRep*> byname;    //we own this, but elements may be taken.
};

class PyResult
{
public:
    PyResult();
    PyResult(PyRep* result);
    PyResult(PyRep* result, PyDict* namedResult);
    PyResult(const PyResult& oth);
    ~PyResult();

    PyResult& operator=(const PyResult& oth);

    PyRep* ssResult;
    PyDict* ssNamedResult;
};


template <class T> struct is_optional : std::false_type {};
template <class T> struct is_optional<std::optional<T>> : std::true_type {};

struct CallHandlerBase {
public:
    virtual PyResult operator() (void* service, PyCallArgs& args) const = 0;
};

template <class S>
struct CallHandler : public CallHandlerBase {
    template<class... Args> CallHandler(PyResult(S::*callHandler)(PyCallArgs&, Args...)) :
            erasedHandler(reinterpret_cast <PyResult(S::*)()> (callHandler)),
            handlerImpl {
                [this](S* service, PyResult(S::*erasedHandler)(), PyCallArgs& args) -> PyResult {
                    auto handler = reinterpret_cast <PyResult(S::*)(PyCallArgs&, Args...)> (erasedHandler);

                    if constexpr (sizeof...(Args) == 0) {
                        // ensure there's no arguments in the data
                        if (args.tuple->size() != 0)
                            throw std::invalid_argument("expected 0 arguments");

                        // all the functions must have the PyCallArgs to know important info about the call
                        // like the client that sent it
                        return (service->*handler) (args);
                    } else {
                        if (this->validateArgs <std::decay_t<Args>...>(args) == false)
                            throw std::invalid_argument("arguments do not match");

                        return this->apply(service, handler, args);
                    }
                }
            }
    { }

    PyResult operator() (void* service, PyCallArgs& args) const override {
        return handlerImpl(reinterpret_cast <S*> (service), erasedHandler, args);
    }

private:
    template <typename T> bool validateArg(size_t index, PyCallArgs& args) {
        // handle optional values
        if constexpr (is_optional <T>::value) {
            // treat Nones on optional values as valid
            if (index >= args.tuple->size() || args.tuple->GetItem(index)->IsNone()) {
                return true;
            } else {
                return validateArg <typename T::value_type>(index, args);
            }
        }

        if (index >= args.tuple->size()) {
            return false;
        }

        if constexpr (std::is_same_v <T, PyRep*>)
            return true;

        PyRep* rep = args.tuple->GetItem(index);

        // validate type with their parameter equivalent
        if constexpr (std::is_same_v <T, PyBool*>)
            return rep->IsBool();
        if constexpr (std::is_same_v <T, PyInt*>)
            return rep->IsInt();
        if constexpr (std::is_same_v <T, PyLong*>)
            return rep->IsLong();
        if constexpr (std::is_same_v <T, PyFloat*>)
            return rep->IsFloat();
        if constexpr (std::is_same_v <T, PyBuffer*>)
            return rep->IsBuffer();
        if constexpr (std::is_same_v <T, PyString*>)
            return rep->IsString();
        if constexpr (std::is_same_v <T, PyWString*>)
            return rep->IsWString();
        if constexpr (std::is_same_v <T, PyToken*>)
            return rep->IsToken();
        if constexpr (std::is_same_v <T, PyTuple*>)
            return rep->IsTuple();
        if constexpr (std::is_same_v <T, PyList*>)
            return rep->IsList();
        if constexpr (std::is_same_v <T, PyDict*>)
            return rep->IsDict();
        if constexpr (std::is_same_v <T, PyNone*>)
            return rep->IsNone();
        if constexpr (std::is_same_v <T, PySubStruct*>)
            return rep->IsSubStruct();
        if constexpr (std::is_same_v <T, PySubStream*>)
            return rep->IsSubStream();
        if constexpr (std::is_same_v <T, PyChecksumedStream*>)
            return rep->IsChecksumedStream();
        if constexpr (std::is_same_v <T, PyObject*>)
            return rep->IsObject();
        if constexpr (std::is_same_v <T, PyObjectEx*>)
            return rep->IsObjectEx();
        if constexpr (std::is_same_v <T, PyPackedRow*>)
            return rep->IsPackedRow();

        return false;
    }

    template <typename T> decltype(auto) getAs(size_t index, PyTuple* tup) {
        if constexpr (is_optional <T>::value) {
            if (index >= tup->size() || tup->GetItem(index)->IsNone()) {
                return T{};
            } else {
                return std::make_optional(getAs <typename T::value_type>(index, tup));
            }
        }

        if (index >= tup->size()) {
            throw std::runtime_error("This should not happen. Trying to get parameter out of bounds. What happened to the validation?!");
        }

        PyRep* rep = tup->GetItem(index);

        if constexpr (std::is_same_v <T, PyRep*>)
            return rep;
        if constexpr (std::is_same_v <T, PyBool*>)
            return rep->AsBool();
        if constexpr (std::is_same_v <T, PyInt*>)
            return rep->AsInt();
        if constexpr (std::is_same_v <T, PyLong*>)
            return rep->AsLong();
        if constexpr (std::is_same_v <T, PyFloat*>)
            return rep->AsFloat();
        if constexpr (std::is_same_v <T, PyBuffer*>)
            return rep->AsBuffer();
        if constexpr (std::is_same_v <T, PyString*>)
            return rep->AsString();
        if constexpr (std::is_same_v <T, PyWString*>)
            return rep->AsWString();
        if constexpr (std::is_same_v <T, PyToken*>)
            return rep->AsToken();
        if constexpr (std::is_same_v <T, PyTuple*>)
            return rep->AsTuple();
        if constexpr (std::is_same_v <T, PyList*>)
            return rep->AsList();
        if constexpr (std::is_same_v <T, PyDict*>)
            return rep->AsDict();
        if constexpr (std::is_same_v <T, PyNone*>)
            return rep->AsNone();
        if constexpr (std::is_same_v <T, PySubStruct*>)
            return rep->AsSubStruct();
        if constexpr (std::is_same_v <T, PySubStream*>)
            return rep->AsSubStream();
        if constexpr (std::is_same_v <T, PyChecksumedStream*>)
            return rep->AsChecksumedStream();
        if constexpr (std::is_same_v <T, PyObject*>)
            return rep->AsObject();
        if constexpr (std::is_same_v <T, PyObjectEx*>)
            return rep->AsObjectEx();
        if constexpr (std::is_same_v <T, PyPackedRow*>)
            return rep->AsPackedRow();
    }

    template<class... Args>
    bool validateArgs(PyCallArgs& args) {
        constexpr size_t ArgCount = sizeof... (Args);
        const bool argCountIsValid = args.tuple->size() <= ArgCount;

        if (argCountIsValid == false) {
            return false;
        }

        bool isValid = true;
        size_t index = 0;

        ((
                isValid = validateArg<Args>(index, args) && isValid,
                        ++index
        ), ...);

        return isValid;
    }

    template<class... Args, size_t... I>
    PyResult applyImpl(
            S* service,
            PyResult(S::*handler)(PyCallArgs& args, Args...),
            PyCallArgs& args,
            std::index_sequence<I...>) {
        return (service->*handler) (args, getAs<std::decay_t<Args>>(I, args.tuple)...);
    }

    template<class... Args>
    PyResult apply(
            S* service,
            PyResult(S::*handler) (PyCallArgs& args, Args...),
            PyCallArgs& args) {
        return applyImpl(
                service, handler, args,
                std::make_index_sequence <sizeof...(Args)>{}
        );
    }

    PyResult(S::*erasedHandler)() = nullptr;
    std::function <PyResult(S* service, PyResult(S::* erasedHandler)(), PyCallArgs& args)> handlerImpl;
};

#endif //EVEMU_CALLABLE_H
