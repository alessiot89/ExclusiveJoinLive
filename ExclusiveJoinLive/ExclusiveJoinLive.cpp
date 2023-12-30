#include <API/ARK/Ark.h>
#pragma comment(lib, "ArkApi.lib")

#include <fstream>
#include <unordered_set>


DECLARE_HOOK( AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin,
              bool,
              AShooterGameMode*,
              AShooterPlayerController* );

void readIds( std::unordered_set<std::string>& playersIDs )
{
    // EOS account IDs are 19 digits, steam  usually 17 digits (16 at least!).
    std::string line{};
    line.reserve( 19 );
    // Read exclusive list file
    std::ifstream playersExclusiveJoinList( "PlayersExclusiveJoinList.txt" );
    while( getline( playersExclusiveJoinList, line ) )
    {
        if( line.size() )
        {
            playersIDs.insert( line );
        }
    }
    playersExclusiveJoinList.close();
}

void writeIDs( std::unordered_set<std::string>& playersIDs )
{
    std::ofstream playersExclusiveJoinList( "PlayersExclusiveJoinList.txt", std::ios::out );
    for( const auto& id : playersIDs )
    {
        playersExclusiveJoinList << id << std::endl;
    }
    playersExclusiveJoinList.close();
}

bool hook_AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin( AShooterGameMode* a_shooter_game_mode,
                                                                     AShooterPlayerController* ForPlayer )
{
    if( ForPlayer )
    {
        try
        {
            // Get account ARK ID
            FString result;
            ForPlayer->PlayerStateField()->UniqueIdField().UniqueNetId->ToHumanReadableString( &result );
            std::string playerIdStr = result.ToString();
            // EOS account IDs are 19 digits, steam 17 digits.
            std::string line{};
            line.reserve( 19 );
            // Read exclusive list file
            std::unordered_set<std::string> playersIds;
            readIds( playersIds );
            if( playersIds.find( playerIdStr ) != playersIds.end() )
            {
                return true;
            }
        }
        catch( ... )
        {
            // Just to avoid crash if there is a lock or an issue on file read
        }
    }
    return false;
    /*
    return AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin_original( a_shooter_game_mode,
                                                                               ForPlayer );
    */
}

namespace
{
    inline void wrongSyntaxAdd( AShooterPlayerController* player )
    {
        ArkApi::GetApiUtils().SendServerMessage( player,
                                                 FLinearColor( 1.0, 0, 0 ),
                                                 L"Incorrect Syntax: cheat ej.add <AccountID>" );
    }

    inline void wrongSyntaxRemove( AShooterPlayerController* player )
    {
        ArkApi::GetApiUtils().SendServerMessage( player,
                                                 FLinearColor( 1.0, 0, 0 ),
                                                 L"Incorrect Syntax: cheat ej.remove <AccountID>" );
    }
}

void addExclusiveId( APlayerController* player_controller,
                     FString* message,
                     bool )
{
    auto player = static_cast<AShooterPlayerController*>( player_controller );
    if( player && player->PlayerStateField() && player->GetPlayerCharacter() && player->bIsAdmin()( ) )
    {
        TArray<FString> parsed;
        message->ParseIntoArray( parsed,
                                 L" ",
                                 true );
        if( parsed.IsValidIndex( 1 ) )
        {
            try
            {
                // Read exclusive list file
                std::unordered_set<std::string> playersIds;
                readIds( playersIds );
                // Get account ARK ID
                auto arkID = std::to_string( std::stoull( parsed[1].ToString().c_str() ) );
                // Steam IDs have at least 16 digits (usually 17, EOS usually 19)
                if( arkID.length() >= 16 )
                {
                    playersIds.insert( arkID );
                    writeIDs( playersIds );
                }
                else
                {
                    ArkApi::GetApiUtils().SendServerMessage( player,
                                                             FLinearColor( 1.0, 0, 0 ),
                                                             L"Invalid AccountID" );
                }
            }
            catch( ... )
            {
            }
        }
        else
        {
            wrongSyntaxAdd( player );
        }
    }
}

void removeExclusiveId( APlayerController* player_controller,
                        FString* message,
                        bool )
{
    auto player = static_cast<AShooterPlayerController*>( player_controller );
    if( player && player->PlayerStateField() && player->GetPlayerCharacter() && player->bIsAdmin().Get() )
    {
        TArray<FString> parsed;
        message->ParseIntoArray( parsed,
                                 L" ",
                                 true );
        if( parsed.IsValidIndex( 1 ) )
        {
            try
            {
                // Read exclusive list file
                std::unordered_set<std::string> playersIds;
                readIds( playersIds );
                // Get account ARK ID
                // Get account ARK ID
                auto arkID = std::to_string( std::stoull( parsed[1].ToString().c_str() ) );
                // Steam IDs have at least 16 digits (usually 17, EOS usually 19)
                if( arkID.length() >= 16 )
                {
                    playersIds.erase( arkID );
                    writeIDs( playersIds );
                }
                else
                {
                    ArkApi::GetApiUtils().SendServerMessage( player,
                                                             FLinearColor( 1.0, 0, 0 ),
                                                             L"Invalid AccountID" );
                }
            }
            catch( ... )
            {
            }
        }
        else
        {
            wrongSyntaxAdd( player );
        }
    }
}

void load()
{
    Log::Get().Init( "Exclusive Join Live" );

    // register hook
    auto& hooks = ArkApi::GetHooks();
    hooks.SetHook( "AShooterGameMode.IsPlayerControllerAllowedToExclusiveJoin",
                   &hook_AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin,
                   &AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin_original );

    ArkApi::GetCommands().AddConsoleCommand( "ej.add",
                                             &addExclusiveId );
    ArkApi::GetCommands().AddConsoleCommand( "ej.remove",
                                             &removeExclusiveId );
}

void unload()
{
    ArkApi::GetHooks().DisableHook( "AShooterGameMode.IsPlayerControllerAllowedToExclusiveJoin",
                                    &hook_AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin );

    ArkApi::GetCommands().RemoveConsoleCommand( "ej.add" );
    ArkApi::GetCommands().RemoveConsoleCommand( "ej.remove" );
}

BOOL APIENTRY DllMain( HINSTANCE /*hinstDLL*/,
                       DWORD fdwReason,
                       LPVOID /*lpvReserved*/ )
{
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        load();
        break;
    case DLL_PROCESS_DETACH:
        unload();
        break;
    }
    return TRUE;
}
