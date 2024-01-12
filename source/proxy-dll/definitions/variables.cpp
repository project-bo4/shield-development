#include <std_include.hpp>
#include "variables.hpp"
#include "component/hashes.hpp"

namespace fnv1a
{
	uint64_t generate_hash(const char* string, uint64_t start)
	{
		uint64_t res = start;

		for (const char* c = string; *c; c++)
		{
			if (*c == '\\')
			{
				res ^= '/';
			}
			else
			{
				res ^= tolower(*c);
			}

			res *= 0x100000001B3;
		}

		uint64_t val = res & 0x7FFFFFFFFFFFFFFF;

		if (start == 0xCBF29CE484222325)
		{
			hashes::add_hash(val, string);
		}

		return val;
	}

	uint64_t generate_hash_pattern(const char* string)
	{
		std::string_view v{ string };

		// basic notations hash_123, file_123, script_123
		if (!v.rfind("hash_", 0)) return std::strtoull(&string[5], nullptr, 16) & 0x7FFFFFFFFFFFFFFF;
		if (!v.rfind("file_", 0)) return std::strtoull(&string[5], nullptr, 16) & 0x7FFFFFFFFFFFFFFF;
		if (!v.rfind("script_", 0)) return std::strtoull(&string[7], nullptr, 16) & 0x7FFFFFFFFFFFFFFF;

		// lua notation x64:123
		if (!v.rfind("x64:", 0))
		{
			if (v.length() <= 0x18 && v.ends_with(".lua"))
			{
				// x64:123456789abcdf.lua
				// 
				// extract hash value
				char tmpbuffer[0x17] = {};

				memcpy(tmpbuffer, string + 4, v.length() - 8);

				// gen the hash and add .lua add the end
				return generate_hash(".lua", std::strtoull(&string[4], nullptr, 16) & 0x7FFFFFFFFFFFFFFF);
			}

			return std::strtoull(&string[4], nullptr, 16) & 0x7FFFFFFFFFFFFFFF;
		}

		// unknown, use hashed value
		return generate_hash(string);
	}
}

namespace variables
{
	std::vector<varEntry> dvars_record =
	{
		{
			"aim_slowdown_enabled",
			"Slowdown the turn rate when the cross hair passes over a target",
			0x3D607772590C64FC
		},
		{
			"aim_lockon_enabled",
			"Aim lock on helps the player to stay on target",
			0xD3FCDCA2CD83783
		},
		{
			"arena_defaultPlaylist",
			"Default Arena Playlist",
			0x7D60181470E5E8E8
		},
		{
			"arena_seasonOverride",
			"Arena Season Override",
			0x4810417D791DD7F8
		},
		{
			"arena_enableListenHosting",
			"Enable listen servers for arena matchmaking",
			0x181DF5AA7D92095
		},
		{
			"arena_qosSkillStart",
			"Base arena point skill range, regardless of search duration",
			0x35A4B7E98B04250D
		},
		{
			"arena_qosBrodenSkillStep",
			"Arena points per interval to increase the distance between the skill and the qos skill",
			0x44A22EC68E0798C5
		},
		{
			"arena_qosBrodenSkillFrequency",
			"Frequency of which the the skill step is increased linearly",
			0x75215E81615C6A0F
		},
		{
			"arena_minHostSkillRange",
			"Skill range from client's arena skill that must be qos'd before hosting lobby or unparking dedi",
			0x2A5FFA081C299437
		},
		{
			"arena_giveBonusStar",
			"When enabled, one bonus star will be awarded every n consecutive wins.",
			0x3F960938DC77B97F
		},
		{
			"arena_bonusStarStreak",
			"Win streak required before awarding bonus star (if enabled)",
			0x16B8856172F03E5E
		},
		{
			"arena_minPlayTime",
			"Time in seconds a player must play an arena match for a loss to count.",
			0x256BA16072C5A454
		},
		{
			"arena_maintenance",
			"When accessing Arena mode, display a maintenance message.",
			0x48972F7E0A188F50
		},
		{
			"arena_lobbyReloadSearchDelayMin",
			"Minimum time in seconds to delay the search for a new arena lobby after a match.",
			0x6ED9351EBCEE08C7
		},
		{
			"arena_lobbyReloadSearchDelayMax",
			"Maximum time in seconds to delay the search for a new arena lobby after a match.",
			0x6EF5231EBD0664E9
		},
		{
			"arena_enableArenaChallenges",
			"Enable arena challenges including Season Vet and Best Rank.",
			0x2E3EEB19A9EE340B
		},
		{
			"arena_seasonVetChallengeWins",
			"Number of arena wins in a season to complete a season vet challenge.",
			0x2F56FC304D6E61EB
		},
		{
			"arena_unfairTeamGap",
			"This is the gap where if a team is down this many players compaired with the other team then the pentalty is waved.",
			0x170F0BEC873AC2D3
		},
		{
			"bg_movingPlatformPitchScale",
			"The scale to apply to pitch from the moving platform that should be applied to the player's pitch",
			0x29FD42871E7EAED2
		},
		{
			"player_view_pitch_up",
			"Maximum angle that the player can look up",
			0x7ED57F474B124FEF
		},
		{
			"player_view_pitch_down",
			"Maximum angle that the player can look down",
			0xDE7DB9DD6AC744A
		},
		{
			"player_view_swim_pitch_up",
			"Maximum angle that the player can look up while swimming",
			0x60B713C5A6FCC60C
		},
		{
			"player_view_swim_pitch_down",
			"Maximum angle that the player can look up while swimming",
			0x3997DA36B5C98979
		},
		{
			"player_lean_shift",
			"Amount to shift the player 3rd person model when leaning(x:left, y:right)",
			0xB8EF752D3C087DE
		},
		{
			"player_lean_shift_crouch",
			"Amount to shift the player 3rd person model when crouch leaning(x:left, y:right)",
			0x671B0241FC57FA9
		},
		{
			"player_lean_rotate",
			"Amount to rotate the player 3rd person model when leaning(x:left, y:right)",
			0x4E6970FC488D8BBD
		},
		{
			"player_lean_rotate_crouch",
			"Amount to rotate the player 3rd person model when crouch leaning(x:left, y:right)",
			0x293B5F04CFDD9DAC
		},
		{
			"bg_prone_yawcap",
			"The maximum angle that a player can look around quickly while prone",
			0x1E0BF5B71128804D
		},
		{
			"bg_aimSpreadMoveSpeedThreshold",
			"When player is moving faster than this speed, the aim spread will increase",
			0x25A81EC42CBDFDB4
		},
		{
			"bg_maxGrenadeIndicatorSpeed",
			"Maximum speed of grenade that will show up in indicator and can be thrown back.",
			0x51A6D8B45C526193
		},
		{
			"player_scopeExitOnDamage",
			"Exit the scope if the player takes damage",
			0x2B90906C1300936B
		},
		{
			"player_sustainAmmo",
			"Firing weapon will not decrease clip ammo.",
			0x47C5EB83E44DCFC4
		},
		{
			"player_clipSizeMultiplier",
			"Changes the clip size of weapons with more than one bullet in their clip.",
			0x68CB46E1460CAB15
		},
		{
			"player_lastStandSuicideDelay",
			"The amount of time that must pass before the player is allowed to suicide",
			0x1038D42009D25460
		},
		{
			"player_sprintTime",
			"The base length of time a player can sprint",
			0x2C0C64DDCFC802FC
		},
		{
			"bg_gravity",
			"Gravity in inches per second per second",
			0x30B9B1B37A543E43
		},
		{
			"bg_lowGravity",
			"Low gravity for slow or floaty objects, in inches per second per second",
			0x2652ABDA75F57CA3
		},
		{
			"bg_moonGravity",
			"Gravity on the moon, in inches per second per second",
			0x7587080AF570DF78
		},
		{
			"bg_waterGravity",
			"Gravity for missiles while in water that use tr_watergravity",
			0x5C5088A687D0F420
		},
		{
			"player_viewLockEnt",
			"Set an entity that the player view will be locked too.",
			0x3F017ABABB9075D4
		},
		{
			"cg_isGameplayActive",
			"",
			0x437875777AEF3616
		},
		{
			"vehLockTurretToPlayerView",
			"Locks the turret angles to the player angles and sets the players rotation speed to the turrets rotRate",
			0x443BAC9ED2A2F4
		},
		{
			"mp_blackjack_consumable_wait",
			"For Blackjack, the time to wait between updating consumable time.",
			0x7A9105508D8C784D
		},
		{
			"tu11_enableClassicMode",
			"",
			0x11732EEF735B5749
		},
		{
			"tu11_enableVehicleMode",
			"",
			0x736B3744DE946B29
		},
		{
			"bg_shieldHitEncodeWidthWorld",
			"The encoding range, in width, of a client's world shield.  A hit in this range is encoded into one of 16 collumns.",
			0x57A8E17675CBFE41LL
		},
		{
			"bg_shieldHitEncodeHeightWorld",
			"The encoding range, in height, of a client's world shield.  A hit in this range is encoded into one of 8 rows.",
			0x5D1E511BDF96E56ALL
		},
		{
			"bg_shieldHitEncodeWidthVM",
			"The decoding range, in width, of a client's viewmodel shield.",
			0xC5F06C878CFD2FE
		},
		{
			"bg_shieldHitEncodeHeightVM",
			"The decoding range, in height, of a client's viewmodel shield.",
			0x5CFD635A296ED3C7
		},
		{
			"bg_disableWeaponPlantingInWater",
			"Disables being able to plant mines in the water.",
			0x7D4734AAB01CCAC0
		},
		{
			"disable_rope",
			"Disables the rope system",
			0x589543C5CA4AA0B0
		},
		{
			"waterbrush_entity",
			"CM_GetWaterHeight function will test against this enitty. Can be used on brushmodels to move the water level",
			0x6D05F3E31E627502
		},
		{
			"playerPushAmount",
			"If this value is set the player will get pushed away from AIs by the amount specified.",
			0x536C6409B3369248
		},
		{
			"bg_serverDelayDamageKickForPing",
			"Turn on delayed damage kick on server to allow for client ping",
			0x3CB742BC7C5039B3
		},
		{
			"bg_useClientDamageKick",
			"Turn on to make the server use the damage kick angles from the client usercmd",
			0x66718A7BBE48B522
		},
		{
			"slam_enabled",
			"Enable slam activation",
			0x3DF86A5C80DE5AFA
		},
		{
			"playerWeaponRaisePostIGC",
			"Temp dvar to give script control over which wepaon anim plays after an IGC",
			0x48B28EA7A73F4D7F
		},
		{
			"bg_allowPlayerRoleTemplateButtonOverrides",
			"When true the PlayerRoleTemplates are allowed to define button overrides per the enum PlayerRoleButtonOverrideTypes.",
			0x39DAA8BF95AE47A0
		},
		{
			"bg_aqs",
			"",
			0x28A5EA43BB11E71A
		},
		{
			"bg_aqsStyle",
			"",
			0x629275EC97C1ED2F
		},
		{
			"bg_ads",
			"",
			0x285DEA43BAD43177
		},
		{
			"bg_bobcycleResetThreshold",
			"The difference between the new and old bob cycle before we treat as if the server reset its bob cycle.",
			0x78821CC8458682CE
		},
		{
			"adsZeroSpread",
			"Immediately zero spreadAmount when fully ADS",
			0x630AAA3ED87A2CD0
		},
		{
			"bg_deferScriptMissileDetonation",
			"Defer G_ExplodeMissile() till after origin is updated in G_RunMissile()",
			0x9AC21A6A320098B
		},
		{
			"bg_isolateDamageFlash",
			"Drive damage flash overlay from isolated variable instead of view kick pitch",
			0x5C7202461031C541
		},
		{
			"bg_limitGrenadeImpacts",
			"Use missileTargetEnt to limit impact damage to the first impact",
			0x1B4BC4AC24D6A8B7
		},
		{
			"bg_zombiePlayerUsesUtilityClip",
			"Use utility clip for zombie player movement",
			0x4C3B22F554427952
		},
		{
			"bg_warmode_version",
			"Temp dvar to control the war mode gametype",
			0x62CE61CC049BBAF
		},
		{
			"gadgetPowerOverrideFactor",
			"Override power factor",
			0x6358FE7CC1E358D9
		},
		{
			"gadgetPowerOverchargePerkTimeFactor",
			"Overcharge perk charge over time multiplier",
			0x72F867BAEAFFED96
		},
		{
			"gadgetPowerOverchargePerkScoreFactor",
			"Overcharge perk score multiplier",
			0x342CA843EBD8A6D9
		},
		{
			"gadgetThiefShutdownFullCharge",
			"If this is set to true then the users will recieve a full gadget charge if they shutdown an enemy. ",
			0x7C3412AD9BC26114
		},
		{
			"gadget_force_slots",
			"Force the gadget slots to be in a defined order: primary, secondary, ability, heavy",
			0x1E1071C9F9971C53
		},
		{
			"mm_keyframeUsageAnimIndex",
			"anim to watch usage graph of",
			0x58BA8669CAF26406
		},
		{
			"cg_minimapPadding",
			"The amount of padding to apply to the circular minimap",
			0x61C6A72FF051961C
		},
		{
			"shoutcastHighlightedClient",
			"Shoutcaster's currently highlighted clientNum.",
			0x69E29B87C17AED9
		},
		{
			"cg_hudMapFriendlyWidth",
			"The size of the friendly icon on the full map",
			0x7F519E0BEE1AB1CA
		},
		{
			"cg_hudMapFriendlyHeight",
			"The size of the friendly icon on the full map",
			0x4619BD1BC36D8445
		},
		{
			"cg_hudMapPlayerWidth",
			"The size of the player's icon on the full map",
			0x6178FDF4AAD2F55C
		},
		{
			"cg_hudMapPlayerHeight",
			"The size of the player's icon on the full map",
			0x24B133C70747FCEB
		},
		{
			"waypointIconWidth",
			"Width of the offscreen pointer.",
			0x7D914A08D198D4D9
		},
		{
			"waypointIconHeight",
			"Height of the offscreen pointer.",
			0x7E8A5C6D93107AC8
		},
		{
			"waypointOffscreenPointerDistance",
			"Distance from the center of the offscreen objective icon to the center its arrow.",
			0x678EB86FAC35A1BB
		},
		{
			"cg_threatDetectorRadius",
			"The radius of the threat detector scan.",
			0x6D04072BA075F624
		},
		{
			"cg_usingClientScripts",
			"True, if client scripts are enabled.",
			0x2651402F8E72FD35
		},
		{
			"cg_drawGun",
			"Draw the view model",
			0x21ECB143EDAE83C0
		},
		{
			"cg_weaponHintsCoD1Style",
			"Draw weapon hints in CoD1 style: with the weapon name, and with the icon below",
			0x386051E08C3DBFCC
		},
		{
			"cg_focalLength",
			"Lens focal length for 3-perf 16x9 super 35mm (24.89mm x 14mm)",
			0x76CB476AD96F6429
		},
		{
			"cg_fov",
			"The field of view angle in degrees",
			0x68E6BD38B3C5F133
		},
		{
			"cg_fovExtraCam",
			"The field of view angle in degrees for the extra cam",
			0x619179B3BFB57936
		},
		{
			"cg_useWeaponBasedVariableZoom",
			"Use weapon based variable zoom instead of player based.",
			0x5F0E3C836B0F7251
		},
		{
			"cg_viewVehicleInfluenceGunner",
			"The influence on the view from being a vehicle gunner",
			0x6CAE8CF76050DCCD
		},
		{
			"cg_viewVehicleInfluenceGunnerFiring",
			"The influence on the view from being a vehicle gunner while firing",
			0x2A04D21251DFF822LL
		},
		{
			"cg_viewVehicleInfluenceGunner_mode",
			"Controls when the viewVehicleInfluenceGunnerFiring is effective. 0 - only on ADS and firing; 1 - on ADS, firing, or moving camera; 2 - always on",
			0x292947732E98C3DD
		},
		{
			"cg_draw2D",
			"Draw 2D screen elements",
			0x578107FC9F13DAEC
		},
		{
			"cg_drawLagometer",
			"Draw lagometer",
			0x548B3E93CD81FC04
		},
		{
			"cg_drawFPS",
			"Draw frames per second",
			0x1A9A9543E9EA5C55
		},
		{
			"cg_drawMaterialImageNum",
			"Use up/down on the dpad to select an image when cg_drawMaterial is enabled.  Press right for more info on that image.",
			0x62201416DD316B1C
		},
		{
			"cg_drawMaterialImageName",
			"Do not use (set by cg_drawMaterialImageNum code)",
			0x1B45AFDA3D2F6497
		},
		{
			"cg_drawCrosshair",
			"Turn on weapon crosshair",
			0x28A680A206CE7AA
		},
		{
			"cg_drawCrosshairCooker",
			"Turn on cook indicator for cooked grenades. Use 1 for cookOffHoldTime grenades and 2 cookOffHoldTime and cookOffHold ones.",
			0x28B74F0EA64BBD81
		},
		{
			"cg_drawCrosshairNames",
			"Draw the name of an enemy under the crosshair",
			0x3DFF0D6F30875126
		},
		{
			"cg_hudGrenadeIconMaxRangeFrag",
			"The minimum distance that a grenade has to be from a player in order to be shown on the grenade indicator",
			0x7E10D8205D5FB7A3
		},
		{
			"cg_lagometer_pos",
			"lagometer position",
			0x2369C26464214397
		},
		{
			"cg_thirdPersonRange",
			"The range of the camera from the player in third person view",
			0x7721647E3B677041
		},
		{
			"cg_thirdPersonAngle",
			"The angle of the camera from the player in third person view",
			0x3B52B3027049DD27
		},
		{
			"cg_thirdPersonRoll",
			"The roll of the camera from the player in third person view",
			0x69BF0372ED5290BF
		},
		{
			"cg_thirdPersonSideOffset",
			"The side offset for the camera from the player in third person view",
			0x137F72894198596C
		},
		{
			"cg_thirdPersonUpOffset",
			"The up offset for the camera from the player in third person view",
			0x1E667A2FDB87697E
		},
		{
			"cg_thirdPersonFocusDist",
			"The distance infront of the player to aim the 3rd person camera at",
			0x4709868684DEF034
		},
		{
			"cg_thirdPersonFocusOffsetUp",
			"An offset to add to the position the camera is looking at",
			0x4AE8BBC003F5814A
		},
		{
			"cg_thirdPersonCamOffsetUp",
			"An offset to add to the camera position",
			0x69C3ABA3C8961E95
		},
		{
			"cg_thirdPersonCamLerpScale",
			"Lerp amount for the camera in 3rd person mode",
			0x38173B979832E468
		},
		{
			"cg_thirdPerson",
			"Use third person view",
			0x39575543F27BBB7C
		},
		{
			"cg_thirdPersonMode",
			"How the camera behaves in third person",
			0x17D6FA8AFA02D41F
		},
		{
			"cg_subtitleWidthWidescreen",
			"The width of the subtitle on a wide-screen",
			0x592E4EB48B2992B3
		},
		{
			"cg_headIconMinScreenRadius",
			"The minumum radius of a head icon on the screen",
			0x577DAAC9CC20E5DB
		},
		{
			"cg_overheadNamesSize",
			"The maximum size to show overhead names",
			0x18CD0DE6C7E17CD
		},
		{
			"cg_overheadIconSize",
			"The maximum size to show overhead icons like 'rank'",
			0x6A685E4DC639DB4C
		},
		{
			"cg_overheadRankSize",
			"The size to show rank text",
			0x13A81F9AA23A7593
		},
		{
			"cg_healthPerBar",
			"How much health is represented per health bar",
			0x442D42EFC73D739A
		},
		{
			"cg_drawFriendlyNames",
			"Whether to show friendly names in game",
			0x58DD8654FFC55717
		},
		{
			"cg_playerHighlightTargetSize",
			"Size of player target highlights.",
			0x20568A24245D86D3
		},
		{
			"cg_ScoresColor_Gamertag_0",
			"player gamertag color on scoreboard",
			0x66BEE54AB204EDA4
		},
		{
			"cg_ScoresColor_Gamertag_1",
			"player gamertag color on scoreboard",
			0x66BEE64AB204EF57
		},
		{
			"cg_ScoresColor_Gamertag_2",
			"player gamertag color on scoreboard",
			0x66BEE74AB204F10A
		},
		{
			"cg_ScoresColor_Gamertag_3",
			"player gamertag color on scoreboard",
			0x66BEE84AB204F2BD
		},
		{
			"hud_healthOverlay_pulseStart",
			"The percentage of full health at which the low-health warning overlay begins flashing",
			0x62097F9D5D1CCCD7
		},
		{
			"cg_drawTalk",
			"Controls which icons CG_TALKER ownerdraw draws",
			0x1CAE64EFA0B7AC5C
		},
		{
			"cg_drawJobsPerf",
			"draw jobs performance information",
			0x666AE70D31B8A985
		},
		{
			"flareDisableEffects",
			"",
			0xA05647541297479
		},
		{
			"showVisionSetDebugInfo",
			"Enables visionset debug info",
			0x3109253A0366F337
		},
		{
			"cl_smoothSnapInterval",
			"Length of the buffer smoothing the snap interval when adjusting the time delta. Value must be a power of 2.",
			0x61D822229C373508
		},
		{
			"con_typewriterColorBase",
			"Base color of typewritten objective text.",
			0x43271A84C67417BF
		},
		{
			"cl_deathMessageWidth",
			"Pixel width of the obituary area",
			0x1A60BB02342FD5F6
		},
		{
			"m_pitch",
			"Default pitch",
			0x68045F57217A8E71
		},
		{
			"m_filter",
			"Allow mouse movement smoothing",
			0x68CB4EF34E0CEDB7
		},
		{
			"m_mouseFilter",
			"Mouse filter",
			0x626B0DAB7BF45F06
		},
		{
			"m_mouseAcceleration",
			"Mouse acceleration",
			0x24898FD2F3E6C0D8
		},
		{
			"m_mouseSensitivity",
			"Mouse sensitivity",
			0x7659B9A54E5830C7
		},
		{
			"cl_freelook",
			"Enable looking with mouse",
			0x1B49BFBB87BE110E
		},
		{
			"cl_motdString",
			"Message of the day",
			0x370CC5B56A1F2E1E
		},
		{
			"cl_ingame",
			"True if the game is active",
			0x64F0BE81B0EA8E0E
		},
		{
			"player_name",
			"Player name",
			0x13A62F542CF8E86E
		},
		{
			"maxVoicePacketsPerFrame",
			"The max number of voice packets that a local client will process per frame",
			0x415248C9D932B45E
		},
		{
			"splitscreen_playerCount",
			"The number of players in a splitscreen game",
			0x6CDDC2FC45915C64
		},
		{
			"splitscreen_horizontal",
			"Draw splitscreen views horizontally",
			0x11734D8E2969A720
		},
		{
			"cl_migrationPingTime",
			"how many seconds between client pings.  used to determine hosting suitability.",
			0x605F4A6B9253BE84
		},
		{
			"con_label_filter_mask",
			"mask to filter tty by label, if flag set then visible (1<<label)",
			0x7673C35E3BC579EA
		},
		{
			"live_whitelistFatal",
			"Sys_error on whitelist failure",
			0x63DE3D28FB03C6BD
		},
		{
			"saveLocalMatchRecordBinaryFile",
			"If set to true, and development-build, save out local copy of match_record",
			0x528D0D816A279C66
		},
		{
			"boostcheatIntercept",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x77D6C1D95CEC3AEB
		},
		{
			"boostcheatHeadshotsTotalMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x2E459C1F4981CC19
		},
		{
			"boostcheatHeadshotsTotalStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x2E4171E6C7482B48
		},
		{
			"boostcheatHeadshotsTotalCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x34872A4F326806C1
		},
		{
			"boostcheatMeanDistanceVictimTraveledMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x3C6DF05BDD412A49
		},
		{
			"boostcheatMeanDistanceVictimTraveledStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x5EEEB9C944871998
		},
		{
			"boostcheatMeanDistanceVictimTraveledCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x43893E8BC6E09BF1
		},
		{
			"boostcheatBitchKillsTotalMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0xFD28CF5C8641BE1
		},
		{
			"boostcheatBitchKillsTotalStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x6BD387C583F22B10
		},
		{
			"boostcheatBitchKillsTotalCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x62425AA274DFDE59
		},
		{
			"boostcheatBitchKillsRatioMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x6EDFF35B6BEE21A4
		},
		{
			"boostcheatBitchKillsRatioStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x110F3F193C008039
		},
		{
			"boostcheatBitchKillsRatioCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x379D1549A22DAA30
		},
		{
			"boostcheatMeanDistanceBitchTraveledMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x205FE014B751A6E3
		},
		{
			"boostcheatMeanDistanceBitchTraveledStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x22749EF17882566
		},
		{
			"boostcheatMeanDistanceBitchTraveledCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x5171C2680DD10033
		},
		{
			"boostcheatBitchHKRatioMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x62048D28DDFE1C32
		},
		{
			"boostcheatBitchHKRatioStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0xC5EC3AFB326CC03
		},
		{
			"boostcheatBitchHKRatioCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x77A2AB7C6C7B086E
		},
		{
			"boostcheatMeanBitchLifetimeMillisecondsMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x50B51916D812938A
		},
		{
			"boostcheatMeanBitchLifetimeMillisecondsStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x785FE2A3C47E1E5B
		},
		{
			"boostcheatMeanBitchLifetimeMillisecondsCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x5DCD76E1076CA156
		},
		{
			"boostcheatKillerXAnomalyMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x67E0A27774065998
		},
		{
			"boostcheatKillerXAnomalyStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x1ABBB3EAF8DC2ACD
		},
		{
			"boostcheatKillerXAnomalyCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x5C1C642433688094
		},
		{
			"boostcheatKillerYAnomalyMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x4765F5181BF5008D
		},
		{
			"boostcheatKillerYAnomalyStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x3BC1A44CF0C42D6C
		},
		{
			"boostcheatKillerYAnomalyCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x6CA433063DDCAB45
		},
		{
			"boostcheatVictimXAnomalyMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x176DAD5D2149C62B
		},
		{
			"boostcheatVictimXAnomalyStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x23DCE595A940480E
		},
		{
			"boostcheatVictimXAnomalyCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x106E4F2D37C2B40B
		},
		{
			"boostcheatVictimYAnomalyMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0xC2488A3B8FCF73E
		},
		{
			"boostcheatVictimYAnomalyStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x37AB4E985AFE3FBF
		},
		{
			"boostcheatVictimYAnomalyCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x760B1631A59F3B1A
		},
		{
			"boostcheatBitchKillTimestampsAnomalyMean",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x669A1B5569515FA4
		},
		{
			"boostcheatBitchKillTimestampsAnomalyStddev",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x63FA4C984CDF6E39
		},
		{
			"boostcheatBitchKillTimestampsAnomalyCoef",
			"Boosting detector parameter (64 bits should be cast to double)",
			0x2F573D439F90E830
		},
		{
			"fx_marks_draw",
			"Toggles whether decals are rendered",
			0x554F283CF5BBB40D
		},
		{
			"fx_debugBolt",
			"Debug effects bolt",
			0x13715E5F63F75BB4
		},
		{
			"fx_visMinTraceDist",
			"Minimum visibility trace size",
			0x689DAFFEFBC5B8A3
		},
		{
			"fx_profile",
			"Turn on FX profiling (specify which local client, with '1' being the first.)",
			0x2828BD4F09D20C31
		},
		{
			"fx_mark_profile",
			"Turn on FX profiling for marks (specify which local client, with '1' being the first.)",
			0x6025BE4FF3B6DC0B
		},
		{
			"fx_drawClouds",
			"Toggles the drawing of particle clouds",
			0x67E5F2422228DF00
		},
		{
			"fx_occlusion_max_player_count",
			"Maximium number of local clients that FX is drawing sprite with occlusion test.)",
			0x7A04AB2A40DD934C
		},
		{
			"ai_useTacticalInfluencers",
			"Tactical Influencers : Debug draw tactical influencers.",
			0xE64F867441D32D2
		},
		{
			"ai_tacticalInfluencersDebug",
			"Tactical Influencers : Debug draw tactical influencers.",
			0x15BD02D3438184FE
		},
		{
			"ai_tacticalInfluencersThreatDebug",
			"Tactical Influencers : Debug draw tactical threat.",
			0x4E9695DF71A2B13E
		},
		{
			"ui_blocksaves",
			"prevents level progression in saves, does not block player progression",
			0x30A8D8634A92B501
		},
		{
			"settings_defaultSubtitles",
			"Any language that allows subtitles (except English) defaults to turn them on",
			0x6A3882A8980DD4B
		},
		{
			"tgraph_debugDrawPoints",
			"Draw points in the graph",
			0x404206C0DE7A2000
		},
		{
			"tgraph_debugShowStats",
			"Show stats information of tgraph",
			0xE9361E3BCD4825F
		},
		{
			"tgraph_debugDrawPointNavFace",
			"Draw the navmesh face points are linked to",
			0x385F502007EBCC1D
		},
		{
			"tgraph_debugTargetPointId",
			"Debug the point with this ID",
			0x4B79E47CE500ED2B
		},
		{
			"tgraph_debugVisMapPointId",
			"Debug the visibility map on this point",
			0x6606CB6775DA0EC
		},
		{
			"tgraph_debugVisMapDrawNonVisible",
			"When debugging vismap, should the blocked sight be drawn (with different color)",
			0x7FB5A6BF01DCCE1C
		},
		{
			"tgraph_debugClearancePointId",
			"Debug the clearance value on this point",
			0x651888DC9308757C
		},
		{
			"tquery_drawQuery",
			"Draw debug information for specific querydef",
			0x1DF75E1978789CF0
		},
		{
			"bot_supported",
			"Whether bots are supported",
			0x1E7ACE24E5F53BFF
		},
		{
			"bot_friends",
			"Number of friends allowed in basic training",
			0x6BAB37F70CE6DC2C
		},
		{
			"bot_enemies",
			"Number of enemies allowed in basic training",
			0x4A8F7194BE344C15
		},
		{
			"bot_difficulty",
			"Difficulty level of the basic training bots",
			0x25BBE9E88BDC0BB2
		},
		{
			"bot_maxFree",
			"Maximum number of bots for TEAM_FREE",
			0x7892BE2130E23461
		},
		{
			"bot_maxAllies",
			"Maximum number of bots for TEAM_ALLIES",
			0x6D3130B2EC402E19
		},
		{
			"bot_maxAxis",
			"Maximum number of bots for TEAM_AXIS",
			0x11C4E0094D23D4C6
		},
		{
			"bot_maxMantleHeight",
			"Max height a bot will attempt a mantle traversal",
			0x281E02EB013CEF23
		},
		{
			"bot_AllowMelee",
			"Allow bots to press the melee button",
			0x79BF75D9C3E06DC0
		},
		{
			"bot_AllowGrenades",
			"Allow bots to throw lethal and tactical grenades",
			0x7441BDEA89ABB723
		},
		{
			"bot_AllowHeroGadgets",
			"Allow bots to use hero gadgets",
			0x122642EF8BF88909
		},
		{
			"g_loadScripts",
			"Disable scripts from loading",
			0xF9EE6F4CF38FDED
		},
		{
			"ui_doa_unlocked",
			"unlock doa in the mission list",
			0x69795D7673E89E70
		},
		{
			"ui_codexindex_unlocked",
			"add the master index to the bookmarks list",
			0x77902CF694444B1F
		},
		{
			"g_password",
			"Password",
			0x7CC5023AED7FBBFA
		},
		{
			"g_speed",
			"Player speed",
			0x3D668E8FB0FEA3A6
		},
		{
			"g_debugDamage",
			"Show debug information for damage",
			0x4855D10120F3F81F
		},
		{
			"g_vehicleDrawSplines",
			"Draws the vehicles splines",
			0x756B9210B98A662F
		},
		{
			"g_vehicleBypassFriendlyFire",
			"Make vehicle always damagable regardless of team",
			0x2120543394DF2956
		},
		{
			"ai_useLeanRunAnimations",
			"whether to use lean run animations instead of strafes",
			0x40D79AF37BA8815F
		},
		{
			"ai_showNodesDist",
			"Maximum distance from the camera at which AI nodes are shown",
			0x2D6C6DEC81F8F2EA
		},
		{
			"ai_showNearestNode",
			"Show nodes closest to AI",
			0x178313DF9CCD820D
		},
		{
			"ai_debugVolumeTool",
			"Show AI navigation volume debug information",
			0x2A6130184529EA53
		},
		{
			"ai_showPaths",
			"Show AI navigation paths",
			0x503E418BF304A803
		},
		{
			"ai_clampToNavMeshEnabled",
			"Periodically clamp actors to the navmesh if they get off it.",
			0x5499795E080D6AC3
		},
		{
			"ai_debugCoverEntityNum",
			"Display debug info for cover",
			0x284D591C2F10FCB
		},
		{
			"ai_showDodge",
			"Display debug information for AI dodging",
			0x31C69EEDA406C6A4
		},
		{
			"ai_debugAnimScripted",
			"Enable debugging information for anim scripted AI.",
			0x350D3015C1C91094
		},
		{
			"ai_debugEntIndex",
			"Entity index of an entity to debug",
			0x2F1E9D0E9CA7FE80
		},
		{
			"ai_debugArrivals",
			"Record AI arrival and exit debug information",
			0xFE513FF6FBCEB55
		},
		{
			"ai_meleeDamage",
			"The amount of damage dealt by AI's melee attack",
			0x6D790B88D21C115D
		},
		{
			"ai_scaleSightUpdateLatency",
			"Scale actor sight update latency based on distance to the nearest player",
			0x2BD4960B68DC94EC
		},
		{
			"ai_accuracy_attackerCountDecrease",
			"Accuracy multiplied by this number for each additional attacker upto ai_accuracy_attackerCountMax",
			0xD5DB8A3B1E7D340
		},
		{
			"g_motd",
			"The message of the day",
			0x6EBC397E70F45327
		},
		{
			"g_allowVote",
			"Enable voting on this server",
			0x4DCD3D13CB022B30
		},
		{
			"g_customTeamName_Allies",
			"User assigned allied team name",
			0x311512429BB852D5
		},
		{
			"g_customTeamName_Axis",
			"User assigned axis team name",
			0x38BE7B1FE08578FA
		},
		{
			"g_TeamName_Allies",
			"Allied team name",
			0xBA6F504C6BB64C4
		},
		{
			"g_TeamName_Axis",
			"Axis team name",
			0x59060A425875477
		},
		{
			"g_quadrotorFlyHeight",
			"Default fly height of the quadrotors",
			0x7BFD916C78DBFEC6
		},
		{
			"g_fogColorReadOnly",
			"Fog color that was set in the most recent call to \"setexpfog\"",
			0x48D7E6F93726DFEE
		},
		{
			"g_fogStartDistReadOnly",
			"Fog start distance that was set in the most recent call to \"setexpfog\"",
			0x1EBF456A7F4465A5
		},
		{
			"g_fogHalfDistReadOnly",
			"Fog start distance that was set in the most recent call to \"setexpfog\"",
			0x5798E3AB5484E6F2
		},
		{
			"spawnsystem_convert_spawns_to_structs",
			"parse all map placed spawns to structs",
			0x1CA87A4ECD338EE3
		},
		{
			"spawnsystem_player_explored_radius",
			"How close a spawn point has to be to be considered 'explored'",
			0x658A64A49033AFCC
		},
		{
			"spawnsystem_use_code_point_enabled",
			"Use code to filter enabled and disabled spawn points",
			0x556525B235545E4
		},
		{
			"vehicle_collision_prediction_time",
			"How far ahead of the vehicle it should predict to do collision damage.",
			0x37A9B3F84D528AE8
		},
		{
			"vehicle_collision_prediction_crash_time",
			"How far ahead of the vehicle it should predict to do collision.",
			0x2D5CE660D3E8EB00
		},
		{
			"r_lightingSunShadowCacheDynamicSiegeDistance",
			"Inches from camera to draw dynamic siege shadows",
			0xFB15EB644CC51ED
		},
		{
			"nv_textureReleaseFrameDelay",
			"nv_textureReleaseFrameDelay",
			0x74772F5FEAB31BB2
		},
		{
			"nv_textureReleaseMaxPerFrame",
			"nv_textureReleaseMaxPerFrame",
			0x15B112392F74E6C0
		},
		{
			"amd_useShaderExtension",
			"use read first lane",
			0x2035B9CFCA6E21A1
		},
		{
			"r_drawFrameDurationGraph",
			"Display a graph of the frame durations.",
			0x3F6C02556D8ADFDF
		},
		{
			"r_lateAllocLimit",
			"Set the VS late alloc limit for parameter cache",
			0x7D131D753AF12A8E
		},
		{
			"r_OIT",
			"Enable Order Independant Transperancy.",
			0x4F1BFEB2452744FE
		},
		{
			"r_backBufferCount",
			"Number of backbuffer, set to 3 to enable triple buffering",
			0x1FA529540F1E4CB2
		},
		{
			"r_fullscreen",
			"Game window mode (window, fullscreen, fullscreen window)",
			0x17119D0EEFF392AB
		},
		{
			"r_convergence",
			"Change 3D convergence",
			0x1022AF0F7BA8C9AB
		},
		{
			"r_deferredIgnoreShadowUpdate",
			"Lighting: Ignore the shadow update flag",
			0x2FC480AE9FE2162A
		},
		{
			"r_deferredForceShadowNeverUpdate",
			"Lighting: Always render shadow only once",
			0x7584D68D5979628B
		},
		{
			"r_viewmodelSelfShadow",
			"Enable viewmodel self shadowing",
			0x90E076F4ED4DA9E
		},
		{
			"r_dedicatedPlayerShadow",
			"Enable player dedicated shadowing",
			0x6FDA5D20BF93E4D6
		},
		{
			"r_dedicatedPlayerSunShadowPenumbraScale",
			"Change player shadow penumbra in sun",
			0x107A4E5B51797B2A
		},
		{
			"r_lightingSunShadowDisableDynamicDraw",
			"Disable rendering to the sun shadow splits",
			0x1FF11E85689F1D27
		},
		{
			"r_lightingSunShadowCacheEnable",
			"Enable caching for sun shadows",
			0x2B8F13916B447A95
		},
		{
			"r_lightingSunShadowSSTMipDrop",
			"Drop mips from the SST (saving GPU memory)",
			0x31FA6BCF24DD9989
		},
		{
			"r_lightingReflectionProbeMipDrop",
			"Number of mips to drop from the reflection probes",
			0x5A46991F6490D9D2
		},
		{
			"r_lightingSpotOmniShadowMaxQuality",
			"Set the maximum spot/omni shadow quality",
			0x6FAD994D7EA928D4
		},
		{
			"r_lightingSpotShadowForceSize",
			"Set the maximum spot shadows size",
			0xCD712C427611FC
		},
		{
			"r_lightingShadowFiltering",
			"Enable shadow filtering",
			0x31DB03CD6D2C3A30
		},
		{
			"r_lightingOmniShadowForceSize",
			"Set the maximum spot shadows size",
			0x4C29E8FDE4B16EBB
		},
		{
			"r_dedicatedPlayerSunShadowResolution",
			"Set the sun dedicated shadow resolution",
			0x628B9435BE1BC22E
		},
		{
			"r_dedicatedPlayerSpotOmniShadowResolution",
			"Set the spot/omni dedicated shadow resolution",
			0xA1970050D530DBD
		},
		{
			"r_viewModelSunShadowResolution",
			"Set the sun view model shadow resolution",
			0x5B5E7FECA1C1E820
		},
		{
			"r_viewModelSpotOmniShadowResolution",
			"Set the spot/omni view mode shadow resolution",
			0x4AD4EB0B31BED373
		},
		{
			"r_spotShadowRes",
			"Spot shadow resolution",
			0x3007F3D0B2A87CE0
		},
		{
			"r_omniShadowRes",
			"Omni shadow resolution",
			0xAB8A3CBD4C298D7
		},
		{
			"r_asyncCompute",
			"Enable all async compute that uses the GfxAsyncComputeScope class.",
			0x4CAE2BA1B11793F7
		},
		{
			"r_extracamDisable",
			"Disable extracam rendering",
			0x22F1E77CFA52C6C5
		},
		{
			"r_forceTextureAniso",
			"Force Anisotropy filter level",
			0x6075FAB09C00BEF8
		},
		{
			"r_forceMaxTextureSize",
			"Force max texture sizes",
			0x36BA04D92CEBDAF9
		},
		{
			"r_autoLodCullRadius",
			"Auto-lod culling radius in screen pixels.",
			0x68D3B225F9F1007E
		},
		{
			"r_lodScaleRigid",
			"Scale the level of detail distance for rigid models (larger reduces detail)",
			0x66F441B5D3DA7128
		},
		{
			"r_lodBiasRigid",
			"Bias the level of detail distance for rigid models (negative increases detail)",
			0x1237E8F3F6E82437
		},
		{
			"r_modelLodBias",
			"LOD scale",
			0x1158EBB90220A543
		},
		{
			"r_modelLodLimit",
			"Hard limit to reject higher lod from being picked",
			0x5D679AB6B6B38EFD
		},
		{
			"r_lodThresholdPixelArea",
			"When screen-space average triangle area falls below this threshold, switch LODs",
			0x64B64F67362C4D71
		},
		{
			"r_zfar",
			"Change the distance at which culling fog reaches 100% opacity; 0 is off",
			0x51A6C7802FCD8D4D
		},
		{
			"r_fog",
			"Set to 0 to disable fog",
			0xC7CFCB268217646
		},
		{
			"r_norefresh",
			"Skips all rendering.  Useful for benchmarking.",
			0x7436EE050770275C
		},
		{
			"r_scaleViewport",
			"Scale 3D viewports by this fraction.  Use this to see if framerate is pixel shader bound.",
			0x7D9F9D5F2716D806
		},
		{
			"r_vsync",
			"Enable v-sync before drawing the next frame to avoid 'tearing' artifacts.",
			0x2CED14A629F3C33
		},
		{
			"r_clearColor",
			"Color to clear the screen to when clearing the frame buffer",
			0xD1E5694DBB641FE
		},
		{
			"r_clearColor2",
			"Color to clear every second frame to (for use during development)",
			0xCEEAF156B1CDA4
		},
		{
			"r_viewportBackingColor",
			"Color to clear the frame buffer with before compositing viewports",
			0x1FE25AD5F9AE4378
		},
		{
			"r_spotLightShadows",
			"Enable shadows for spot lights.",
			0x6A858EA14B4A96AB
		},
		{
			"r_spotLightSModelShadows",
			"Enable static model shadows for spot lights.",
			0x53BD27FD02686355
		},
		{
			"r_spotLightEntityShadows",
			"Enable entity shadows for spot lights.",
			0x390B0B07740A02
		},
		{
			"r_lockPvsInCode",
			"Indicates if pvslock is currently on, in code. Only code should modified this dvar",
			0x4D3C80F0FCB83C0A
		},
		{
			"r_lockFrameRateTo30Hz",
			"Locks Frame Rate to 30Hz: -1:Nochange 0:60Hz 1:30Hz",
			0x302C3DFAB639D8C
		},
		{
			"r_lockFrameRateTo30Hz_enable",
			"Enable 30Hz frame rate lock",
			0x311DAA8036698392
		},
		{
			"r_lockFrameRateTo30Hz_dynResOff",
			"Disable dynamic resolution when 30Hz is forced",
			0x4B7CAAE12A82F877
		},
		{
			"r_useStrict30HzConditions",
			"",
			0x192AC9C8F6A3F513
		},
		{
			"r_forceLod",
			"Force all level of detail to this level",
			0x1B7AD1C68F68858
		},
		{
			"r_zombieNameAllowFriendsList",
			"Allow zombie name to be from friends list",
			0x2C17001DFE001EC5
		},
		{
			"r_zombieNameAllowDevList",
			"Allow zombie name to be from dev list",
			0x5B8AA65489D0E9B7
		},
		{
			"r_tilingHighlight",
			"",
			0x9372FCBFC7AACAF
		},
		{
			"r_tilingLightCount",
			"",
			0x177A63C69A2A005E
		},
		{
			"r_tilingProbeCount",
			"",
			0x4ADBA09931E2FDB0
		},
		{
			"r_tilingSunShadow",
			"",
			0x24FB014803E1807D
		},
		{
			"r_streamReadLog",
			"Log image and mesh reading",
			0x7A1AD490F5DE6EDC
		},
		{
			"r_streamTextureMemoryMax",
			"Limit size of stream buffer in MB (0 is no limit)",
			0x1EB239BAB8F4C64C
		},
		{
			"r_streamFreezeState",
			"Freeze the state of all streaming memory - don't allow streaming or un-streaming of any images.",
			0x64A44B0A964FE5C6
		},
		{
			"r_stereo3DAvailable",
			"3D Mode available",
			0x69D310DFC9DB3A64
		},
		{
			"r_stereo3DOn",
			"3D on off toggle",
			0x79AE57C71B3EE024
		},
		{
			"r_stereo3DMode",
			"3D Rendering mode",
			0xED4A478D5975BD6
		},
		{
			"r_gpuCullingDisableDepthTest",
			"disable the gpu culling depth part",
			0x28E84101A211F0B1
		},
		{
			"r_dedicatedShadowsUmbra",
			"Cull dedicated player and viewmodel shadows",
			0x26AC75F33E63A84D
		},
		{
			"r_cachedSpotShadowCopyMode",
			"spot shadow copy mode",
			0x605E17F1604A24B6
		},
		{
			"r_smaaQuincunx",
			"Enables quincunx temporal resolve (for T2x modes)",
			0x3F2D6BC066490BC7
		},
		{
			"r_fxaaDebug",
			"FXAA debug mode",
			0x64A37EC518A161A1
		},
		{
			"r_fxaaContrastThreshold",
			"FXAA contrast threshold",
			0x4DED251EC3283C07
		},
		{
			"r_fxaaSubpixelRemoval",
			"FXAA subpixel removal",
			0x54AD7417CE2CC11C
		},
		{
			"r_aaTechnique",
			"Anti-aliasing technique",
			0x24B8757D7CB6ED3A
		},
		{
			"r_aaAllowTemporalMultiGpu",
			"Anti-aliasing temporal overide for SLI",
			0x24718B7D628332CA
		},
		{
			"r_ssaoTechnique",
			"SSAO technique.",
			0x210198A6861793FE
		},
		{
			"r_smodel_partialSortWorkers",
			"Number of workers to sort the smodel",
			0x799AB83AEF380629
		},
		{
			"r_smodel_splitWorkers",
			"Number of workers to split smodel",
			0x7AD33EF60F4D0E12
		},
		{
			"r_smodel_combineWorkers",
			"Number of workers to combine smodel",
			0x43257AF5C86D9659
		},
		{
			"r_ssao_gtaoTweak",
			"Turn on to tweak ssao",
			0x62F7477D896BDACE
		},
		{
			"r_ssao_gtaoStrength",
			"GTAO strength value. The default (1.0) will produce ground truth values",
			0x69AC7C51E47AC9A7
		},
		{
			"r_ssao_gtaoLow",
			"GTAO shadow tone",
			0x495EA0E56358BEA4
		},
		{
			"r_ssao_gtaoMid",
			"GTAO mid tone",
			0x502CA6E566AD0FEC
		},
		{
			"r_ssao_gtaoHigh",
			"GTAO high tone",
			0x6236BBEA7F9351DE
		},
		{
			"r_sssblurEnable",
			"SSS Blur enable",
			0x6C71069B8320C70F
		},
		{
			"r_flame_allowed",
			"Allow flame effect.",
			0x577E65B2DE936C16
		},
		{
			"r_filmTweakLut",
			"Tweak Film LUT Index.",
			0xDDF8EF33974A79
		},
		{
			"r_enablePlayerShadow",
			"Enable First Person Player Shadow.",
			0x6A609E5D7353A45C
		},
		{
			"r_xcamsEnabled",
			"Enable/disable the xcam system controlling the camera (does not effect scene playback).",
			0x60A9D9F8E304DF73
		},
		{
			"r_expAuto",
			"toggle auto exposure",
			0x7695E873F54C6DBA
		},
		{
			"r_bloomUseLutALT",
			"enable alternate lut",
			0x207D6F85DC11BB4C
		},
		{
			"vc_LUT",
			"Lut index",
			0x516AD05C93DC83B4
		},
		{
			"r_adsWorldFocalDistanceMax",
			"Ads World Blur max focalDistance",
			0x6E13DDF7E61BF9CC
		},
		{
			"r_adsWorldFocalDistanceMin",
			"Ads World Blur min focalDistance",
			0x6E2FEBF7E6348C4E
		},
		{
			"r_adsWorldFocalDistanceTrackBackScale",
			"Ads World Blur track back distance scale",
			0x13E1438EE9AFCB64
		},
		{
			"r_adsBloomDownsample",
			"Enables Ads Blur shared bloom downsample",
			0x78496CBA0C7280D
		},
		{
			"r_flameFX_distortionScaleFactor",
			"Distortion uv scales (Default to 1)",
			0x6141FA14F5864298
		},
		{
			"r_flameFX_magnitude",
			"Distortion magnitude",
			0x1D2E80C7447180D6
		},
		{
			"r_flameFX_FPS",
			"fire frames per sec",
			0x39EE849E99489A77
		},
		{
			"r_flameFX_fadeDuration",
			"Sets fade duration in seconds",
			0x1978F0B7F3D90962
		},
		{
			"r_waterSheetingFX_allowed",
			"Enable the water sheeting effect",
			0x1DBAB58A4A6E1991
		},
		{
			"r_waterSheetingFX_enable",
			"Enable the water sheeting effect",
			0x367DA853EE82C020
		},
		{
			"r_waterSheetingFX_distortionScaleFactor",
			"Distortion uv scales (Default to 1)",
			0x12DD20A3B2700945
		},
		{
			"r_waterSheetingFX_magnitude",
			"Distortion magnitude",
			0x1F0CEC3072D042B3
		},
		{
			"r_waterSheetingFX_radius",
			"Tweak dev var; Glow radius in pixels at 640x480",
			0x123CB1F564CA51C5
		},
		{
			"r_anaglyphFX_enable",
			"Enable red/green Anaglyph 3DS",
			0x5004E7975889CB2E
		},
		{
			"r_graphicContentBlur",
			"Enable Fullscreen Blur",
			0x10F2E56A987A88FE
		},
		{
			"r_postFxWobble",
			"Enable Fullscreen Wobble",
			0x4EF76460F853E0EB
		},
		{
			"r_eacPathFX_enable",
			"Enable EAC path overlay",
			0x541FA64C61C05142
		},
		{
			"r_eacPath_Posn",
			"EAC Path drone position",
			0x40106F7467E0D12B
		},
		{
			"r_eacPath_Radius",
			"EAC Path Radius",
			0x2ED1AF502DD3BFD5
		},
		{
			"r_eacPath_Step",
			"EAC Path Step",
			0x3383BA5C25ED0DB5
		},
		{
			"r_blurAndTintEnable",
			"Enable Blur and Tint",
			0x4671CE3EBE139326
		},
		{
			"r_blurAndTintLevel",
			"Blur Level",
			0x42C0B0567E60F185
		},
		{
			"r_circleMaskRadius",
			"Circle Mask Enable",
			0x7366BAF9D9024CD4
		},
		{
			"r_splitScreenExpandFull",
			"Split Screen is expanded to Fullscreen",
			0x21DEC544A0C3A8B7
		},
		{
			"cl_secondaryPlayerMenuControlDisable",
			"Disable menu control for the secondary player",
			0x1DA89F87A79F721E
		},
		{
			"r_ev_distance",
			"EV distance",
			0x4618C3DA2528EEA5
		},
		{
			"r_ev_width",
			"EV pulse width",
			0x384825CC11C53C3A
		},
		{
			"r_ev_targetwidth",
			"EV pulse target width",
			0xE876CF04933E061
		},
		{
			"r_ev_rate",
			"EV pulse rate",
			0x8DDD7E5D456DCFC
		},
		{
			"r_ev_geometryrange",
			"EV geometry range",
			0x3332E69B8FF565F
		},
		{
			"r_ev_targetrange",
			"EV target range",
			0x420C13F6DEAB7184
		},
		{
			"r_ev_edgewidth",
			"EV edge pulse width",
			0x7B38CBCEFB71DB39
		},
		{
			"r_ev_edgethickness",
			"EV edge thickness",
			0x27652C3FA638B3DF
		},
		{
			"r_ev_testenable",
			"EV test enable",
			0x486204D2814A6D05
		},
		{
			"r_ev_screen_scale",
			"EV screen scale",
			0x1C3FFACB79E52597
		},
		{
			"r_ev_screen_threshold",
			"EV screen threshold",
			0x419B0C4F1BBD16C2
		},
		{
			"r_ev_random_ground",
			"EV random ground",
			0x4DADFA65C0869B7D
		},
		{
			"r_ev_random_edge",
			"EV random edge",
			0x3271B40FCCE6C5FB
		},
		{
			"r_aberrationFX_enable",
			"Enable Aberration",
			0xE00C20B4C9BA4A7
		},
		{
			"r_postFxIndex",
			"Display PostFx with this index",
			0x4D2B35C9F7F30A92
		},
		{
			"r_postFxSubIndex",
			"Tweak PostFx sub index for keyline variants",
			0x7EC250F3FB82DDD8
		},
		{
			"r_postFxUseTweaks",
			"Override PostFx Params with dvars",
			0x76D972344D0D0BE0
		},
		{
			"r_catsEyeReset",
			"Reset CatsEye dvars to default",
			0x4D6CAAC1E88A8C7B
		},
		{
			"r_catsEyeDecolor",
			"Cats Eye: decolor",
			0x4E99EE8A9A403A76
		},
		{
			"r_catsEyeNoise",
			"Cats Eye: noise",
			0x1E0A6709CD45271E
		},
		{
			"r_catsEyeTexture",
			"Cats Eye: texture level",
			0x2B698574B4648EC7
		},
		{
			"r_catsEyeDistortion",
			"Cats Eye: distortion &scale",
			0x49F7AC0C7A4D50BB
		},
		{
			"r_catsEyeAberation",
			"Cats Eye: aberation",
			0xA9505FAF22F71B1
		},
		{
			"r_catsEyeBlur",
			"Cats Eye: blur amount",
			0x112C91F71C6B78B5
		},
		{
			"r_catsEyeBlurTint",
			"Cats Eye: blur tint",
			0x126DF1AD270E7C5C
		},
		{
			"r_chaserFX_enable",
			"Enable Chaser PostFX",
			0x46DB079A3DB9210E
		},
		{
			"r_heatPulseFX_enable",
			"Enable Heat Pulse Gun Effect",
			0x6A38D430C5FF06CF
		},
		{
			"r_radioactiveFX_enable",
			"Enable Radiaoctive Effect",
			0x12630EAD38002B61
		},
		{
			"r_radioactiveBlur",
			"Radioactive: blur",
			0x4893B293B2E5673A
		},
		{
			"r_radioactiveSpeed",
			"Radioactive: fire animation speed",
			0x62A25B76918294BA
		},
		{
			"r_radioactiveIntensity",
			"Radioactive: fire intensity",
			0x39E525954A1F963C
		},
		{
			"r_uiHudFX_enable",
			"Enable UI HUD Effect",
			0x377F54AB10632357
		},
		{
			"r_maxPOMSamples",
			"Additional ceiling on maximum number of samples in POM raycast, scales performance for testing",
			0x14F0D7A5C690A863
		},
		{
			"r_POMLODStart",
			"Distance to begin scaling down POM effect",
			0x7CAABBBF6AA68E2F
		},
		{
			"r_POMLODEnd",
			"Distance to finish scaling down POM effect",
			0x2CF0E69797B7BADA
		},
		{
			"r_poisonFX_pulse",
			"pulse rate for distortion",
			0x102E887BB790736
		},
		{
			"r_poisonFX_blurMin",
			"blur min",
			0x1072C8A66BA085DC
		},
		{
			"r_poisonFX_blurMax",
			"blur max",
			0x1057B6A66B899F8E
		},
		{
			"r_fogTweak",
			"enable dvar tweaks",
			0x2E30FB8B48D71276
		},
		{
			"r_fogBaseDist",
			"start distance",
			0x410DDEFA7BFABCEB
		},
		{
			"r_fogHalfDist",
			"distance at which fog is 50%",
			0x1436D6A4846E4543
		},
		{
			"r_fogBaseHeight",
			"start height",
			0x3178DC89624DE864
		},
		{
			"r_fogHalfHeight",
			"height at which fog is 50%",
			0x26D7B47F7E253EFC
		},
		{
			"r_fogSkyHalfHeightOffset",
			"sky fog density half height offset",
			0x380C854BFB372DBE
		},
		{
			"r_fogColor",
			"color",
			0x40434CABE4B9C409
		},
		{
			"r_fogIntensity",
			"intensity",
			0x2D068F6D38A1EB17
		},
		{
			"r_fogOpacity",
			"opacity",
			0x44E211D9E74864CF
		},
		{
			"r_fogSunColor",
			"sun color",
			0x70BED414D474FC17
		},
		{
			"r_fogSunIntensity",
			"sun color",
			0x4859CD09327426A1
		},
		{
			"r_fogSunOpacity",
			"sun opacity",
			0x7452B1925FC01AF5
		},
		{
			"r_fogSunPitchOffset",
			"sun pitch offset",
			0x11CD5A11136723D1
		},
		{
			"r_fogSunYawOffset",
			"sun yaw offset",
			0x7428F54EE23C07F8
		},
		{
			"r_fogSunInner",
			"sun angle start",
			0x2A9390CCB413F320
		},
		{
			"r_fogSunOuter",
			"sun angle end",
			0x5F3B8889FB93
		},
		{
			"r_atmospherefogcolor",
			"atmosphere fog color (Raleigh)",
			0xBEC84CC65CA975D
		},
		{
			"r_atmospherefogdensity",
			"atmosphere fog density",
			0x19B5AA85A8E03DAE
		},
		{
			"r_atmospherehazecolor",
			"atmosphere haze color (Mie)",
			0x2C4BA6B721F10B7
		},
		{
			"r_atmospherehazedensity",
			"atmosphere sun haze strength (overall density that is allocated to haze with the remainder assigned to fog)",
			0x2B1CA97A67160D20
		},
		{
			"r_atmospherehazespread",
			"atmosphere sun haze spread",
			0x6987E682ECA9322F
		},
		{
			"r_atmosphereinscatterstrength",
			"atmosphere fog Brightnesss/In-scatter strength",
			0x4428C3E62FD3EB8C
		},
		{
			"r_atmosphereextinctionstrength",
			"atmosphere fog Opacity/Extinction strength",
			0x53ED525094455850
		},
		{
			"r_atmospheresunstrength",
			"atmosphere enable sun",
			0x2BEC3A68814687FD
		},
		{
			"r_atmospherehazebasedist",
			"Distance at which haze density is 0",
			0x43DFBAD6F4986AC9
		},
		{
			"r_atmospherehazefadedist",
			"Distance over which to fade in the haze density to 1.0",
			0x4F6DDD5AEBFA892A
		},
		{
			"r_atmospherepbramount",
			"0 == Not PBR, 1 == PBR ",
			0x6067CE892BFA6F56
		},
		{
			"r_worldfogskysize",
			"world fog sky distance",
			0xC1419241263B92A
		},
		{
			"r_exposureTweak",
			"enable the exposure dvar tweak",
			0x3610F004E5080F3D
		},
		{
			"r_exposureValue",
			"exposure ev stops",
			0x5038383CD3B3CB5C
		},
		{
			"r_exposureEyeWeight",
			"exposure eye vs. probe ratio",
			0x6223A653E39487B8
		},
		{
			"r_num_viewports",
			"number of viewports to expect to render",
			0x3396B8EB8A8C97BE
		},
		{
			"r_takeScreenShot",
			"Takes a screenshot",
			0x12C1333189502381
		},
		{
			"r_skyRotation",
			"Sky rotation angle.",
			0x58B46C9D56BC459
		},
		{
			"r_skyBoxColorIDX",
			"SkyBox Color Index",
			0x2F32973828BFE33E
		},
		{
			"r_sortDrawSurfsBsp",
			"Sort BSP draw surfaces.",
			0x15E374A6B3422E76
		},
		{
			"r_sortDrawSurfsStaticModel",
			"Sort static model draw surfaces.",
			0x72E8837AB9A41238
		},
		{
			"r_videoMode",
			"pc video output setting",
			0x5AAA15D9B94E876A
		},
		{
			"r_foveaAcuityColorMin",
			"full color angle",
			0x7155492C9BB4388F
		},
		{
			"r_foveaAcuityColorMax",
			"reduced color angle",
			0x713A372C9B9D5241
		},
		{
			"r_FilmIsoMin",
			"sensor / film min ISO",
			0x3680E6DF58DDD0F7
		},
		{
			"r_FilmIsoMax",
			"sensor / film max ISO",
			0x369BF4DF58F4B079
		},
		{
			"r_FilmIsoNoise",
			"ISO noise scale",
			0x1267B15CA6DF0D51
		},
		{
			"r_dof_aperture_override",
			"Overrides the aperture of the lens when the dofMode is set to DOF_MAX_BLUR",
			0x4E57F35E1924ADE5
		},
		{
			"r_dof_max_override",
			"Overrides the dof max value when the dofMode is set to DOF_MAX_BLUR and the override is non-negative",
			0x7010715C03B0A7B
		},
		{
			"r_dof_min_override",
			"Overrides the dof min value when the dofMode is set to DOF_MAX_BLUR and the override is non-negative",
			0x45F7E34E32545A39
		},
		{
			"r_volumetric_lighting_blur_depth_threshold",
			"Set volumetrics blur edge threshold",
			0xB64218EB27216B3
		},
		{
			"r_volumetric_lighting_upsample_depth_threshold",
			"Set volumetrics upsample edge threshold",
			0x3B54ADB06BC6423D
		},
		{
			"r_litfog_bank_select",
			"bank select",
			0x6E8B45A795354905
		},
		{
			"r_fx_backlighting_amount",
			"",
			0x4CE72EBCA3906B63
		},
		{
			"r_paperWhite",
			"SDR paperWhite nits value",
			0x43F74403764FA29D
		},
		{
			"r_newLensFlares_offscreen_buffer_size",
			"Set the size of the out of screen buffer (in pixels based on a 1080p resolution) ",
			0x33DAD47783A23FEA
		},
		{
			"r_newLensFlares",
			"enable the new lens flare system",
			0x7E8391792B3ABC71
		},
		{
			"r_motionVectorGenerateEnable",
			"toggles motion vector generation",
			0x2A44F5D7F6E54233
		},
		{
			"r_motionVectorStaticComputeAsync",
			"enables static motion vector generation to run async",
			0x7653BCA0DD5DDC6A
		},
		{
			"r_motionBlurMode",
			"motion blur mode",
			0x44F87480528FF262
		},
		{
			"r_motionBlurStrength",
			"control motion blur strength",
			0x38C17AD45D6603C0
		},
		{
			"r_motionBlurQuality",
			"motion blur quality preset",
			0x6BCA444A94600EF6
		},
		{
			"r_shaderDebug",
			"r_shaderDebug",
			0x47C54C7B96974FA6
		},
		{
			"r_useCachedSpotShadow",
			"Enable pre-built spot shadow map rendering",
			0x4F314A20067FE2D7
		},
		{
			"r_fxShadows",
			"Particle shadow support",
			0x519546D6E70793A5
		},
		{
			"r_xanim_disableExtraChannel",
			"Disable the extra channel (tension) calculation",
			0x1E2D0174CE8089F9
		},
		{
			"r_xanim_disableCosmeticBones",
			"Disable the cosmetic bones calculation",
			0x5ABF7E0574DF8EEA
		},
		{
			"r_dedicatedPlayerShadowCull",
			"Does not render shadow map if light is facing player",
			0x3B81AB5A52AAAD5E
		},
		{
			"r_dedicatedPlayerShadowCullAngle",
			"Angle from the foot to the view forward axis where the view model culling cuts off",
			0x4ADAAB8F89145425
		},
		{
			"r_cmdbuf_worker",
			"Process command buffer in a separate thread",
			0x1779BB751B7E4D3E
		},
		{
			"r_addLightWorker",
			"use a worker for the add lights and probes",
			0x5C3305FC7FF11965
		},
		{
			"r_useSimpleDObj",
			"use the simple dobj render path",
			0x14CE64D0C0288A02
		},
		{
			"hkai_pathfindIterationLimit",
			"The maximum number of iterations that the astar algorithm is allowed to execute before giving up.",
			0x6FA90AF593EF6209
		},
		{
			"hkai_additionalPathfindIterationLimit",
			"Additional number of iterations added to hkai_pathfindIterationLimit only for GenerateNavmeshPath requests.",
			0x499D8F9966887540
		},
		{
			"hkai_showTimers",
			"Whether or not to show the hkai timer debug information.",
			0x68E7AD0193C9C678
		},
		{
			"hkai_resetTimers",
			"Reset the hkai timer debug information.",
			0x28CF02874DF8931C
		},
		{
			"hkai_timerTestActive",
			"Whether we're currently engaged in a timer test.",
			0x4CE120697751C072
		},
		{
			"hkai_storeClearanceRecalcStats",
			"Store results of refilling teh clearance cache to file.",
			0x224659262CC519D1
		},
		{
			"hkai_dumpMemoryLeaks",
			"Dump the outstanding havok memory allocations the next time HKAI_FreeMapData is called.",
			0xF41557C84B102F2
		},
		{
			"hkai_warnPathFindFailures",
			"Show on screen print-warning if an actor fails to find a path",
			0x11F904550C88DB48
		},
		{
			"dw_sendBufSize",
			"Size in bytes for socketrouter's socket OS send buffer",
			0x332A652B0EB9C9AD
		},
		{
			"dwNetMaxWaitMs",
			"Milliseconds we'll wait in dwgetlocalcommonaddr before giving up",
			0x74F856896FA90DC6
		},
		{
			"tu5_dwNetFatalErrors",
			"Should errors encountered during DTLS handling be fatal",
			0x6633CCC2EA57A597
		},
		{
			"live_social_quickjoin",
			"Enable quickjoin widget: 0-disabled, 1-friends, 2-friends+groups",
			0x474143D29C242A6B
		},
		{
			"live_social_quickjoin_count",
			"number of freinds/group members in the quickjoin list",
			0x131B67E51F2A16E3
		},
		{
			"live_social_quickjoin_cache",
			"QuickJoin presence cache autoupdate",
			0x7EF63D384E7B7CD6
		},
		{
			"contracts_enabled_mp",
			"enables contracts",
			0x1B5AFF095BA580F6
		},
		{
			"noDW",
			"Use DW",
			0x3C2F09BAD1862417
		},
		{
			"totalSampleRateQoS",
			"Total sample rate for sandbox - random + static - for QoS channel",
			0x525F34304F9BAAEE
		},
		{
			"totalSampleRateBlackBox",
			"Total sample rate for sandbox - random + static - for BB channel",
			0x4DA2489E226B28ED
		},
		{
			"totalSampleRateSurvey",
			"Total sample rate for sandbox - random + static - for Survey channel",
			0x78C2FC960283DCA7
		},
		{
			"totalSampleClientTrack",
			"Total sample rate for sandbox - random + static - for Survey channel",
			0x6FC1DFC8F3F2A237
		},
		{
			"dwConsideredConnectedTime",
			"Time in milliseconds between the disconnect from Demonware happens and a Com_Error for that reason is thrown.",
			0x68DF6EA751918D38
		},
		{
			"live_useUno",
			"Use Uno Account Features",
			0x424F5443BC394E81
		},
		{
			"live_useUmbrella",
			"Use Umbrella Account Features",
			0x65B6131438338453
		},
		{
			"live_connect_mode",
			"LiveConnect operation mode:0-disabled, 1:auto, 2:initial auto+manual reconnect, 3:manual",
			0x6C851E2925BC63E4
		},
		{
			"motdDelay",
			"Delay after which the MOTD is shown again.",
			0x6BCD346E1EA28F68
		},
		{
			"motd_enabled",
			"Dvar to enable/disable the Treyarch MOTD message.",
			0x304FF2012C26D8FB
		},
		{
			"liveNeverHostServer",
			"Set dvar to true if you like the client to never host.",
			0xDD0E0921DED03A2
		},
		{
			"liveDedicatedOnly",
			"Set dvar to true if dedicated-only playlist",
			0x4E506F1662EB2DF6
		},
		{
			"allowAllNAT",
			"",
			0x47B0B453BFEB728
		},
		{
			"live_statscaching",
			"If true will cache statchanged msgs from server and apply atomically",
			0x6F1D92299A221B60
		},
		{
			"ui_enableConnectionMetricGraphs",
			"Control showing the Connection Metrics graph in the game options menu. 1 will turn on mode 1, 2 mode 2, 3 both. 0 would turn both off",
			0x76C3A47AF788F9D2
		},
		{
			"partyChatDisallowed",
			"Whether to disallow Xbox Live Party Chat",
			0x4E21BBE3614F731D
		},
		{
			"live_autoEventPumpDelay",
			"Update delay for auto events",
			0x2E3D83ED91FA41A
		},
		{
			"live_autoEventPumpTime",
			"Next pump time",
			0xB6BF07547EA7712
		},
		{
			"live_autoEventEnabled",
			"Enable/disable the autoevent system",
			0x67112C4349F6F3CE
		},
		{
			"survey_chance",
			"Chance that a match will have post-game surveys shown",
			0x1583519CFDFE4574
		},
		{
			"survey_count",
			"Number of active surveys",
			0x1BCC8BA98DA6B407
		},
		{
			"qos_minProbes",
			"Minimum probe results required before early outing qos",
			0x1F123BB720024336
		},
		{
			"qos_minPercent",
			"Minimum percentage of probe results required before early outing qos",
			0xBF3D51224EADDAA
		},
		{
			"qos_firstUpdateMS",
			"MS to wait before deciding to early out qos",
			0x95ED89C6C609D26
		},
		{
			"qos_lastUpdateMS",
			"MS since last update required to early out qos",
			0x3127D354AE4B1E8A
		},
		{
			"qos_maxProbeWait",
			"Max MS for matchmaking QoS. Used only for percentage estimation.",
			0x21188DC24F4B9776
		},
		{
			"qos_minEchoServers",
			"Minimum number of echo servers per pop. Code will pad echo servers to this number. Set to zero to disable padding.",
			0x15F32F91BBB1326C
		},
		{
			"qos_echo_chance",
			"Percent chance to run and capture qos echo results",
			0x6F12E145A026520F
		},
		{
			"bandwidth_retry_interval",
			"Interval at which Bandwidth test will be retried",
			0x77BDC1245ED90121
		},
		{
			"reportUserInterval",
			"The interval in minutes you wait before getting another vote on this console as long as the console is turned on.",
			0x6E26804EB71C551
		},
		{
			"live_presence_platform",
			"Presence through first party, 0:none, 1:basic(online/offline), 2:intitle, 3:extended(in title details), 4:extended+party",
			0x4EB27945E931C905
		},
		{
			"live_presence_incremental_fail_delay",
			"Every time console presence fails to set, the retry delay is incremented by << 1ms. This will limit exponential delay growth.",
			0x3FCF04213052818B
		},
		{
			"live_presence_party",
			"Support list of party members in the presence",
			0x5AC48222CED6A732
		},
		{
			"live_presence_features",
			"Bitmask for various presence features",
			0xA8BA2294A7603F1
		},
		{
			"live_friends_enabled",
			"Platform Friends system is enabled",
			0xA9F5FC541DA5097
		},
		{
			"live_friends_max",
			"Friends/Recent sorrting, 0-unsorted, 1-Most REcent, 2-Alphabetical",
			0x21E7A7B2114172AC
		},
		{
			"live_friends_batch_size",
			"Platform Friends system batched fetch, batch size",
			0x254AC3CB1B9104DA
		},
		{
			"live_friends_update_interval",
			"Minimum time required before friends can be updated again.",
			0x399F5C63B7975825
		},
		{
			"live_friends_sort",
			"Friends sorrting, 0-unsorted, 1-Most Recent Ingame/Online/Offline, 2-Alphabetical",
			0x3ED8A520F194AACA
		},
		{
			"live_friends_features",
			"Bitmask for various presence features",
			0x651EC207263FB25D
		},
		{
			"groups_enabled",
			"This is used to enable the groups feature.",
			0x41E464135723AE03
		},
		{
			"groups_self_groups_refresh_time",
			"Time to periodically update our own groups.",
			0x54EF72964B3C7EB7
		},
		{
			"groups_admins_refresh_time",
			"Time to periodically update admins for each group.",
			0x3E99730627DFD44E
		},
		{
			"groups_invites_refresh_time",
			"Time to periodically update group invites.",
			0xEF40B6CFA80F7F4
		},
		{
			"groups_applications_refresh_time",
			"Time to periodically update group join requests.",
			0xAAE3B9B281F217F
		},
		{
			"groups_quickjoin_players_refresh_time",
			"Time to periodically update quickjoin list with group members.",
			0x36E74BC20938CCAC
		},
		{
			"groups_service_failure_backoff_time",
			"Time to back off before sending off any more tasks in case teams service is off.",
			0x58F9DA716EA2F405
		},
		{
			"groups_presence_refresh_time",
			"Interval between refreshing presence for fetched group members, 0 is disabled",
			0x10ABABCD3F4F79E1
		},
		{
			"ui_hideLeaderboards",
			"Hide leaderboard buttons in the UI",
			0x62C8BE4B980621A0
		},
		{
			"live_leaderboardResetTime",
			"Amount of time to wait for the leaderboard reset task to finish before canceling",
			0x5512AB9A52712167
		},
		{
			"liveVoteTaskDelay",
			"Delay between consequent vote history page tasks",
			0x74BF17E7AC8ADCA1
		},
		{
			"liveVoteErrorBackoff",
			"Backoff time if a vote history page task fails",
			0x356505486BCDE425
		},
		{
			"keyarchiveWriteDelay",
			"Minimum delay between writes to keyarchive",
			0x7F63A76B09D09120
		},
		{
			"inventory_enabled",
			"This is used to enable the user inventory feature.",
			0x2FF6459C5809D381
		},
		{
			"inventory_maxPages",
			"Max pages to fetch for player inventory.",
			0x17CF1E28E946A712
		},
		{
			"inventory_itemsPerPage",
			"Number of items to fetch per inventory page.",
			0x53C15C57528BB850
		},
		{
			"inventory_fetch_cooloff",
			"Time in milliseconds to wait between inventory pages.",
			0x7643C86B1447FD11
		},
		{
			"inventory_retry_delay",
			"Time in milliseconds to wait between inventory fetch retryies.",
			0x68DD22741E79C516
		},
		{
			"inventory_retry_max",
			"Max inventory fetch retryies.",
			0x5DEB6273E96414EF
		},
		{
			"balances_retry_delay",
			"Time in milliseconds to wait between balance fetch retryies.",
			0x689D3B2D91721335
		},
		{
			"balances_retry_max",
			"Max balance fetch retryies.",
			0x15E95E5481231F84
		},
		{
			"inventory_blocking",
			"if false, don't wait on the inventory before allowing online play",
			0x15D2ECA1C83CB4CF
		},
		{
			"rare_crate_bundle_id",
			"Loot crate Bundle ID.",
			0x3BE43DD3422DB56E
		},
		{
			"weapon_contract_incentive_id",
			"Special contract incentive item/sku id.",
			0x2E2BF6D1BCC707BA
		},
		{
			"weapon_contract_max",
			"Weapon contract max inventory sentinel items.",
			0x7534937EDEFDEE09
		},
		{
			"weapon_contract_target_value",
			"Target value needed to complete the weapon contract.",
			0x39D4B13D578204EA
		},
		{
			"enable_weapon_contract",
			"Special Contract is enabled/disabled",
			0xD012B74617CA116
		},
		{
			"daily_contract_cryptokey_reward_count",
			"Number of cryptokeys to reward for completing a daily contract.",
			0x3195D6AE64B02E04
		},
		{
			"weekly_contract_cryptokey_reward_count",
			"Number of cryptokeys to reward for completing a set of weekly contracts.",
			0x36153C059DD14D34
		},
		{
			"weekly_contract_blackjack_contract_reward_count",
			"Number of Blackjack contracts to reward for completing a set of weekly contracts.",
			0x6B679E22FF03E151
		},
		{
			"skip_contract_rewards",
			"Skip giving rewards for the special contract and daily/weekly contracts for debug purposes.",
			0x70473DE34E310162
		},
		{
			"incentive_rare_drop_id",
			"ID for rare crates awarded by contract or season pass incentives",
			0x2A123E4596C717C3
		},
		{
			"incentive_weapon_drop_id",
			"ID for weapon bribe crates awarded by contract or season pass incentives",
			0x62D82ADCD1920153
		},
		{
			"platformSessionShowErrorCodes",
			"Show platform session error codes",
			0x9D38D804EC061EB
		},
		{
			"platformSessionPartyPrivacy",
			"Show platform session party privacy",
			0x639176BF65C21B9A
		},
		{
			"platformSessionLaunchInviteJoinProcessDelay",
			"Platform session launch invite join process delay",
			0x5670F1660AB82174
		},
		{
			"live_umbrella_maxUmbrellaLoginAttempts",
			"The maximum # of attempts we will make toauth Umbrella for a given controller",
			0x7E10B161BE74E264
		},
		{
			"fshSearchTaskDelay",
			"Gap in milliseconds between file share search tasks.",
			0x6F9E730BA10DB9F6
		},
		{
			"fshThrottleEnabled",
			"Whether the dcache upload throttling is enabled.",
			0x6915A46003EC7133
		},
		{
			"fshThrottleKBytesPerSec",
			"Dcache upload throttle limit in K Bytes per second.",
			0x577C012AE260F19E
		},
		{
			"fileshare_enabled",
			"Enable or disable the Fileshare feature.",
			0x456FCD2DEBF35884
		},
		{
			"fileshare_tier",
			"current fileshare tier for all users on this console.",
			0x764D93244523D85F
		},
		{
			"fshRecentsXUID",
			"Override recent games to come from this player instead",
			0x6A0CF60E8B642C10
		},
		{
			"fshSummaryDelay",
			"Delay between summary tasks. Tweak to ensure no BD_TOO_MANY_TASKS.",
			0x1F228C612B53B861
		},
		{
			"fshRetryDelay",
			"Delay after which a failed fileshare fetch will be retried",
			0x3AED76678509D01F
		},
		{
			"fshEnableRender",
			"Turn on or off the ability to render clips.",
			0x26CB0F57709C8BC9
		},
		{
			"fileshareRetry",
			"Retry fetching fileshare data on failure.",
			0x7F571DFAB6ED0368
		},
		{
			"fileshareAllowDownload",
			"Allow Fileshare downloads",
			0x7560F792B04412B3
		},
		{
			"fileshareAllowDownloadingOthersFiles",
			"Allow downloading other users' Fileshare files",
			0x2C36A3EDFB2B89E5
		},
		{
			"fileshareAllowPaintjobDownload",
			"Allow Fileshare Paintjob downloads",
			0x78A461C8EB7B3384
		},
		{
			"fileshareAllowEmblemDownload",
			"Allow Fileshare Emblem downloads",
			0x3D672BA05F9BEF25
		},
		{
			"emblemVersion",
			"Version for Emblems.",
			0x532016D3C3BE9875
		},
		{
			"paintjobVersion",
			"Version for Paintjob.",
			0x7A93BC0BE62DB7A6
		},
		{
			"enable_camo_materials_tab",
			"Enable the camo materials tab.",
			0x38B4EC18019A2C9A
		},
		{
			"live_username",
			"DW userename",
			0x7BCEE5B863A1078
		},
		{
			"profileDirtyInterval",
			"minimum interval (in milliseconds) between updating our profile, except when an update is forced",
			0x250B6E555808876B
		},
		{
			"profileGetInterval",
			"minimum interval between own profile retrieval",
			0x39F6886A5D58B86F
		},
		{
			"groupUploadInterval",
			"Minimum interval to wait before setting new group counts",
			0x5052073EEEE7CE50
		},
		{
			"groupDownloadInterval",
			"Minimum interval to wait before getting new group counts",
			0x158AACA299D61F5
		},
		{
			"groupCountsVisible",
			"Toggles the group counts in the lobby and playlist menus.",
			0x1ADD80F31337B482
		},
		{
			"pcache_privacy",
			"Controlls the support for fetching Privacy settings from 1st party through pcache",
			0xFEBACE7D36303D4
		},
		{
			"loot_cryptokeyCost",
			"lootxp required to buy one crypto key. Must match the DW backend modifier.",
			0x5BAEC7131FED35D5
		},
		{
			"loot_cryptokeySku",
			"SKU used to purchase crypto keys by spending lootxp. Exchange rate is governed by 'crypto_key_cost'. Must match sku setup on DW.",
			0x1A200EE709FDC2C9
		},
		{
			"lootxp_multiplier",
			"multiplier for double lootxp",
			0x54CB81F80166E79F
		},
		{
			"loot_commonCrate_dwid",
			"common crate dw id",
			0x657DC9FCEB46B345
		},
		{
			"loot_bribeCrate_dwid",
			"bribe 1 crate dw id",
			0x42983944283E2F4E
		},
		{
			"loot_rareCrate_dwid",
			"common crate dw id",
			0x61E6A8834E746E50
		},
		{
			"loot_mpItemVersions",
			"MP loot item version",
			0x27EC5101FF8361D3
		},
		{
			"loot_zmItemVersions",
			"ZM loot item version",
			0x1FE5ADAF8E375DF
		},
		{
			"loot_enabled",
			"Enabled or disables loot",
			0x3EC641DDC0A465E9
		},
		{
			"loot_cryptoCheckDelay",
			"Cooloff period checking for lootxp->cryptokey conversion",
			0x32D8C55014C9A25A
		},
		{
			"tu11_lootCryptoAutoRetry",
			"Enabled or disables auto lootxp->cryptokey conversion",
			0x7889330B0D3267E5
		},
		{
			"loot_burnBatchSize",
			"Batch size for burning duplicate items",
			0x21D791CABDDF87A
		},
		{
			"loot_burnCooloff",
			"Cooloff period before burning the next batch of items",
			0x7CF2540405181DE1
		},
		{
			"loot_burnRefetchOnSuccess",
			"Refetch inventory when a burn is successful",
			0x10FA7626F75D5DC4
		},
		{
			"loot_burnCommonRefund",
			"Cryptokeys refunded when a Common item is burned",
			0x1F47DF8371DDB95E
		},
		{
			"loot_burnRareRefund",
			"Cryptokeys refunded when a Rare item is burned",
			0x3529C093CB10C247
		},
		{
			"loot_burnLegendaryRefund",
			"Cryptokeys refunded when a Legendary item is burned",
			0x610F1F0B8B9F240
		},
		{
			"loot_burnEpicRefund",
			"Cryptokeys refunded when an Epic item is burned",
			0x18866B4E6E036D8A
		},
		{
			"loot_burnMinMegaRequired",
			"Minimum number of mega gobblegum required to burn",
			0x7C2541F3FD37CB64
		},
		{
			"loot_burnMinRareRequired",
			"Minimum number of rare gobblegum required to burn",
			0x679FDEC337F446A2
		},
		{
			"loot_burnMinUltraRequired",
			"Minimum number of ultra gobblegum required to burn",
			0xBA9673DB515C92A
		},
		{
			"live_store_enable",
			"Enable/Disable Store button in UI.",
			0x514F69F96CEB06F9
		},
		{
			"live_store_enable_inventory",
			"Enable/Disable inventory fetch.",
			0x1861A7458471BFC6
		},
		{
			"store_item_viewed_timer",
			"Minimum time (in ms) player should look at a store item to register a comscore event",
			0x6B1BF1B329DA7BA7
		},
		{
			"live_store_show_details",
			"Enable/Disable product browse.",
			0x4DF1F8667B658AF6
		},
		{
			"live_store_disable_lang",
			"Disable store for the available language based on value of dvar loc_availableLanguages.",
			0xED2201F94165577
		},
		{
			"live_store_disable_region",
			"Disable store for the given SKU region.",
			0x3A06182194A98F5F
		},
		{
			"live_enablePolls",
			"If true, polls will fire off demonware tasks",
			0x7D6F7033B3CBEFFF
		},
		{
			"xblive_matchEndingSoon",
			"True if the match is ending soon",
			0x1930826547682687
		},
		{
			"live_pubSemaphoreCheckIntervalSeconds",
			"Interval in seconds between checking the backend to see if the pubsemaphore timestamp has changed",
			0x46298D1906A8D521
		},
		{
			"live_pubSemaphoreJitterSeconds",
			"Jitter in seconds to apply to live_pubSemaphoreCheckIntervalSeconds",
			0x79FB304CA592D50A
		},
		{
			"live_pubSemaphoreForceChange",
			"If true pubsemaphore will always signal as changed",
			0xD886CFC3E211B9E
		},
		{
			"live_pubSemaphoreUserTriggerTime",
			"time when the user confirmed a game settings changed popup",
			0x61A5DB82D8C14329
		},
		{
			"stat_version",
			"Stats version number",
			0x2D3ACDB765E3A044
		},
		{
			"stats_version_check",
			"Reset stats if version numbers do not match",
			0x265573B1313AE7CC
		},
		{
			"maxStatsBackupInterval",
			"Maximum number of days before performing stats backup to Demonware User Storage",
			0x1DF65DE154336663
		},
		{
			"zero_stats_check",
			"Check for zeroed out stats from Demonware",
			0x511151061E142E78
		},
		{
			"dwFileFetchTryIntervalBase",
			"Delay in seconds after file fetch fails to the next retry.  Exponential growth each retry.",
			0x3CCF9927A600C7BB
		},
		{
			"dwFileFetchTryIntervalMax",
			"Max delay in seconds between file fetch tries.",
			0x395223811BA33748
		},
		{
			"dwFileFetchTryMaxAttempts",
			"Max retry attempts before stopping altogether.",
			0x6589D56B17D70C3D
		},
		{
			"waitOnStatsTimeout",
			"Time in seconds to wait for stats to be fetched while dev mapping.",
			0x112A3EC30994F0FB
		},
		{
			"minDelayForOtherPlayerStatsFetch",
			"Delay in milliseconds to refetch the stats of a player.",
			0xEB0B5C7E1D13D45
		},
		{
			"statsLocationFatal",
			"If true, a zombie stats error will cause the game to end, if false a warning is printed to the console and the game continues",
			0x3D6403AE9B540CAF
		},
		{
			"statsCheckXuidBeforeStatsUpload",
			"Checks the XUID of the stats before uploading. Used as bitmask for each eMode(1 << mode)",
			0x2F96F30B7A7EEBC9
		},
		{
			"statsErrorNormal",
			"Error if an invalid stats access is requested on NORMAL stats.",
			0x3AA8AD957724ADA9
		},
		{
			"statsErrorStable",
			"Error if an invalid stats access is requested on STABLE stats.",
			0x63C563C0735A0D
		},
		{
			"statsErrorOtherPlayer",
			"Error if an invalid stats access is requested on OTHERPLAYER stats.",
			0x5BC9EEC9F12DCE9D
		},
		{
			"livestorage_fakeNoob",
			"Force this player to be treated like a noob",
			0x12ABEB1A03756136
		},
		{
			"purchasedClassSetCount",
			"Number of class sets available when purchased",
			0x2142A22063B24F79
		},
		{
			"defaultClassSetCount",
			"Number of class sets available when not purchased",
			0xFF412D378A4FF95
		},
		{
			"live_timewarp",
			"Offset calculated live time by this amount of days",
			0x59E760D0F14AE13B
		},
		{
			"tu9_backupBadDecompress",
			"Create an online backup when decompression fails",
			0x6A8952240D0AC6F0
		},
		{
			"tu9_backupBadSpace",
			"Create an online backup when there is insufficient space for decompression",
			0x57C84253DD30221B
		},
		{
			"tu10_validationFatal",
			"Disconnect completely when a stats validation error is detected",
			0x7EF64CF71033AD43
		},
		{
			"tu12_delayFFOTD",
			"ffotd waits on the playlist to first be downloaded, the execs everything in the proper sequence.",
			0x1BB581108319DD0E
		},
		{
			"tu10_enableCyclicalDailyChallenges",
			"Enable cyclical daily challenges",
			0x4BAA56D6CA76F50
		},
		{
			"skill_teamBeta",
			"Beta value for team based skill calculation",
			0x55DAE8F1DBE070F0
		},
		{
			"skill_onboardingEnter",
			"Initial skill for players entering the onboarding pool",
			0x5D596089B4139224
		},
		{
			"skill_onboardingExit",
			"Initial skill for players leaving the onboarding pool",
			0x3813AEE9B163746A
		},
		{
			"skill_onboardingThreshold",
			"Skill value that will force players out of the onboarding pool",
			0x573F2FA1606C09FD
		},
		{
			"skill_onboardingGames",
			"Number of games to player before moving the player out of the onboarding pool",
			0x1735443A940137E9
		},
		{
			"skill_onboardingAdjustOnExit",
			"Whether of not skill values should be adjusted when exiting onboarding pool",
			0x359776C98A2ECB4C
		},
		{
			"skill_onboardingEnabled",
			"Whether or not onboarding is enabled for new players.",
			0x35FD28B79E60A31F
		},
		{
			"allowHost_matchesHostedRatio",
			"Required match completion/hosted ratio. 0 is off.",
			0x14D959F9CE846C4C
		},
		{
			"allowHost_matchesPlayedRatio",
			"Required match completion/played ratio. 0 is off.",
			0x60A221B0500B366C
		},
		{
			"disableHost_matchesHostedRatio",
			"Required match hosted completion/failure ratio. 0 is off.",
			0x2ED3F5BDC6787D0B
		},
		{
			"disableHost_matchesPlayedRatio",
			"Required match played completion/failure ratio. 0 is off.",
			0x19B1C23FBE338457
		},
		{
			"behaviorTrackerVersionNumber",
			"Version number for the behavior tracker",
			0x696D9FC45BE57EA
		},
		{
			"probation_version",
			"",
			0x15791DA116824B0C
		},
		{
			"probation_public_enabled",
			"Probation enabled",
			0x1161F73C69A2E4E7
		},
		{
			"probation_public_quitScalar",
			"How much each quit is factored into the probation",
			0x4D3075E7488FB0C1
		},
		{
			"probation_public_timeoutScalar",
			"How much each timeout is factored into the probation",
			0xC2C15F1DCB07EA5
		},
		{
			"probation_public_kickScalar",
			"How much each kick is factored into the probation",
			0x7F9B6B07BA2C9F38
		},
		{
			"probation_public_matchHistoryWindow",
			"How many games are evaluated to determin probation.",
			0x193941C02761329D
		},
		{
			"probation_public_probationThreshold",
			"How many violations puts the player over the limit.",
			0x64D06AFB221D250D
		},
		{
			"probation_public_warningThreshold",
			"How many violations causes the player to get warned.",
			0x178D54890E821071
		},
		{
			"probation_public_probationTime",
			"Time the player gets punished in minutes.",
			0x7B0E75C7433D0183
		},
		{
			"probation_public_forgiveCount",
			"How many violations to forgive when probation is given",
			0x27B7BAD494922037
		},
		{
			"probation_league_enabled",
			"Probation enabled",
			0x2CA03D36EA11AB0B
		},
		{
			"probation_league_quitScalar",
			"How much each quit is factored into the probation",
			0x23369C53BFC4A83D
		},
		{
			"probation_league_timeoutScalar",
			"How much each timeout is factored into the probation",
			0x512A4ADD52B273C9
		},
		{
			"probation_league_kickScalar",
			"How much each kick is factored into the probation",
			0x2B1667544F18824
		},
		{
			"probation_league_dashboardScalar",
			"How much each dashboard is factored into the probation",
			0x19E3BFFA3914C0F6
		},
		{
			"probation_league_matchHistoryWindow",
			"How many games are evaluated to determin probation.",
			0x296988DCFD7F3FA9
		},
		{
			"probation_league_probationThreshold",
			"How many violations puts the player over the limit.",
			0x58849275CA1E3AE9
		},
		{
			"probation_league_warningThreshold",
			"How many violations causes the player to get warned.",
			0x2122E9BA9041C1ED
		},
		{
			"probation_league_probationTime",
			"Time the player gets punished in minutes.",
			0x4F631B62128031D7
		},
		{
			"probation_league_forgiveCount",
			"How many violations to forgive when probation is given",
			0x7097B82B46A66AC3
		},
		{
			"fixNegativeLosses",
			"Fix negative losses in combat record",
			0x536CD41D57579E2C
		},
		{
			"livestats_skipFirstTime",
			"Disables first time flow for all session modes.",
			0x62BE8FB76AD6AEA3
		},
		{
			"loot_taskWindow",
			"time alloted to the reward task, after which a retry will occur",
			0x1A52CF5F59246D05
		},
		{
			"loot_taskMaxAttempts",
			"max retries before the reward task is considered a failure",
			0x17598E3F123CF14B
		},
		{
			"tu4_enableCodPoints",
			"Enable display and spending of COD Points",
			0x524F4493E0257D85
		},
		{
			"local_media_tier",
			"current local media tier for all users on this console.",
			0x4FA6A41CE0F84F78
		},
		{
			"statsVersionMP",
			"design version for MP stats",
			0x7AEFEDDC58F9E985
		},
		{
			"statsVersionCP",
			"design version for CP stats",
			0x7ACDF1DC58DD09EB
		},
		{
			"statsVersionZM",
			"design version for ZM stats",
			0x7B1BE0DC591F0E59
		},
		{
			"statsVersionFR",
			"design version for FR stats",
			0x7AD7E1DC58E54340
		},
		{
			"loadoutVersionMP",
			"design version for MP loadouts",
			0x62C3C580035FD180
		},
		{
			"loadoutVersionZM",
			"design version for ZM loadouts",
			0x62A5C2800346CED8
		},
		{
			"lobby_hostBots",
			"Enables lobby bots",
			0x547BB8673052413C
		},
		{
			"lobbyvm_forceGC",
			"Force garbage collection after lobby pump to keep memory usage in check",
			0x550FF919028CC160
		},
		{
			"ui_lobbyDebugVis",
			"master switch for turning on or off lobby debug ui",
			0x868D54ACE3910EB
		},
		{
			"ui_lobbyDebugClients",
			"Enables and sets the Lobby Type for Client Debugging",
			0x7D92D9DB09C4181D
		},
		{
			"ui_lobbyDebugOverlay",
			"Turn Lobby Debug Overlay ON/OFF",
			0x6EF2722E9DA903F9
		},
		{
			"ui_lobbyDebugBaseX",
			"debug ui base x",
			0x706255C3ACBF781A
		},
		{
			"ui_lobbyDebugBaseY",
			"debug ui base x",
			0x706256C3ACBF79CD
		},
		{
			"ui_lobbyDebugExtraInfoLoc",
			"game mode loc",
			0x6DF332536E7673DF
		},
		{
			"ui_lobbyDebugSessionSQJ",
			"Enables/Disables session search debugging",
			0x5102D263505BECD9
		},
		{
			"ui_lobbyDebugVoice",
			"Enables/Disables lobby voice debugging",
			0x47514411BBEAF
		},
		{
			"ui_abortKeyboardOnUIChange",
			"Abort any onscreen keyboard operations when the lobby system changes the UI screen",
			0x5FA1AEC845987540
		},
		{
			"tu11_matchmakingPriorityMP",
			"MP matchmaking priority setting",
			0x3B7E7D205283F9AA
		},
		{
			"tu11_matchmakingPriorityMPIgnoreTime",
			"MP amount of time that a session will be ignored for",
			0x393504697ED9246D
		},
		{
			"party_minplayers",
			"Minimum number of players in a party",
			0x2500E4DB2F10F5EE
		},
		{
			"party_maxplayers",
			"Maximum number of players in a party",
			0x596545C72F0E4FE4
		},
		{
			"party_autoteams",
			"Enables/Disables auto team functionality",
			0x2482606B77C310A7
		},
		{
			"lobby_maxLocalPlayers",
			"Enables/Disables auto team functionality",
			0x66BAC8B42067D2C5
		},
		{
			"lobby_forceLAN",
			"Force LAN lobby if > 0, 1-LAN offline, 2-LAN and connected to dw",
			0x44DE8F206FD45EEA
		},
		{
			"lobby_showSVDebug",
			"shows sv debug info in the frontend",
			0x28B546FBE4B2AF13
		},
		{
			"lobbymigrate_MaxBWPackets",
			"number of bw test packets to send when we have the token",
			0x1CB0320F843FECB
		},
		{
			"lobbymigrate_MaxPacketWaitTime",
			"ms to wait for test packet responses after last packet is sent",
			0x5E0E655C82BD8DD9
		},
		{
			"lobbymigrate_NomineeWaitMS",
			"ms to wait for each nominee to send us a partystate",
			0x3461F602C3EC7F19
		},
		{
			"lobbymigrate_HostWaitMS",
			"ms to send hostannounce packets before sending partystates",
			0x3D04D8184D5237F6
		},
		{
			"lobbymigrate_NomineeRecalcInterval",
			"ms to wait between recalculating migration nominees",
			0x63766A793BE4AC8D
		},
		{
			"lobbymigrate_TestInterval",
			"base ms to wait between test rounds, see lobbymigrate_TestIntervalJitter",
			0x2D7E4E3258446878
		},
		{
			"lobbymigrate_TestIntervalJitter",
			"ms jitter between test rounds",
			0x29CD2D8303C0B9EE
		},
		{
			"lobbymigrate_MinScoreDiff",
			"minimum score nominee must differ by before rank will change",
			0x20EFC56A1BCFC618
		},
		{
			"lobbymigrate_WirelessLatencyIncrease",
			"percentage increase for wireless client latency during host selection",
			0x48EF2DF9DA3140B5
		},
		{
			"lobbymigrate_useStdDev",
			"If true we'll use the standard devation of the clients latency as a tie-breaker instead of the clientnum",
			0x7D62DC2DCC9C357C
		},
		{
			"lobbyMigrate_Enabled",
			"Set to false to disable lobby migration",
			0x1B20187487D3509E
		},
		{
			"lobbyMigrate_EnabledLAN",
			"Set to false to disable lobby migration in LAN mode",
			0x6210238C373449C1
		},
		{
			"lobbyMigrate_migrateToBestGameHostEnabled",
			"Set to false to disable migrate to best game host migration",
			0x77255714A6A48B0A
		},
		{
			"lobbyMigrate_dedicatedOnly",
			"only allow dedicated servers to host",
			0x47EBF1500DC54DA4
		},
		{
			"lobbyMigrate_considerStreamingForHostMigration",
			"When true, will consider the factor of whether the host is streaming or not, when nominating for host migration",
			0x1403A1EDE03BB4A5
		},
		{
			"lobby_clientTimeout",
			"Seconds with no received packets from host until a timeout occurs",
			0x6B76B42B0169563E
		},
		{
			"lobby_clientWarnTimeout",
			"Seconds with no received packets from host until a host missing warning is displayed",
			0x3400A1AC84221B6E
		},
		{
			"lobby_clientContentTimeout",
			"Milli-Seconds to finish sending the active client data until a timeout occurs",
			0x7DB3090EE0D467AF
		},
		{
			"lobby_clientEndCooloffTime",
			"Miliseconds before dropping net chan channels to host",
			0x44D076BCC8EA1A3F
		},
		{
			"lobby_clientEndFakeSendLoss",
			"When a match end and are returning to the front end, check if we have full loaded the UI level",
			0x7B6F8090CE75BC9C
		},
		{
			"lobby_clientLoadingIntoUICheck",
			"When a match end and are returning to the front end, check if we have full loaded the UI level",
			0x21A55E1609045893
		},
		{
			"lobbyLaunch_fadeToBlackDelay",
			"Once the launch game timer has started, when the fade to black will happen",
			0x1F6674CAE730AA08
		},
		{
			"lobbyLaunch_gameLaunchDelay",
			"Once the launch game timer has started, when the launch will happen",
			0x3AC3D72F21D83F1B
		},
		{
			"lobbyLaunch_fadeToBlackDelayOnConnect",
			"Once the launch game timer has started, when the fade to black will happen",
			0xCD73B6D54F107A5
		},
		{
			"lobbyLaunch_gameLaunchDelayOnConnect",
			"Once the launch game timer has started, when the launch will happen",
			0x68663791C854DEC4
		},
		{
			"lobbyLaunch_waitForClientAckDelay",
			"Time to wait for acknowledgments from the clients before launching",
			0x2E0639684531C4C8
		},
		{
			"lobby_hostTimeout",
			"seconds without any message from client before they are disconnected",
			0x3C2A63995EDF4741
		},
		{
			"lobby_hostIntervalState",
			"lobby state message intervals",
			0x21ACD63ED36D63B4
		},
		{
			"lobby_hostIntervalHeartbeat",
			"lobby heartbeat message intervals",
			0x25E441ECA4C55181
		},
		{
			"lobby_hostContentTimeout",
			"Seconds to finish sending the host content until a timeout occurs",
			0x39777C9A27DF8E2A
		},
		{
			"lobby_enablePSDataStreamingInGame",
			"Turn on the in-game streaming of paintshop data to all clients marked dirty",
			0x2836622FCD9AC11F
		},
		{
			"lobby_enableLoadoutDataStreamingInGame",
			"Turn on the in-game streaming of loadout data to all clients marked dirty",
			0x7867B95DC8C4230A
		},
		{
			"lobby_hostPSFragmentRetryInterval",
			"Paintshop data fragment retry interval",
			0x2600410670BD4182
		},
		{
			"lobby_enablePSDataStreamingInLobby",
			"Turn on the paintshop data to be shared in the frontend lobbies",
			0x7639D7847E1D3B
		},
		{
			"lobby_ingamePSDataStreamingBW",
			"Control In-Game Paintshop data streaming Bandwidth(total bumber of paintshop bytes to be sent per snapshop)",
			0x49D12FB1ED3F005B
		},
		{
			"lobby_moveLobby",
			"seconds before a lobby move is considered a failure",
			0x7050EF8E2D8622E7
		},
		{
			"lobby_hostJoinRequestReject",
			"Have the host reject incomming connections with random reasons",
			0x2C16E97C130C620C
		},
		{
			"join_agreementWindow",
			"all clients must agree to join the new host in this window",
			0x57043A928B14422A
		},
		{
			"join_handshakeWindow",
			"server must respond to reservation request within this window",
			0x790D64342518F3FB
		},
		{
			"join_completionWindow",
			"once a join is complete, the old host remains active for this amount of time before deactivating itself",
			0x2638FCB20DC52814
		},
		{
			"join_infoProbeWindow",
			"time to wait for an info probe IM response",
			0x7943C77D227FF348
		},
		{
			"join_retryDelay",
			"if the host is busy, we'll try re-joining after this cooloff",
			0x69A67CC8E74D05CF
		},
		{
			"join_retryCount",
			"if the host is still busy after these many retries, move on",
			0x5D50F5CB3997496F
		},
		{
			"join_reservationWindow",
			"host join reservation window in ms",
			0x2C13C8872AEA0758
		},
		{
			"lobbymsg_prints",
			"Turns on and off the message debug prints",
			0xFEFA8EC975C9D0D
		},
		{
			"lobbyDebugLogJoins",
			"Logs join results to the kvs",
			0x15325B3EB423F269
		},
		{
			"lobbyDebugLogJoinSuccess",
			"Includes join success results while logging",
			0xBAD693F0A579CA3
		},
		{
			"lobby_ptptimeout",
			"Time out for connection test",
			0x10FFB6C091A1744B
		},
		{
			"partyPrivacyEnabled",
			"Lobby party privacy on/off",
			0x1E52CB4B7B32961A
		},
		{
			"partyPrivacyPumpEnabled",
			"Lobby party privacy pump on/off",
			0x2FF3E7BFBD5CE85C
		},
		{
			"partyPrivacyCheckInterval",
			"Party privacy check interval",
			0x1C018601EF2EA99A
		},
		{
			"leaveWithPartyEnabled",
			"Lobby leave with party on/off",
			0x61CD7A5D0E152F8D
		},
		{
			"lobbyMergeEnabled",
			"Lobby merge on/off",
			0x406AB21D919144AA
		},
		{
			"lobbyMergeInterval",
			"Lobby merge interval",
			0x747EDB66E0FE444C
		},
		{
			"lobbyMergeDedicatedEnabled",
			"Lobby dedicated merge on/off",
			0x36D033E0551BFF83
		},
		{
			"lobbyAdvertiseSkip",
			"Lobby Advertise: Skip advertisment",
			0x462E6E8627E5372B
		},
		{
			"lobbyAdvertiseDirty",
			"Lobby Advertise: Session is dirty/needs updating",
			0x60506FA050DCE73C
		},
		{
			"lobbyAdvertiseMaxPlayers",
			"Lobby Advertise Param: Max Players",
			0x452B5D4465EE5AD8
		},
		{
			"lobbyAdvertiseNumPlayers",
			"Lobby Advertise Param: Num Players",
			0x58D7611F93A7FCFE
		},
		{
			"lobbyAdvertiseServerType",
			"Lobby Advertise Param: server type",
			0x64EC2C20CC5CBE0F
		},
		{
			"lobbyAdvertiseShowInMatchmaking",
			"Lobby Advertise Param: Show in matchmaking",
			0x50E5350E8F7BD556
		},
		{
			"lobbyAdvertiseNetcodeVersion",
			"Lobby Advertise Param: Netcode version",
			0x21B1541ECD98A72
		},
		{
			"lobbyAdvertiseMapPacks",
			"Lobby Advertise Param: Map packs",
			0x7A96CE8EF5319D80
		},
		{
			"lobbyAdvertisePlaylistVersion",
			"Lobby Advertise Param: Playlist Version",
			0x460990550E224156
		},
		{
			"lobbyAdvertisePlaylistNumber",
			"Lobby Advertise Param: Playlist Number",
			0x705B789559A59177
		},
		{
			"lobbyAdvertiseIsEmpty",
			"Lobby Advertise Param: Is Empty",
			0x688AAC69E4F7C18D
		},
		{
			"lobbyAdvertiseTeamSizeMax",
			"Lobby Advertise Param: Team Size Max",
			0x2A90AF2FA81CF8EA
		},
		{
			"lobbyAdvertiseSkill",
			"Lobby Advertise Param: Skill level",
			0x264614F5CA9FF2B9
		},
		{
			"lobbyAdvertiseGeo1",
			"Lobby Advertise Param: Geographical loc 1",
			0x3C83EFEB4F222B40
		},
		{
			"lobbyAdvertiseGeo2",
			"Lobby Advertise Param: Geographical loc 2",
			0x3C83F2EB4F223059
		},
		{
			"lobbyAdvertiseGeo3",
			"Lobby Advertise Param: Geographical loc 3",
			0x3C83F1EB4F222EA6
		},
		{
			"lobbyAdvertiseGeo4",
			"Lobby Advertise Param: Geographical loc 4",
			0x3C83F4EB4F2233BF
		},
		{
			"lobbyAdvertiseServerLocation",
			"Server location to advertise",
			0x27D325BF9B693A80
		},
		{
			"lobbyAdvertiseLatencyBand",
			"Server latency band to advertise",
			0x2686027EBE0C93F5
		},
		{
			"lobbyAdvertiseMap",
			"Lobby Advertise Param: Map",
			0x61F1674454C3251A
		},
		{
			"lobbySearchSkip",
			"Lobby Search: Skip searches",
			0x1654E95D79AE2D20
		},
		{
			"lobbyDedicatedSearchSkip",
			"Lobby Search: Skip dedicated server searches",
			0x7E7A52A88079870B
		},
		{
			"lobbySearchDelay",
			"Lobby Search: Time delay between searches",
			0x58CAF432075AC8A
		},
		{
			"lobbySearchGeoMin",
			"Lobby Search: Minimum number of geo tiers that must match",
			0x7E01BF4D20878528
		},
		{
			"lobbySearchQueryId",
			"Lobby Search Param: Query ID",
			0xD6F0384367E76DC
		},
		{
			"lobbySearchGameSecurityId",
			"Lobby Search Param: Game security id, used in communication encryption",
			0x180772AC2D5D37FA
		},
		{
			"lobbySearchShowInMatchmaking",
			"Lobby Search Param: Show in matchmaking, flag for shown or hidden games",
			0x778CB42E7042F8CB
		},
		{
			"lobbySearchNetcodeVersion",
			"Lobby Search Param: Netcode Version, of the code base",
			0x1B6D149B6BCE7B6D
		},
		{
			"lobbySearchMapPacks",
			"Lobby Search Param: Search for emabled map packs",
			0x5FD32115F3E81D0F
		},
		{
			"lobbySearchPlaylistVersion",
			"Lobby Search Param: Playlist version of session",
			0x71DCDE4E59ACB1DF
		},
		{
			"lobbySearchPlaylistNumber",
			"Lobby Search Param: Playlist number of session",
			0x257D6B84159AE9C8
		},
		{
			"lobbySearchServerType",
			"Lobby Search Param: Server type of session",
			0x2107859BFDA22E40
		},
		{
			"lobbySearchIsEmpty",
			"Lobby Search Param: If the session is empty",
			0x420D3B772483054C
		},
		{
			"lobbySearchTeamSize",
			"Lobby Search Param: Team size",
			0x7F60C6CF17A46E6B
		},
		{
			"lobbySearchSkill",
			"Lobby Search Param: Session skill level",
			0x20D4F5D5C347C308
		},
		{
			"lobbySearchSkillWeight",
			"Lobby Search Param: Session skill level weight",
			0x4BD1AD7AFB19CD88
		},
		{
			"lobbySearchGeo1",
			"Lobby Search Param: Geographical loc 1",
			0x7F78BBF8409E5E7F
		},
		{
			"lobbySearchGeo2",
			"Lobby Search Param: Geographical loc 2",
			0x7F78BCF8409E6032
		},
		{
			"lobbySearchGeo3",
			"Lobby Search Param: Geographical loc 3",
			0x7F78BDF8409E61E5
		},
		{
			"lobbySearchGeo4",
			"Lobby Search Param: Geographical loc 4",
			0x7F78B6F8409E5600
		},
		{
			"lobbySearchPingBand",
			"Lobby search param: Ping band",
			0x3BD37C49236A4C70
		},
		{
			"lobbySearchDediUnparkPingLimit",
			"Limit on RTT in milliseconds to a given serverloc we'll consider for unparking",
			0x339E15BDB813DCCB
		},
		{
			"lobbySearchServerLocation1",
			"Lobby search param: Server location 1",
			0x2599C296FF29D0A
		},
		{
			"lobbySearchServerLocation2",
			"Lobby search param: Server location 2",
			0x2599B296FF29B57
		},
		{
			"lobbySearchServerLocation3",
			"Lobby search param: Server location 3",
			0x2599A296FF299A4
		},
		{
			"lobbySearchServerLocation4",
			"Lobby search param: Server location 4",
			0x25999296FF297F1
		},
		{
			"lobbySearchServerLocation5",
			"Lobby search param: Server location 5",
			0x25998296FF2963E
		},
		{
			"lobbySearchPingBandWeight1",
			"Lobby Search Param: Ping band weight1 for dedicated matchmaking query",
			0x786E5B4B359DD833
		},
		{
			"lobbySearchPingBandWeight2",
			"Lobby Search Param: Ping band weight2 for dedicated matchmaking query",
			0x786E5C4B359DD9E6
		},
		{
			"lobbySearchPingBandWeight3",
			"Lobby Search Param: Ping band weight3 for dedicated matchmaking query",
			0x786E5D4B359DDB99
		},
		{
			"lobbySearchPingBandWeight4",
			"Lobby Search Param: Ping band weight4 for dedicated matchmaking query",
			0x786E5E4B359DDD4C
		},
		{
			"lobbySearchPingBandWeight5",
			"Lobby Search Param: Ping band weight5 for dedicated matchmaking query",
			0x786E5F4B359DDEFF
		},
		{
			"lobbySearchForceLocation",
			"If true we'll ignore qos results and only search for sessions in this location",
			0x60A007B37323C55D
		},
		{
			"lobbySearchDatacenterType",
			"Do we look for Gameservers or Thunderhead datacenters?",
			0x1811D3265B27B866
		},
		{
			"lobbySearchDatacenterTypeGeo",
			"Gameservers/Thunderhead geo-filter",
			0x7AF990C759653E57
		},
		{
			"lobbySearchDatacenterTypeOverride",
			"Do we look for Gameservers or Thunderhead datacenters - override if != ANY?",
			0x40BF9CEA4DBF2906
		},
		{
			"lobbySearchGeo1Weight",
			"Lobby Search Param: Geographical loc latitude weight for weighted distance matchmaking queries",
			0x66A0C9D1B5E5E5BB
		},
		{
			"lobbySearchGeo2Weight",
			"Lobby Search Param: Geographical loc latitude weight for weighted distance matchmaking queries",
			0x4C6A5C1204B980DA
		},
		{
			"lobbySearchGeo3Weight",
			"Lobby Search Param: Geographical loc latitude weight for weighted distance matchmaking queries",
			0xB39CA390B7FC7D9
		},
		{
			"lobbySearchGeo4Weight",
			"Lobby Search Param: Geographical loc latitude weight for weighted distance matchmaking queries",
			0x7C17DF3E855A4940
		},
		{
			"lobbySearchMaxLatencyBandDiff",
			"Max diff in latency before we reject this host as a possible host",
			0x5C37AB5C77EECFBB
		},
		{
			"lobbySearchMinDediSearchTime",
			"The minimum amount of time required to search/unpark dedi before any listen search",
			0x71372F42FAD45694
		},
		{
			"lobbySearchMinDediSearchClientAdd",
			"The amount of time added to minimum dedi search/unpark for each party member",
			0x3A5FA123002E0DAD
		},
		{
			"lobbySearchBaseSkillRange",
			"Base skill range for public matchmaking",
			0x1AED798FF3820324
		},
		{
			"lobbySearchSkillRangeMultiplier",
			"Rate at which skill range increases for each stage retry",
			0x73D85BE005E3364E
		},
		{
			"lobbySearchForceUnparkLobbySize",
			"Party size that will force a dedicated server unpark",
			0x7A3A27C2D1E550D6
		},
		{
			"lobbySearchSkipUnparkProbability",
			"Probability that we skip unparking",
			0x608CF9277DBC2F6A
		},
		{
			"lobbySearchSkipDLCProbability",
			"Probability we'll search for original maps instead of DLC1",
			0x728AC8324738DCB0
		},
		{
			"lobbySearchPingBandEnabled",
			"If true set the lobbySearchPingBand value before the search",
			0x398C582C5C1D64D1
		},
		{
			"lobbySearchExperimentDatacenter",
			"Datacenter to use for pingband experiment",
			0x5449839A25C9ABE9
		},
		{
			"lobbySearchForceXuid",
			"If set, all search results will be discarded except for specified xuid",
			0x4C7745FD758DB978
		},
		{
			"qosPreferredPing",
			"QoS preferred ping",
			0x7B331A08D4780C2B
		},
		{
			"qosMaxAllowedPing",
			"QoS max allowed ping",
			0x2D12B3AE4A4BEBB8
		},
		{
			"qosLocklessQueue",
			"If true we'll queue up qos results locklessly and push them to the VM on the main thread later",
			0x722E69B3B9F428D3
		},
		{
			"excellentPing",
			"Anything less than this is consider excellent",
			0x19A8F14A21E7424D
		},
		{
			"goodPing",
			"Anything less thant this is considered decent",
			0x38E0DBD8C71F9D10
		},
		{
			"terriblePing",
			"Just awful, limit of playability",
			0xB2C59B4EE116AEA
		},
		{
			"lobbyTimerStartInterval",
			"Lobby timer interval for start match",
			0x21A227834B17E155
		},
		{
			"lobbyCPTimerStartInterval",
			"Lobby CP timer interval for start match",
			0x44832F85F8B2FA6A
		},
		{
			"lobbyCPZMTimerStartInterval",
			"Lobby CPZM or Nightmares timer interval for start match",
			0x405CFC2E0315533
		},
		{
			"lobbyTimerStatusVotingInterval",
			"Lobby timer interval for voting",
			0x4A0A01C39967B404
		},
		{
			"lobbyTimerStatusBeginInterval",
			"Lobby timer interval for match begin",
			0x18A58F5C4D4B8CBE
		},
		{
			"lobbyTimerStatusStartInterval",
			"Lobby timer interval for match start",
			0x1AEDDAFEB3887F37
		},
		{
			"lobbyTimerStatusPostGameInterval",
			"Lobby timer interval for post match",
			0x5F28A3041BBCDF83
		},
		{
			"lobbyTimerStatusVotingInterval_Arena",
			"Lobby timer interval for arena pro voting",
			0x4140BB40F5D43CAC
		},
		{
			"lobbyTimerZMStartInterval",
			"Lobby timer interval for zm start match",
			0x7676A47FE42DA15A
		},
		{
			"lobbyMapVotePrevMapPlayCount",
			"Number of times a previous map cazn be voted for",
			0x2FDE55C3193279A9
		},
		{
			"lobby_beginPlay",
			"use to signal UI_CoD_KeyEvent to force beginPlay on the Main thread",
			0x66CCDF6439ECB1EF
		},
		{
			"lobby_readyUpPercentRequired",
			"The fraction of the waiting players who need to be ready for a party to start",
			0x8A2424003E8E98A
		},
		{
			"lobbyProbedXuidOverride",
			"How long to check new connecting clients against the probed xuids of already connected clients",
			0x3935876169435D5F
		},
		{
			"lobby_MatchmakingLoggingLevel",
			"0 = off, 1 = all on, etc.",
			0x4E2F4F37C098A2A1
		},
		{
			"lobby_MatchmakingLoggingChance",
			"Probability of uploading a telemetry event record.  Throttle control.",
			0x42170DC52C04ACA5
		},
		{
			"ui_lobbyDebugJoinLoc",
			"lobby Join debug loc",
			0x4D0BB6AA5A213F17
		},
		{
			"ui_lobbyDebugMsgLoc",
			"lobby Msg debug loc",
			0x1AA41CEB2A0F867A
		},
		{
			"ui_lobbyDebugReservationLoc",
			"lobby Reservation debug loc",
			0xE4596014B16CF17
		},
		{
			"ui_lobbyDebugResCountLoc",
			"lobby ResCount debug loc",
			0x33481A4A36A771AC
		},
		{
			"ui_lobbyDebugAgreementLoc",
			"lobby Agreement debug loc",
			0x47EDF2B82532CC4D
		},
		{
			"ui_lobbyDebugAgreeCountLoc",
			"lobby AgreeCount debug loc",
			0x7F0B910B664147FA
		},
		{
			"ui_lobbyDebugPrivateHostSessionLoc",
			"lobby PrivateHostSession debug loc",
			0x194F549185C1A50A
		},
		{
			"ui_lobbyDebugPrivateClientSessionLoc",
			"lobby PrivateClientSession debug loc",
			0x413FCDA52C5CD2BB
		},
		{
			"ui_lobbyDebugGameHostSessionLoc",
			"lobby GameHostSession debug loc",
			0x5B7EA066041EA125
		},
		{
			"ui_lobbyDebugGameClientSessionLoc",
			"lobby GameClientSession debug loc",
			0x32FF1036F4665D70
		},
		{
			"ui_lobbyDebugDWSessionLoc",
			"lobby DWSession debug loc",
			0xF527311279E40B4
		},
		{
			"ui_lobbyDebugLobbyTasksLoc",
			"lobby LobbyTasks debug loc",
			0x67FC6A044DF7E4C9
		},
		{
			"ui_lobbyDebugVoiceHostLoc",
			"lobby VoiceHost debug loc",
			0x7F992163F8807079
		},
		{
			"ui_lobbyDebugVoiceClientLoc",
			"lobby VoiceClient debug loc",
			0x4FD12895DE1C7E36
		},
		{
			"ui_lobbyDebugPlatformSessionLoc",
			"lobby PlatformSession debug loc",
			0x37C37EA31BFE7056
		},
		{
			"lpc_forceDownloadFiles",
			"Force downloading of files even when they pass the FSCK",
			0x161764104EC9A361
		},
		{
			"marketing_enabled",
			"Used to turn off/on the marketing mtx comms system",
			0xE0901B68E1F4FF1
		},
		{
			"marketing_refreshTime",
			"The refresh time for the marketing messsages.",
			0x168B66C54B816252
		},
		{
			"marketing_autorefresh",
			"Automatically download new messages after reporting any message read",
			0x21FC65B413AD213A
		},
		{
			"marketing_waitTimeOnLogin",
			"Used to enable the wait on fetching marketing messages on login. Time value in ms.",
			0x75DD9083C75B9B66
		},
		{
			"constBaseline_allow",
			"If true, enable use of const baselines",
			0x4AA780B9C974A68D
		},
		{
			"net_ip",
			"Network IP address",
			0x14BBB153CC56743C
		},
		{
			"net_port",
			"Network port",
			0x3208549148778326
		},
		{
			"playlist_debug",
			"Enable the debug display for additional playlist information",
			0x5F2487E6FB3C1ECB
		},
		{
			"playlist_linearMapCycle",
			"We will cycle through the maps linearly if this is true.",
			0x8FB704DB4195CBD
		},
		{
			"playlist_linearMapCycleRandom",
			"We will cycle through the maps linearly if this is true but picks a random start map.",
			0x63F3D820E8C560EA
		},
		{
			"playlist_excludeMap",
			"",
			0x6E821F385A3BDAD0
		},
		{
			"playlist_excludeGametype",
			"",
			0x5D2D4B0CC74E8DD4
		},
		{
			"playlist_allowdevmaps",
			"Allow maps that aren't in the map/presence table to be selected",
			0x2D295AE4CBA401DB
		},
		{
			"playlist_forceEvent",
			"Force a particular playlist event to be active",
			0x5C6628C09B69B14D
		},
		{
			"playlist_autoEvent",
			"Playlist event currently active as set by the autoevent schedule",
			0x477094C6DDA9299
		},
		{
			"playlist_autoEventsEnabled",
			"Allow autoevent scheduler to run playlist events",
			0x3A5446A46890E11B
		},
		{
			"com_profileLoadingForceSend",
			"Force Blackbox send after loading done",
			0x4D32C3E41139CD1D
		},
		{
			"com_smoothFrames",
			"Enable frame times smoothing.",
			0x4E0D20851CFAA043
		},
		{
			"com_maxclients",
			"Maximum amount of clients on the server",
			0x65A2E5EE8014325D
		},
		{
			"sv_maxclients",
			"The maximum number of clients that can connect to a server",
			0x5EC68A2C5682907F
		},
		{
			"ui_level_sv",
			"Load the UI level and start a listen server",
			0x77E97FB748A90E2E
		},
		{
			"ui_execdemo",
			"Show or hide certain options in the UI for demo purposes.",
			0x1E017690F8837DD6
		},
		{
			"ui_execdemo_cp",
			"Show or hide certain options in the UI for demo purposes, for campaign demos only.",
			0x139AC704E09BFB98
		},
		{
			"ui_execdemo_beta",
			"Show or hide certain options in the UI for demo purposes, for public beta only.",
			0x242A6A3568FF9F81
		},
		{
			"ui_execdemo_gamescom",
			"Show or hide certain options in the UI for demo purposes, for gamescom only.",
			0x5F52444CC548D39F
		},
		{
			"ui_execdemo_gamescom_host",
			"Show or hide certain options in the UI for demo purposes, for gamescom host machine only.",
			0x14AEFFF49B0FCC12
		},
		{
			"ui_disablePSControlWidgets",
			"Disable all the control widgets in PS. This dvar is added for recording video for DPS Demo.",
			0x2619094D06AF4CBF
		},
		{
			"com_maxfps",
			"Cap frames per second",
			0x4A1149C362763AF2
		},
		{
			"ui_error_callstack_ship",
			"Enable LUI error callstack reporting to show in the UI",
			0x6BBE933116225B20
		},
		{
			"ui_timescale",
			"Scale time of each frame of LUI animation",
			0x6CFC8AA220A5D8F9
		},
		{
			"ui_disable_blur",
			"Disable LUI blur",
			0x381EC2D6BD9DBE18
		},
		{
			"zombie_devgui",
			"",
			0x1C968190FE36E472
		},
		{
			"zm_private_rankedmatch",
			"Zombies private online match will be ranked and no join in progress is allowed",
			0x48CA6583B1EFAB4B
		},
		{
			"com_fixedtime_float",
			"Use a fixed time rate for each frame",
			0x30831B5184168C87
		},
		{
			"long_blocking_call",
			"Enable SCR_DrawPleaseWait dialog",
			0x146BF4F090F03BB4
		},
		{
			"sv_paused",
			"Pause the server",
			0x3F07F66E96FC06C7
		},
		{
			"cl_paused",
			"Pause the client",
			0x4BF32EA04B74B241
		},
		{
			"cl_gamepadCheatsEnabled",
			"Enable God and Demigod controller cheats",
			0x68729BCBA76FFC13
		},
		{
			"com_sv_running",
			"Server is running",
			0x74A8CF2E4D0E6736
		},
		{
			"com_forceSVLockStep",
			"Only allow the server frame to run when it gets told to by the client thread",
			0x39FFAE4F4746F20E
		},
		{
			"com_wideScreen",
			"True if the game video is running in 16x9 aspect, false if 4x3.",
			0x3E634AF40BAA3108
		},
		{
			"log_filename",
			"Open log file with a given filename",
			0x39767A2C213B3A67
		},
		{
			"showMapErrors",
			"Show pointless error message after quitting level",
			0x3B90D40A821362F5
		},
		{
			"ui_mapname",
			"Current map name",
			0x873561D7387A6D9
		},
		{
			"ui_enableAllHeroes",
			"Enables all Heroes.",
			0x9E317343EDF4396
		},
		{
			"ui_tu11_splitscreen_scoreboard_snap_check",
			"Enables a force update of the scoreboard for splitscreen clients on the first snapshot.",
			0x77E1104928E25049
		},
		{
			"tu11_maxQuadCacheAlloc",
			"Splits up large quadCache allocations into smaller ones, to fix a UI out of memory error with long strings.",
			0x1B1FCFB3B23177D5
		},
		{
			"ui_errorUndefLocals",
			"Turns on whether LUI will error or warning for undefined local variables.",
			0x296B7991ACA20308
		},
		{
			"ui_deadquote",
			"",
			0x41ECE7435D602BE
		},
		{
			"ui_errorMessage",
			"Most recent error message",
			0x35C7129C26B55101
		},
		{
			"ui_autoContinue",
			"Automatically 'click to continue' after loading a level",
			0x51B3A199470FFC52
		},
		{
			"ui_playLogoMovie",
			"Play the initial loading movie with the company logos",
			0x404C70893D7EA01B
		},
		{
			"ui_allowDisplayContinue",
			"if true, allow the skip button to be displayed during loading cinematic",
			0x725305F6F4755B1C
		},
		{
			"scr_hostmigrationtest",
			"",
			0x17A381319703271E
		},
		{
			"debug_show_viewpos",
			"",
			0x289F07B47495E02E
		},
		{
			"scr_killcam",
			"",
			0x4E9123863104C903
		},
		{
			"nextarena",
			"",
			0x7472515618429E7
		},
		{
			"ClickToContinue",
			"",
			0x33060DEBB7B116F3
		},
		{
			"saved_gameskill",
			"",
			0x1715F21BBCA499A8
		},
		{
			"upload_bps_dedicated",
			"dedicated server bandwidth req'd",
			0x1EBD3BA96FF47B5A
		},
		{
			"testMissingContentPacks",
			"When the DLC flag changes for a player, sent the result to Demonware.",
			0x443D55B29B2317B6
		},
		{
			"addMapPackFlagsUserInfo",
			"Send availableMappackFlags out with the User info.",
			0x4259A4A90D12FC5D
		},
		{
			"missingCommandWillError",
			"When true a missing command or config file will cause an ERR_DROP.",
			0x6CEDC002A76A2D4E
		},
		{
			"doAutoExecDevConfig",
			"Exec autoexec developer config files.",
			0x2152755F67286EDF
		},
		{
			"doAutoExecUserConfig",
			"Exec autoexec user config files.",
			0x38DE9F32E6BA54DB
		},
		{
			"tu6_doublePistolSpreadAmmo",
			"Show double ammo in the HUD for pistol spread weapons",
			0x2560E60DB190DF0A
		},
		{
			"tu9_skipPerlocalClientAllocation",
			"",
			0x2A389B201BD3752A
		},
		{
			"tu10_ClearWeapNextHeldInputOnSwitchWeaponCmd",
			"clear cl->weapNextHeld in the SVSCMD_SWITCH_WEAPON response. We should not allow controller input to weapon cycling on the same frame that the server tells us to switch weapons.",
			0x255705FD12C32E19LL
		},
		{
			"scr_xpscaleCP",
			"Controls the xp scaling for CP",
			0x61E39CDFEB605A5D
		},
		{
			"scr_xpscaleMP",
			"Controls the xp scaling for MP",
			0x61F880DFEB7286D3
		},
		{
			"scr_xpscaleZM",
			"Controls the xp scaling for ZM",
			0x62319BDFEBA279FB
		},
		{
			"scr_xpscaleZMPromo",
			"Controls the xp scaling for the ZM DoubleXP Promo",
			0x5EFDC8092AE5DA76
		},
		{
			"scr_vialsAwardedScale",
			"Controls how many vials are awarded to the player",
			0x7536E9929D361703
		},
		{
			"scr_gunxpscaleCP",
			"Controls the gun xp scaling for CP",
			0xC9129B7B2A939CF
		},
		{
			"scr_gunxpscaleMP",
			"Controls the gun xp scaling for MP",
			0xC9735B7B2ADB9E1
		},
		{
			"scr_gunxpscaleZM",
			"Controls the gun xp scaling for ZM",
			0xCDF40B7B2EB8235
		},
		{
			"scr_scorestreaks",
			"",
			0x779A6F3044AFFE65
		},
		{
			"scr_scorestreaks_maxstacking",
			"",
			0x57E72F51FA6FF960
		},
		{
			"scr_rampagebonusscale",
			"",
			0x60D40843E7CC6090
		},
		{
			"scr_disableChallenges",
			"",
			0x1E8C2F621BF6361E
		},
		{
			"scr_disableStatTracking",
			"",
			0x62BF0AA3DA13811F
		},
		{
			"scr_disableSetDStat",
			"",
			0x237B114DC8A1EEDE
		},
		{
			"com_logfile",
			"Write to log file - 0 = disabled, 1 = async file write, 2 = Sync every write",
			0x56204B9D7E74AB6D
		},
		{
			"tu9_highestAvailableDLC",
			"Highest DLC to show purchase prompts for.  Should be ovewritten in playlists.info when DLC is released",
			0x2C1AA405FFD7FE8E
		},
		{
			"tu10_enableHighestAvailableDLCEntitlementCheck",
			"Enable/Disable the highest available DLC entitlement check.",
			0xA9045D6278B9AB0
		},
		{
			"net_broadcast_ip",
			"IP address to use for broadcast packets",
			0x1BE72842E3F0F668
		},
		{
			"netstats_dummy_tracker_data",
			"Turn on dummy random data generation for the netstats tracker",
			0x5032E7C7436812EC
		},
		{
			"netchan_debugSpew",
			"Print very verbose debug info about netchan traffic",
			0x6080BA478FB700F7
		},
		{
			"netchan_debugSpewChan",
			"If turned on, debug spew for this channel will be displayed",
			0x3554CFC5A2660F2B
		},
		{
			"netchan_fakeLoad",
			"Additional ms lard to add to netchan frame to simulate heavy load / context switch",
			0x755D3806119FB980
		},
		{
			"net_emu_latency",
			"Emulated network latency in ms",
			0x578086F2DA99CAB3
		},
		{
			"net_emu_jitter",
			"Emulated network latency jitter in ms",
			0x6A69D4ECF9FE4829
		},
		{
			"net_emu_packet_loss",
			"Emulated network %% packet loss",
			0x584C87C7E223C767
		},
		{
			"net_emu_server",
			"Server network emulation info string",
			0x7DB6414CC9DA9F38
		},
		{
			"net_emu_client",
			"Client network emulation info string",
			0x6526FB7A61BAB7C
		},
		{
			"streamMBPerSecond",
			"Throttle the streamer to the specified megabytes per second",
			0x3FD16FBB21B5CF85
		},
		{
			"sv_connectionLogProbability",
			"Probability of uploading connection log data.",
			0x2479F0A6D085F098
		},
		{
			"sv_connectionLogSamplesPerSecond",
			"Number of samples per second per client.",
			0x656231BE534BDD67
		},
		{
			"sv_mapname",
			"Current map name",
			0x3B7B241B78207C96
		},
		{
			"sv_mapcrc",
			"crc of the map file",
			0x3C727A79C9E602C9
		},
		{
			"cp_queued_level",
			"Queued level",
			0x78D438D99BE5C86F
		},
		{
			"sv_skipto",
			"Current skipto",
			0x70FA26C2326002DB
		},
		{
			"sv_saveGameSkipto",
			"Skipto that gets saved to the save game",
			0x1D8621150BCF0C0C
		},
		{
			"sv_privateClients",
			"Maximum number of private clients allowed on the server",
			0x6D4930096BA3E4D4
		},
		{
			"sv_privateClientsForClients",
			"The # of private clients (we send this to clients)",
			0x12A1B7450A49430D
		},
		{
			"sv_hostname",
			"Host name of the server",
			0x37C96E4191305118
		},
		{
			"sv_noname",
			"Player name assigned to players that fail name validation",
			0x181B1023195812AD
		},
		{
			"sv_penetrationCount",
			"Maximum number of private clients allowed on the server",
			0xF93473F7D04DCE5
		},
		{
			"sv_connectTimeout",
			"seconds without any message when a client is loading",
			0x36A999BA53305150
		},
		{
			"sv_writeConfigStrings",
			"Write out the config string file",
			0x1A5A3BDD393BD184
		},
		{
			"scr_writeConfigStrings",
			"Special script mode for writing config string files",
			0x46704500104207ED
		},
		{
			"sv_generateConstBaselines",
			"Whether or not to generate the const baseline files.",
			0x3572DBDFC80618CB
		},
		{
			"sv_cheats",
			"Enable cheats on the server",
			0x2F2097BC767C1B1
		},
		{
			"sv_sendCmdAngles",
			"Send cmdAngles to clients for increased viewangles fidelity during killcam",
			0x6022021F1A5EE5CD
		},
		{
			"sv_mapRotation",
			"List of maps for the server to play",
			0x40285E3E4165AE4F
		},
		{
			"sv_mapRotationCurrent",
			"Current map in the map rotation",
			0x248FA32EBFBBEE40
		},
		{
			"sv_rateBoosting",
			"Determines whether we should allow rate boosting: 0 - disabled, 1 - enabled based on huristic, 2 - force enabled",
			0x6470CCD761596532
		},
		{
			"sv_maxRate",
			"Maximum server->client rate",
			0x4E15EFBF1B205C0F
		},
		{
			"sv_zlib_threshold",
			"Message size threshold which triggers more aggressive compression",
			0x3A39BC141F6CD4E0
		},
		{
			"sv_timeout",
			"seconds without any message",
			0x4056F2DCDC894774
		},
		{
			"sv_mapSwitch",
			"in-game map switching: 0,disabled|1,enabled|(default)2,enabled but devmap will turn it off",
			0x786D1EF57C183CA1
		},
		{
			"sv_potm_debug",
			"Debug info for the Play of the Match system",
			0x17ABBB7F8E9C8C87
		},
		{
			"migration_blocksperframe",
			"Number of blocks to send out each server frame.  A value of 0 means base it off our tested bandwidth.",
			0x6A6B27E043217AE2
		},
		{
			"migration_msgtimeout",
			"timeout time in ms of migration messages",
			0x62859B8FF4AB9C18
		},
		{
			"migration_timeBetween",
			"milliseconds between migration_limit dvar's value of migrations.",
			0x7AB6A203C9196427
		},
		{
			"migration_forceHost",
			"force a particular client index to be host.",
			0x48B07629370FF5D9
		},
		{
			"migration_soak",
			"Soak test the migration. Migrate every migrationTimeBetween milliseconds",
			0x55D2448BFF1A461A
		},
		{
			"storage_offline",
			"Live features are stored locally instead of on Demownare.",
			0x7EF5B59EA5F493E2
		},
		{
			"storage_maxfailures",
			"Number of failures before maxing out backoff increases.",
			0x53854EF6368F3310
		},
		{
			"storage_backoff",
			"Base for backing off exponentially",
			0xEE22CA89B1CE239
		},
		{
			"loc_availableLanguages",
			"",
			0x746B5D9C4A7CE4C0
		},
		{
			"loc_language",
			"Language",
			0x3A97AE527D90FB24
		},
		{
			"loc_systemLanguage",
			"System language",
			0x22A8E1B081BE1169
		},
		{
			"loc_forceEnglish",
			"Force english localized strings",
			0x762BB067CC1A6FF3
		},
		{
			"loc_warnings",
			"Enable localization warnings",
			0x361B832B45F743AB
		},
		{
			"loc_warningsAsErrors",
			"Throw an error for any unlocalized string",
			0x27B8B7C2DAED657C
		},
		{
			"loc_languageSaved",
			"Has a language value been chosen and saved",
			0x4A833C8D8F59EE5F
		},
		{
			"st_LODDistScale",
			"Distance scale for terrain LOD - when this increases, detail at a given distance is reduced",
			0x3834D608A126064C
		},
		{
			"ui_emblemDisableGroups",
			"Disable the emblem group feature.",
			0x3211555FF0E63362
		},
		{
			"ui_creditSkipTo",
			"Line number to skip to in the credits?",
			0x47104A72C3388B7B
		},
		{
			"ui_smallFont",
			"Small font scale",
			0x75C89B766727AA1A
		},
		{
			"ui_bigFont",
			"Big font scale",
			0xE3775D5A36206AB
		},
		{
			"ui_demoname",
			"Current selected demo name",
			0x4CB3FA6C5D46BBA8
		},
		{
			"ui_liveEventViewerOpen",
			"If the liveEventViewer menu is currently open",
			0x5CE320CFE22FBC0
		},
		{
			"ui_keyboardtitle",
			"",
			0x709BA994593CFF9D
		},
		{
			"ui_keyboard_dvar_edit",
			"",
			0x61FF91986A2313BC
		},
		{
			"scr_hardcore",
			"",
			0x67646047091E1652
		},
		{
			"ui_hud_showobjicons",
			"",
			0x46ABEE835CBEF546
		},
		{
			"ui_hud_obituaries",
			"",
			0x31F420F8D8A1D739
		},
		{
			"ui_currMenuNameList",
			"Current menu name list set in lua, for debug display.",
			0x7FEB365B406388
		},
		{
			"ui_currFocusList",
			"Current focus stack in lua, for debug display and ship crash dumps.",
			0x3B8B4D0064475D4
		},
		{
			"ui_currFocusStates",
			"Current focus stack, with states, set in lua, for debug display.",
			0x2196FDA5E89973C6
		},
		{
			"ui_usingFreeCursor",
			"Turns the free cursor on/off in console builds.",
			0x48E0F129D51A415C
		},
		{
			"ui_scrollSpeed",
			"",
			0x129E807CE7642720
		},
		{
			"ui_hud_hardcore",
			"Whether the HUD should be suppressed for hardcore mode",
			0x70BBA44D358FF7CC
		},
		{
			"ui_allow_classchange",
			"Whether the UI should allow changing class",
			0x32A7215CE28F3782
		},
		{
			"ui_hud_visible",
			"Whether the 2D HUD should be displayed or not.",
			0x737582CB19486988
		},
		{
			"ls_gametype",
			"",
			0x6D55675CD612876F
		},
		{
			"ls_mapname",
			"",
			0x7E247074D08441DA
		},
		{
			"ui_busyBlockIngameMenu",
			"1 if ingame menu should be blocked for pending action",
			0x54488B7C651BD0EC
		},
		{
			"ui_heatMapColor",
			"Heat Map Blob Color",
			0x460242CEAD08594F
		},
		{
			"ui_heatMapColorForPlayer",
			"Heat Map Blob Color for player deaths only",
			0x5BA69E7B82A5FC53
		},
		{
			"ui_useloadingmovie",
			"Play and render a loading movie instead of static screen (mapname_load)",
			0x7BD8C108B14CF06B
		},
		{
			"ui_badWordSeverity",
			"1 to block partial and full profanity matches, 2 to only block full matches",
			0x6339CAD319524344
		},
		{
			"ui_badWordMaxTaskWaits",
			"The number of cycles *10 ms for a remoteTask before we decide to cancel the isBadWord task",
			0x2052CA8D71E32144
		},
		{
			"ui_friendsListOpen",
			"True when viewing Friends List.",
			0x1F305A5C795ADA5D
		},
		{
			"mp_prototype",
			"Enabled T8 prototype features (int because this was used as a command line int-only property)",
			0x5D15C63764CE79B9
		},
		{
			"ui_autoControlledPlayer",
			"Enables the player to select a ui option to transfer the player's control to Bot",
			0x1A94323F2C9E094C
		},
		{
			"blackboxMode",
			"",
			0x44F8B711C6A78FFA
		},
		{
			"cg_drawFramePerformanceWarnings",
			"Draw onscreen timers for client, server, and gps if they exceed their frame budgets.",
			0x5F9C1EEA7170D2E
		},
		{
			"devmaponline",
			"devmap into online mode",
			0x5A76FEC4DAB557D1
		},
		{
			"phys_player_step_on_actors",
			"phys_player_step_on_actors",
			0x4C1224C82006E50D
		},
		{
			"snd_losOcclusion",
			"occlusion based off of LOS",
			0x78C926CA413487C2
		},
		{
			"ui_drawCrosshair",
			"Whether to draw crosshairs.",
			0x3147A8F5BF451AA6
		},
		{
			"voice_enabled",
			"Voice communication enabled",
			0x52A69EB47F9E465F
		},
		{
			"cg_fov_default",
			"User default field of view angle in degrees",
			0x57ED85B05E9BCDB9
		},
		{
			"compassFriendlyHeight",
			"The size of the friendly icon on the compass",
			0x1F5D7DB9E25EC97F
		},
		{
			"compassFriendlyWidth",
			"The size of the friendly icon on the compass",
			0x44D392B641449348
		},
		{
			"cg_weaponCycleDelay",
			"The delay after cycling to a new weapon to prevent holding down the cycle weapon button from cycling too fast",
			0x4EB1835CF0F023D
		},
		{
			"cl_packetdup",
			"Enable packet duplication",
			0x7F3427FA1A7374C8
		},
		{
			"debug_audio",
			"Enables debug rendering for audio.",
			0x3A868AD5FB0774F9
		},
		{
			"debug_mover",
			"Turns on mover debug mode",
			0x1DE09BE0EE1A8012
		},
		{
			"demo_drawdebuginformation",
			"Used to draw debug information.",
			0x5D02584AB86AA8C2
		},
		{
			"g_reviveTime",
			"The time it takes to revive a player in last stand (do not change as it is tied to the third person animations)",
			0x6F1B1794F8F39B93
		},
		{
			"slide_subsequentSlideScale",
			"Percent the slide speed should be scaled down by per subsequent slide",
			0x1B823EBA6DD7C7CB
		},
		{
			"player_swimSprintSpeedScale",
			"Scale the swim speed by this amount when sprinting",
			0x7DB976AB518CA222
		},
		{
			"melee_attachmentMultiplier",
			"If the melee attachment is equipped, the melee base damage is multiplied by this amount",
			0x2A89C38ADF39A3EA
		},
		{
			"tm_taskTrackerOn",
			"If true, tasks will be tracked by the tracking system",
			0x2AB491F3E86C44DF
		},
		{
			"tm_taskManagerDebug",
			"Shows more information as the number is increased",
			0x716C5A7404912A18
		},
		{
			"tm_startedThreshold",
			"If this many tasks of a single definition are started, a warning message will pop-up.",
			0x1FB1BC2C2EFCEE0F
		},
		{
			"tm_failsThreshold",
			"If this many tasks of a single definition fail, a warning message will pop-up.",
			0x3CA374003C95549B
		},
		{
			"tm_succeedsThreshold",
			"If this many tasks of a single definition succeed, a warning message will pop-up.",
			0x4EA659694387C23D
		},
		{
			"tm_pendingeThreshold",
			"If this many tasks of a single definition are concurrently pending, a warning message will pop-up.",
			0xD5441818C92A64
		},
		{
			"tm_typeToDisplay",
			"Chooses which type of task to display in the log",
			0x27835F42462F0E68
		},
		{
			"tm_logTimeLimit",
			"In seconds, how long should pass before an event is shown in the log",
			0x2247CB6C6F557F3D
		},
		{
			"tm_currentErrorPage",
			"In seconds, how long should pass before an event is shown in the log",
			0x632902C39A5AA12D
		},
		{
			"sv_cheats",
			"External Dvar",
			0x2F2097BC767C1B1
		},
		{
			"allClientDvarsEnabled",
			"Allow host to set all client dvars",
			0x60C8FCA8AB2A735C
		},
		{
			"profileDwUpdateInterval",
			"Interval (in seconds) between updating recorded performance information in DW (negative value disable recording)",
			0xE8DC9F4D91759DF
		},
		{
			"reliableResendTime",
			"Amount of time (in ms) to wait before resending an unack'd reliable message",
			0x22B6677D5D184B9
		},
		{
			"reliableTimeoutTime",
			"Amount of time (in ms) to wait before timing out a client for not ack'ing a message",
			0x3FDFE198FF2CC267
		},
		{
			"phys_gravity",
			"Physics gravity in units/sec^2.",
			0x569DC74596BBB254
		},
		{
			"phys_gravity_dir",
			"Sets the gravity direction.",
			0x4B6BB42074C7A840
		},
		{
			"phys_buoyancy",
			"Used to turn on/off buoyancy",
			0x75CEE65DE741344
		},
		{
			"phys_ragdoll_buoyancy",
			"Used to turn on/off the buoyancy for he ragdolls",
			0x2DAC8B2B9D305718
		},
		{
			"phys_disableEntsAndDynEntsCollision",
			"Disable rigid body collision with dyn ents and regular entities as an optimization.  Vehicles do not apply.",
			0x24FE8BEB1C5AD1D6
		},
		{
			"enable_moving_paths",
			"enable moving paths",
			0x214D5952AABA3E6A
		},
		{
			"enable_new_prone_check",
			"enable new prone check",
			0x3C40F1AD7C84AAB9
		},
		{
			"phys_wind_enabled",
			"Used to turn off/on physics wind sim",
			0x60A2BCF45461E9CA
		},
		{
			"phys_wind_vehicle_scale",
			"Scale the vehicle wind reactions by this value",
			0x3EDB64F8EFE5F48E
		},
		{
			"snd_speakerConfiguration",
			"Speaker configuration",
			0x53BB4B101A1369B8
		},
		{
			"glassDamageMultiplier",
			"Multiplier the damage applied to the glass (server).",
			0x4B8A7BB010C32F6D
		},
		{
			"maxAIGlassHits",
			"Maximum number of hits it takes an AI to break the glass.",
			0x23AEA643B2775AD7
		},
		{
			"demo_enabled",
			"Used to turn the system on/off.",
			0x561CEAA988616530
		},
		{
			"demo_recordPrivateMatch",
			"Used to turn the private match recording on/off.",
			0x67284532BECB20B2
		},
		{
			"demo_recordSystemlinkMatch",
			"Used to turn the systemlink match recording on/off.",
			0x27E3565C7550EDF0
		},
		{
			"demo_recordOfflineMatch",
			"Used to turn the offline match recording on/off.",
			0x5BFDFC14138D3B06
		},
		{
			"demo_debug",
			"Debug info for the Server Snapshot Demo system",
			0x1E197923F31FE4A2
		},
		{
			"demo_client",
			"Current viewing player",
			0x72DE091D1D0C2B8C
		},
		{
			"demo_recordingrate",
			"Used to tweak the rate(in msec) at which we write a super snapshot",
			0x2772FD9F0557FBF4
		},
		{
			"demo_fileblockWriteRate",
			"Used to tweak the rate(in sec) at which we write the file blocks during demo recording",
			0x3266962B55BCDEF
		},
		{
			"demo_keyframerate",
			"Used to specify the rate(in sec) at which we generate a keyframe during playback.",
			0x104BE62213ED4629
		},
		{
			"demo_pause",
			"Used to pause a demo playback.",
			0x2FF56845FD43D403
		},
		{
			"demo_pauseOnNextSnapshot",
			"Used to jump to the next snapshot during network analysis.",
			0x5B081E0BCDCD8993
		},
		{
			"demo_maxTimeScale",
			"The maximum timescale rate we will support.",
			0x665BE4626D4D30D0
		},
		{
			"demo_downloadEntireFile",
			"Used toggle the ability to download the entire file instead of parts.",
			0x2A048F122F8470A0
		},
		{
			"demo_livestreaming",
			"Used to turn the live streaming system on/off",
			0x4B22DC55CD99A0CF
		},
		{
			"demo_downloadLiveStreamThrottleTime",
			"The maximum amount of time (in msec) we can starve the download of live stream callback.",
			0x562235A0A6CFE3DC
		},
		{
			"demo_summaryReadEnabled",
			"Used to turn meta summary read on/off",
			0xE1DEC861F6BF51E
		},
		{
			"demo_writePaintShopInformation",
			"Used to turn on/off the paintshop data write into the demo file",
			0x4BD70EF582CFA310
		},
		{
			"demo_bookmarkEventThresholdTime",
			"The time duration for which we want to show the bookmark event image in the demo timeline.",
			0x39E372A043D4A591
		},
		{
			"demo_freeCameraLockOnOrbitRadius",
			"Radius of the lockon orbit.",
			0x11D58469A34F979E
		},
		{
			"demo_highlightReelPlayerFilter",
			"Demo Highlight Reel player filter control",
			0x5FF295469C16F65A
		},
		{
			"demo_highlightReelTransitionFilter",
			"Demo Highlight Reel transition filter control",
			0x102761C252F95DF4
		},
		{
			"demo_highlightReelStylesFilter",
			"Demo Highlight Reel styles filter control",
			0x592196442EA9D201
		},
		{
			"demo_highlightReelNumberOfSegments",
			"Demo Highlight Reel number of segments",
			0x795C6EE8CF31F403
		},
		{
			"demo_highlightReelMinimumStarsFilter",
			"Minimum stars used for the filtering of events in the highlight.",
			0x5FA6C62A685B3352
		},
		{
			"demo_highlightReelGameResultFilter",
			"Demo highlight reel round outcome filter.",
			0x27F56CCA950310C0
		},
		{
			"demo_packetsPerSecondMin",
			"Min amount of packets to send per second before throttling.",
			0x6771EE54572E19ED
		},
		{
			"demo_bytesPerSecondMin",
			"Min amount of bytes to send per second before throttling.",
			0x43ECC22C3CD4C007
		},
		{
			"demo_packetsPerSecondMax",
			"Max amount of packets to send per second before throttling.",
			0x6756D85457172CD3
		},
		{
			"demo_bytesPerSecondMax",
			"Max amount of bytes to send per second before throttling.",
			0x4407B02C3CEB6929
		},
		{
			"demo_inLobbyThrottleBandwidthPercent",
			"Bandwidth percent to be released when we are not going to throttle in the lobby.",
			0x531AE3C9B8F3E1EA
		},
		{
			"demo_inGameThrottleBandwidthPercent",
			"Bandwidth percent to be released when we are not going to throttle inside the game.",
			0x647D504203ADF4A4
		},
		{
			"demo_streamingAdjustmentFactor",
			"The adjustment factor for howScrewedAreWe in the streaming.",
			0x55FF38576DFA03C9
		},
		{
			"demo_enableSvBandwidthLimitThrottle",
			"Used to enable the server bandwidth based throttling system.",
			0x324732789DCF2EDD
		},
		{
			"demo_svBandwidthLimitThrottleMaxBytesPercent",
			"The percentage of the max. bandwidth per frame within which we want to apply the throttle.",
			0x6E6DD67487DD3964LL
		},
		{
			"demo_svBandwidthLimitThrottleTimeout",
			"The maximum amount of time (in msec) we can starve the write to stream callback, due to the server hitting the maximum bandwidth limit per frame.",
			0x61B1FD239D69A10DLL
		},
		{
			"demo_streamingSendSocketBuffSizeForRecording",
			"The send socket buffer size which will be used for recording",
			0x15A37D18D6A1B438
		},
		{
			"demo_streamingSendSocketBuffSizeForOtherUploads",
			"The send socket buffer size which will be used for other uploads",
			0x56E207A2D8B2E2B1
		},
		{
			"demo_downloadStreamThrottleTime",
			"The maximum amount of time (in msec) we can starve the download stream callback.",
			0x3CF9A5CF8FF93606
		},
		{
			"demo_downloadStreamRetryWaitTime",
			"The maximum amount of time (in msec) we will have to wait before we retry the download.",
			0x20B2FAE4FB152DDF
		},
		{
			"demo_downloadStreamMaxRetryAttemps",
			"The maximum number of retry attempts before we mark the downloadstream as failed..",
			0x18CECC6FF1929FF1
		},
		{
			"perk_weapReloadMultiplier",
			"Percentage of weapon reload time to use",
			0x4458F4E852DB0E55
		},
		{
			"perk_killstreakReduction",
			"Reduced number of kills required to obtain a killstreak.",
			0x30DB5C3D6375C831
		},
		{
			"perk_killstreakMomentumMultiplier",
			"Multiplies the amount of momentum received by the player",
			0x52BCDEC14ED72605
		},
		{
			"perk_killstreakAnteUpResetValue",
			"Scorestreak reset value when the ante up perk is in use.",
			0x2D6BDF06C07C91FD
		},
		{
			"perk_delayExplosiveTime",
			"Time in seconds to delay a proximity explosive",
			0x62D6967E8CA82E61
		},
		{
			"perk_disarmExplosiveTime",
			"Time in seconds to disarm a proximity explosive",
			0x63DA9311C0D217AA
		},
		{
			"perk_deathStreakCountRequired",
			"Number of deaths required to obtain a deathstreak.",
			0x7C155EA09F9D8070
		},
		{
			"perk_healthRegenMultiplier",
			"Multiplier to the time which the player starts to regenerate health after damage",
			0x7B66853FFF18AF66
		},
		{
			"perk_flakJacket",
			"Percent of explosive damage done that will apply to player.",
			0x3EAF6A2850D1FCCE
		},
		{
			"perk_flakJacket_hardcore",
			"Percent of explosive damage done that will apply to player in hardcore.",
			0x70CF0DA8B63FB1D9
		},
		{
			"perk_fireproof",
			"Percent to reduce flame damage done to the player.",
			0x23FF287409E487B4
		},
		{
			"perk_gpsjammer_min_speed",
			"Minimum speed a player must be moving for gpsjammer to protect them from UAV sweeps.  Based on player speed used in g_speed (I believe this is units/sec).  If 0, player will always be protected from UAV sweeps with this specialty.",
			0x57DB5142A091AE9DLL
		},
		{
			"perk_gpsjammer_min_distance",
			"Minimum distance a player must have moved in a time period for gpsjammer to protect them from UAV sweeps.  If 0, player will always be protected from UAV sweeps with this specialty.",
			0x162F55DAA69B421LL
		},
		{
			"perk_gpsjammer_time_period",
			"Time period inbetween checks, in MSec. Please use multiples of 50 msec as that is the period of server script ",
			0x66B53538163C5622
		},
		{
			"perk_gpsjammer_graceperiods",
			"Number of time period a player can be below the required velocity before removing protection.",
			0x5D3C723F26B11B75
		},
		{
			"perk_sgjammer_min_speed",
			"Minimum speed a player must be moving for gpsjammer to protect them from Sensor Grenade sweeps.  Based on player speed used in g_speed (I believe this is units/sec).  If 0, player will always be protected from Sensor Grenade sweeps with this specialty.",
			0x4A76E168621E07D7LL
		},
		{
			"perk_sgjammer_min_distance",
			"Minimum distance a player must have moved in a time period for gpsjammer to protect them from Sensor Grenade sweeps.  If 0, player will always be protected from UAV sweeps with this specialty.",
			0x6FF59096FF8B6487LL
		},
		{
			"perk_sgjammer_time_period",
			"Time period inbetween checks, in MSec. Please use multiples of 50 msec as that is the period of server script ",
			0x75B67E026EB962B4
		},
		{
			"perk_sgjammer_graceperiods",
			"Number of time period a player can be below the required velocity before removing protection.",
			0x5E68178D11E9D667
		},
		{
			"perk_nottargetedbyai_min_speed",
			"Minimum speed a player must be moving for nottargetedbyai to protect them from AI controlled killstreaks. If 0, player will always be protected from AI controlled killstreaks with this specialty.",
			0x47982F834510FFFBLL
		},
		{
			"perk_nottargetedbyai_graceperiod",
			"Time after which the player drops below min_gpsjammer_speed that they are still protected from ai support targetting.  0-10000 int (in milliseconds), 0 default.",
			0x42A7884708587E8ALL
		},
		{
			"perk_tracker_fx_foot_height",
			"Height of footsteps for tracker perk.",
			0x56DE6E0B6C136484
		},
		{
			"perk_tracker_fx_fly_height",
			"Height of flying markers for tracker perk.",
			0x4A3DF32AEBF30903
		},
		{
			"enable_global_wind",
			"enable wind effect",
			0x76C3F9094AEEF287
		},
		{
			"enable_grass_wind",
			"enable grass wind effect",
			0x350A9A0FDAFE92BE
		},
		{
			"wind_global_vector",
			"global wind force in units/sec. This is also the hi-level wind",
			0x52F7C1DFF8F57DCF
		},
		{
			"wind_global_low_altitude",
			"altitude of global low-level wind",
			0x67FA4884FE9A2BE9
		},
		{
			"wind_global_hi_altitude",
			"altitude of global hi-level wind",
			0x2048DC9AD2E7D6EA
		},
		{
			"wind_global_low_strength_percent",
			"strength of global low-level wind, expressed as a percentage of wind_global_vector",
			0x262DB7094325F8BC
		},
		{
			"wind_leaf_scale",
			"global wind grass tension (stiffness)",
			0x5945330E01E3D861
		},
		{
			"wind_grass_gustinterval",
			"global wind force gust interval",
			0x6F2BE374CBBF4099
		},
		{
			"wind_grass_scale",
			"global wind force scaler",
			0x34A90FB9F8A09F27
		},
		{
			"wind_grass_tension",
			"global wind grass tension (stiffness)",
			0x290FCF75EFEB5373
		},
		{
			"wind_grass_gust_speed",
			"speed multiplier for gusts",
			0x664AC6A4D7D61778
		},
		{
			"wind_grass_gust_radius",
			"size of global wind gust effector",
			0x5648FE2DCB2B8F8B
		},
		{
			"wind_grass_gust_distance",
			"max distance at which to place gusts",
			0x2C6E04600BBE184A
		},
		{
			"aim_target_closest_first",
			"Target the closest AI based on 3d distance",
			0x419BA7D17BB018C7
		},
		{
			"aim_autoaim_enabled",
			"Turn on auto aim",
			0x2DE12E2AFE23ADCF
		},
		{
			"bg_viewKickScale",
			"The scale to apply to the damage done to caluclate damage view kick",
			0x4BF878D35CAD6F8A
		},
		{
			"bg_viewKickMax",
			"The maximum view kick",
			0x393CD911FFBB3B46
		},
		{
			"bg_viewKickMin",
			"The minimum view kick",
			0x3921CB11FFA45BC4
		},
		{
			"bg_viewKickDeflectTime",
			"The random direction scale view kick",
			0x268E1E3AA428B72A
		},
		{
			"bg_viewKickRecoverTime",
			"The random direction scale view kick",
			0xAE16241B8D06F6D
		},
		{
			"bg_viewKickScaleAds",
			"The scale to apply to the damage done to caluclate damage view kick",
			0x298FEF62A5A1CCB4
		},
		{
			"bg_viewKickMaxAds",
			"The maximum view kick",
			0x7BF23BA0928AC620
		},
		{
			"bg_viewKickMinAds",
			"The minimum view kick",
			0x55C59F681B31E1F6
		},
		{
			"bg_viewKickDeflectTimeAds",
			"The random direction scale view kick",
			0x5BC2B1A69CB282D4
		},
		{
			"bg_viewKickRecoverTimeAds",
			"The random direction scale view kick",
			0x672E3C15A41C1C69
		},
		{
			"bg_scriptCanDisableStances",
			"Whether script is allowed to disable stances",
			0x138E0DBBCCF56F1B
		},
		{
			"bg_useThrowButtonForDetonatorGrenades",
			"Whether the throw button should be returned as the fire button for grenades and mines that have a detonator",
			0x87286E123370AC4
		},
		{
			"bg_heavyWeaponsAlwaysPlayFirstRaise",
			"Always play the first raise animation when switching to heavy weapons",
			0xF8628384A6BF6F2
		},
		{
			"player_useWaterFriction",
			"Change the players walk friction when in water, using water friction scales",
			0x8932E7EFEE3D6CD
		},
		{
			"player_useWaterWadeScale",
			"Scale the players command based on their depth in the water",
			0x327565589F891CD6
		},
		{
			"bg_friendlyFire",
			"Friendly fire status",
			0x225326DCF1D8C634
		},
		{
			"bg_friendlyFireMode",
			"Enables friendly fire mode, cannot shoot down friendlies at close range",
			0x4D80B80B9ED81737
		},
		{
			"bg_grenadeMinimumThrowbackTime",
			"Time added to grenade countdown to stop it from blowing up during throwback.",
			0x3E6D6D1E70394899
		},
		{
			"bg_weaponCompatibleIfSharedAmmo",
			"Allow leeching ammo from weapons touched on the ground if the player has a weapon with matching shared ammo",
			0x2E6A89AD001FE5A9
		},
		{
			"disableLookAtEntityLogic",
			"Disables the Player_UpdateLookAtEntity logic (expensive)",
			0x5CF9AE63F3BFE638
		},
		{
			"player_lastStandBleedoutTime",
			"The time taken by the player to completely bleedout when in last stand (pro version).",
			0x414B968F85361F6
		},
		{
			"player_lastStandDistScale",
			"Tweaks some of the movement in last stand.",
			0x6EEEC609104F37FF
		},
		{
			"player_ProneMinPitchDiff",
			"Minimum pitch difference allowed to move in prone.",
			0x6D16F2F249198008
		},
		{
			"player_ProneLaststandFreeMove",
			"Allow freer movement in last stand than normal prone movement",
			0x29E2C87F14934BC
		},
		{
			"player_lastStandCrawlTransition",
			"Transition to the last stand crawl set, and not regular last stand animation",
			0x429EB921B546A6F1
		},
		{
			"bg_useT7Melee",
			"use the T7 melee system.",
			0x2A78F8D99222AC09
		},
		{
			"player_meleeChargeMaxSpeed",
			"Max speed to clamp when doing a charge melee",
			0x4F0998E4E4BA6BB6
		},
		{
			"player_meleeChargeMinSpeed",
			"Min speed to clamp when doing a charge melee",
			0x7A62D62E37F7A5F0
		},
		{
			"player_meleeLungeAcceleration",
			"Dvar to control the acceleration for the lunge attack",
			0x94D4FAB8CE278AE
		},
		{
			"player_meleeLungeUnderwaterSpeedScale",
			"Scaler for the underwater lunge speed",
			0x3FF643A885AAEB28
		},
		{
			"player_meleeLungeSpeedCutoff",
			"Cutoff the lunge attacks if the velocity falls bellow this value",
			0x34414D4498355154
		},
		{
			"player_meleeLungeTimeCutoff",
			"Max duration of the lunge attack",
			0x1946DDA26B1162B2
		},
		{
			"vehicle_riding",
			"Turning this on enables players to walk around on top of vehicles",
			0x559448DC6708AFDD
		},
		{
			"vehicle_selfCollision",
			"Enabled the vehicle to shoot itself with it's own turret.",
			0x43496C21C39056FA
		},
		{
			"tu11_IgnoreItemsWithPLevel100InIsItemLockedForChallenge",
			"Treats items with PLevel 100 or more as never unlocked in IsItemLockedForChallenge().",
			0x6C09B0BFF51AC81C
		},
		{
			"bg_disableWeaponPlantingGroundTrace",
			"Disables the ground trace check for weapon planting.",
			0x4392DE0CF30D6CCE
		},
		{
			"slide_forceBaseSlide",
			"Force the player to use the base slide, do not allow boost reduced or boost",
			0x4C6022373CF3066C
		},
		{
			"actors_walkable",
			"Players can walk on AI without sliding off",
			0x125B4A64B58DC66D
		},
		{
			"trm_enabled",
			"Enable or disable the player's mantle",
			0x54E4287F98A644F4
		},
		{
			"juke_enabled",
			"Enable player evasive move",
			0x30BC3CBFB08B248A
		},
		{
			"sprint_capSpeedEnabled",
			"Enable physics cap on speed during sprinting",
			0x234789E28481E1D2
		},
		{
			"weaponrest_enabled",
			"Enable weapon rest, player turns gun so that it does not appear to intersect geo",
			0xA92A344E2E38925
		},
		{
			"traverse_mode",
			"1 = new traversal 2 = pre-BO3 mantle system",
			0x6C71BF6810FA346F
		},
		{
			"sprint_allowRestore",
			"Restore the players sprint state after certain actions, wallrun, traverse etc.  ",
			0x62FA420611F210F3
		},
		{
			"bg_weaponSwitchHero",
			"The HeroWeaponType to switch to on a press of button WEAPON_SWITCH_HERO.",
			0x2F5602FEACAEB597
		},
		{
			"bg_specialOffhandInventorySwitch",
			"The HeroWeaponType to switch to on a press of button OFFHAND_SPECIAL.",
			0x641EB1218246D20A
		},
		{
			"bg_specialOffhandInventoryAllowBallSwitch",
			"When true the special offhand input switch to inventory weapon allows ball switch.",
			0x1BA8872A25A6A67A
		},
		{
			"friendlyContentOutlines",
			"Use this to enable the outlines on friendlies and disable the icons",
			0x7F6CF93A7A55D6A8
		},
		{
			"player_dpad_gadget_scheme",
			"Dpad gadget control scheme,  0 - mp system, 1 - career mode (dapd controls 3 gadgets)",
			0x57ABDD7E4A691B01
		},
		{
			"compassObjectiveIconWidth",
			"The size of the objective on the full map",
			0x2127B85AA4AD5C49
		},
		{
			"compassObjectiveIconHeight",
			"The size of the objective on the full map",
			0x623281D39B08B738
		},
		{
			"compassDrawLastStandIcon",
			"Draw the last stand icon on the minimap",
			0x58369F73B6EF74AE
		},
		{
			"waypointOffscreenPadLeft",
			"Offset from the edge.",
			0x16611D1C3A11368D
		},
		{
			"waypointOffscreenPadRight",
			"Offset from the edge.",
			0x5E8218284D9BB186
		},
		{
			"waypointOffscreenPadTop",
			"Offset from the edge.",
			0x50694F1B92EB913F
		},
		{
			"waypointOffscreenPadBottom",
			"Offset from the edge.",
			0xBC868C8D709B137
		},
		{
			"cg_aggressiveCullRadius",
			"The radius to use for entity CG_ProcessEntity frustrus culling. 0 means disabled.",
			0x72E5D4F083B92862
		},
		{
			"slam_blur_enabled",
			"Is screen blur enabled for slam",
			0x58950029C9E7384A
		},
		{
			"cg_drawBreathHint",
			"Draw a 'hold breath to steady' hint",
			0x4825068A6A96C54D
		},
		{
			"cg_hudDamageIconTime",
			"The amount of time for the damage icon to stay on screen after damage is taken",
			0x4B30A71B20B23194
		},
		{
			"cg_crosshairVehicleSentientCheck",
			"Whether to show colored crosshairs when targetting vehicles iff sentient exists",
			0x44548156F075E666
		},
		{
			"cg_updateScoreboardAfterGameEnded",
			"Controls whether to update or freeze the scoreboard when the game ends.",
			0x44C35D5960CC531B
		},
		{
			"cg_DrawRemoteVehiclePlayerNames",
			"Player's gamertag displayed above remote controlled vehicles",
			0x7C1B72369DF3CBC7
		},
		{
			"cg_DrawPlayerNamesWhileInVehicle",
			"Player's gamertags are displayed to player who is in vehicle",
			0x650C3842D88563E7
		},
		{
			"cg_DrawOnlyFriendlyPlayerNamesWhileInVehicle",
			"Only Friendly Player's gamertags are displayed to player who is in vehicle",
			0x241E40FA8E872528
		},
		{
			"cg_DrawOnScreenFriendlyAI",
			"Draws all friendly AI names who are currently on screen.",
			0x63FDE83AC1820602
		},
		{
			"cg_thirdPersonLastStand",
			"Uses the third person view when the player is in laststand.",
			0x3EB163CDE8DA7ACC
		},
		{
			"cg_WeaponButtVictimCameraShakeScale",
			"Scale of the camera Shake applied to a weapon butt Victim.",
			0x5AC135B499C4483A
		},
		{
			"cg_WeaponButtVictimCameraShakeDuration",
			"Duration in milliseconds of the camera Shake applied to a weapon butt Victim.",
			0x465C02A95F508068
		},
		{
			"cl_dpadLeftHeavyWeapon",
			"When true player switches to the inventory weapon when pressing dpad left.",
			0x3D5BCFA1163E653E
		},
		{
			"cl_bspmismatchFatal",
			"If true client will ERR_DROP if its BSP doesn't match the server's",
			0x39FEE69EFD4711F2
		},
		{
			"cl_bitfieldmismatchFatal",
			"If true client will ERR_DROP if its bitfield doesn't match the server's",
			0xC266A8932CF9CEC
		},
		{
			"dynEnt_disable_rb_collision",
			"Disable rigid body collision for dyn ents with point or hinge constraints.",
			0x1AAD2B07AF7736D4
		},
		{
			"dynEnt_small_cylinder_dimension",
			"Treats dyn ents with small cylinders less than this number in the X or Y as small for the purposes of angular velocity. Applies to dyn ents with point or hinge constraints. Set to 0 to disable.",
			0x61292D4020F08F5BLL
		},
		{
			"dynEnt_delete_expensive",
			"Delete expensive dyn ents.",
			0x4D979C5083D355F
		},
		{
			"g_pickupPromptsForDroppedWeapons",
			"Whether dropped weapons allow a pickup prompt",
			0x17F313F6BB56ABAE
		},
		{
			"ai_avoidNavBoundaries",
			"enables/disables the nearby boundary avoidance",
			0x1F2AEA7F8944E6A4
		},
		{
			"ai_codeGameskill",
			"Use code based gameskill buildup accuracy instead of script's implementation.",
			0x646A06A52703219C
		},
		{
			"ai_secondaryThreats",
			"Allow secondary targets",
			0x272F8D946AE3E82F
		},
		{
			"player_throwbackOuterRadius",
			"The radius player is allow to throwback a grenade once the player has been in the inner radius",
			0x5EAF4E0C0373CE31
		},
		{
			"player_useRadius",
			"The radius within which a player can use things",
			0x1194DB96AF781D52
		},
		{
			"player_useTouchScore",
			"The touch score for player use checks",
			0x3AE4FFCB7525206F
		},
		{
			"g_avoidEnabled",
			"Disable the vehicle avoidance",
			0x6420FE8F4634BCC1
		},
		{
			"g_throttleTempEnts",
			"Whether to throttle the number of temp ents created",
			0x3D8266A7CC590A3B
		},
		{
			"g_allowLastStandForActiveClients",
			"Whether last stand is possible by default for active clients",
			0x53E6EECA9F56D0BF
		},
		{
			"dynEnt_shouldCullEntitiesForSplitscreen",
			"Cull alternating entities in splitscreen",
			0x3ABAC99330C30387
		},
		{
			"spawnsystem_sight_check_sentient_ai",
			"Consider AI sentients while evaluating spawnpoint visibility",
			0x68475AF4558AFA16
		},
		{
			"r_splitscreenBlurEdges",
			"Turns on blurring the side edges in splitscreen",
			0x189B2FBABA6985FF
		},
		{
			"r_maxSpotShadowUpdates",
			"Set the number of spot &omni shadow updates per frame (each omni light account for 6 updates)",
			0x3D8DDE2C8465F2A
		},
		{
			"ai_generateNavmeshRegions",
			"Generate navmesh regions.",
			0x4E13CC2C0E16356A
		},
		{
			"useClassSets",
			"Enable class sets",
			0x23E5A4387FF1BAE5
		},
		{
			"com_pauseSupported",
			"Whether is pause is ever supported by the game mode",
			0x72369C90B4ADFCC7
		},
		{
			"gts_validation_enabled",
			"Enables gametype settings validation in online modes",
			0x2882717B2065EA05
		},
		{
			"path_checkDangerousNodes",
			"Whether to mark and check dangerous nodes",
			0x15A1D9FE7E66AE48
		},
		{
			"path_alwaysFindNearestNode",
			"Use to force finding some nearest node in, generally to try to ensure getting close enough to melee",
			0x772A87292B07D429
		},
		{
			"path_boundsFudge",
			"Fudge the size of the ai's bounds when finding the nearest node",
			0x1883E5500901F485
		},
		{
			"path_minzBias",
			"Bias to prevent missing valid cells below the origin",
			0x430BA728A48A5E4
		},
		{
			"path_double_wide_checks",
			"Whether to do double wide checks when finding a start node for a pathfinding check",
			0x379B878A381D53E0
		},
		{
			"ui_keepLoadingScreenUntilAllPlayersConnected",
			"Whether to keep the loading screen up until all players have initially connected",
			0x40811D6D0A69715D
		},
		{
			"upload_bps_demosystem",
			"demo system bandwidth req'd",
			0x5D47051C209221EF
		},
		{
			"upload_bps_perplayer",
			"per player bandwidth req'd",
			0x315659FEDA74DD21
		},
		{
			"sv_antilagAnim",
			"Rewind animations during antilag rewind",
			0x1294D23BCA5F2FE6
		},
		{
			"g_gametype",
			"Current game type",
			0x1FF54369D6573B91
		},
		{
			"migration_minclientcount",
			"Minimum real client count, below which the host will not migrate.",
			0x58BFAAB543C1D66
		},
		{
			"migration_limit",
			"maximum number of migrations within migration_timeBetween dvar's value in milliseconds.",
			0x434E1DE25E9449DF
		},
		{
			"phys_usePhysicsForLocalClientIfPossible",
			"Use the client physics simulation for the local vehicle if possible instead of the server snapshots",
			0x444A7FAD68CB733B
		},
		{
			"tu10_cleanupRagdollsOnExit",
			"Cleanup ragdolls on shutdown.",
			0x11A578FDEE103601
		},
		{
			"tu11_cleanupRagdollsOnDemoJump",
			"Cleanup ragdolls on demo jump.",
			0x461D77AAA527E2FD
		},
		{
			"demo_dollycamLeaveAtPreviousMarker",
			"Whether to place the camera at the previous dolly cam marker when leaving the mode.",
			0x2A8C932E8F7EEE21
		},
		{
			"demo_freeCameraUseHeliHeightLockAsCeiling",
			"Whether to treat the heli_height_lock as a ceiling.",
			0x4EA01EB65008EC48
		},
		{
			"demo_freeCameraLockOnMissileAllowed",
			"Whether missile entities are eligible for lockon",
			0x2DDDF1803E519648
		},
		{
			"demo_streamUploadKeepAliveFrequency",
			"Specifies the frequency that we send the keep alive on the upload connection if we are otherwise starved for data to send (0 means not in use), used mainly to account for pause.",
			0x357723BC86B953BALL
		},
		{
			"demo_oldposInsteadOfMapCenter",
			"Use the oldpos instead of CL_GetMapCenter() when in freecam",
			0x29023B1D815E0BD8
		},
		{
			"demo_filmStartInformationScreenStayTime",
			"How long the film start information screen will stay on screen in ms.",
			0x1727557D9DA1919F
		},
		{
			"demo_viewTraceMask",
			"The contents mask to use for view traces.",
			0x412217DB434E3CB7
		},
		{
			"demo_useMapNameAsDefaultName",
			"Use the map name instead of the gametype as the default recording name.",
			0x4A18A9BCE200B967
		},
		{
			"perk_requireScavengerPerk",
			"Should the player interact with the scavenger pickup without the perk",
			0xAD6B56033F77267
		},
		{
			"allCollectiblesUnlocked",
			"Allows all Collectibles to be available in the Safehouse",
			0x41D589BB75561DCA
		},
		{
			"equipmentAsGadgets",
			"Enable equipment as gadgets in CAC",
			0x9F6426BBE2E6248
		}
	};

	std::vector<varEntry> commands_record =
	{
		{
			"quit",
			"Shutdown the Game [Com_Quit_f]",
			0x1DEE6107B26F8BB6
		},
		{
			"reload_mods",
			"Reload the shield mods",
			0x6cb53357b4ef835c
		}
	};

	std::vector<const char*> get_dvars_list()
	{
		static std::vector<const char*> dvars;
		if (!dvars.empty()) return dvars;

		for (const auto& dvar : dvars_record)
		{
			dvars.push_back(dvar.name.data());
		}

		return dvars;
	}

	std::vector<const char*> get_commands_list()
	{
		static std::vector<const char*> commands;
		if (!commands.empty()) return commands;

		for (const auto& cmd : commands_record)
		{
			commands.push_back(cmd.name.data());
		}

		return commands;
	}
}