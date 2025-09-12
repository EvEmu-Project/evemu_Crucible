#include "eve-server.h"
#include "admin/commands/ICommand.h"
#include "Client.h"
#include "EVEServerConfig.h"
#include "services/ServiceManager.h"

namespace Commands {

    bool ICommand::ValidateParameters(const std::vector<std::string>& args, std::string& errorMsg) const {
        auto params = GetParameters();
        
        // Check required parameter count
        size_t requiredCount = 0;
        for (const auto& param : params) {
            if (param.required) {
                requiredCount++;
            }
        }
        
        if (args.size() < requiredCount) {
            errorMsg = "Not enough arguments. Required: " + std::to_string(requiredCount) + ", provided: " + std::to_string(args.size());
            return false;
        }
        
        if (args.size() > params.size()) {
            errorMsg = "Too many arguments. Maximum: " + std::to_string(params.size()) + ", provided: " + std::to_string(args.size());
            return false;
        }
        
        // Validate parameter types
        for (size_t i = 0; i < args.size() && i < params.size(); ++i) {
            const auto& param = params[i];
            const auto& arg = args[i];
            
            if (arg.empty() && param.required) {
                errorMsg = "Parameter '" + param.name + "' is required but empty";
                return false;
            }
            
            if (!arg.empty()) {
                switch (param.type) {
                    case ParameterType::Integer: {
                        int32 value;
                        if (!ParseInt(arg, value)) {
                            errorMsg = "Parameter '" + param.name + "' must be an integer";
                            return false;
                        }
                        break;
                    }
                    case ParameterType::Float: {
                        float value;
                        if (!ParseFloat(arg, value)) {
                            errorMsg = "Parameter '" + param.name + "' must be a number";
                            return false;
                        }
                        break;
                    }
                    case ParameterType::Boolean: {
                        bool value;
                        if (!ParseBool(arg, value)) {
                            errorMsg = "Parameter '" + param.name + "' must be true/false or 1/0";
                            return false;
                        }
                        break;
                    }
                    case ParameterType::EntityID:
                    case ParameterType::ItemID:
                    case ParameterType::TypeID: {
                        if (arg != "me" && arg != "self") {
                            uint32 id = ParseEntityID(arg);
                            if (id == 0) {
                                errorMsg = "Parameter '" + param.name + "' must be a valid ID or 'me'";
                                return false;
                            }
                        }
                        break;
                    }
                    case ParameterType::String:
                        // String type requires no special validation
                        break;
                }
            }
        }
        
        return true;
    }

    std::string ICommand::GenerateHelp() const {
        std::string help = "Command: /" + GetName() + "\n";
        help += "Category: " + GetCategory() + "\n";
        help += "Description: " + GetDescription() + "\n";
        help += "Required Role: " + std::to_string(GetRequiredRole()) + "\n\n";
        
        auto params = GetParameters();
        if (!params.empty()) {
            help += "Parameters:\n";
            for (const auto& param : params) {
                help += "  " + param.name;
                if (!param.required) {
                    help += " (optional)";
                }
                help += ": " + param.description;
                if (!param.defaultValue.empty()) {
                    help += " [default: " + param.defaultValue + "]";
                }
                help += "\n";
            }
            help += "\n";
        }
        
        auto examples = GetExamples();
        if (!examples.empty()) {
            help += "Examples:\n";
            for (const auto& example : examples) {
                help += "  /" + GetName() + " " + example + "\n";
            }
        }
        
        return help;
    }

    bool ICommand::ParseInt(const std::string& str, int32& value) const {
        try {
            value = std::stoi(str);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool ICommand::ParseFloat(const std::string& str, float& value) const {
        try {
            value = std::stof(str);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool ICommand::ParseBool(const std::string& str, bool& value) const {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
            value = true;
            return true;
        } else if (lower == "false" || lower == "0" || lower == "no" || lower == "off") {
            value = false;
            return true;
        }
        
        return false;
    }

    uint32 ICommand::ParseEntityID(const std::string& str) const {
        try {
            uint32 id = std::stoul(str);
            return id;
        } catch (...) {
            return 0;
        }
    }

    bool ICommand::HasPermission(Client* client, int64 requiredRole) const {
        if (!client) {
            return false;
        }
        
        return (client->GetAccountRole() & requiredRole) == requiredRole;
    }

    Client* ICommand::GetTargetClient(const std::string& target, Client* executor) const {
        if (target == "me" || target == "self" || target.empty()) {
            return executor;
        }
        
        // Try to find client by name
        // Need to access client manager here, return nullptr for now
        // TODO: Implement client lookup logic
        return nullptr;
    }

} // namespace Commands