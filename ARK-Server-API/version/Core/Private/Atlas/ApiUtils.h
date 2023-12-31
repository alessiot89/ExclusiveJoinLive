#pragma once

#include <IApiUtils.h>

namespace AtlasApi
{
	class ApiUtils : public ArkApi::IApiUtils
	{
	public:
		ApiUtils() = default;

		ApiUtils(const ApiUtils&) = delete;
		ApiUtils(ApiUtils&&) = delete;
		ApiUtils& operator=(const ApiUtils&) = delete;
		ApiUtils& operator=(ApiUtils&&) = delete;

		~ApiUtils() override = default;

		UWorld* GetWorld() const override;
		AShooterGameMode* GetShooterGameMode() const override;
		ArkApi::ServerStatus GetStatus() const override;
		UShooterCheatManager* GetCheatManager() const override;

		void SetWorld(UWorld* uworld);
		void SetShooterGameMode(AShooterGameMode* shooter_game_mode);
		void SetStatus(ArkApi::ServerStatus status);
		void SetCheatManager(UShooterCheatManager* cheatmanager);

		AShooterPlayerController* FindPlayerFromSteamId_Internal(uint64 steam_id) const override;
		void SetPlayerController(AShooterPlayerController* player_controller);
		void RemovePlayerController(AShooterPlayerController* player_controller);

	private:
		UWorld* u_world_{nullptr};
		AShooterGameMode* shooter_game_mode_{nullptr};
		ArkApi::ServerStatus status_{0};
		UShooterCheatManager* cheatmanager_{ nullptr };
		std::unordered_map<uint64, AShooterPlayerController*> steam_id_map_;
	};
} // namespace AtlasApi
