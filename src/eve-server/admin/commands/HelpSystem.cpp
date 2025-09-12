#include "eve-server.h"
#include "admin/commands/HelpSystem.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cmath>

namespace Commands {

    // Global help system instance
    SmartHelpSystem* g_helpSystem = nullptr;

    // StandardHelpGenerator implementation
    StandardHelpGenerator::StandardHelpGenerator() {
    }

    StandardHelpGenerator::~StandardHelpGenerator() {
    }

    std::string StandardHelpGenerator::GenerateCommandHelp(const std::string& commandName, 
                                                         const ICommand* command, 
                                                         HelpFormat format) const {
        if (!command) {
            return "Command '" + commandName + "' not found.";
        }

        std::ostringstream help;
        
        // Command title
        help << FormatHeader("Command: /" + commandName, 1, format) << "\n\n";
        
        // Description
        std::string description = command->GetDescription();
        if (!description.empty()) {
            help << FormatBold("Description:", format) << "\n";
            help << description << "\n\n";
        }
        
        // Usage
        help << FormatBold("Usage:", format) << "\n";
        help << FormatCode("/" + commandName, format);
        
        auto parameters = command->GetParameters();
        for (const auto& param : parameters) {
            help << " ";
            if (!param.required) {
                help << "[";
            }
            help << "<" << param.name << ">";
            if (!param.required) {
                help << "]";
            }
        }
        help << "\n\n";
        
        // Parameter details
        if (!parameters.empty()) {
            help << FormatBold("Parameters:", format) << "\n";
            help << GenerateParameterTable(parameters, format) << "\n";
        }
        
        // Permission requirements
        int64 requiredRole = command->GetRequiredRole();
        if (requiredRole != 0) {
            std::vector<std::string> roles;
            // Convert role bitmask to role names
            if (requiredRole & 1) roles.push_back("PLAYER");
            if (requiredRole & 2) roles.push_back("ADMIN");
            help << GeneratePermissionInfo(roles, format) << "\n";
        }
        
        // Examples
        // TODO: Get examples from example system
        
        return help.str();
    }

    std::string StandardHelpGenerator::GenerateCategoryHelp(const std::string& category, 
                                                          const std::vector<std::string>& commands,
                                                          HelpFormat format) const {
        std::ostringstream help;
        
        help << FormatHeader("Category: " + category, 1, format) << "\n\n";
        
        if (commands.empty()) {
            help << "No commands available in this category.\n";
            return help.str();
        }
        
        help << FormatBold("Available Commands:", format) << "\n";
        
        std::vector<std::string> formattedCommands;
        for (const auto& cmd : commands) {
            formattedCommands.push_back("/" + cmd);
        }
        
        help << FormatList(formattedCommands, format) << "\n";
        
        help << "\nUse " << FormatCode("/help <command>", format) << " for detailed information about a specific command.\n";
        
        return help.str();
    }

    std::string StandardHelpGenerator::GenerateOverviewHelp(const std::map<std::string, std::vector<std::string>>& categories,
                                                          HelpFormat format) const {
        std::ostringstream help;
        
        help << FormatHeader("EVE-Server Command System", 1, format) << "\n\n";
        
        help << "Welcome to the EVE-Server administrative command system. ";
        help << "This system provides powerful tools for server management, ";
        help << "game administration, and debugging.\n\n";
        
        help << FormatBold("Command Categories:", format) << "\n\n";
        
        for (const auto& category : categories) {
            help << FormatHeader(category.first, 2, format) << "\n";
            help << "Commands: " << std::to_string(category.second.size()) << "\n";
            
            // Show first few commands as preview
            std::vector<std::string> preview;
            size_t previewCount = std::min(size_t(3), category.second.size());
            for (size_t i = 0; i < previewCount; ++i) {
                preview.push_back("/" + category.second[i]);
            }
            if (category.second.size() > previewCount) {
                preview.push_back("...");
            }
            
            help << "Examples: " << FormatCode("", format);
            for (size_t i = 0; i < preview.size(); ++i) {
                if (i > 0) help << ", ";
                help << preview[i];
            }
            help << "\n\n";
        }
        
        help << FormatBold("Getting Help:", format) << "\n";
        help << "• " << FormatCode("/help", format) << " - Show this overview\n";
        help << "• " << FormatCode("/help <command>", format) << " - Get help for a specific command\n";
        help << "• " << FormatCode("/help <category>", format) << " - List commands in a category\n";
        help << "• " << FormatCode("/list", format) << " - List all available commands\n";
        help << "• " << FormatCode("/search <query>", format) << " - Search for commands\n\n";
        
        return help.str();
    }

    std::string StandardHelpGenerator::GenerateUsageExamples(const std::string& commandName,
                                                           const std::vector<CommandExample>& examples,
                                                           HelpFormat format) const {
        if (examples.empty()) {
            return "No examples available for this command.\n";
        }
        
        std::ostringstream help;
        
        help << FormatBold("Examples:", format) << "\n\n";
        
        for (size_t i = 0; i < examples.size(); ++i) {
            const auto& example = examples[i];
            
            help << FormatBold("Example " + std::to_string(i + 1) + ":", format) << "\n";
            help << example.description << "\n";
            help << FormatCode(example.command, format) << "\n";
            
            if (!example.expectedOutput.empty()) {
                help << "Expected output: " << example.expectedOutput << "\n";
            }
            
            if (!example.prerequisites.empty()) {
                help << "Prerequisites: ";
                for (size_t j = 0; j < example.prerequisites.size(); ++j) {
                    if (j > 0) help << ", ";
                    help << example.prerequisites[j];
                }
                help << "\n";
            }
            
            help << "\n";
        }
        
        return help.str();
    }

    std::string StandardHelpGenerator::FormatHeader(const std::string& text, int level, HelpFormat format) const {
        switch (format) {
            case HelpFormat::Markdown:
                return std::string(level, '#') + " " + text;
            case HelpFormat::HTML:
                return "<h" + std::to_string(level) + ">" + EscapeForFormat(text, format) + "</h" + std::to_string(level) + ">";
            case HelpFormat::Colored:
                return "\033[1;36m" + text + "\033[0m"; // Cyan bold
            case HelpFormat::Plain:
            default:
                return text + "\n" + std::string(text.length(), level == 1 ? '=' : '-');
        }
    }

    std::string StandardHelpGenerator::FormatBold(const std::string& text, HelpFormat format) const {
        switch (format) {
            case HelpFormat::Markdown:
                return "**" + text + "**";
            case HelpFormat::HTML:
                return "<strong>" + EscapeForFormat(text, format) + "</strong>";
            case HelpFormat::Colored:
                return "\033[1m" + text + "\033[0m"; // Bold
            case HelpFormat::Plain:
            default:
                return text;
        }
    }

    std::string StandardHelpGenerator::FormatCode(const std::string& text, HelpFormat format) const {
        switch (format) {
            case HelpFormat::Markdown:
                return "`" + text + "`";
            case HelpFormat::HTML:
                return "<code>" + EscapeForFormat(text, format) + "</code>";
            case HelpFormat::Colored:
                return "\033[32m" + text + "\033[0m"; // Green
            case HelpFormat::Plain:
            default:
                return text;
        }
    }

    std::string StandardHelpGenerator::FormatList(const std::vector<std::string>& items, HelpFormat format) const {
        std::ostringstream result;
        
        for (const auto& item : items) {
            switch (format) {
                case HelpFormat::Markdown:
                case HelpFormat::Plain:
                    result << "  • " << item << "\n";
                    break;
                case HelpFormat::HTML:
                    if (result.tellp() == 0) result << "<ul>\n";
                    result << "  <li>" << EscapeForFormat(item, format) << "</li>\n";
                    break;
                case HelpFormat::Colored:
                    result << "  \033[33m•\033[0m " << item << "\n";
                    break;
            }
        }
        
        if (format == HelpFormat::HTML && !items.empty()) {
            result << "</ul>";
        }
        
        return result.str();
    }

    std::string StandardHelpGenerator::FormatTable(const std::vector<std::vector<std::string>>& rows, HelpFormat format) const {
        if (rows.empty()) return "";
        
        std::ostringstream result;
        
        switch (format) {
            case HelpFormat::HTML: {
                result << "<table>\n";
                for (size_t i = 0; i < rows.size(); ++i) {
                    result << "  <tr>";
                    for (const auto& cell : rows[i]) {
                        std::string tag = (i == 0) ? "th" : "td";
                        result << "<" << tag << ">" << EscapeForFormat(cell, format) << "</" << tag << ">";
                    }
                    result << "</tr>\n";
                }
                result << "</table>";
                break;
            }
            case HelpFormat::Markdown: {
                // Calculate column width
                std::vector<size_t> colWidths(rows[0].size(), 0);
                for (const auto& row : rows) {
                    for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
                        colWidths[i] = std::max(colWidths[i], row[i].length());
                    }
                }
                
                // Output table
                for (size_t rowIdx = 0; rowIdx < rows.size(); ++rowIdx) {
                    result << "|";
                    for (size_t colIdx = 0; colIdx < rows[rowIdx].size(); ++colIdx) {
                        result << " " << std::left << std::setw(colWidths[colIdx]) << rows[rowIdx][colIdx] << " |";
                    }
                    result << "\n";
                    
                    // Add separator line (only after header row)
                    if (rowIdx == 0) {
                        result << "|";
                        for (size_t colIdx = 0; colIdx < colWidths.size(); ++colIdx) {
                            result << " " << std::string(colWidths[colIdx], '-') << " |";
                        }
                        result << "\n";
                    }
                }
                break;
            }
            case HelpFormat::Plain:
            case HelpFormat::Colored:
            default: {
                // Calculate column width
                std::vector<size_t> colWidths(rows[0].size(), 0);
                for (const auto& row : rows) {
                    for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
                        colWidths[i] = std::max(colWidths[i], row[i].length());
                    }
                }
                
                // Output table
                for (size_t rowIdx = 0; rowIdx < rows.size(); ++rowIdx) {
                    for (size_t colIdx = 0; colIdx < rows[rowIdx].size(); ++colIdx) {
                        if (colIdx > 0) result << "  ";
                        result << std::left << std::setw(colWidths[colIdx]) << rows[rowIdx][colIdx];
                    }
                    result << "\n";
                    
                    // Add separator line (only after header row)
                    if (rowIdx == 0) {
                        for (size_t colIdx = 0; colIdx < colWidths.size(); ++colIdx) {
                            if (colIdx > 0) result << "  ";
                            result << std::string(colWidths[colIdx], '-');
                        }
                        result << "\n";
                    }
                }
                break;
            }
        }
        
        return result.str();
    }

    std::string StandardHelpGenerator::GenerateParameterTable(const std::vector<CommandParameter>& parameters, HelpFormat format) const {
        if (parameters.empty()) {
            return "No parameters required.\n";
        }
        
        std::vector<std::vector<std::string>> rows;
        rows.push_back({"Name", "Type", "Required", "Description"});
        
        for (const auto& param : parameters) {
            std::vector<std::string> row;
            row.push_back(param.name);
            row.push_back(FormatParameterType(param.type));
            row.push_back(param.required ? "Yes" : "No");
            
            std::string desc = param.description;
            if (!param.defaultValue.empty()) {
                desc += " (default: " + param.defaultValue + ")";
            }
            row.push_back(desc);
            
            rows.push_back(row);
        }
        
        return FormatTable(rows, format);
    }

    std::string StandardHelpGenerator::GeneratePermissionInfo(const std::vector<std::string>& requiredRoles, HelpFormat format) const {
        std::ostringstream info;
        
        info << FormatBold("Required Permissions:", format) << "\n";
        
        if (requiredRoles.empty()) {
            info << "No special permissions required.\n";
        } else {
            std::vector<std::string> formattedRoles;
            for (const auto& role : requiredRoles) {
                formattedRoles.push_back(role);
            }
            info << FormatList(formattedRoles, format);
        }
        
        return info.str();
    }

    std::string StandardHelpGenerator::FormatParameterType(ParameterType type) const {
        switch (type) {
            case ParameterType::String: return "String";
            case ParameterType::Integer: return "Integer";
            case ParameterType::Float: return "Number";
            case ParameterType::Boolean: return "Boolean";
            default: return "Unknown";
        }
    }

    std::string StandardHelpGenerator::EscapeForFormat(const std::string& text, HelpFormat format) const {
        if (format != HelpFormat::HTML) {
            return text;
        }
        
        std::string escaped = text;
        std::string::size_type pos = 0;
        
        // Replace HTML special characters
        while ((pos = escaped.find('&', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "&amp;");
            pos += 5;
        }
        pos = 0;
        while ((pos = escaped.find('<', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "&lt;");
            pos += 4;
        }
        pos = 0;
        while ((pos = escaped.find('>', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "&gt;");
            pos += 4;
        }
        
        return escaped;
    }

    // HelpTopicManager implementation
    HelpTopicManager::HelpTopicManager() {
    }

    HelpTopicManager::~HelpTopicManager() {
    }

    void HelpTopicManager::AddTopic(const HelpTopic& topic) {
        auto topicPtr = std::make_unique<HelpTopic>(topic);
        IndexTopic(*topicPtr);
        m_topics[topic.id] = std::move(topicPtr);
    }

    void HelpTopicManager::RemoveTopic(const std::string& topicId) {
        auto it = m_topics.find(topicId);
        if (it != m_topics.end()) {
            UnindexTopic(topicId);
            m_topics.erase(it);
        }
    }

    bool HelpTopicManager::HasTopic(const std::string& topicId) const {
        return m_topics.find(topicId) != m_topics.end();
    }

    const HelpTopic* HelpTopicManager::GetTopic(const std::string& topicId) const {
        auto it = m_topics.find(topicId);
        return (it != m_topics.end()) ? it->second.get() : nullptr;
    }

    std::vector<std::string> HelpTopicManager::GetCategories() const {
        std::vector<std::string> categories;
        for (const auto& pair : m_categorizedTopics) {
            categories.push_back(pair.first);
        }
        return categories;
    }

    std::vector<std::string> HelpTopicManager::GetTopicsInCategory(const std::string& category) const {
        auto it = m_categorizedTopics.find(category);
        return (it != m_categorizedTopics.end()) ? it->second : std::vector<std::string>();
    }

    std::vector<HelpSearchResult> HelpTopicManager::SearchTopics(const std::string& query, 
                                                               HelpTopicType typeFilter,
                                                               int maxResults) const {
        std::vector<HelpSearchResult> results;
        auto queryTokens = TokenizeQuery(query);
        
        for (const auto& pair : m_topics) {
            const auto& topic = *pair.second;
            
            if (typeFilter != HelpTopicType::Reference && topic.type != typeFilter) {
                continue;
            }
            
            float score = CalculateRelevanceScore(topic, query);
            if (score > 0.1f) { // Minimum relevance threshold
                std::string snippet = topic.content.substr(0, 100);
                if (topic.content.length() > 100) {
                    snippet += "...";
                }
                
                results.emplace_back(topic.id, topic.title, snippet, score, topic.type);
            }
        }
        
        // Sort by relevance
        std::sort(results.begin(), results.end(), 
                 [](const HelpSearchResult& a, const HelpSearchResult& b) {
                     return a.relevanceScore > b.relevanceScore;
                 });
        
        // Limit result count
        if (results.size() > static_cast<size_t>(maxResults)) {
            results.resize(maxResults);
        }
        
        return results;
    }

    std::vector<HelpSearchResult> HelpTopicManager::SearchByTag(const std::string& tag, int maxResults) const {
        std::vector<HelpSearchResult> results;
        
        auto it = m_taggedTopics.find(tag);
        if (it != m_taggedTopics.end()) {
            for (const auto& topicId : it->second) {
                const auto* topic = GetTopic(topicId);
                if (topic) {
                    std::string snippet = topic->content.substr(0, 100);
                    if (topic->content.length() > 100) {
                        snippet += "...";
                    }
                    
                    results.emplace_back(topic->id, topic->title, snippet, 1.0f, topic->type);
                    
                    if (results.size() >= static_cast<size_t>(maxResults)) {
                        break;
                    }
                }
            }
        }
        
        return results;
    }

    std::vector<std::string> HelpTopicManager::GetRelatedTopics(const std::string& topicId) const {
        const auto* topic = GetTopic(topicId);
        return topic ? topic->relatedTopics : std::vector<std::string>();
    }

    size_t HelpTopicManager::GetTopicCount() const {
        return m_topics.size();
    }

    size_t HelpTopicManager::GetCategoryCount() const {
        return m_categorizedTopics.size();
    }

    void HelpTopicManager::IndexTopic(const HelpTopic& topic) {
        // Category index
        if (!topic.category.empty()) {
            m_categorizedTopics[topic.category].push_back(topic.id);
        }
        
        // Tag index
        for (const auto& tag : topic.tags) {
            m_taggedTopics[tag].push_back(topic.id);
        }
    }

    void HelpTopicManager::UnindexTopic(const std::string& topicId) {
        // Remove from category index
        for (auto& pair : m_categorizedTopics) {
            auto& topics = pair.second;
            topics.erase(std::remove(topics.begin(), topics.end(), topicId), topics.end());
        }
        
        // Remove from tag index
        for (auto& pair : m_taggedTopics) {
            auto& topics = pair.second;
            topics.erase(std::remove(topics.begin(), topics.end(), topicId), topics.end());
        }
    }

    float HelpTopicManager::CalculateRelevanceScore(const HelpTopic& topic, const std::string& query) const {
        auto queryTokens = TokenizeQuery(query);
        if (queryTokens.empty()) {
            return 0.0f;
        }
        
        float score = 0.0f;
        
        // Title match (highest weight)
        std::string lowerTitle = topic.title;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        
        for (const auto& token : queryTokens) {
            if (lowerTitle.find(token) != std::string::npos) {
                score += 3.0f;
            }
        }
        
        // Content match
        std::string lowerContent = topic.content;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
        
        for (const auto& token : queryTokens) {
            size_t pos = 0;
            while ((pos = lowerContent.find(token, pos)) != std::string::npos) {
                score += 1.0f;
                pos += token.length();
            }
        }
        
        // Tag match
        for (const auto& tag : topic.tags) {
            std::string lowerTag = tag;
            std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), ::tolower);
            
            for (const auto& token : queryTokens) {
                if (lowerTag.find(token) != std::string::npos) {
                    score += 2.0f;
                }
            }
        }
        
        // Normalize score
        return score / (queryTokens.size() * 3.0f);
    }

    std::vector<std::string> HelpTopicManager::TokenizeQuery(const std::string& query) const {
        std::vector<std::string> tokens;
        std::string lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        
        std::istringstream iss(lowerQuery);
        std::string token;
        
        while (iss >> token) {
            // Remove punctuation
            token.erase(std::remove_if(token.begin(), token.end(), ::ispunct), token.end());
            
            if (!token.empty() && token.length() > 2) { // Ignore words that are too short
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }

    // SmartHelpSystem implementation
    SmartHelpSystem::SmartHelpSystem()
        : m_helpGenerator(std::make_unique<StandardHelpGenerator>())
        , m_topicManager(std::make_unique<HelpTopicManager>())
        , m_defaultFormat(HelpFormat::Plain)
        , m_maxSearchResults(10)
        , m_colorOutputEnabled(false) {
    }

    SmartHelpSystem::~SmartHelpSystem() {
    }

    void SmartHelpSystem::Initialize() {
        LoadBuiltinTopics();
        LoadCommandTopics();
    }

    void SmartHelpSystem::RegisterCommand(const std::string& commandName, const ICommand* command) {
        m_registeredCommands[commandName] = command;
        
        // Auto categorization
        std::string category = GetCommandCategory(commandName);
        m_commandCategories[category].push_back(commandName);
        
        // Create command topic
        HelpTopic topic("cmd_" + commandName, "Command: /" + commandName, 
                       command->GetDescription(), HelpTopicType::Command);
        topic.category = category;
        topic.tags.push_back("command");
        topic.tags.push_back(category);
        
        m_topicManager->AddTopic(topic);
    }

    void SmartHelpSystem::UnregisterCommand(const std::string& commandName) {
        m_registeredCommands.erase(commandName);
        
        // Remove from category
        for (auto& pair : m_commandCategories) {
            auto& commands = pair.second;
            commands.erase(std::remove(commands.begin(), commands.end(), commandName), commands.end());
        }
        
        // Remove topic
        m_topicManager->RemoveTopic("cmd_" + commandName);
    }

    std::string SmartHelpSystem::GetCommandHelp(const std::string& commandName, HelpFormat format) const {
        auto it = m_registeredCommands.find(commandName);
        if (it == m_registeredCommands.end()) {
            std::string suggestion = GenerateDidYouMean(commandName);
            std::string error = "Command '" + commandName + "' not found.";
            if (!suggestion.empty()) {
                error += " Did you mean: " + suggestion + "?";
            }
            return error;
        }
        
        std::string help = m_helpGenerator->GenerateCommandHelp(commandName, it->second, format);
        
        // Add usage examples
        auto exampleIt = m_commandExamples.find(commandName);
        if (exampleIt != m_commandExamples.end()) {
            help += "\n" + m_helpGenerator->GenerateUsageExamples(commandName, exampleIt->second, format);
        }
        
        return help;
    }

    std::string SmartHelpSystem::GetCategoryHelp(const std::string& category, HelpFormat format) const {
        auto commands = GetCommandsInCategory(category);
        return m_helpGenerator->GenerateCategoryHelp(category, commands, format);
    }

    std::string SmartHelpSystem::GetOverviewHelp(HelpFormat format) const {
        return m_helpGenerator->GenerateOverviewHelp(m_commandCategories, format);
    }

    std::vector<HelpSearchResult> SmartHelpSystem::SearchHelp(const std::string& query, int maxResults) const {
        return m_topicManager->SearchTopics(query, HelpTopicType::Reference, maxResults);
    }

    std::vector<std::string> SmartHelpSystem::GetSimilarCommands(const std::string& commandName) const {
        std::vector<std::pair<std::string, float>> similarities;
        
        for (const auto& pair : m_registeredCommands) {
            if (pair.first != commandName) {
                float similarity = CalculateCommandSimilarity(commandName, pair.first);
                if (similarity > 0.3f) {
                    similarities.emplace_back(pair.first, similarity);
                }
            }
        }
        
        std::sort(similarities.begin(), similarities.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::vector<std::string> result;
        for (const auto& pair : similarities) {
            result.push_back(pair.first);
            if (result.size() >= 5) break;
        }
        
        return result;
    }

    std::vector<std::string> SmartHelpSystem::GetCommandSuggestions(const std::string& partialCommand) const {
        std::vector<std::string> suggestions;
        
        for (const auto& pair : m_registeredCommands) {
            if (pair.first.find(partialCommand) == 0) {
                suggestions.push_back(pair.first);
            }
        }
        
        std::sort(suggestions.begin(), suggestions.end());
        return suggestions;
    }

    std::string SmartHelpSystem::GetContextualHelp(const std::string& commandName, 
                                                  const std::vector<std::string>& args,
                                                  const std::string& errorMessage) const {
        std::ostringstream help;
        
        if (!errorMessage.empty()) {
            help << "Error: " << errorMessage << "\n\n";
        }
        
        help << GetCommandHelp(commandName, m_defaultFormat);
        
        // Add error-based suggestions
        if (!errorMessage.empty()) {
            help << "\nTroubleshooting:\n";
            if (errorMessage.find("parameter") != std::string::npos) {
                help << "• Check parameter types and values\n";
                help << "• Ensure all required parameters are provided\n";
            }
            if (errorMessage.find("permission") != std::string::npos) {
                help << "• Verify you have the required permissions\n";
                help << "• Contact an administrator if needed\n";
            }
        }
        
        return help.str();
    }

    std::string SmartHelpSystem::GetParameterHelp(const std::string& commandName, 
                                                 const std::string& parameterName) const {
        auto it = m_registeredCommands.find(commandName);
        if (it == m_registeredCommands.end()) {
            return "Command not found.";
        }
        
        auto parameters = it->second->GetParameters();
        for (const auto& param : parameters) {
            if (param.name == parameterName) {
                std::ostringstream help;
                help << "Parameter: " << param.name << "\n";
                std::string typeStr;
                switch (param.type) {
                    case ParameterType::String: typeStr = "String"; break;
                    case ParameterType::Integer: typeStr = "Integer"; break;
                    case ParameterType::Float: typeStr = "Number"; break;
                    case ParameterType::Boolean: typeStr = "Boolean"; break;
                    case ParameterType::EntityID: typeStr = "EntityID"; break;
                    case ParameterType::ItemID: typeStr = "ItemID"; break;
                    case ParameterType::TypeID: typeStr = "TypeID"; break;
                    default: typeStr = "Unknown"; break;
                }
                help << "Type: " << typeStr << "\n";
                help << "Required: " << (param.required ? "Yes" : "No") << "\n";
                help << "Description: " << param.description << "\n";
                
                if (!param.defaultValue.empty()) {
                    help << "Default: " << param.defaultValue << "\n";
                }
                
                // Note: Examples are handled at command level, not parameter level
                
                return help.str();
            }
        }
        
        return "Parameter '" + parameterName + "' not found for command '" + commandName + "'.";
    }

    void SmartHelpSystem::AddCommandExample(const std::string& commandName, const CommandExample& example) {
        m_commandExamples[commandName].push_back(example);
    }

    std::vector<CommandExample> SmartHelpSystem::GetCommandExamples(const std::string& commandName) const {
        auto it = m_commandExamples.find(commandName);
        return (it != m_commandExamples.end()) ? it->second : std::vector<CommandExample>();
    }

    std::string SmartHelpSystem::GenerateTutorial(const std::string& topicName) const {
        // TODO: Implement tutorial generation
        return "Tutorial for '" + topicName + "' is not yet available.";
    }

    std::vector<std::string> SmartHelpSystem::GetMostUsedCommands() const {
        std::vector<std::pair<std::string, int>> usage;
        for (const auto& pair : m_usageStats) {
            usage.emplace_back(pair.first, pair.second);
        }
        
        std::sort(usage.begin(), usage.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::vector<std::string> result;
        for (const auto& pair : usage) {
            result.push_back(pair.first);
            if (result.size() >= 10) break;
        }
        
        return result;
    }

    std::vector<std::string> SmartHelpSystem::GetRecentlyAddedCommands() const {
        // TODO: Implement timestamp-based recently added commands
        std::vector<std::string> recent;
        for (const auto& pair : m_registeredCommands) {
            recent.push_back(pair.first);
            if (recent.size() >= 5) break;
        }
        return recent;
    }

    std::map<std::string, int> SmartHelpSystem::GetCommandUsageStats() const {
        return m_usageStats;
    }

    void SmartHelpSystem::SetDefaultFormat(HelpFormat format) {
        m_defaultFormat = format;
    }

    void SmartHelpSystem::SetMaxSearchResults(int maxResults) {
        m_maxSearchResults = maxResults;
    }

    void SmartHelpSystem::EnableColorOutput(bool enable) {
        m_colorOutputEnabled = enable;
    }

    void SmartHelpSystem::AddCustomTopic(const HelpTopic& topic) {
        m_topicManager->AddTopic(topic);
    }

    void SmartHelpSystem::SetCommandDescription(const std::string& commandName, const std::string& description) {
        m_customDescriptions[commandName] = description;
    }

    void SmartHelpSystem::SetCategoryDescription(const std::string& category, const std::string& description) {
        m_categoryDescriptions[category] = description;
    }

    void SmartHelpSystem::LoadBuiltinTopics() {
        // Add built-in help topics
        HelpTopic overview("overview", "Command System Overview", 
                          "The EVE-Server command system provides administrative tools for server management.",
                          HelpTopicType::Overview);
        overview.tags = {"overview", "introduction", "getting-started"};
        m_topicManager->AddTopic(overview);
        
        // Add more built-in topics...
    }

    void SmartHelpSystem::LoadCommandTopics() {
        // Create topics for registered commands
        for (const auto& pair : m_registeredCommands) {
            RegisterCommand(pair.first, pair.second);
        }
    }

    std::string SmartHelpSystem::GetCommandCategory(const std::string& commandName) const {
        // Infer category based on command name
        if (commandName.find("gm") == 0 || commandName.find("spawn") != std::string::npos ||
            commandName.find("give") != std::string::npos) {
            return "GM Commands";
        }
        if (commandName.find("debug") != std::string::npos || commandName.find("trace") != std::string::npos) {
            return "Debug Commands";
        }
        if (commandName.find("server") != std::string::npos || commandName.find("status") != std::string::npos) {
            return "System Commands";
        }
        return "General Commands";
    }

    std::vector<std::string> SmartHelpSystem::GetCommandsInCategory(const std::string& category) const {
        auto it = m_commandCategories.find(category);
        return (it != m_commandCategories.end()) ? it->second : std::vector<std::string>();
    }

    float SmartHelpSystem::CalculateCommandSimilarity(const std::string& cmd1, const std::string& cmd2) const {
        // Simple edit distance similarity calculation
        size_t len1 = cmd1.length();
        size_t len2 = cmd2.length();
        
        if (len1 == 0) return len2 == 0 ? 1.0f : 0.0f;
        if (len2 == 0) return 0.0f;
        
        std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));
        
        for (size_t i = 0; i <= len1; ++i) dp[i][0] = i;
        for (size_t j = 0; j <= len2; ++j) dp[0][j] = j;
        
        for (size_t i = 1; i <= len1; ++i) {
            for (size_t j = 1; j <= len2; ++j) {
                int cost = (cmd1[i-1] == cmd2[j-1]) ? 0 : 1;
                dp[i][j] = std::min({dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1] + cost});
            }
        }
        
        int editDistance = dp[len1][len2];
        int maxLen = std::max(len1, len2);
        
        return 1.0f - (float)editDistance / maxLen;
    }

    std::vector<std::string> SmartHelpSystem::GenerateTypoSuggestions(const std::string& input) const {
        std::vector<std::pair<std::string, float>> candidates;
        
        for (const auto& pair : m_registeredCommands) {
            float similarity = CalculateCommandSimilarity(input, pair.first);
            if (similarity > 0.6f) {
                candidates.emplace_back(pair.first, similarity);
            }
        }
        
        std::sort(candidates.begin(), candidates.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::vector<std::string> suggestions;
        for (const auto& candidate : candidates) {
            suggestions.push_back(candidate.first);
            if (suggestions.size() >= 3) break;
        }
        
        return suggestions;
    }

    std::string SmartHelpSystem::GenerateDidYouMean(const std::string& input) const {
        auto suggestions = GenerateTypoSuggestions(input);
        if (suggestions.empty()) {
            return "";
        }
        
        std::string result = "/" + suggestions[0];
        if (suggestions.size() > 1) {
            result += " or /" + suggestions[1];
        }
        
        return result;
    }

    // HelpSystemFactory implementation
    std::unique_ptr<SmartHelpSystem> HelpSystemFactory::CreateDefault() {
        auto helpSystem = std::make_unique<SmartHelpSystem>();
        helpSystem->Initialize();
        return helpSystem;
    }

    std::unique_ptr<IHelpGenerator> HelpSystemFactory::CreateGenerator(const std::string& type) {
        if (type == "standard" || type.empty()) {
            return std::make_unique<StandardHelpGenerator>();
        }
        // Can add other types of generators
        return std::make_unique<StandardHelpGenerator>();
    }

    std::unique_ptr<HelpTopicManager> HelpSystemFactory::CreateTopicManager() {
        return std::make_unique<HelpTopicManager>();
    }

} // namespace Commands