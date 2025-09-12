#ifndef __ALLCOMMANDREGISTRARS_H_INCL__
#define __ALLCOMMANDREGISTRARS_H_INCL__

#include "admin/commands/CommandManager.h"
#include "admin/commands/GMCommands.h"
#include "admin/commands/SystemCommands.h"
#include "admin/commands/DebugCommands.h"

namespace Commands {

    // Master command registrar - Responsible for registering all command categories
    class MasterCommandRegistrar {
    public:
        static void RegisterAllCommands(CommandManager& manager);
        
    private:
        static void RegisterGMCommands(CommandManager& manager);
        static void RegisterSystemCommands(CommandManager& manager);
        static void RegisterDebugCommands(CommandManager& manager);
        static void RegisterUtilityCommands(CommandManager& manager);
    };

    // Utility command registrar
    class UtilityCommandRegistrar : public CommandRegistrarBase<UtilityCommandRegistrar> {
    public:
        void DoRegisterCommands(CommandManager& manager);
    };

    // Help command
    class HelpCommand : public CommandBase<HelpCommand> {
    public:
        HelpCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetCommandHelp(const std::string& commandName, CommandManager& manager) const;
        std::string GetCategoryHelp(const std::string& category, CommandManager& manager) const;
        std::string GetAllCommandsHelp(CommandManager& manager) const;
    };

    // List command
    class ListCommand : public CommandBase<ListCommand> {
    public:
        ListCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string FormatCommandList(const std::vector<std::string>& commands) const;
        std::string FormatCategoryList(const std::map<std::string, std::vector<std::string>>& categories) const;
    };

    // Permission check command
    class PermissionCommand : public CommandBase<PermissionCommand> {
    public:
        PermissionCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetUserPermissions(Client* client) const;
        std::string GetCommandPermissions(const std::string& commandName, CommandManager& manager) const;
        std::string FormatRoleFlags(int64 roleFlags) const;
    };

    // Version info command
    class VersionCommand : public CommandBase<VersionCommand> {
    public:
        VersionCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetServerVersion() const;
        std::string GetBuildInfo() const;
        std::string GetCompilerInfo() const;
        std::string GetDependencyVersions() const;
    };

    // Time command
    class TimeCommand : public CommandBase<TimeCommand> {
    public:
        TimeCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetServerTime() const;
        std::string GetUptime() const;
        std::string GetEVETime() const;
    };

    // Statistics command
    class StatsCommand : public CommandBase<StatsCommand> {
    public:
        StatsCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetPlayerStats() const;
        std::string GetSystemStats() const;
        std::string GetPerformanceStats() const;
        std::string GetCommandUsageStats() const;
    };

    // Clear command
    class ClearCommand : public CommandBase<ClearCommand> {
    public:
        ClearCommand();
        CommandResult DoExecute(const CommandContext& context) override;
    };

    // Echo command
    class EchoCommand : public CommandBase<EchoCommand> {
    public:
        EchoCommand();
        CommandResult DoExecute(const CommandContext& context) override;
    };

    // Test command
    class TestCommand : public CommandBase<TestCommand> {
    public:
        TestCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void RunConnectionTest() const;
        void RunDatabaseTest() const;
        void RunMemoryTest() const;
        void RunPerformanceTest() const;
    };

} // namespace Commands

#endif // __ALLCOMMANDREGISTRARS_H_INCL__