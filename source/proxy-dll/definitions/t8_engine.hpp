#pragma once

#include "definitions\discovery.hpp"

#define WEAK __declspec(selectany)

namespace game
{
	extern std::string version_string;

	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];
	
	struct T8_Hash_t
	{
		int64_t value;
		int64_t wtf;
	};
	
	struct ScreenPlacement
	{
		vec2_t scaleVirtualToReal;
		vec2_t scaleVirtualToFull;
		vec2_t scaleRealToVirtual;
		vec2_t virtualViewableMin;
		vec2_t virtualViewableMax;
		vec2_t virtualTweakableMin;
		vec2_t virtualTweakableMax;
		vec2_t realViewportBase;
		vec2_t realViewportSize;
		vec2_t realViewportMid;
		vec2_t realViewableMin;
		vec2_t realViewableMax;
		vec2_t realTweakableMin;
		vec2_t realTweakableMax;
		vec2_t subScreen;
		float hudSplitscreenScale;
	};

	enum itemTextStyle
	{
		ITEM_TEXTSTYLE_NORMAL = 0,
		ITEM_TEXTSTYLE_SHADOWED = 3,
		ITEM_TEXTSTYLE_SHADOWEDMORE = 6,
		ITEM_TEXTSTYLE_BORDERED = 7,
		ITEM_TEXTSTYLE_BORDEREDMORE = 8,
		ITEM_TEXTSTYLE_MONOSPACE = 128,
		ITEM_TEXTSTYLE_MONOSPACESHADOWED = 132,
	};

	enum errorParm
	{
		ERR_FATAL = 0,
		ERR_DROP = 1,
		ERR_SERVERDISCONNECT = 2,
		ERR_DISCONNECT = 3,
		ERR_SCRIPT = 4,
		ERR_SCRIPT_DROP = 5,
		ERR_LOCALIZATION = 6,
		ERR_MAPLOADERRORSUMMARY = 7,
	};

	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t address)
			: address_(reinterpret_cast<T*>(address))
		{
		}

		T* get() const
		{
			return address_;
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* address_;
	};

	// Main Functions
	WEAK symbol<void(const char* file, int line, int code, const char* fmt, ...)> Com_Error_{ 0x14288B410_g };

	// Live Functions
	WEAK symbol<bool(uint64_t, int*)> Live_GetConnectivityInformation{ 0x1437FA460_g };
	
	// Rendering Functions
	WEAK symbol<void(const char* text, int maxChars, void* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style, int cursorPos, char cursor, float padding)> T8_AddBaseDrawTextCmd{ 0x143616B60_g };
	WEAK symbol<void*(ScreenPlacement* scrPlace, int fontEnum, float scale)> UI_GetFontHandle{ 0x143CD0A30_g };
	WEAK symbol<int(void* font)> R_TextHeight{ 0x1435B2350_g }; // [BO4-BNET-2023]

	WEAK symbol<ScreenPlacement* (int localClientNum)> ScrPlace_GetView{ 0x142876E70_g };
	

#define R_AddCmdDrawText(TXT, MC, F, X, Y, XS, YS, R, C, S) \
	T8_AddBaseDrawTextCmd(TXT, MC, F, X, Y, XS, YS, R, C, S, -1, 0, 0)

#define R_AddCmdDrawTextWithCursor(TXT, MC, F, X, Y, XS, YS, R, C, S, CP, CC) \
	T8_AddBaseDrawTextCmd(TXT, MC, F, X, Y, XS, YS, R, C, S, CP, CC, 0)
	
#define Com_Error(code, fmt, ...) \
		Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)
}