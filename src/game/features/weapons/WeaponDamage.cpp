#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Players.hpp"
#include "game/features/Features.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	class WeaponDamage : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			if (!Self::PlayerPed || PED::IS_PED_DEAD_OR_DYING(Self::PlayerPed, true) || ENTITY::IS_ENTITY_DEAD(Self::PlayerPed))
			{
				PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(Self::PlayerPed, 10.0f);
				return;
			}
		}

		virtual void OnDisable() override
		{
			PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(Self::PlayerPed, 1.0f);
		}
	};

	static WeaponDamage _WeaponDamage{"weapondamage", "Weapon Damage Multiplier", "Increases weapon damage 10x"};
}