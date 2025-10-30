#ifndef __ICOMMAND_H_INCL__
#define __ICOMMAND_H_INCL__

#include <string>
#include <vector>
#include <memory>
#include "account/AccountService.h"

class Client;
class EVEServiceManager;
class PyResult;

namespace Commands {

    // Command parameter type enumeration
    enum class ParameterType {
        String,
        Integer,
        Float,
        Boolean,
        EntityID,
        ItemID,
        TypeID
    };

    // Command parameter definition
    struct CommandParameter {
        std::string name;
        ParameterType type;
        bool required;
        std::string description;
        std::string defaultValue;
        
        CommandParameter(const std::string& n, ParameterType t, bool req, const std::string& desc, const std::string& def = "")
            : name(n), type(t), required(req), description(desc), defaultValue(def) {}
    };

    // Command execution context
    struct CommandContext {
        Client* client;
        EVEServiceManager* services;
        std::vector<std::string> arguments;
        
        CommandContext(Client* c, EVEServiceManager* s, const std::vector<std::string>& args)
            : client(c), services(s), arguments(args) {}
    };

    // Command execution result
    struct CommandResult {
        bool success;
        std::string message;
        std::shared_ptr<PyResult> data;
        
        CommandResult(bool s, const std::string& msg, std::shared_ptr<PyResult> d = nullptr)
            : success(s), message(msg), data(d) {}
        
        static CommandResult Success(const std::string& msg = "", std::shared_ptr<PyResult> data = nullptr) {
            return CommandResult(true, msg, data);
        }
        
        static CommandResult Error(const std::string& msg) {
            return CommandResult(false, msg);
        }
    };

    // Command interface
    class ICommand {
    public:
        virtual ~ICommand() = default;
        
        // Get command name
        virtual std::string GetName() const = 0;
        
        // Get command description
        virtual std::string GetDescription() const = 0;
        
        // Get command category
        virtual std::string GetCategory() const = 0;
        
        // Get required permissions
        virtual int64 GetRequiredRole() const = 0;
        
        // Get parameter definitions
        virtual std::vector<CommandParameter> GetParameters() const = 0;
        
        // Get usage examples
        virtual std::vector<std::string> GetExamples() const = 0;
        
        // Validate parameters
        virtual bool ValidateParameters(const std::vector<std::string>& args, std::string& errorMsg) const;
        
        // Execute command
        virtual CommandResult Execute(const CommandContext& context) = 0;
        
        // Generate help text
        virtual std::string GenerateHelp() const;
        
    protected:
        // Helper methods: Parse parameters
        bool ParseInt(const std::string& str, int32& value) const;
        bool ParseFloat(const std::string& str, float& value) const;
        bool ParseBool(const std::string& str, bool& value) const;
        uint32 ParseEntityID(const std::string& str) const;
        
        // Helper methods: Permission check
        bool HasPermission(Client* client, int64 requiredRole) const;
        
        // Helper methods: Get target client
        Client* GetTargetClient(const std::string& target, Client* executor) const;
    };

    // Command base class template
    template<typename TDerived>
    class CommandBase : public ICommand {
    public:
        CommandBase() = default;
        
        std::string GetName() const override {
            return m_name;
        }
        
        std::string GetDescription() const override {
            return m_description;
        }
        
        std::string GetCategory() const override {
            return m_category;
        }
        
        int64 GetRequiredRole() const override {
            return m_requiredRole;
        }
        
        std::vector<CommandParameter> GetParameters() const override {
            return m_parameters;
        }
        
        std::vector<std::string> GetExamples() const override {
            return m_examples;
        }
        
        CommandResult Execute(const CommandContext& context) override {
            return DoExecute(context);
        }
        
    protected:
        virtual CommandResult DoExecute(const CommandContext& context) = 0;
        
        std::string m_name;
        std::string m_description;
        std::string m_category;
        int64 m_requiredRole;
        std::vector<CommandParameter> m_parameters;
        std::string m_usage;
        std::vector<std::string> m_examples;
    };

} // namespace Commands

#endif // __ICOMMAND_H_INCL__