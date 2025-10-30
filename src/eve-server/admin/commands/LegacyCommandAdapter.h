#ifndef __LEGACY_COMMAND_ADAPTER_H__
#define __LEGACY_COMMAND_ADAPTER_H__

#include "eve-server.h"
#include "admin/commands/CommandManager.h"
#include "admin/commands/HelpSystem.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
class Client;
class CommandDB;

namespace Commands {

    // Legacy command function type definition
    typedef void (*LegacyCommandFunction)(Client* who, const Seperator& args);
    
    // Legacy command information structure
    struct LegacyCommandInfo {
        std::string name;
        LegacyCommandFunction function;
        std::string description;
        std::vector<std::string> requiredRoles;
        bool isDeprecated;
        std::string replacementCommand;
        
        LegacyCommandInfo(const std::string& name, LegacyCommandFunction func, 
                         const std::string& desc = "", 
                         const std::vector<std::string>& roles = {})
            : name(name), function(func), description(desc), requiredRoles(roles), 
              isDeprecated(false) {}
    };

    // Legacy command wrapper - wraps legacy commands as new framework commands
    class LegacyCommandWrapper : public ICommand {
    public:
        LegacyCommandWrapper(const LegacyCommandInfo& info);
        virtual ~LegacyCommandWrapper();
        
        // ICommand interface implementation
        CommandResult Execute(const CommandContext& context) override;
        std::string GetName() const override { return m_info.name; }
        std::string GetDescription() const override;
        std::string GetCategory() const override { return "Legacy"; }
        int64 GetRequiredRole() const override;
        std::vector<CommandParameter> GetParameters() const override;
        std::vector<std::string> GetExamples() const override { return {}; }
        
        // Legacy-specific methods
        std::vector<std::string> GetRequiredRoles() const { return m_info.requiredRoles; }
        
        // Legacy command specific methods
        bool IsDeprecated() const { return m_info.isDeprecated; }
        std::string GetReplacementCommand() const { return m_info.replacementCommand; }
        void MarkAsDeprecated(const std::string& replacement = "");
        
    private:
        LegacyCommandInfo m_info;
        std::vector<EnhancedParameter> m_parameters;
        
        void InitializeParameters();
        Seperator ConvertArgsToSeperator(const std::vector<std::string>& args) const;
    };

    // Modern command adapter - adapts new framework commands to legacy system
    class ModernCommandAdapter {
    public:
        ModernCommandAdapter(const std::string& commandName, ICommand* command);
        ~ModernCommandAdapter();
        
        // Legacy command function interface
        static void ExecuteLegacyStyle(Client* who, const Seperator& args);
        
        // Register to legacy system
        void RegisterToLegacySystem();
        void UnregisterFromLegacySystem();
        
        // Get information
        std::string GetCommandName() const { return m_commandName; }
        ICommand* GetCommand() const { return m_command; }
        
    private:
        std::string m_commandName;
        ICommand* m_command;
        static std::map<std::string, ModernCommandAdapter*> s_adapters;
        
        CommandContext CreateContextFromLegacy(Client* who, const Seperator& args) const;
        void SendResultToClient(Client* who, const CommandResult& result) const;
    };

    // Command migration manager
    class CommandMigrationManager {
    public:
        CommandMigrationManager();
        ~CommandMigrationManager();
        
        // Initialization
        void Initialize(CommandManager* modernManager);
        void Shutdown();
        
        // Legacy command registration
        void RegisterLegacyCommand(const LegacyCommandInfo& info);
        void UnregisterLegacyCommand(const std::string& commandName);
        
        // Modern command registration
        void RegisterModernCommand(const std::string& commandName, ICommand* command);
        void UnregisterModernCommand(const std::string& commandName);
        
        // Command migration
        bool MigrateCommand(const std::string& commandName);
        void MigrateAllCommands();
        bool IsCommandMigrated(const std::string& commandName) const;
        
        // Deprecation management
        void DeprecateCommand(const std::string& commandName, const std::string& replacement = "");
        std::vector<std::string> GetDeprecatedCommands() const;
        
        // Statistics
        size_t GetLegacyCommandCount() const;
        size_t GetModernCommandCount() const;
        size_t GetMigratedCommandCount() const;
        float GetMigrationProgress() const;
        
        // Compatibility check
        bool CheckCommandCompatibility(const std::string& commandName) const;
        std::vector<std::string> GetIncompatibleCommands() const;
        
        // Report generation
        std::string GenerateMigrationReport() const;
        std::string GenerateCompatibilityReport() const;
        
        // Configuration
        void SetMigrationMode(bool enableAutoMigration);
        void SetDeprecationWarnings(bool enable);
        
    private:
        CommandManager* m_modernManager;
        std::map<std::string, std::unique_ptr<LegacyCommandWrapper>> m_legacyCommands;
        std::map<std::string, std::unique_ptr<ModernCommandAdapter>> m_modernAdapters;
        std::vector<std::string> m_migratedCommands;
        
        bool m_autoMigrationEnabled;
        bool m_deprecationWarningsEnabled;
        bool m_initialized;
        
        void CreateLegacyWrapper(const LegacyCommandInfo& info);
        void CreateModernAdapter(const std::string& commandName, ICommand* command);
        bool ValidateCommandMigration(const std::string& commandName) const;
    };

    // Legacy system integrator
    class LegacySystemIntegrator {
    public:
        LegacySystemIntegrator();
        ~LegacySystemIntegrator();
        
        // Initialize integration
        bool InitializeIntegration();
        void ShutdownIntegration();
        
        // Legacy system hooks
        static bool InterceptLegacyCommand(const std::string& commandName, Client* who, const Seperator& args);
        static void RegisterLegacyCommandHook();
        static void UnregisterLegacyCommandHook();
        
        // Command routing
        bool RouteCommand(const std::string& commandName, Client* who, const Seperator& args);
        
        // System status
        bool IsIntegrationActive() const { return m_integrationActive; }
        std::string GetIntegrationStatus() const;
        
        // Performance monitoring
        struct PerformanceMetrics {
            size_t totalCommands;
            size_t legacyCommands;
            size_t modernCommands;
            double averageExecutionTime;
            size_t errorCount;
        };
        
        PerformanceMetrics GetPerformanceMetrics() const;
        void ResetPerformanceMetrics();
        
    private:
        bool m_integrationActive;
        CommandMigrationManager* m_migrationManager;
        mutable PerformanceMetrics m_metrics;
        
        static LegacySystemIntegrator* s_instance;
        
        void UpdateMetrics(bool isLegacy, double executionTime, bool hasError);
    };

    // Migration utility class
    class MigrationUtilities {
    public:
        // Parameter conversion
        static std::vector<std::string> SeperatorToVector(const Seperator& sep);
        static Seperator VectorToSeperator(const std::vector<std::string>& args);
        
        // Permission conversion
        static std::vector<std::string> ConvertLegacyRoles(const std::string& legacyRole);
        static std::string ConvertModernRoles(const std::vector<std::string>& modernRoles);
        
        // Error message conversion
        static std::string ConvertErrorMessage(const std::string& legacyError);
        static CommandResult ConvertLegacyResult(bool success, const std::string& message = "");
        
        // Command name normalization
        static std::string NormalizeCommandName(const std::string& name);
        static bool IsValidCommandName(const std::string& name);
        
        // Parameter inference
        static std::vector<EnhancedParameter> InferParametersFromUsage(const std::string& commandName, 
                                                                      const std::string& usage);
        
        // Compatibility check
        static bool CheckParameterCompatibility(const std::vector<EnhancedParameter>& modern,
                                               const std::string& legacyUsage);
        
        // Documentation generation
        static std::string GenerateMigrationGuide(const std::string& commandName);
        static std::string GenerateDeprecationNotice(const std::string& oldCommand, 
                                                    const std::string& newCommand);
    };

    // Global migration manager instances
    extern CommandMigrationManager* g_migrationManager;
    extern LegacySystemIntegrator* g_legacyIntegrator;
    
    // Convenience macros
    #define MIGRATION_MANAGER() (Commands::g_migrationManager)
    #define LEGACY_INTEGRATOR() (Commands::g_legacyIntegrator)
    
    // Legacy command registration macro (backward compatible)
    #define REGISTER_LEGACY_COMMAND(name, func, desc, roles) \
        do { \
            if (MIGRATION_MANAGER()) { \
                Commands::LegacyCommandInfo info(name, func, desc, roles); \
                MIGRATION_MANAGER()->RegisterLegacyCommand(info); \
            } \
        } while(0)
    
    // Modern command registration macro
    #define REGISTER_MODERN_COMMAND(name, command) \
        do { \
            if (MIGRATION_MANAGER()) { \
                MIGRATION_MANAGER()->RegisterModernCommand(name, command); \
            } \
        } while(0)
    
    // Command migration macro
    #define MIGRATE_COMMAND(name) \
        do { \
            if (MIGRATION_MANAGER()) { \
                MIGRATION_MANAGER()->MigrateCommand(name); \
            } \
        } while(0)

} // namespace Commands

#endif // __LEGACY_COMMAND_ADAPTER_H__