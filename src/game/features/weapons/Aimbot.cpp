#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Players.hpp"
#include "game/features/Features.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	//Needs testing
	class Aimbot : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		static constexpr int headBone = 21030;
		static constexpr int neckBone = 14283;

		Player player_nearest_center;

		Player get_player_nearest_center()
		{
			float min_distance    = 0.10f; // Change this to whatever we want our scan radius threshold to be
			Player nearest_player = nullptr;

			for (auto& [id, player] : Players::GetPlayers())
			{
				// Skip self and players that are not visible
				if (player == Self::Id || !player.GetId() || !player.GetPed().IsAlive()
				    || !ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(Self::PlayerPed, player.GetPed().GetHandle(), 17))
					continue;



				Vector3 cam_pos    = CAM::GET_GAMEPLAY_CAM_COORD();
				Vector3 player_pos = ENTITY::GET_ENTITY_COORDS(player.GetPed().GetHandle(), true, true);

				float distance_from_camera = sqrt(
				    pow(cam_pos.x - player_pos.x, 2) + pow(cam_pos.y - player_pos.y, 2) + pow(cam_pos.z - player_pos.z, 2));

				// Skip players that are too far away
				if (distance_from_camera > 800.0f)
					continue;

				Vector2 screen = {0.0f, 0.0f};

				float _pos[3] = {player_pos.x, player_pos.y, player_pos.z};
				Pointers.WorldToScreen(_pos, &screen.x, &screen.y);

				float distance_from_center = sqrt(pow(screen.x - 0.5f, 2) + pow(screen.y - 0.5f, 2));

				if (distance_from_center < min_distance)
				{
					min_distance   = distance_from_center;
					nearest_player = player;
				}
			}
			return nearest_player;
		}

		virtual void OnTick() override
		{
			// If we've stopped free aiming, no more aimbot
			if (!PLAYER::IS_PLAYER_FREE_AIMING(Self::Id))
			{
				player_nearest_center = nullptr;
				return;
			}

			// If we have no nearest player, find one (target acquisition)
			if (player_nearest_center == nullptr)
			{
				player_nearest_center = get_player_nearest_center();
			}

			// If the nearest player is dead, invisible, or not valid, clear them
			if (!player_nearest_center.GetId() || !player_nearest_center.GetPed().IsAlive()
			    || !ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(Self::PlayerPed, player_nearest_center.GetPed().GetHandle(), 17))
			{
				player_nearest_center = nullptr;
				return;
			}

			// By now we should have the target in player_nearest_center

			// Get coordinates of player's head bone
			Vector3 head_bone_coords = PED::GET_PED_BONE_COORDS(player_nearest_center.GetPed().GetHandle(), headBone, 0.0f, 0.0f, 0.0f);
			Vector3 game_cam_coords = CAM::GET_GAMEPLAY_CAM_COORD();

			Vector3 self_velocity   = ENTITY::GET_ENTITY_VELOCITY(Self::PlayerPed, 1);
			Vector3 player_velocity = ENTITY::GET_ENTITY_VELOCITY(player_nearest_center.GetPed().GetHandle(), 1);

			game_cam_coords  = game_cam_coords + self_velocity * 0.01;
			head_bone_coords = head_bone_coords + player_velocity * 0.01;

			Vector3 cam_to_target = head_bone_coords - game_cam_coords;

			constexpr float RADPI = 57.295779513082320876798154814105f;
			float magnitude       = std::hypot(cam_to_target.x, cam_to_target.y, cam_to_target.z);
			
			float camera_heading  = atan2f(cam_to_target.x, cam_to_target.y) * RADPI;
			float camera_pitch = asinf(cam_to_target.z / magnitude) * RADPI;

			float self_heading = ENTITY::GET_ENTITY_HEADING(Self::PlayerPed);
			float self_pitch   = ENTITY::GET_ENTITY_PITCH(Self::PlayerPed);

			// Adjust camera heading for R* natives
			if (camera_heading >= 0.0f && camera_heading <= 180.0f)
			{
				camera_heading = 360.0f - camera_heading;
			}
			else if (camera_heading <= -0.0f && camera_heading >= -180.0f)
			{
				camera_heading = -camera_heading;
			}

			CAM::SET_GAMEPLAY_CAM_RELATIVE_HEADING(camera_heading - self_heading, 1.0f);
			CAM::SET_GAMEPLAY_CAM_RELATIVE_PITCH(camera_pitch - self_pitch, 1.0f);
		}
	};

	static Aimbot _Aimbot{"aimbot", "Aimbot", "Auto aim at enemies"};
}