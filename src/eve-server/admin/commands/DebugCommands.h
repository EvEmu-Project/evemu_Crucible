#ifndef __DEBUGCOMMANDS_H_INCL__
#define __DEBUGCOMMANDS_H_INCL__

#include "admin/commands/ICommand.h"
#include "admin/commands/CommandManager.h"

namespace Commands {

    // Debug command registrar
    class DebugCommandRegistrar : public CommandRegistrarBase<DebugCommandRegistrar> {
    public:
        void DoRegisterCommands(CommandManager& manager);
    };

    // Packet debug command
    class PacketDebugCommand : public CommandBase<PacketDebugCommand> {
    public:
        PacketDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void EnablePacketLogging(const std::string& packetType) const;
        void DisablePacketLogging(const std::string& packetType) const;
        std::string GetPacketStats() const;
        bool ValidatePacketType(const std::string& packetType) const;
    };

    // Memory debug command
    class MemoryDebugCommand : public CommandBase<MemoryDebugCommand> {
    public:
        MemoryDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetMemoryLeaks() const;
        std::string GetMemoryPools() const;
        std::string GetAllocationStats() const;
        void DumpMemoryMap() const;
    };

    // Database debug command
    class DBDebugCommand : public CommandBase<DBDebugCommand> {
    public:
        DBDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void EnableQueryLogging() const;
        void DisableQueryLogging() const;
        std::string GetSlowQueries() const;
        std::string GetConnectionStats() const;
        void AnalyzeQuery(const std::string& query) const;
    };

    // Network debug command
    class NetworkDebugCommand : public CommandBase<NetworkDebugCommand> {
    public:
        NetworkDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetConnectionList() const;
        std::string GetBandwidthStats() const;
        std::string GetLatencyStats() const;
        void TraceConnection(uint32 clientID) const;
    };

    // Service debug command
    class ServiceDebugCommand : public CommandBase<ServiceDebugCommand> {
    public:
        ServiceDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetServiceList() const;
        std::string GetServiceStats(const std::string& serviceName) const;
        void RestartService(const std::string& serviceName) const;
        bool ValidateServiceName(const std::string& serviceName) const;
    };

    // Item debug command
    class ItemDebugCommand : public CommandBase<ItemDebugCommand> {
    public:
        ItemDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetItemInfo(uint32 itemID) const;
        std::string GetItemHierarchy(uint32 itemID) const;
        std::string GetOrphanedItems() const;
        void ValidateItemIntegrity() const;
    };

    // Character debug command
    class CharacterDebugCommand : public CommandBase<CharacterDebugCommand> {
    public:
        CharacterDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetCharacterInfo(const std::string& characterName) const;
        std::string GetCharacterSkills(const std::string& characterName) const;
        std::string GetCharacterAttributes(const std::string& characterName) const;
        void ValidateCharacterData(const std::string& characterName) const;
    };

    // Market debug command
    class MarketDebugCommand : public CommandBase<MarketDebugCommand> {
    public:
        MarketDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetMarketOrders(uint32 regionID) const;
        std::string GetMarketHistory(uint32 typeID) const;
        std::string GetMarketStats() const;
        void ValidateMarketData() const;
    };

    // System debug command
    class SystemDebugCommand : public CommandBase<SystemDebugCommand> {
    public:
        SystemDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetSystemInfo(uint32 systemID) const;
        std::string GetSystemObjects(uint32 systemID) const;
        std::string GetSystemPlayers(uint32 systemID) const;
        void ValidateSystemData(uint32 systemID) const;
    };

    // Event debug command
    class EventDebugCommand : public CommandBase<EventDebugCommand> {
    public:
        EventDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void EnableEventLogging(const std::string& eventType) const;
        void DisableEventLogging(const std::string& eventType) const;
        std::string GetEventHistory() const;
        std::string GetEventStats() const;
    };

    // Cache debug command
    class CacheDebugCommand : public CommandBase<CacheDebugCommand> {
    public:
        CacheDebugCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetCacheStats() const;
        void ClearCache(const std::string& cacheType) const;
        void RefreshCache(const std::string& cacheType) const;
        bool ValidateCacheType(const std::string& cacheType) const;
    };

    // Profiling command
    class ProfileCommand : public CommandBase<ProfileCommand> {
    public:
        ProfileCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void StartProfiling(const std::string& target) const;
        void StopProfiling() const;
        std::string GetProfilingResults() const;
        bool ValidateProfilingTarget(const std::string& target) const;
    };

} // namespace Commands

#endif // __DEBUGCOMMANDS_H_INCL__