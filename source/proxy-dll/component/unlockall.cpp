#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utilities/hook.hpp>

namespace unlockall
{
	namespace
	{
		uint32_t zm_loot_table[] =
		{
			1000001,			// zm_bgb_ctrl_z
			1000002,			// zm_bgb_dead_of_nuclear_winter
			1000003,			// zm_bgb_in_plain_sight
			1000004,			// zm_bgb_licensed_contractor
			1000005,			// zm_bgb_phantom_reload
			1000006,			// zm_bgb_sword_flay
			1000007,			// zm_bgb_whos_keeping_score
			1000008,			// zm_bgb_alchemical_antithesis
			1000009,			// zm_bgb_blood_debt
			1000010,			// zm_bgb_extra_credit
			1000011,			// zm_bgb_immolation_liquidation
			1000012,			// zm_bgb_kill_joy
			1000013,			// zm_bgb_shields_up
			1000014,			// zm_bgb_free_fire
			1000015,			// zm_bgb_power_keg
			1000016,			// zm_bgb_undead_man_walking
			1000017,			// zm_bgb_wall_to_wall_clearance
			1000018,			// zm_bgb_cache_back
			1000019,			// zm_bgb_join_the_party
			1000020,			// zm_bgb_wall_power
			1000021,			// [0x596A42B9715FCF4E]
			1000022,			// [0x3ECEF105EDFCE221]
			1000023,			// [0x4A2042B3984A50AC]
			1000024,			// [0x4C39AFAA7B8D508C]
			1000025,			// [0x616F409871DE1C2A]
			1000026,			// [0x233AD55AC2B835D0]
			1000027,			// [0x3E64B28EDF21732A]
			1000028,			// [0xD2C0474517A8626 ]
			1000029,			// [0x589C4CE1578064B7]
			1000030,			// [0x589C4DE15780666A]
			1000031,			// [0x589C4EE15780681D]
			1000032,			// [0x589C47E157805C38]
			1000033,			// [0x1C519F7F8371ECF8]
			1000034,			// [0x70EEBDA916971B02]
			1000035,			// [0x1A0802EC72B26A24]
			1000036,			// [0xCAFC6BD7BFA105A ]
			1000037,			// [0x6BD593B03DBE4709]
			1000038,			// [0x7BA5A74038BFFAB4]
			1000039,			// [0x7BA5AA4038BFFFCD]
			1000040,			// [0x7BA5A94038BFFE1A]
			1000041,			// [0x7BA5A44038BFF59B]
			1000042,			// [0x2DA3AF324B74C125]
			1000043,			// [0x24D9B998E4BC4F8 ]
			1000044,			// [0x17AFAE5369A40BC2]
			1000045,			// [0x17AFAD5369A40A0F]
			1000046,			// [0x17AFAC5369A4085C]
			1000047,			// [0x17AFAB5369A406A9]
			1000048,			// [0x54709F2CDFEC5C52]
			1000049,			// [0x150670241DFB72AF]
			1000050,			// [0x576221EC95A6B4D1]
			1000051,			// [0x15066F241DFB70FC]
			1000052,			// [0x5567F18899A438F1]
			1000053,			// zm_bgb_refresh_mint
			1000054,			// zm_bgb_perk_up
			1000055,			// zm_bgb_conflagration_liquidation
			1000056,			// zm_bgb_bullet_boost
			1000057,			// zm_bgb_talkin_bout_regeneration
			1000058,			// zm_bgb_dividend_yield
			1000059,			// zm_bgb_suit_up
			1000060,			// [0x1734F461CBE4850D]
			1000061,			// [0x4053DCE90F31AA76]
			1000062,			// zm_bgb_perkaholic
			1000063,			// zm_bgb_near_death_experience
			1000064,			// zm_bgb_shopping_free
			1000065,			// zm_bgb_reign_drops
			1000066,			// zm_bgb_phoenix_up
			1000067,			// zm_bgb_head_drama
			1000068,			// zm_bgb_secret_shopper
			1000069 			// zm_bgb_power_vacuum
		};

		inline bool is_zm_loot(int item_id)
		{
			auto it = std::find(
				std::begin(zm_loot_table), std::end(zm_loot_table), item_id);
			if (it != std::end(zm_loot_table)) {
				return true;
			}
			return false;
		}

		int liveinventory_getitemquantity(int controller_index, int item_id)
		{
			int result = is_zm_loot(item_id) ? 999 : 1;

			return result;
		}
	}
	
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utilities::hook::jump(0x1437F6ED0_g, liveinventory_getitemquantity);
		}
	};
}

REGISTER_COMPONENT(unlockall::component)