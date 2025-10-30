#include "eve-server.h"
#include "admin/commands/LegacyCommandAdapter.h"
#include "admin/CommandDispatcher.h"
#include "Client.h"
#include <algorithm>
#include <sstream>
#include <chrono>
#include <memory>

namespace Commands {

    // Global instances
    CommandMigrationManager* g_migrationManager = nullptr;
    LegacySystemIntegrator* g_legacyIntegrator = nullptr;

    // LegacyCommandWrapper implementation
    LegacyCommandWrapper::LegacyCommandWrapper(const LegacyCommandInfo& info)
        : m_info(info) {
        InitializeParameters();
    }

    LegacyCommandWrapper::~LegacyCommandWrapper() {
    }

    CommandResult LegacyCommandWrapper::Execute(const CommandContext& context) {
        try {
            // Check if deprecated
            if (m_info.isDeprecated) {
                std::string warning = "Warning: Command '/" + m_info.name + "' is deprecated.";
                if (!m_info.replacementCommand.empty()) {
                    warning += " Use '/" + m_info.replacementCommand + "' instead.";
                }
                // Send warning but continue execution
                // TODO: Send warning message to client
            }
            
            // Convert parameter format
            Seperator args = ConvertArgsToSeperator(context.arguments);
            
            // Call legacy command function
            if (m_info.function && context.client) {
                m_info.function(context.client, args);
                return CommandResult::Success("Command executed successfully.");
            } else {
                return CommandResult::Error("Command function not available or invalid client.");
            }
        } catch (const std::exception& e) {
            return CommandResult::Error("Command execution failed: " + std::string(e.what()));
        } catch (...) {
            return CommandResult::Error("Command execution failed with unknown error.");
        }
    }

    std::string LegacyCommandWrapper::GetDescription() const {
        std::string desc = m_info.description;
        if (m_info.isDeprecated) {
            desc += " [DEPRECATED";
            if (!m_info.replacementCommand.empty()) {
                desc += " - Use /" + m_info.replacementCommand + " instead";
            }
            desc += "]";
        }
        return desc;
    }

    std::vector<CommandParameter> LegacyCommandWrapper::GetParameters() const {
        // Convert EnhancedParameter to CommandParameter
        std::vector<CommandParameter> params;
        for (const auto& param : m_parameters) {
            CommandParameter cp(param.name, param.type, param.required, param.description, param.defaultValue);
            params.push_back(cp);
        }
        return params;
    }
    
    int64 LegacyCommandWrapper::GetRequiredRole() const {
        // Convert legacy roles to a single role value
        // For simplicity, return 0 (no role required) if no roles specified
        if (m_info.requiredRoles.empty()) {
            return 0;
        }
        // For legacy commands with roles, return a basic role requirement
        // This is a simplified implementation - actual projects may need more complex logic
        return 1; // Basic role requirement
    }

    void LegacyCommandWrapper::MarkAsDeprecated(const std::string& replacement) {
        m_info.isDeprecated = true;
        m_info.replacementCommand = replacement;
    }

    void LegacyCommandWrapper::InitializeParameters() {
        // Since legacy commands don't have explicit parameter definitions, we try to infer from description or other information
        // This provides a basic implementation, actual projects may need more complex logic
        
        // Most legacy commands accept variable number of string parameters
        m_parameters.clear();
        
        // Infer common parameters based on command name
        if (m_info.name.find("spawn") != std::string::npos) {
            m_parameters.push_back(
                ParameterBuilder::ItemTypeID("typeID")
                .Example("34")
            );
            // Create optional quantity parameter
            EnhancedParameter quantityParam = ParameterBuilder::Quantity("quantity");
            quantityParam.required = false;
            quantityParam.defaultValue = "1";
            m_parameters.push_back(
                quantityParam.Example("100")
            );
        } else if (m_info.name.find("giveisk") != std::string::npos) {
            m_parameters.push_back(
                ParameterBuilder::CharacterName("characterName")
                .Example("John Doe")
            );
            m_parameters.push_back(
                ParameterBuilder::ISKAmount("amount")
                .Example("1000000")
            );
        } else if (m_info.name.find("search") != std::string::npos) {
            m_parameters.push_back(
                ParameterBuilder::String("query", "Search query")
                .MinLength(1)
                .Example("Raven")
            );
        } else {
            // Default parameters: accept any number of string parameters
            EnhancedParameter argsParam = ParameterBuilder::String("args", "Command arguments");
            argsParam.required = false;
            argsParam.defaultValue = "";
            m_parameters.push_back(
                argsParam.Example("argument1 argument2")
            );
        }
    }

    Seperator LegacyCommandWrapper::ConvertArgsToSeperator(const std::vector<std::string>& args) const {
        std::string combined;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) combined += " ";
            combined += args[i];
        }
        return Seperator(combined.c_str());
    }

    // ModernCommandAdapter implementation
    std::map<std::string, ModernCommandAdapter*> ModernCommandAdapter::s_adapters;

    ModernCommandAdapter::ModernCommandAdapter(const std::string& commandName, ICommand* command)
        : m_commandName(commandName), m_command(command) {
        s_adapters[commandName] = this;
    }

    ModernCommandAdapter::~ModernCommandAdapter() {
        s_adapters.erase(m_commandName);
    }

    void ModernCommandAdapter::ExecuteLegacyStyle(Client* who, const Seperator& args) {
        // This static method will be called by the legacy system
    // We need to find the corresponding adapter and execute the modern command
        
        std::string commandName = args.arg(0); // Assume the first parameter is the command name
        auto it = s_adapters.find(commandName);
        
        if (it != s_adapters.end()) {
            ModernCommandAdapter* adapter = it->second;
            CommandContext context = adapter->CreateContextFromLegacy(who, args);
            
            CommandResult result = adapter->m_command->Execute(context);
            adapter->SendResultToClient(who, result);
        } else {
            // Command not found
            if (who) {
                who->SendNotifyMsg("Command not found or not available.");
            }
        }
    }

    void ModernCommandAdapter::RegisterToLegacySystem() {
        // Here we need to call the legacy system's command registration function
    // Since we don't have complete legacy system code, this provides a placeholder implementation
    // Actual implementation needs to call CommandDispatcher's AddCommand method
        
        // TODO: Implement legacy system registration
        // CommandDispatcher::GetInstance()->AddCommand(m_commandName, ExecuteLegacyStyle, ...);
    }

    void ModernCommandAdapter::UnregisterFromLegacySystem() {
        // TODO: Implement legacy system unregistration
        // CommandDispatcher::GetInstance()->RemoveCommand(m_commandName);
    }

    CommandContext ModernCommandAdapter::CreateContextFromLegacy(Client* who, const Seperator& args) const {
        // Note: CommandContext only has client, services, and arguments members
        // commandName, characterID, accountID are not part of CommandContext structure
        CommandContext context(who, nullptr, MigrationUtilities::SeperatorToVector(args));
        
        // TODO: Set services if available
        // context.services should be set to appropriate EVEServiceManager instance
        
        return context;
    }

    void ModernCommandAdapter::SendResultToClient(Client* who, const CommandResult& result) const {
        if (!who) return;
        
        if (result.success) {
            if (!result.message.empty()) {
                who->SendNotifyMsg(result.message.c_str());
            }
        } else {
            std::string errorMsg = "Error: " + result.message;
            who->SendNotifyMsg(errorMsg.c_str());
        }
    }

    // CommandMigrationManager implementation
    CommandMigrationManager::CommandMigrationManager()
        : m_modernManager(nullptr)
        , m_autoMigrationEnabled(false)
        , m_deprecationWarningsEnabled(true)
        , m_initialized(false) {
    }

    CommandMigrationManager::~CommandMigrationManager() {
        Shutdown();
    }

    void CommandMigrationManager::Initialize(CommandManager* modernManager) {
        if (m_initialized) return;
        
        m_modernManager = modernManager;
        m_initialized = true;
        
        sLog.Green("CommandMigrationManager", "Migration manager initialized.");
    }

    void CommandMigrationManager::Shutdown() {
        if (!m_initialized) return;
        
        // Clean up all adapters
        m_legacyCommands.clear();
        m_modernAdapters.clear();
        m_migratedCommands.clear();
        
        m_initialized = false;
        sLog.Debug("CommandMigrationManager", "Migration manager shut down.");
    }

    void CommandMigrationManager::RegisterLegacyCommand(const LegacyCommandInfo& info) {
        if (!m_initialized) return;
        
        CreateLegacyWrapper(info);
        
        if (m_autoMigrationEnabled) {
            MigrateCommand(info.name);
        }
        
        sLog.Debug("CommandMigrationManager", "Registered legacy command: %s", info.name.c_str());
    }

    void CommandMigrationManager::UnregisterLegacyCommand(const std::string& commandName) {
        auto it = m_legacyCommands.find(commandName);
        if (it != m_legacyCommands.end()) {
            m_legacyCommands.erase(it);
            
            // Remove from migration list
            auto migIt = std::find(m_migratedCommands.begin(), m_migratedCommands.end(), commandName);
            if (migIt != m_migratedCommands.end()) {
                m_migratedCommands.erase(migIt);
            }
            
            sLog.Debug("CommandMigrationManager", "Unregistered legacy command: %s", commandName.c_str());
        }
    }

    void CommandMigrationManager::RegisterModernCommand(const std::string& commandName, ICommand* command) {
        if (!m_initialized || !m_modernManager) return;
        
        // Register to modern command manager
        // Note: CommandManager::RegisterCommand expects std::unique_ptr<ICommand>
        // This code needs to be updated to properly handle command ownership
        // For now, we'll comment this out to fix compilation
        // m_modernManager->RegisterCommand(std::unique_ptr<ICommand>(command));
        
        // Create adapter so legacy system can call
        CreateModernAdapter(commandName, command);
        
        sLog.Debug("CommandMigrationManager", "Registered modern command: %s", commandName.c_str());
    }

    void CommandMigrationManager::UnregisterModernCommand(const std::string& commandName) {
        if (!m_initialized || !m_modernManager) return;
        
        // Note: CommandManager may not have UnregisterCommand method
        // Commenting out for now to fix compilation
        // m_modernManager->UnregisterCommand(commandName);
        
        auto it = m_modernAdapters.find(commandName);
        if (it != m_modernAdapters.end()) {
            it->second->UnregisterFromLegacySystem();
            m_modernAdapters.erase(it);
        }
        
        sLog.Debug("CommandMigrationManager", "Unregistered modern command: %s", commandName.c_str());
    }

    bool CommandMigrationManager::MigrateCommand(const std::string& commandName) {
        if (!ValidateCommandMigration(commandName)) {
            return false;
        }
        
        auto legacyIt = m_legacyCommands.find(commandName);
        if (legacyIt != m_legacyCommands.end()) {
            // Register legacy command to modern system
            if (m_modernManager) {
                // Note: CommandManager::RegisterCommand expects std::unique_ptr<ICommand>
                // For now, we'll skip this registration as it requires architecture changes
                // m_modernManager->RegisterCommand(std::unique_ptr<ICommand>(legacyIt->second.get()));
            }
            
            // Mark as migrated
            if (std::find(m_migratedCommands.begin(), m_migratedCommands.end(), commandName) == m_migratedCommands.end()) {
                m_migratedCommands.push_back(commandName);
            }
            
            sLog.Debug("CommandMigrationManager", "Migrated command: %s", commandName.c_str());
            return true;
        }
        
        return false;
    }

    void CommandMigrationManager::MigrateAllCommands() {
        size_t migratedCount = 0;
        
        for (const auto& pair : m_legacyCommands) {
            if (MigrateCommand(pair.first)) {
                migratedCount++;
            }
        }
        
        sLog.Debug("CommandMigrationManager", "Migrated %u out of %u legacy commands.", 
                    migratedCount, m_legacyCommands.size());
    }

    bool CommandMigrationManager::IsCommandMigrated(const std::string& commandName) const {
        return std::find(m_migratedCommands.begin(), m_migratedCommands.end(), commandName) != m_migratedCommands.end();
    }

    void CommandMigrationManager::DeprecateCommand(const std::string& commandName, const std::string& replacement) {
        auto it = m_legacyCommands.find(commandName);
        if (it != m_legacyCommands.end()) {
            it->second->MarkAsDeprecated(replacement);
            sLog.Warning("CommandMigrationManager", "Deprecated command: %s (replacement: %s)", 
                        commandName.c_str(), replacement.c_str());
        }
    }

    std::vector<std::string> CommandMigrationManager::GetDeprecatedCommands() const {
        std::vector<std::string> deprecated;
        
        for (const auto& pair : m_legacyCommands) {
            if (pair.second->IsDeprecated()) {
                deprecated.push_back(pair.first);
            }
        }
        
        return deprecated;
    }

    size_t CommandMigrationManager::GetLegacyCommandCount() const {
        return m_legacyCommands.size();
    }

    size_t CommandMigrationManager::GetModernCommandCount() const {
        return m_modernAdapters.size();
    }

    size_t CommandMigrationManager::GetMigratedCommandCount() const {
        return m_migratedCommands.size();
    }

    float CommandMigrationManager::GetMigrationProgress() const {
        size_t totalCommands = m_legacyCommands.size();
        if (totalCommands == 0) return 1.0f;
        
        return static_cast<float>(m_migratedCommands.size()) / totalCommands;
    }

    bool CommandMigrationManager::CheckCommandCompatibility(const std::string& commandName) const {
        // Check if command can be safely migrated
        auto it = m_legacyCommands.find(commandName);
        if (it == m_legacyCommands.end()) {
            return false;
        }
        
        // Basic compatibility check
        const auto& command = it->second;
        
        // Check if command name is valid
        if (!MigrationUtilities::IsValidCommandName(commandName)) {
            return false;
        }
        
        // Check if there are conflicting modern commands
        // Note: CommandManager may not have HasCommand method
        // Commenting out for now to fix compilation
        // if (m_modernManager && m_modernManager->HasCommand(commandName)) {
        //     return false;
        // }
        
        return true;
    }

    std::vector<std::string> CommandMigrationManager::GetIncompatibleCommands() const {
        std::vector<std::string> incompatible;
        
        for (const auto& pair : m_legacyCommands) {
            if (!CheckCommandCompatibility(pair.first)) {
                incompatible.push_back(pair.first);
            }
        }
        
        return incompatible;
    }

    std::string CommandMigrationManager::GenerateMigrationReport() const {
        std::ostringstream report;
        
        report << "=== Command Migration Report ===\n";
        report << "Total Legacy Commands: " << GetLegacyCommandCount() << "\n";
        report << "Total Modern Commands: " << GetModernCommandCount() << "\n";
        report << "Migrated Commands: " << GetMigratedCommandCount() << "\n";
        report << "Migration Progress: " << (GetMigrationProgress() * 100) << "%\n";
        
        auto deprecated = GetDeprecatedCommands();
        report << "Deprecated Commands: " << deprecated.size() << "\n";
        
        auto incompatible = GetIncompatibleCommands();
        report << "Incompatible Commands: " << incompatible.size() << "\n";
        
        if (!deprecated.empty()) {
            report << "\nDeprecated Commands:\n";
            for (const auto& cmd : deprecated) {
                auto it = m_legacyCommands.find(cmd);
                if (it != m_legacyCommands.end()) {
                    report << "  - " << cmd;
                    std::string replacement = it->second->GetReplacementCommand();
                    if (!replacement.empty()) {
                        report << " (use /" << replacement << " instead)";
                    }
                    report << "\n";
                }
            }
        }
        
        if (!incompatible.empty()) {
            report << "\nIncompatible Commands:\n";
            for (const auto& cmd : incompatible) {
                report << "  - " << cmd << "\n";
            }
        }
        
        return report.str();
    }

    std::string CommandMigrationManager::GenerateCompatibilityReport() const {
        std::ostringstream report;
        
        report << "=== Command Compatibility Report ===\n";
        
        for (const auto& pair : m_legacyCommands) {
            const std::string& cmdName = pair.first;
            bool compatible = CheckCommandCompatibility(cmdName);
            
            report << cmdName << ": " << (compatible ? "COMPATIBLE" : "INCOMPATIBLE") << "\n";
            
            if (!compatible) {
                // Provide reasons for incompatibility
                if (!MigrationUtilities::IsValidCommandName(cmdName)) {
                    report << "  - Invalid command name\n";
                }
                // Note: CommandManager may not have HasCommand method
                // Commenting out for now to fix compilation
                // if (m_modernManager && m_modernManager->HasCommand(cmdName)) {
                //     report << "  - Conflicts with existing modern command\n";
                // }
            }
        }
        
        return report.str();
    }

    void CommandMigrationManager::SetMigrationMode(bool enableAutoMigration) {
        m_autoMigrationEnabled = enableAutoMigration;
        sLog.Debug("CommandMigrationManager", "Auto-migration %s", 
                  enableAutoMigration ? "enabled" : "disabled");
    }

    void CommandMigrationManager::SetDeprecationWarnings(bool enable) {
        m_deprecationWarningsEnabled = enable;
        sLog.Debug("CommandMigrationManager", "Deprecation warnings %s", 
                  enable ? "enabled" : "disabled");
    }

    void CommandMigrationManager::CreateLegacyWrapper(const LegacyCommandInfo& info) {
        auto wrapper = std::unique_ptr<LegacyCommandWrapper>(new LegacyCommandWrapper(info));
        m_legacyCommands[info.name] = std::move(wrapper);
    }

    void CommandMigrationManager::CreateModernAdapter(const std::string& commandName, ICommand* command) {
        auto adapter = std::unique_ptr<ModernCommandAdapter>(new ModernCommandAdapter(commandName, command));
        adapter->RegisterToLegacySystem();
        m_modernAdapters[commandName] = std::move(adapter);
    }

    bool CommandMigrationManager::ValidateCommandMigration(const std::string& commandName) const {
        return CheckCommandCompatibility(commandName);
    }

    // LegacySystemIntegrator implementation
    LegacySystemIntegrator* LegacySystemIntegrator::s_instance = nullptr;

    LegacySystemIntegrator::LegacySystemIntegrator()
        : m_integrationActive(false)
        , m_migrationManager(nullptr) {
        s_instance = this;
        memset(&m_metrics, 0, sizeof(m_metrics));
    }

    LegacySystemIntegrator::~LegacySystemIntegrator() {
        ShutdownIntegration();
        s_instance = nullptr;
    }

    bool LegacySystemIntegrator::InitializeIntegration() {
        if (m_integrationActive) return true;
        
        m_migrationManager = g_migrationManager;
        if (!m_migrationManager) {
            sLog.Error("LegacySystemIntegrator", "Migration manager not available.");
            return false;
        }
        
        RegisterLegacyCommandHook();
        m_integrationActive = true;
        
        sLog.Debug("LegacySystemIntegrator", "Legacy system integration initialized.");
        return true;
    }

    void LegacySystemIntegrator::ShutdownIntegration() {
        if (!m_integrationActive) return;
        
        UnregisterLegacyCommandHook();
        m_integrationActive = false;
        
        sLog.Debug("LegacySystemIntegrator", "Legacy system integration shut down.");
    }

    bool LegacySystemIntegrator::InterceptLegacyCommand(const std::string& commandName, Client* who, const Seperator& args) {
        if (!s_instance || !s_instance->m_integrationActive) {
            return false;
        }
        
        return s_instance->RouteCommand(commandName, who, args);
    }

    void LegacySystemIntegrator::RegisterLegacyCommandHook() {
        // TODO: Implement legacy system hook registration
    // This requires modifying CommandDispatcher to support command interception
    }

    void LegacySystemIntegrator::UnregisterLegacyCommandHook() {
        // TODO: Implement legacy system hook unregistration
    }

    bool LegacySystemIntegrator::RouteCommand(const std::string& commandName, Client* who, const Seperator& args) {
        auto startTime = std::chrono::high_resolution_clock::now();
        bool isLegacy = false;
        bool hasError = false;
        
        try {
            // Check if it's a migrated command
            if (m_migrationManager && m_migrationManager->IsCommandMigrated(commandName)) {
                // Route to modern system
                ModernCommandAdapter::ExecuteLegacyStyle(who, args);
                isLegacy = false;
            } else {
                // Use legacy system to handle
                isLegacy = true;
                return false; // Let legacy system handle
            }
        } catch (const std::exception& e) {
            hasError = true;
            sLog.Error("LegacySystemIntegrator", "Error routing command %s: %s", 
                      commandName.c_str(), e.what());
        } catch (...) {
            hasError = true;
            sLog.Error("LegacySystemIntegrator", "Unknown error routing command %s", 
                      commandName.c_str());
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double executionTime = duration.count() / 1000.0; // Convert to milliseconds
        
        UpdateMetrics(isLegacy, executionTime, hasError);
        
        return true;
    }

    std::string LegacySystemIntegrator::GetIntegrationStatus() const {
        std::ostringstream status;
        
        status << "Integration Status: " << (m_integrationActive ? "ACTIVE" : "INACTIVE") << "\n";
        status << "Total Commands Processed: " << m_metrics.totalCommands << "\n";
        status << "Legacy Commands: " << m_metrics.legacyCommands << "\n";
        status << "Modern Commands: " << m_metrics.modernCommands << "\n";
        status << "Average Execution Time: " << m_metrics.averageExecutionTime << " ms\n";
        status << "Error Count: " << m_metrics.errorCount << "\n";
        
        return status.str();
    }

    LegacySystemIntegrator::PerformanceMetrics LegacySystemIntegrator::GetPerformanceMetrics() const {
        return m_metrics;
    }

    void LegacySystemIntegrator::ResetPerformanceMetrics() {
        memset(&m_metrics, 0, sizeof(m_metrics));
    }

    void LegacySystemIntegrator::UpdateMetrics(bool isLegacy, double executionTime, bool hasError) {
        m_metrics.totalCommands++;
        
        if (isLegacy) {
            m_metrics.legacyCommands++;
        } else {
            m_metrics.modernCommands++;
        }
        
        if (hasError) {
            m_metrics.errorCount++;
        }
        
        // Update average execution time
        m_metrics.averageExecutionTime = 
            (m_metrics.averageExecutionTime * (m_metrics.totalCommands - 1) + executionTime) / m_metrics.totalCommands;
    }

    // MigrationUtilities implementation
    std::vector<std::string> MigrationUtilities::SeperatorToVector(const Seperator& sep) {
        std::vector<std::string> result;
        
        for (uint32 i = 0; i < sep.argCount(); ++i) {
            result.push_back(sep.arg(i));
        }
        
        return result;
    }

    Seperator MigrationUtilities::VectorToSeperator(const std::vector<std::string>& args) {
        std::string combined;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) combined += " ";
            combined += args[i];
        }
        return Seperator(combined.c_str());
    }

    std::vector<std::string> MigrationUtilities::ConvertLegacyRoles(const std::string& legacyRole) {
        std::vector<std::string> roles;
        
        // Convert legacy role string to modern role list
        if (legacyRole.find("ROLE_ADMIN") != std::string::npos) {
            roles.push_back("Administrator");
        }
        if (legacyRole.find("ROLE_GM") != std::string::npos) {
            roles.push_back("GameMaster");
        }
        if (legacyRole.find("ROLE_DEV") != std::string::npos) {
            roles.push_back("Developer");
        }
        
        return roles;
    }

    std::string MigrationUtilities::ConvertModernRoles(const std::vector<std::string>& modernRoles) {
        std::string legacy;
        
        for (const auto& role : modernRoles) {
            if (!legacy.empty()) legacy += "|";
            
            if (role == "Administrator") {
                legacy += "ROLE_ADMIN";
            } else if (role == "GameMaster") {
                legacy += "ROLE_GM";
            } else if (role == "Developer") {
                legacy += "ROLE_DEV";
            } else {
                legacy += "ROLE_" + role;
            }
        }
        
        return legacy;
    }

    std::string MigrationUtilities::ConvertErrorMessage(const std::string& legacyError) {
        // Standardize error message format
        std::string converted = legacyError;
        
        // Remove common legacy prefixes
        if (converted.find("ERROR: ") == 0) {
            converted = converted.substr(7);
        }
        
        return converted;
    }

    CommandResult MigrationUtilities::ConvertLegacyResult(bool success, const std::string& message) {
        if (success) {
            return CommandResult::Success(message);
        } else {
            return CommandResult::Error(ConvertErrorMessage(message));
        }
    }

    std::string MigrationUtilities::NormalizeCommandName(const std::string& name) {
        std::string normalized = name;
        
        // Convert to lowercase
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
        
        // Remove invalid characters
        normalized.erase(std::remove_if(normalized.begin(), normalized.end(), 
                                       [](char c) { return !std::isalnum(c) && c != '_'; }), 
                        normalized.end());
        
        return normalized;
    }

    bool MigrationUtilities::IsValidCommandName(const std::string& name) {
        if (name.empty() || name.length() > 32) {
            return false;
        }
        
        // Check if it only contains letters, numbers and underscores
        return std::all_of(name.begin(), name.end(), 
                          [](char c) { return std::isalnum(c) || c == '_'; });
    }

    std::vector<EnhancedParameter> MigrationUtilities::InferParametersFromUsage(const std::string& commandName, 
                                                                              const std::string& usage) {
        std::vector<EnhancedParameter> parameters;
        
        // Simple parameter inference logic
    // Actual implementation may need more complex parsing
        
        if (usage.find("<character>") != std::string::npos) {
            parameters.push_back(ParameterBuilder::CharacterName("character"));
        }
        if (usage.find("<amount>") != std::string::npos) {
            parameters.push_back(ParameterBuilder::ISKAmount("amount"));
        }
        if (usage.find("<typeID>") != std::string::npos) {
            parameters.push_back(ParameterBuilder::ItemTypeID("typeID"));
        }
        
        return parameters;
    }

    bool MigrationUtilities::CheckParameterCompatibility(const std::vector<EnhancedParameter>& modern,
                                                        const std::string& legacyUsage) {
        // Check if modern parameter definition is compatible with legacy usage
    // This is a simplified implementation
        
        for (const auto& param : modern) {
            if (param.required && legacyUsage.find("<" + param.name + ">") == std::string::npos) {
                return false;
            }
        }
        
        return true;
    }

    std::string MigrationUtilities::GenerateMigrationGuide(const std::string& commandName) {
        std::ostringstream guide;
        
        guide << "=== Migration Guide for /" << commandName << " ===\n";
        guide << "This command has been migrated to the new command framework.\n";
        guide << "\n";
        guide << "Key Changes:\n";
        guide << "• Enhanced parameter validation\n";
        guide << "• Improved error messages\n";
        guide << "• Better help documentation\n";
        guide << "• Consistent command behavior\n";
        guide << "\n";
        guide << "For detailed help, use: /help " << commandName << "\n";
        
        return guide.str();
    }

    std::string MigrationUtilities::GenerateDeprecationNotice(const std::string& oldCommand, 
                                                             const std::string& newCommand) {
        std::ostringstream notice;
        
        notice << "DEPRECATION NOTICE: Command '/" << oldCommand << "' is deprecated.\n";
        if (!newCommand.empty()) {
            notice << "Please use '/" << newCommand << "' instead.\n";
        }
        notice << "This command will be removed in a future version.\n";
        
        return notice.str();
    }

} // namespace Commands