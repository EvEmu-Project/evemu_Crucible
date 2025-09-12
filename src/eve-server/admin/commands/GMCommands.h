#ifndef __GMCOMMANDS_H_INCL__
#define __GMCOMMANDS_H_INCL__

#include "admin/commands/ICommand.h"
#include "admin/commands/CommandManager.h"

namespace Commands {

    // GM command registrar
    class GMCommandRegistrar : public CommandRegistrarBase<GMCommandRegistrar> {
    public:
        void DoRegisterCommands(CommandManager& manager);
    };

    // Search command
    class SearchCommand : public CommandBase<SearchCommand> {
    public:
        SearchCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void SearchItems(const CommandContext& context, const std::string& searchTerm, CommandResult& result);
        void SearchCharacters(const CommandContext& context, const std::string& searchTerm, CommandResult& result);
        void SearchSystems(const CommandContext& context, const std::string& searchTerm, CommandResult& result);
    };

    // Give ISK command
    class GiveISKCommand : public CommandBase<GiveISKCommand> {
    public:
        GiveISKCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool ValidateAmount(double amount) const;
        Client* FindTargetClient(const std::string& characterName) const;
    };

    // Spawn item command
    class SpawnCommand : public CommandBase<SpawnCommand> {
    public:
        SpawnCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool ValidateItemType(uint32 typeID) const;
        bool ValidateQuantity(uint32 quantity) const;
        InventoryItemRef CreateItem(uint32 typeID, uint32 quantity, Client* client) const;
    };

    // Set attribute command
    class SetAttributeCommand : public CommandBase<SetAttributeCommand> {
    public:
        SetAttributeCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool ValidateAttributeID(uint32 attributeID) const;
        bool ValidateAttributeValue(double value) const;
        Client* FindTargetClient(const std::string& characterName) const;
    };

    // Teleport command
    class TeleportCommand : public CommandBase<TeleportCommand> {
    public:
        TeleportCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool ValidateCoordinates(double x, double y, double z) const;
        bool ValidateSystemID(uint32 systemID) const;
        SystemManager* GetSystemManager(uint32 systemID) const;
    };

    // Kick player command
    class KickCommand : public CommandBase<KickCommand> {
    public:
        KickCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        Client* FindTargetClient(const std::string& characterName) const;
        void DisconnectClient(Client* client, const std::string& reason) const;
    };

    // Mute command
    class MuteCommand : public CommandBase<MuteCommand> {
    public:
        MuteCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        Client* FindTargetClient(const std::string& characterName) const;
        bool ValidateDuration(uint32 minutes) const;
        void ApplyMute(Client* client, uint32 minutes, const std::string& reason) const;
    };

    // Server info command
    class ServerInfoCommand : public CommandBase<ServerInfoCommand> {
    public:
        ServerInfoCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        std::string GetServerUptime() const;
        std::string GetPlayerCount() const;
        std::string GetMemoryUsage() const;
        std::string GetSystemLoad() const;
    };

    // Reload config command
    class ReloadConfigCommand : public CommandBase<ReloadConfigCommand> {
    public:
        ReloadConfigCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        bool ReloadServerConfig() const;
        bool ReloadDatabaseConfig() const;
        bool ReloadLogConfig() const;
    };

    // Broadcast message command
    class BroadcastCommand : public CommandBase<BroadcastCommand> {
    public:
        BroadcastCommand();
        CommandResult DoExecute(const CommandContext& context) override;
        
    private:
        void SendBroadcast(const std::string& message, const std::string& sender) const;
        bool ValidateMessage(const std::string& message) const;
    };

} // namespace Commands

#endif // __GMCOMMANDS_H_INCL__