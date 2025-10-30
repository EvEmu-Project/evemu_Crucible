#include "eve-server.h"
#include "admin/commands/AllCommandRegistrars.h"
#include "Client.h"
#include "services/ServiceManager.h"
#include "EVE_Roles.h"
#include "python/PyRep.h"

using namespace Acct::Role;

namespace Commands {

    // Main command registrar implementation
    void MasterCommandRegistrar::RegisterAllCommands(CommandManager& manager) {
        sLog.Blue("MasterCommandRegistrar", "Registering all command categories...");
        
        RegisterGMCommands(manager);
        RegisterSystemCommands(manager);
        RegisterDebugCommands(manager);
        RegisterUtilityCommands(manager);
        
        sLog.Green("MasterCommandRegistrar", "All command categories registered successfully");
    }

    void MasterCommandRegistrar::RegisterGMCommands(CommandManager& manager) {
        auto registrar = std::make_unique<GMCommandRegistrar>();
        manager.RegisterCommandRegistrar(std::move(registrar));
        sLog.Cyan("MasterCommandRegistrar", "GM commands registered");
    }

    void MasterCommandRegistrar::RegisterSystemCommands(CommandManager& manager) {
        auto registrar = std::make_unique<SystemCommandRegistrar>();
        manager.RegisterCommandRegistrar(std::move(registrar));
        sLog.Cyan("MasterCommandRegistrar", "System commands registered");
    }

    void MasterCommandRegistrar::RegisterDebugCommands(CommandManager& manager) {
        auto registrar = std::make_unique<DebugCommandRegistrar>();
        manager.RegisterCommandRegistrar(std::move(registrar));
        sLog.Cyan("MasterCommandRegistrar", "Debug commands registered");
    }

    void MasterCommandRegistrar::RegisterUtilityCommands(CommandManager& manager) {
        auto registrar = std::make_unique<UtilityCommandRegistrar>();
        manager.RegisterCommandRegistrar(std::move(registrar));
        sLog.Cyan("MasterCommandRegistrar", "Utility commands registered");
    }

    // Utility command registrar implementation
    void UtilityCommandRegistrar::DoRegisterCommands(CommandManager& manager) {
        REGISTER_COMMAND(manager, HelpCommand);
        REGISTER_COMMAND(manager, ListCommand);
        REGISTER_COMMAND(manager, PermissionCommand);
        REGISTER_COMMAND(manager, VersionCommand);
        REGISTER_COMMAND(manager, TimeCommand);
        REGISTER_COMMAND(manager, StatsCommand);
        REGISTER_COMMAND(manager, ClearCommand);
        REGISTER_COMMAND(manager, EchoCommand);
        REGISTER_COMMAND(manager, TestCommand);
    }

    // Help command implementation
    HelpCommand::HelpCommand() {
        m_name = "help";
        m_description = "Display help information for commands";
        m_category = "Utility";
        m_requiredRole = Acct::Role::PLAYER;
        m_parameters = {
            {"command", ParameterType::String, false, "Specific command to get help for"},
            {"category", ParameterType::String, false, "Command category to list"}
        };
        m_usage = "/help [command|category]";
        m_examples = {
            "/help",
            "/help spawn",
            "/help GM"
        };
    }

    CommandResult HelpCommand::DoExecute(const CommandContext& context) {
        CommandManager* manager = reinterpret_cast<CommandManager*>(context.services);
        
        if (context.arguments.empty()) {
            return CommandResult::Success(GetAllCommandsHelp(*manager));
        }
        
        std::string target = context.arguments[0];
        
        // Check if it's a specific command
        if (manager->FindCommand(target)) {
            return CommandResult::Success(GetCommandHelp(target, *manager));
        }
        
        // Check if it's a command category
        auto categories = manager->GetCommandsByCategory(context.client);
        if (categories.find(target) != categories.end()) {
            return CommandResult::Success(GetCategoryHelp(target, *manager));
        }
        
        return CommandResult::Error("Unknown command or category: " + target);
    }

    std::string HelpCommand::GetCommandHelp(const std::string& commandName, CommandManager& manager) const {
        return manager.GetCommandHelp(commandName);
    }

    std::string HelpCommand::GetCategoryHelp(const std::string& category, CommandManager& manager) const {
        auto categories = manager.GetCommandsByCategory();
        auto it = categories.find(category);
        
        if (it == categories.end()) {
            return "Category not found: " + category;
        }
        
        std::string help = "=== " + category + " Commands ===\n";
        for (const auto& command : it->second) {
            ICommand* cmd = manager.FindCommand(command);
            if (cmd) {
                help += "  " + command + " - " + cmd->GetDescription() + "\n";
            }
        }
        
        return help;
    }

    std::string HelpCommand::GetAllCommandsHelp(CommandManager& manager) const {
        std::string help = "=== Available Command Categories ===\n";
        auto categories = manager.GetCommandsByCategory();
        
        for (const auto& pair : categories) {
            help += "  " + pair.first + " (" + std::to_string(pair.second.size()) + " commands)\n";
        }
        
        help += "\nUse '/help <category>' to list commands in a category\n";
        help += "Use '/help <command>' to get detailed help for a specific command";
        
        return help;
    }

    // List command implementation
    ListCommand::ListCommand() {
        m_name = "list";
        m_description = "List available commands";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {
            {"category", ParameterType::String, false, "Filter by category"}
        };
        m_usage = "/list [category]";
        m_examples = {
            "/list",
            "/list GM",
            "/list System"
        };
    }

    CommandResult ListCommand::DoExecute(const CommandContext& context) {
        CommandManager* manager = reinterpret_cast<CommandManager*>(context.services);
        
        if (context.arguments.empty()) {
            auto categories = manager->GetCommandsByCategory(context.client);
            return CommandResult::Success(FormatCategoryList(categories));
        }
        
        std::string category = context.arguments[0];
        auto categories = manager->GetCommandsByCategory(context.client);
        auto it = categories.find(category);
        
        if (it == categories.end()) {
            return CommandResult::Error("Category not found: " + category);
        }
        
        return CommandResult::Success(FormatCommandList(it->second));
    }

    std::string ListCommand::FormatCommandList(const std::vector<std::string>& commands) const {
        if (commands.empty()) {
            return "No commands available";
        }
        
        std::string result = "Available commands: ";
        for (size_t i = 0; i < commands.size(); ++i) {
            if (i > 0) result += ", ";
            result += commands[i];
        }
        
        return result;
    }

    std::string ListCommand::FormatCategoryList(const std::map<std::string, std::vector<std::string>>& categories) const {
        std::string result = "=== Command Categories ===\n";
        
        for (const auto& pair : categories) {
            result += pair.first + ": ";
            for (size_t i = 0; i < pair.second.size(); ++i) {
                if (i > 0) result += ", ";
                result += pair.second[i];
            }
            result += "\n";
        }
        
        return result;
    }

    // Permission check command implementation
    PermissionCommand::PermissionCommand() {
        m_name = "permissions";
        m_description = "Check user permissions and command access";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {
            {"command", ParameterType::String, false, "Check permissions for specific command"}
        };
        m_usage = "/permissions [command]";
        m_examples = {
            "/permissions",
            "/permissions spawn"
        };
    }

    CommandResult PermissionCommand::DoExecute(const CommandContext& context) {
        if (context.arguments.empty()) {
            return CommandResult::Success(GetUserPermissions(context.client));
        }
        
        std::string commandName = context.arguments[0];
        CommandManager* manager = reinterpret_cast<CommandManager*>(context.services);
        
        return CommandResult::Success(GetCommandPermissions(commandName, *manager));
    }

    std::string PermissionCommand::GetUserPermissions(Client* client) const {
        if (!client) {
            return "Error: Invalid client";
        }
        
        std::string result = "=== Your Permissions ===\n";
        result += "Character: " + std::string(client->GetName()) + "\n";
        result += "Role Flags: " + FormatRoleFlags(client->GetAccountRole()) + "\n";
        
        return result;
    }

    std::string PermissionCommand::GetCommandPermissions(const std::string& commandName, CommandManager& manager) const {
        ICommand* command = manager.FindCommand(commandName);
        if (!command) {
            return "Command not found: " + commandName;
        }
        
        std::string result = "=== Command Permissions ===\n";
        result += "Command: " + commandName + "\n";
        result += "Required Role: " + FormatRoleFlags(command->GetRequiredRole()) + "\n";
        result += "Category: " + command->GetCategory() + "\n";
        
        return result;
    }

    std::string PermissionCommand::FormatRoleFlags(int64 roleFlags) const {
        std::vector<std::string> roles;
        
        if (roleFlags & Acct::Role::PLAYER) roles.push_back("PLAYER");
        if (roleFlags & Acct::Role::GMH) roles.push_back("GMH");
        if (roleFlags & Acct::Role::GML) roles.push_back("GML");
        if (roleFlags & Acct::Role::ADMIN) roles.push_back("ADMIN");
        if (roleFlags & Acct::Role::PROGRAMMER) roles.push_back("PROGRAMMER");
        
        if (roles.empty()) {
            return "None (" + std::to_string(roleFlags) + ")";
        }
        
        std::string result;
        for (size_t i = 0; i < roles.size(); ++i) {
            if (i > 0) result += ", ";
            result += roles[i];
        }
        
        return result + " (" + std::to_string(roleFlags) + ")";
    }

    // Simple implementation of other utility commands
    VersionCommand::VersionCommand() {
        m_name = "version";
        m_description = "Display server version information";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {};
        m_usage = "/version";
        m_examples = {"/version"};
    }

    CommandResult VersionCommand::DoExecute(const CommandContext& context) {
        std::string info = "=== Server Version ===\n";
        info += "EVEmu Server: " + GetServerVersion() + "\n";
        info += "Build: " + GetBuildInfo() + "\n";
        info += "Compiler: " + GetCompilerInfo() + "\n";
        info += "Dependencies: " + GetDependencyVersions();
        
        return CommandResult::Success(info);
    }

    std::string VersionCommand::GetServerVersion() const {
        return "Development Build";
    }

    std::string VersionCommand::GetBuildInfo() const {
        return __DATE__ " " __TIME__;
    }

    std::string VersionCommand::GetCompilerInfo() const {
#ifdef _MSC_VER
        return "MSVC " + std::to_string(_MSC_VER);
#elif defined(__GNUC__)
        return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#else
        return "Unknown";
#endif
    }

    std::string VersionCommand::GetDependencyVersions() const {
        return "MySQL, Python, etc.";
    }

    // Time command implementation
    TimeCommand::TimeCommand() {
        m_name = "time";
        m_description = "Display server time information";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {};
        m_usage = "/time";
        m_examples = {"/time"};
    }

    CommandResult TimeCommand::DoExecute(const CommandContext& context) {
        std::string info = "=== Time Information ===\n";
        info += "Server Time: " + GetServerTime() + "\n";
        info += "Uptime: " + GetUptime() + "\n";
        info += "EVE Time: " + GetEVETime();
        
        return CommandResult::Success(info);
    }

    std::string TimeCommand::GetServerTime() const {
        time_t now = time(0);
        return ctime(&now);
    }

    std::string TimeCommand::GetUptime() const {
        return "Unknown";
    }

    std::string TimeCommand::GetEVETime() const {
        return "YC 125";
    }

    // Placeholder implementation of other commands
    StatsCommand::StatsCommand() {
        m_name = "stats";
        m_description = "Display server statistics";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {};
        m_usage = "/stats";
        m_examples = {"/stats"};
    }

    CommandResult StatsCommand::DoExecute(const CommandContext& context) {
        return CommandResult::Error("Stats command not yet implemented");
    }

    ClearCommand::ClearCommand() {
        m_name = "clear";
        m_description = "Clear the chat window";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {};
        m_usage = "/clear";
        m_examples = {"/clear"};
    }

    CommandResult ClearCommand::DoExecute(const CommandContext& context) {
        return CommandResult::Success("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    }

    EchoCommand::EchoCommand() {
        m_name = "echo";
        m_description = "Echo back the provided text";
        m_category = "Utility";
        m_requiredRole = PLAYER;
        m_parameters = {
            {"text", ParameterType::String, true, "Text to echo back"}
        };
        m_usage = "/echo <text>";
        m_examples = {"/echo Hello World"};
    }

    CommandResult EchoCommand::DoExecute(const CommandContext& context) {
        if (context.arguments.empty()) {
            return CommandResult::Error("Usage: " + m_usage);
        }
        
        std::string message;
        for (size_t i = 0; i < context.arguments.size(); ++i) {
            if (i > 0) message += " ";
            message += context.arguments[i];
        }
        
        return CommandResult::Success("Echo: " + message);
    }

    TestCommand::TestCommand() {
        m_name = "test";
        m_description = "Run system tests";
        m_category = "Utility";
        m_requiredRole = ADMIN;
        m_parameters = {
            {"type", ParameterType::String, false, "Test type: connection, database, memory, performance"}
        };
        m_usage = "/test [type]";
        m_examples = {"/test", "/test database"};
    }

    CommandResult TestCommand::DoExecute(const CommandContext& context) {
        return CommandResult::Error("Test command not yet implemented");
    }

    // Placeholder method implementations
    std::string StatsCommand::GetPlayerStats() const { return "Unknown"; }
    std::string StatsCommand::GetSystemStats() const { return "Unknown"; }
    std::string StatsCommand::GetPerformanceStats() const { return "Unknown"; }
    std::string StatsCommand::GetCommandUsageStats() const { return "Unknown"; }
    
    void TestCommand::RunConnectionTest() const {}
    void TestCommand::RunDatabaseTest() const {}
    void TestCommand::RunMemoryTest() const {}
    void TestCommand::RunPerformanceTest() const {}

} // namespace Commands