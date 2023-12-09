#include <std_include.hpp>
#include "gsc_custom.hpp"
#include "gsc_funcs.hpp"
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"
#include <utilities/hook.hpp>
#include <utilities/json_config.hpp>

namespace gsc_custom
{
	namespace
	{
		struct gsic_link_detour_data
		{
			const gsic_info& gsic_info;
			uint32_t latest_script_index{};
		};

		template<typename T>
		inline byte* align_ptr(byte* ptr)
		{
			return reinterpret_cast<byte*>((reinterpret_cast<uintptr_t>(ptr) + sizeof(T) - 1) & ~(sizeof(T) - 1));
		}

		std::vector<gsic_link_detour_data> gsic_data[game::SCRIPTINSTANCE_MAX]{ {}, {} };

		byte* find_export(game::scriptInstance_t inst, uint64_t target_script, uint32_t name_space, uint32_t name)
		{
			uint32_t count = game::gObjFileInfoCount[inst];

			game::objFileInfo_t* end = (*game::gObjFileInfo)[inst] + count;

			auto its = std::find_if((*game::gObjFileInfo)[inst], end,
				[target_script](const game::objFileInfo_t& info) { return info.activeVersion->name == target_script; });

			if (its == end)
			{
				return nullptr; // can't find target script
			}

			game::GSC_OBJ* obj = its->activeVersion;

			auto ite = std::find_if(obj->get_exports(), obj->get_exports_end(), 
				[name_space, name](const game::GSC_EXPORT_ITEM& exp) { return exp.name == name && exp.name_space == name_space; });

			if (ite == obj->get_exports_end())
			{
				return nullptr; // can't find target export
			}

			return obj->magic + ite->address;
		}

		byte* find_detour(game::scriptInstance_t inst, byte* origin, uint64_t target_script, uint32_t name_space, uint32_t name)
		{
			for (const gsic_link_detour_data& data : gsic_data[inst])
			{
				for (const gsic_detour& detour : data.gsic_info.detours)
				{
					if (detour.target_script != target_script || name_space != detour.replace_namespace || name != detour.replace_function)
					{
						continue; // not our target
					}
					if (detour.fixup_function <= origin && detour.fixup_function + detour.fixup_offset > origin)
					{
						continue; // inside the detour
					}
					return detour.fixup_function;
				}
			}

			return nullptr;
		}


		void clear_gsic(game::scriptInstance_t inst)
		{
			// clear previously register GSIC
			gsic_data[inst].clear();
		}

		void link_detours(game::scriptInstance_t inst)
		{
			// link the GSIC detours

			for (gsic_link_detour_data& data : gsic_data[inst])
			{
				for (; data.latest_script_index < game::gObjFileInfoCount[inst]; data.latest_script_index++)
				{
					game::GSC_OBJ* obj = (*game::gObjFileInfo)[inst][data.latest_script_index].activeVersion;
					// link the detour
					for (const gsic_detour& detour : data.gsic_info.detours)
					{
						if (std::find(obj->get_includes(), obj->get_includes_end(), detour.target_script) == obj->get_includes_end() && obj->name != detour.target_script)
						{
							continue; // the target script isn't in the includes, so we have nothing to link
						}

						// reading imports

						game::GSC_IMPORT_ITEM* import_item = obj->get_imports();

						for (size_t i = 0; i < obj->imports_count; i++)
						{
							uint32_t* addresses = reinterpret_cast<uint32_t*>(import_item + 1);

							if (import_item->name_space == detour.replace_namespace && import_item->name == detour.replace_function)
							{
								// replace the linking

								// see GscObjResolve(scriptInstance_t, GSC_OBJ*)0x142746A30_g for info

								int offset;
								switch (import_item->flags & 0xF)
								{
								case 1: // &namespace::function
								{
									offset = 0; // only function/method calls are using params
								}
								break;
								case 2: // func()
								case 3: // thread func()
								case 4: // childthread func()
								case 5: // self method()
								case 6: // self thread method()
								case 7: // self childthread method()
								{
									offset = 1;
								}
								break;
								default:
									logger::write(logger::LOG_TYPE_ERROR, std::format("can't link import item with flag {:x}", import_item->flags & 0xF));
									assert(false); // if the game didn't crash before this point it's impressive
									return;
								}

								for (size_t j = 0; j < import_item->num_address; j++)
								{
									// opcode loc
									byte* loc = align_ptr<uint16_t>(obj->magic + addresses[j]);

									if (loc >= detour.fixup_function && loc < detour.fixup_function + detour.fixup_size)
									{
										continue; // this import is inside the detour definition, we do not replace it
									}

									// align for ptr
									byte** ptr_loc = (byte**)align_ptr<uintptr_t>(loc + 2 + offset);
#ifdef _DEBUG_DETOUR
									logger::write(logger::LOG_TYPE_DEBUG, 
										std::format(
											"linking detours for namespace_{:x}<script_{:x}>::function_{:x} at {} : {} -> {} (0x{:x})",
											detour.replace_namespace, obj->name, detour.replace_function, 
											(void*)ptr_loc,
											(void*)(*ptr_loc),
											(void*)(detour.fixup_function), *(uint64_t*)detour.fixup_function)
									);
#endif

									// TODO: replace opcode for api function detours
									//uint16_t* opcode_loc = (uint16_t*)loc;

									*ptr_loc = detour.fixup_function;
								}
							}

							// goto to the next element after the addresses
							import_item = reinterpret_cast<game::GSC_IMPORT_ITEM*>(addresses + import_item->num_address);
						}
					}
				}
			}
		}
	}

	void sync_gsic(game::scriptInstance_t inst, gsic_info& info)
	{
		// add a new GSIC file for this instance

		auto& inst_data = gsic_data[inst];

		if (std::find_if(inst_data.begin(), inst_data.end(), [&info](const gsic_link_detour_data& data) { return &data.gsic_info == &info; }) != inst_data.end())
		{
			return; // already sync
		}

		for (const gsic_link_detour_data& link_data : inst_data)
		{
			for (const gsic_detour& detour : link_data.gsic_info.detours)
			{
				auto it = std::find_if(info.detours.begin(), info.detours.end(),
					[&detour](const gsic_detour& detour2)
					{
						return detour.target_script == detour2.target_script
							&& detour.replace_function == detour2.replace_function
							&& detour.replace_namespace == detour2.replace_namespace;
					}
				);

				if (it != info.detours.end())
				{
					gsc_funcs::gsc_error("the detour namespace_%x<script_%llx>::function_%x was registered twice", inst, true, detour.replace_namespace, detour.target_script, detour.replace_function);
					return;
				}
			}
		}

		inst_data.emplace_back(info);
	}

	void vm_op_custom_lazylink(game::scriptInstance_t inst, game::function_stack_t* fs_0, game::ScrVmContext_t* vmc, bool* terminate)
	{
		byte* base = align_ptr<uint32_t>(fs_0->pos);
		uint32_t name_space = *(uint32_t*)base;
		uint32_t name = *(uint32_t*)(base + 4);
		uint64_t script = *(uint64_t*)(base + 8);

		// pass the data
		fs_0->pos = base + 0x10;

		// find the detour first
		byte* detour_result = find_detour(inst, base, script, name_space, name);

		if (detour_result)
		{
			// push detour function
			fs_0->top++;

			fs_0->top->type = game::TYPE_SCRIPT_FUNCTION;
			fs_0->top->u.codePosValue = detour_result;
		}
		else if (script)
		{
			// lazy link script function
			byte* exp = find_export(inst, script, name_space, name);


			// push the result or undefined
			fs_0->top++;

			if (exp)
			{
				fs_0->top->type = game::TYPE_SCRIPT_FUNCTION;
				fs_0->top->u.codePosValue = exp;
			}
			else
			{
				fs_0->top->type = game::TYPE_UNDEFINED;
				fs_0->top->u.intValue = 0;
			}
		}
		else
		{
			// lazy link api function
			int type = 0;
			int unused = 0;
			void* func = nullptr;
			
			if (inst)
			{
				func = game::CScr_GetFunction(name, &type, &unused, &unused);
				if (!func || (type && !gsc_funcs::enable_dev_func))
				{
					func = game::CScr_GetMethod(name, &type, &unused, &unused);
				}
			}
			else
			{
				func = game::Scr_GetFunction(name, &type, &unused, &unused);
				if (!func || (type && !gsc_funcs::enable_dev_func))
				{
					func = game::Scr_GetMethod(name, &type, &unused, &unused);
				}
			}

			fs_0->top++;

			if (func && (!type || gsc_funcs::enable_dev_func))
			{
				// do not allow dev functions if it is not asked by the user
				fs_0->top->type = game::TYPE_API_FUNCTION;
				fs_0->top->u.codePosValue = (byte*)func;
			}
			else
			{
				fs_0->top->type = game::TYPE_UNDEFINED;
				fs_0->top->u.intValue = 0;
			}
		}

	}

	utilities::hook::detour scr_get_gsc_obj_hook;
	void scr_get_gsc_obj_stub(game::scriptInstance_t inst, game::BO4_AssetRef_t* name, bool runScript)
	{
		if (game::gObjFileInfoCount[inst] == 0)
		{
			// first script for this instance, we can clear previous GSIC
			gsc_custom::clear_gsic(inst);
		}

		scr_get_gsc_obj_hook.invoke<void>(inst, name, runScript);

		// link the detours with the new linked scripts group
		gsc_custom::link_detours(inst);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// t8compiler custom opcode
			game::gVmOpJumpTable[0x16] = vm_op_custom_lazylink;

			// group gsc link
			scr_get_gsc_obj_hook.create(0x142748BB0_g, scr_get_gsc_obj_stub);
		}
	};
}

REGISTER_COMPONENT(gsc_custom::component)