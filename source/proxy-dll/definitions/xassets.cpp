#include <std_include.hpp>
#include "xassets.hpp"

namespace xassets
{
	namespace
	{
		const char* cache_names[]
		{
			"invalid",
			"vehicle",
			"model",
			"aitype",
			"character",
			"xmodelalias",
			"weapon",
			"gesture",
			"gesturetable",
			"zbarrier",
			"rumble",
			"shellshock",
			"statuseffect",
			"xcam",
			"destructible",
			"streamerhint",
			"flowgraph",
			"xanim",
			"sanim",
			"scriptbundle",
			"talent",
			"statusicon",
			"locationselector",
			"menu",
			"material",
			"string",
			"eventstring",
			"moviefile",
			"objective",
			"fx",
			"lui_menu_data",
			"lui_elem",
			"lui_elem_uid",
			"radiant_exploder",
			"soundalias",
			"client_fx",
			"client_tagfxset",
			"client_lui_elem",
			"client_lui_elem_uid",
			"requires_implements"
		};
	}

	BGCacheTypes BG_Cache_GetTypeIndex(const char* name)
	{
		//BG_Cache_GetTypeIndexInternal base isn't available after unpack
		for (size_t i = 0; i < BGCacheTypes::BG_CACHE_TYPE_COUNT; i++)
		{
			if (!_strcmpi(name, cache_names[i]))
			{
				return (BGCacheTypes)i;
			}
		}
		return BG_CACHE_TYPE_INVALID;
	}

	const char* BG_Cache_GetTypeName(BGCacheTypes type)
	{
		return cache_names[type];
	}

	XAssetType DB_GetXAssetTypeIndex(const char* name)
	{
		for (size_t i = 0; i < XAssetType::ASSET_TYPE_COUNT; i++)
		{
			if (!_strcmpi(name, s_XAssetTypeInfo[i].name))
			{
				return (XAssetType)i;
			}
		}
		return XAssetType::ASSET_TYPE_COUNT;
	}

	const char* DB_GetXAssetTypeName(XAssetType type)
	{
		if (type >= ASSET_TYPE_COUNT)
		{
			return "invalid";
		}
		return s_XAssetTypeInfo[type].name;
	}
}