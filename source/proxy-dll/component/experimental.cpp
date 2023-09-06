#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace experimental
{
	namespace
	{
		utils::hook::detour liveinventory_getitemquantity_hook;
		int liveinventory_getitemquantity_stub(int controller_index, const int item_id)
		{
			return 999;

			return liveinventory_getitemquantity_hook.invoke<int>(controller_index, item_id);
		}
	}
	
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			liveinventory_getitemquantity_hook.create(0x1437F6ED0_g, liveinventory_getitemquantity_stub);

			utils::hook::set(0x142DD0E10_g, 0xC301B0); // Live_Qos_Finished
			utils::hook::set(0x1438C2C70_g, 0xC301B0); // Live_Contracts? related to bdUNK125
		}
	};
}

REGISTER_COMPONENT(experimental::component)
