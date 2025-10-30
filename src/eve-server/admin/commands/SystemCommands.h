#ifndef __SYSTEMCOMMANDS_H_INCL__
#define __SYSTEMCOMMANDS_H_INCL__

#include "admin/commands/ICommand.h"
#include "admin/commands/CommandManager.h"

namespace Commands {

    // System command registrar
    class SystemCommandRegistrar : public CommandRegistrarBase<SystemCommandRegistrar> {
    public:
        void DoRegisterCommands(CommandManager& manager);
    };

    // Server status command
    class StatusCommand : public CommandBase<StatusCommand> {
    public:
        StatusCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetDatabaseStatus() const;
        std::string GetServiceStatus() const;
        std::string GetNetworkStatus() const;
    };

    // Server shutdown command
    class ShutdownCommand : public CommandBase<ShutdownCommand> {
    public:
        ShutdownCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void InitiateShutdown(uint32 delaySeconds, const std::string& reason) const;
        bool ValidateDelay(uint32 seconds) const;
    };

    // Server restart command
    class RestartCommand : public CommandBase<RestartCommand> {
    public:
        RestartCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void InitiateRestart(uint32 delaySeconds, const std::string& reason) const;
        bool ValidateDelay(uint32 seconds) const;
    };

    // Save data command
    class SaveCommand : public CommandBase<SaveCommand> {
    public:
        SaveCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool SaveCharacterData() const;
        bool SaveCorporationData() const;
        bool SaveMarketData() const;
        bool SaveSystemData() const;
    };

    // Garbage collection command
    class GarbageCollectCommand : public CommandBase<GarbageCollectCommand> {
    public:
        GarbageCollectCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void CleanupExpiredSessions() const;
        void CleanupOrphanedItems() const;
        void CleanupOldLogs() const;
        std::string GetMemoryStats() const;
    };

    // Database query command
    class DBQueryCommand : public CommandBase<DBQueryCommand> {
    public:
        DBQueryCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool ValidateQuery(const std::string& query) const;
        bool IsSafeQuery(const std::string& query) const;
        PyResult* ExecuteQuery(const std::string& query) const;
    };

    // Configuration management command
    class ConfigCommand : public CommandBase<ConfigCommand> {
    public:
        ConfigCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetConfigValue(const std::string& key) const;
        bool SetConfigValue(const std::string& key, const std::string& value) const;
        std::vector<std::string> ListConfigKeys() const;
        bool ValidateConfigKey(const std::string& key) const;
    };

    // Log level command
    class LogLevelCommand : public CommandBase<LogLevelCommand> {
    public:
        LogLevelCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool SetLogLevel(const std::string& category, const std::string& level) const;
        std::string GetLogLevel(const std::string& category) const;
        std::vector<std::string> GetLogCategories() const;
        bool ValidateLogLevel(const std::string& level) const;
    };

    // Performance monitoring command
    class PerfMonCommand : public CommandBase<PerfMonCommand> {
    public:
        PerfMonCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetCPUUsage() const;
        std::string GetMemoryUsage() const;
        std::string GetDiskUsage() const;
        std::string GetNetworkStats() const;
        std::string GetDatabaseStats() const;
    };

    // Thread information command
    class ThreadInfoCommand : public CommandBase<ThreadInfoCommand> {
    public:
        ThreadInfoCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetThreadList() const;
        std::string GetThreadStats() const;
        std::string GetDeadlockInfo() const;
    };

} // namespace Commands

#endif // __SYSTEMCOMMANDS_H_INCL__