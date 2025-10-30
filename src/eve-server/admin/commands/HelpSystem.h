#ifndef __HELP_SYSTEM_H__
#define __HELP_SYSTEM_H__

#include "eve-server.h"
#include "admin/commands/ICommand.h"
#include "admin/commands/ParameterValidator.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace Commands {

    // Help topic types
    enum class HelpTopicType {
        Command,        // Individual command help
        Category,       // Command category help
        Overview,       // System overview
        Tutorial,       // Tutorial
        Reference       // Reference documentation
    };

    // Help format types
    enum class HelpFormat {
        Plain,          // Plain text
        Colored,        // Colored text
        HTML,           // HTML format
        Markdown        // Markdown format
    };

    // Help topic
    struct HelpTopic {
        std::string id;
        std::string title;
        std::string content;
        HelpTopicType type;
        std::vector<std::string> tags;
        std::vector<std::string> relatedTopics;
        std::string category;
        int priority;
        
        HelpTopic(const std::string& id, const std::string& title, 
                 const std::string& content, HelpTopicType type = HelpTopicType::Reference)
            : id(id), title(title), content(content), type(type), priority(0) {}
    };

    // Command usage example
    struct CommandExample {
        std::string command;
        std::string description;
        std::string expectedOutput;
        std::vector<std::string> prerequisites;
        
        CommandExample(const std::string& cmd, const std::string& desc)
            : command(cmd), description(desc) {}
    };

    // Help search result
    struct HelpSearchResult {
        std::string topicId;
        std::string title;
        std::string snippet;
        float relevanceScore;
        HelpTopicType type;
        
        // Default constructor
        HelpSearchResult() : relevanceScore(0.0f), type(HelpTopicType::Reference) {}
        
        HelpSearchResult(const std::string& id, const std::string& title, 
                        const std::string& snippet, float score, HelpTopicType type)
            : topicId(id), title(title), snippet(snippet), relevanceScore(score), type(type) {}
    };

    // Help generator interface
    class IHelpGenerator {
    public:
        virtual ~IHelpGenerator() = default;
        
        virtual std::string GenerateCommandHelp(const std::string& commandName, 
                                               const ICommand* command, 
                                               HelpFormat format = HelpFormat::Plain) const = 0;
        
        virtual std::string GenerateCategoryHelp(const std::string& category, 
                                                const std::vector<std::string>& commands,
                                                HelpFormat format = HelpFormat::Plain) const = 0;
        
        virtual std::string GenerateOverviewHelp(const std::map<std::string, std::vector<std::string>>& categories,
                                                HelpFormat format = HelpFormat::Plain) const = 0;
        
        virtual std::string GenerateUsageExamples(const std::string& commandName,
                                                 const std::vector<CommandExample>& examples,
                                                 HelpFormat format = HelpFormat::Plain) const = 0;
    };

    // Standard help generator
    class StandardHelpGenerator : public IHelpGenerator {
    public:
        StandardHelpGenerator();
        virtual ~StandardHelpGenerator();
        
        // IHelpGenerator implementation
        std::string GenerateCommandHelp(const std::string& commandName, 
                                       const ICommand* command, 
                                       HelpFormat format = HelpFormat::Plain) const override;
        
        std::string GenerateCategoryHelp(const std::string& category, 
                                        const std::vector<std::string>& commands,
                                        HelpFormat format = HelpFormat::Plain) const override;
        
        std::string GenerateOverviewHelp(const std::map<std::string, std::vector<std::string>>& categories,
                                        HelpFormat format = HelpFormat::Plain) const override;
        
        std::string GenerateUsageExamples(const std::string& commandName,
                                         const std::vector<CommandExample>& examples,
                                         HelpFormat format = HelpFormat::Plain) const override;
        
        // Formatting methods
        std::string FormatHeader(const std::string& text, int level, HelpFormat format) const;
        std::string FormatBold(const std::string& text, HelpFormat format) const;
        std::string FormatCode(const std::string& text, HelpFormat format) const;
        std::string FormatList(const std::vector<std::string>& items, HelpFormat format) const;
        std::string FormatTable(const std::vector<std::vector<std::string>>& rows, HelpFormat format) const;
        
    private:
        std::string GenerateParameterTable(const std::vector<CommandParameter>& parameters, HelpFormat format) const;
        std::string GeneratePermissionInfo(const std::vector<std::string>& requiredRoles, HelpFormat format) const;
        std::string FormatParameterType(ParameterType type) const;
        std::string EscapeForFormat(const std::string& text, HelpFormat format) const;
    };

    // Help topic manager
    class HelpTopicManager {
    public:
        HelpTopicManager();
        ~HelpTopicManager();
        
        // Topic management
        void AddTopic(const HelpTopic& topic);
        void RemoveTopic(const std::string& topicId);
        bool HasTopic(const std::string& topicId) const;
        const HelpTopic* GetTopic(const std::string& topicId) const;
        
        // Category management
        std::vector<std::string> GetCategories() const;
        std::vector<std::string> GetTopicsInCategory(const std::string& category) const;
        
        // Search functionality
        std::vector<HelpSearchResult> SearchTopics(const std::string& query, 
                                                  HelpTopicType typeFilter = HelpTopicType::Reference,
                                                  int maxResults = 10) const;
        
        std::vector<HelpSearchResult> SearchByTag(const std::string& tag, int maxResults = 10) const;
        
        // Related topics
        std::vector<std::string> GetRelatedTopics(const std::string& topicId) const;
        
        // Statistics
        size_t GetTopicCount() const;
        size_t GetCategoryCount() const;
        
    private:
        std::map<std::string, std::unique_ptr<HelpTopic>> m_topics;
        std::map<std::string, std::vector<std::string>> m_categorizedTopics;
        std::map<std::string, std::vector<std::string>> m_taggedTopics;
        
        void IndexTopic(const HelpTopic& topic);
        void UnindexTopic(const std::string& topicId);
        float CalculateRelevanceScore(const HelpTopic& topic, const std::string& query) const;
        std::vector<std::string> TokenizeQuery(const std::string& query) const;
    };

    // Smart help system
    class SmartHelpSystem {
    public:
        SmartHelpSystem();
        ~SmartHelpSystem();
        
        // Initialization
        void Initialize();
        void RegisterCommand(const std::string& commandName, const ICommand* command);
        void UnregisterCommand(const std::string& commandName);
        
        // Help generation
        std::string GetCommandHelp(const std::string& commandName, HelpFormat format = HelpFormat::Plain) const;
        std::string GetCategoryHelp(const std::string& category, HelpFormat format = HelpFormat::Plain) const;
        std::string GetOverviewHelp(HelpFormat format = HelpFormat::Plain) const;
        
        // Search and discovery
        std::vector<HelpSearchResult> SearchHelp(const std::string& query, int maxResults = 10) const;
        std::vector<std::string> GetSimilarCommands(const std::string& commandName) const;
        std::vector<std::string> GetCommandSuggestions(const std::string& partialCommand) const;
        
        // Context help
        std::string GetContextualHelp(const std::string& commandName, 
                                     const std::vector<std::string>& args,
                                     const std::string& errorMessage = "") const;
        
        std::string GetParameterHelp(const std::string& commandName, 
                                   const std::string& parameterName) const;
        
        // Tutorials and examples
        void AddCommandExample(const std::string& commandName, const CommandExample& example);
        std::vector<CommandExample> GetCommandExamples(const std::string& commandName) const;
        std::string GenerateTutorial(const std::string& topicName) const;
        
        // Statistics and analysis
        std::vector<std::string> GetMostUsedCommands() const;
        std::vector<std::string> GetRecentlyAddedCommands() const;
        std::map<std::string, int> GetCommandUsageStats() const;
        
        // Configuration
        void SetDefaultFormat(HelpFormat format);
        void SetMaxSearchResults(int maxResults);
        void EnableColorOutput(bool enable);
        
        // Custom help content
        void AddCustomTopic(const HelpTopic& topic);
        void SetCommandDescription(const std::string& commandName, const std::string& description);
        void SetCategoryDescription(const std::string& category, const std::string& description);
        
    private:
        std::unique_ptr<IHelpGenerator> m_helpGenerator;
        std::unique_ptr<HelpTopicManager> m_topicManager;
        
        std::map<std::string, const ICommand*> m_registeredCommands;
        std::map<std::string, std::vector<std::string>> m_commandCategories;
        std::map<std::string, std::vector<CommandExample>> m_commandExamples;
        std::map<std::string, std::string> m_customDescriptions;
        std::map<std::string, std::string> m_categoryDescriptions;
        std::map<std::string, int> m_usageStats;
        
        HelpFormat m_defaultFormat;
        int m_maxSearchResults;
        bool m_colorOutputEnabled;
        
        void LoadBuiltinTopics();
        void LoadCommandTopics();
        std::string GetCommandCategory(const std::string& commandName) const;
        std::vector<std::string> GetCommandsInCategory(const std::string& category) const;
        
        // Smart suggestion algorithms
        float CalculateCommandSimilarity(const std::string& cmd1, const std::string& cmd2) const;
        std::vector<std::string> GenerateTypoSuggestions(const std::string& input) const;
        std::string GenerateDidYouMean(const std::string& input) const;
    };

    // Help system factory
    class HelpSystemFactory {
    public:
        static std::unique_ptr<SmartHelpSystem> CreateDefault();
        static std::unique_ptr<IHelpGenerator> CreateGenerator(const std::string& type = "standard");
        static std::unique_ptr<HelpTopicManager> CreateTopicManager();
    };

    // Global help system instance
    extern SmartHelpSystem* g_helpSystem;
    
    // Convenience macros
    #define HELP_SYSTEM() (Commands::g_helpSystem)
    #define GET_COMMAND_HELP(cmd) (HELP_SYSTEM()->GetCommandHelp(cmd))
    #define SEARCH_HELP(query) (HELP_SYSTEM()->SearchHelp(query))

} // namespace Commands

#endif // __HELP_SYSTEM_H__