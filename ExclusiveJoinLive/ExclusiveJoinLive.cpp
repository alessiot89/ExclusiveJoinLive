#include <API/ARK/Ark.h>
#pragma comment(lib, "ArkApi.lib")

#include <fstream>


DECLARE_HOOK( AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin,
              bool,
              AShooterGameMode*,
              AShooterPlayerController* );

bool Hook_AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin( AShooterGameMode* a_shooter_game_mode,
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
            std::ifstream playersExclusiveJoinList( "PlayersExclusiveJoinList.txt" );
            while( getline( playersExclusiveJoinList, line ) )
            {
                if( !std::strcmp( line.c_str(), playerIdStr.c_str() ) )
                {
                    return true;
                }
            }
        }
        catch( const std::exception& )
        {
            // Just to avoid crash if thre is a lock or an issue on file read
        }
    }
    return false;
    /*
    return AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin_original( a_shooter_game_mode,
                                                                               ForPlayer );
    */
}

void load()
{
    Log::Get().Init( "ASE_TEST" );

    // register hook
    auto& hooks = ArkApi::GetHooks();
    hooks.SetHook( "AShooterGameMode.IsPlayerControllerAllowedToExclusiveJoin",
                   &Hook_AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin,
                   &AShooterGameMode_IsPlayerControllerAllowedToExclusiveJoin_original );
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
        break;
    }
    return TRUE;
}
