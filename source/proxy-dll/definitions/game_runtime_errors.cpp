#include <std_include.hpp>
#include "definitions/game.hpp"
#include "definitions/xassets.hpp"
#include "game_runtime_errors.hpp"


namespace game::runtime_errors
{
	namespace
	{
		std::unordered_map<uint64_t, const char*> errors = 
		{
			{ 1045192683, "Scr_RandomFloatRange's second parameter must be greater than the first." },
			{ 1047729873, "exitlevel already called" },
			{ 104978404, "cannot cast type to canon" },
			{ 1072585573, "Raw file is not a file of the right type" },
			{ 1088278929, "Raw file is not a file of the right type" },
			{ 1099145600, "Can't find asset" },
			{ 1132507782, "bad opcode" },
			{ 1137123674, "GScr_LUINotifyEvent: Expected Istrings, hash or integers only" },
			{ 1252503459, "caller is not an entity" },
			{ 1273214009, "CScr_PlayFX: invalid effect" },
			{ 1333649786, "IsMature can only be called on a player." },
			{ 1364331101, "Object must be an array" },
			{ 1377489376, "Gesture key can't have the higher bit set" },
			{ 1385570291, "assert fail (with message)" },
			{ 1402557361, "bad opcode" },
			{ 1403832952, "Attempt to register ClientField failed.  Client Field set either already contains a field, or a hash collision has occurred." },
			{ 1412130119, "parameter can't be cast to a hash" },
			{ 1427704235, "expected struct type to add value pair" },
			{ 1480037573, "Invalid canonical name hash" },
			{ 1480821566, "Error registering client field.  Attempted field size is not acceptable bit number range 1->32" },
			{ 1517473035, "Value out of range.  Allowed values: 0 to 2" },
			{ 1609894461, "bad entity" },
			{ 1670707254, "linking error" },
			{ 1850691545, "Debug Break" },
			{ 1895566756, "dvar is not a 3d vector, but GetDvarVector3D has been called on it" },
			{ 1909233687, "Optional argument must be a vector type" },
			{ 1915758537, "RegisterClientField can only accept bit ranges between 1 and 32." },
			{ 1957162421, "Can't find bgCache entry" },
			{ 1999906612, "type is not a integer or float" },
			{ 2078816051, "not a valid name for a clientfield set." },
			{ 209668787, "RandomInt parm must be positive integer." },
			{ 2116335949, "function called with too many parameters" },
			{ 219569925, "hasperk() can only be called on local players" },
			{ 219686544, "object is not an array index" },
			{ 2253722136, "CamAnimScripted can only be called on a player." },
			{ 2269096660, "vector scale expecting vector" },
			{ 2279633554, "SV_SetConfigstring: bad index" },
			{ 2331090760, "stack does not have the pool" },
			{ 2344222932, "assert fail" },
			{ 2355618801, "Gesture table key can't have the higher bit set" },
			{ 2408700928, "not a valid name for a clientfield set." },
			{ 2448966512, "string too long" },
			{ 245612264, "foreach should be used with an array" },
			{ 247375020, "Attempting to register client field in client field set using bits, but system is out of space." },
			{ 2479879368, "RegisterClientField can only accept bit ranges between 1 and 32." },
			{ 249068885, "Failed to allocate from state pool" },
			{ 2517242050, "parameter does not exist" },
			{ 2532286589, "error message" },
			{ 2538360978, "not a function pointer" },
			{ 2572009355, "vector scale expecting vector" },
			{ 2606724305, "parameter does not exist" },
			{ 2626909173, "exceeded maximum number of script strings" },
			{ 2681972741, "bad clientfield for name" },
			{ 2687742442, "Forced script exception." },
			{ 269518924, "Ammo count must not be negative" },
			{ 2706458388, "Trying to get a local client index for a client that is not a local client." },
			{ 2734491973, "Error need at least one argument for LUINotifyEvent." },
			{ 2737681163, "assert fail (with message)" },
			{ 2751867714, "self isn't a field object" },
			{ 2792722947, "RegisterClientField can only accept 5 parameters" },
			{ 280703902, "parameter does not exist" },
			{ 2838301872, "Gesture table key can't have the higher bit set" },
			{ 2855209542, "Cannot call IncrementClientField on a non-'counter' type clientfield." },
			{ 2940210599, "Invalid Version Handling. Grab Bat !!!" },
			{ 3015158315, "getperks() can only be called on local players" },
			{ 3016026156, "Can't find asset" },
			{ 3030895505, "Whitelist failure for title" },
			{ 3059411687, "Argument and parameter count mismatch for LUINotifyEvent" },
			{ 3086288875, "playFx called with (0 0 0) forward direction" },
			{ 3122940489, "caller is not an entity" },
			{ 312545010, "not a vector" },
			{ 3143575744, "parameter does not exist" },
			{ 317100267, "unmatching types" },
			{ 3189465155, "Invalid bgCache type" },
			{ 3221522156, "Failed to alloc client field" },
			{ 3222417139, "size cannot be applied to type" },
			{ 3251676101, "Could not load raw file" },
			{ 3255107847, "LUINotifyEvent: entity must be a player entity" },
			{ 3288551912, "expected array type to add value pair" },
			{ 3385008561, "cannot set field on type" },
			{ 3459949409, "Failed to alloc client field - MAX_CLIENTFIELD_FIELDS_IN_SET=512 exceeded." },
			{ 3523382186, "ScrEvent map is full, unable to register new event" },
			{ 3524483616, "Ammo count must not be negative" },
			{ 3592841213, "cannot directly set the origin on AI.  Use the teleport command instead." },
			{ 359760836, "G_Spawn: no free entities" },
			{ 3654063291, "ScrEvent map is full, unable to register new event" },
			{ 3679846953, "LUINotifyEvent: entity must be a player entity" },
			{ 3699844858, "parameter does not exist" },
			{ 3761634992, "not a pointer" },
			{ 3894031202, "Can't find gamedata/playeranim/playeranimtypes.txt" },
			{ 3967909977, "Trying to get version of a demo when the demo system is idle." },
			{ 3990130335, "player getperks(<localClientNum>): localClientNum out of range" },
			{ 4039057166, "LUINotifyEvent: entity must be a player entity" },
			{ 4047738848, "Invalid opcode (Recovery)" },
			{ 409067247, "No clientfield named found in set." },
			{ 4100218247, "Error sending LUI notify event: LUI event name is not precached." },
			{ 4103906837, "Entity is not an item." },
			{ 4104994143, "can't allocate more script variables" },
			{ 4106063796, "key value provided for array is not valid" },
			{ 4163774148, "Optional argument must be a float or integer type" },
			{ 4178589916, "Model was not cached by the linker." },
			{ 4196473479, "parameter does not exist" },
			{ 4213634562, "precacheLeaderboards must be called before any wait statements in the gametype or level script" },
			{ 4220857104, "Cannot call IncrementClientField on a 'counter' type clientfield on the frame it is spawned, since newEnts on the clientside will not process 'counter' type clientfields." },
			{ 467754466, "Error registering client field.  bCallbacksFor0WhenNew (CF_CALLBACK_ZERO_ON_NEW_ENT) is disallowed for CF_SCRIPTMOVERFIELDS." },
			{ 512306404, "not a function pointer" },
			{ 57350207, "Unknown perk" },
			{ 580674660, "Unknown ent type in GScr_GetFieldSetForEntityType." },
			{ 619241173, "Failed to allocate from element pool" },
			{ 647662103, "parameter can't be cast to a string" },
			{ 657813230, "Error registering client field.  bCallbacksFor0WhenNew (CF_CALLBACK_ZERO_ON_NEW_ENT) is disallowed for counter type clientfields. Due to it's treatment of the old and new val as a ring buffer, the counter type is not valid on a new snapshot, new ent, or demojump" },
			{ 665902298, "Parameter must be an array" },
			{ 71894325, "CamAnimScripted or ExtraCamAnimScripted can only be called on a player." },
			{ 732489269, "Non-player entity passed to UploadStats()" },
			{ 744499668, "too many vehicles" },
			{ 750896894, "Invalid bgCache type" },
			{ 753495682, "RandomIntRange's second parameter must be greater than the first." },
			{ 754846421, "invalid vehicle spawn origin" },
			{ 829015102, "var isn't a field object" },
			{ 876169112, "key value provided for struct is not valid" },
			{ 887228744, "origin being set to NAN." },
			{ 904544970, "Attempt to register Client Field post finalization of Client Field registration period failed." },
			{ 941828720, "exitlevel already called" },
			{ 946363963, "Invalid opcode" },
			{ 949934674, "Invalid Version Handling. Grab Bat !!!" },
			{ 952690413, "parameter can't be cast to a boolean" },
			{ 962032109, "Entity does not exist." },
			{ 968521323, "player hasperk(<localClientNum>, <perk>): localClientNum out of range" },
			{ 4088624643, "Can't find asset" },
			{ game::runtime_errors::custom_error_id, "Shield Error" }
		};
	}

	const char* get_error_message(uint64_t code)
	{
		auto it = errors.find(code);
		// known error
		if (it != errors.end())
		{
			return it->second;
		}

		// read from the csv
		static game::BO4_AssetRef_t custom_errors_file = game::AssetRef(fnv1a::generate_hash("gamedata/shield/custom_errors.csv"));

		xassets::stringtable_header* table = xassets::DB_FindXAssetHeader(xassets::ASSET_TYPE_STRINGTABLE, &custom_errors_file, false, -1).stringtable;

		if (!table || !table->columns_count || table->columns_count < 2)
		{
			return nullptr;
		}

		for (size_t i = 0; i < table->rows_count; i++)
		{
			auto* rows = &table->values[i * table->columns_count];

			if (rows[0].type != xassets::STC_TYPE_INT || rows[1].type != xassets::STC_TYPE_STRING)
			{
				continue; // bad types
			}

			if (rows[0].value.hash_value == code)
			{
				return rows[1].value.string_value;
			}
		}

		// unknown
		return nullptr;
	}

}

