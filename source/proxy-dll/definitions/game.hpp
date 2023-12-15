#pragma once

#include "definitions/discovery.hpp"
#include "definitions/variables.hpp"

#define WEAK __declspec(selectany)

namespace game
{
	//////////////////////////////////////////////////////////////////////////
	//                              VARIABLES                               //
	//////////////////////////////////////////////////////////////////////////
	
	extern std::string version_string;

	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];

	typedef uint32_t ScrVarIndex_t;
	typedef uint64_t ScrVarNameIndex_t;
	
	//////////////////////////////////////////////////////////////////////////
	//                               STRUCTS                                //
	//////////////////////////////////////////////////////////////////////////
	
	struct BO4_AssetRef_t
	{
		__int64 hash;
		__int64 null;
	};
	
	inline BO4_AssetRef_t 
		AssetRef(uint64_t hashRef)
	{
		BO4_AssetRef_t m128i;
		m128i.hash = hashRef;

		return m128i;
	}

	typedef void (*xcommand_t)(void);

	struct cmd_function_t
	{
		cmd_function_t* next;
		uint64_t name;
		uint64_t pad0;
		uint64_t pad1;
		uint64_t pad2;
		xcommand_t function;
	};

	struct GSC_IMPORT_ITEM
	{
		uint32_t name;
		uint32_t name_space;
		uint16_t num_address;
		uint8_t param_count;
		uint8_t flags;
	};

	struct GSC_EXPORT_ITEM
	{
		uint32_t checksum;
		uint32_t address;
		uint32_t name;
		uint32_t name_space;
		uint32_t callback_event;
		uint8_t param_count;
		uint8_t flags;
	};

	struct GSC_OBJ
	{
		byte magic[8];
		int32_t crc;
		int32_t pad;
		uint64_t name;
		int32_t include_offset;
		uint16_t string_count;
		uint16_t exports_count;
		int32_t ukn20;
		int32_t string_offset;
		int16_t imports_count;
		uint16_t fixup_count;
		int32_t ukn2c;
		int32_t exports_offset;
		int32_t ukn34;
		int32_t imports_offset;
		uint16_t globalvar_count;
		int32_t fixup_offset;
		int32_t globalvar_offset;
		int32_t script_size;
		int32_t ukn4c_offset;
		int32_t ukn50;
		int32_t ukn54;
		uint16_t include_count;
		byte ukn5a;
		byte ukn4c_count;

		inline GSC_EXPORT_ITEM* get_exports()
		{
			return reinterpret_cast<GSC_EXPORT_ITEM*>(magic + exports_offset);
		}

		inline GSC_IMPORT_ITEM* get_imports()
		{
			return reinterpret_cast<GSC_IMPORT_ITEM*>(magic + imports_offset);
		}

		inline uint64_t* get_includes()
		{
			return reinterpret_cast<uint64_t*>(magic + include_offset);
		}

		inline GSC_EXPORT_ITEM* get_exports_end()
		{
			return get_exports() + exports_count;
		}

		inline uint64_t* get_includes_end()
		{
			return get_includes() + include_count;
		}
	};


	enum scriptInstance_t : int32_t
	{
		SCRIPTINSTANCE_SERVER = 0x0,
		SCRIPTINSTANCE_CLIENT = 0x1,
		SCRIPTINSTANCE_MAX = 0x2,
	};

	typedef void (*BuiltinFunction)(scriptInstance_t);


	struct BO4_BuiltinFunctionDef
	{
		uint32_t canonId;
		uint32_t min_args;
		uint32_t max_args;
		BuiltinFunction actionFunc;
		uint32_t type;
	};

	struct __declspec(align(4)) BO4_scrVarGlobalVars_t
	{
		uint32_t name;
		ScrVarIndex_t id;
		bool persist;
	};


	enum ScrVarType_t : uint32_t {
		TYPE_UNDEFINED = 0x0,
		TYPE_POINTER = 0x1,
		TYPE_STRING = 0x2,
		TYPE_VECTOR = 0x3,
		TYPE_HASH = 0x4,
		TYPE_FLOAT = 0x5,
		TYPE_INTEGER = 0x6,
		TYPE_UINTPTR = 0x7,
		TYPE_ENTITY_OFFSET = 0x8,
		TYPE_CODEPOS = 0x9,
		TYPE_PRECODEPOS = 0xA,
		TYPE_API_FUNCTION = 0xB,
		TYPE_SCRIPT_FUNCTION = 0xC,
		TYPE_STACK = 0xD,
		TYPE_THREAD = 0xE,
		TYPE_NOTIFY_THREAD = 0xF,
		TYPE_TIME_THREAD = 0x10,
		TYPE_FRAME_THREAD = 0x11,
		TYPE_CHILD_THREAD = 0x12,
		TYPE_CLASS = 0x13,
		TYPE_SHARED_STRUCT = 0x14,
		TYPE_STRUCT = 0x15,
		TYPE_REMOVED_ENTITY = 0x16,
		TYPE_ENTITY = 0x17,
		TYPE_ARRAY = 0x18,
		TYPE_REMOVED_THREAD = 0x19,
		TYPE_FREE = 0x1a,
		TYPE_THREAD_LIST = 0x1b,
		TYPE_ENT_LIST = 0x1c
	};


	struct BO4_scrVarPub {
		const char* fieldBuffer;
		const char* error_message;
		byte* programBuffer;
		byte* endScriptBuffer;
		byte* programHunkUser; // HunkUser
		BO4_scrVarGlobalVars_t globalVars[16];
		ScrVarNameIndex_t entFieldNameIndex;
		ScrVarIndex_t freeEntList;
		ScrVarIndex_t tempVariable;
		uint32_t checksum;
		uint32_t entId;
		uint32_t varHighWatermark;
		uint32_t numScriptThreads;
		uint32_t numVarAllocations;
		int32_t varHighWatermarkId;
	};

	union ScrVarValueUnion_t
	{
		int64_t intValue;
		uintptr_t uintptrValue;
		float floatValue;
		int32_t stringValue;
		const float* vectorValue;
		byte* codePosValue;
		ScrVarIndex_t pointerValue;
	};

	struct ScrVarValue_t
	{
		ScrVarValueUnion_t u;
		ScrVarType_t type;
	};

	struct function_stack_t
	{
		byte* pos;
		ScrVarValue_t* top;
		ScrVarValue_t* startTop;
		ScrVarIndex_t threadId;
		uint16_t localVarCount;
		uint16_t profileInfoCount;
	};

	struct ScrVmContext_t
	{
		ScrVarIndex_t fieldValueId;
		ScrVarIndex_t objectId;
		byte* lastGoodPos;
		ScrVarValue_t* lastGoodTop;
	};

	typedef void (*VM_OP_FUNC)(scriptInstance_t, function_stack_t*, ScrVmContext_t*, bool*);

	struct objFileInfo_t
	{
		GSC_OBJ* activeVersion;
		int slot;
		int refCount;
		uint32_t groupId;
	};

	enum keyNum_t
	{
		K_NONE = 0x00,
		K_BUTTON_A = 0x01,
		K_BUTTON_B = 0x02,
		K_BUTTON_X = 0x03,
		K_BUTTON_Y = 0x04,
		K_BUTTON_LSHLDR = 0x05,
		K_BUTTON_RSHLDR = 0x06,
		K_UNK7 = 0x07,
		K_UNK8 = 0x08,
		K_TAB = 0x09,
		K_UNK10 = 0x0A,
		K_UNK11 = 0x0B,
		K_UNK12 = 0x0C,
		K_ENTER = 0x0D,
		K_BUTTON_START = 0x0E,
		K_BUTTON_BACK = 0x0F,
		K_BUTTON_LSTICK = 0x10,
		K_BUTTON_RSTICK = 0x11,
		K_BUTTON_LTRIG = 0x12,
		K_BUTTON_RTRIG = 0x13,
		K_UNK20 = 0x14,
		K_UNK21 = 0x15,
		K_DPAD_UP = 0x16,
		K_DPAD_DOWN = 0x17,
		K_DPAD_LEFT = 0x18,
		K_DPAD_RIGHT = 0x19,
		K_UNK26 = 0x1A,
		K_ESCAPE = 0x1B,
		K_APAD_UP = 0x1C,
		K_APAD_DOWN = 0x1D,
		K_APAD_LEFT = 0x1E,
		K_APAD_RIGHT = 0x1F,
		K_SPACE = 0x20,
		K_UNK33 = 0x21,
		K_UNK34 = 0x22,
		K_UNK35 = 0x23,
		K_UNK36 = 0x24,
		K_UNK37 = 0x25,
		K_UNK38 = 0x26,
		K_UNK39 = 0x27,
		K_UNK40 = 0x28,
		K_UNK41 = 0x29,
		K_UNK42 = 0x2A,
		K_UNK43 = 0x2B,
		K_UNK44 = 0x2C,
		K_UNK45 = 0x2D,
		K_UNK46 = 0x2E,
		K_UNK47 = 0x2F,
		K_UNK48 = 0x30,
		K_UNK49 = 0x31,
		K_UNK50 = 0x32,
		K_UNK51 = 0x33,
		K_UNK52 = 0x34,
		K_UNK53 = 0x35,
		K_UNK54 = 0x36,
		K_UNK55 = 0x37,
		K_UNK56 = 0x38,
		K_UNK57 = 0x39,
		K_UNK58 = 0x3A,
		K_SEMICOLON = 0x3B,
		K_UNK60 = 0x3C,
		K_UNK61 = 0x3D,
		K_UNK62 = 0x3E,
		K_UNK63 = 0x3F,
		K_UNK64 = 0x40,
		K_UNK65 = 0x41,
		K_UNK66 = 0x42,
		K_UNK67 = 0x43,
		K_UNK68 = 0x44,
		K_UNK69 = 0x45,
		K_UNK70 = 0x46,
		K_UNK71 = 0x47,
		K_UNK72 = 0x48,
		K_UNK73 = 0x49,
		K_UNK74 = 0x4A,
		K_UNK75 = 0x4B,
		K_UNK76 = 0x4C,
		K_UNK77 = 0x4D,
		K_UNK78 = 0x4E,
		K_UNK79 = 0x4F,
		K_UNK80 = 0x50,
		K_UNK81 = 0x51,
		K_UNK82 = 0x52,
		K_UNK83 = 0x53,
		K_UNK84 = 0x54,
		K_UNK85 = 0x55,
		K_UNK86 = 0x56,
		K_UNK87 = 0x57,
		K_UNK88 = 0x58,
		K_UNK89 = 0x59,
		K_UNK90 = 0x5A,
		K_UNK91 = 0x5B,
		K_UNK92 = 0x5C,
		K_UNK93 = 0x5D,
		K_UNK94 = 0x5E,
		K_UNK95 = 0x5F,
		K_GRAVE = 0x60,
		K_UNK97 = 0x61,
		K_UNK98 = 0x62,
		K_UNK99 = 0x63,
		K_UNK100 = 0x64,
		K_UNK101 = 0x65,
		K_UNK102 = 0x66,
		K_UNK103 = 0x67,
		K_UNK104 = 0x68,
		K_UNK105 = 0x69,
		K_UNK106 = 0x6A,
		K_UNK107 = 0x6B,
		K_UNK108 = 0x6C,
		K_UNK109 = 0x6D,
		K_UNK110 = 0x6E,
		K_UNK111 = 0x6F,
		K_UNK112 = 0x70,
		K_UNK113 = 0x71,
		K_UNK114 = 0x72,
		K_UNK115 = 0x73,
		K_UNK116 = 0x74,
		K_UNK117 = 0x75,
		K_UNK118 = 0x76,
		K_UNK119 = 0x77,
		K_UNK120 = 0x78,
		K_UNK121 = 0x79,
		K_UNK122 = 0x7A,
		K_UNK123 = 0x7B,
		K_UNK124 = 0x7C,
		K_UNK125 = 0x7D,
		K_TILDE = 0x7E,
		K_BACKSPACE = 0x7F,
		K_CAPSLOCK = 0x80,
		K_PAUSE = 0x81,
		K_PRINTSCREEN = 0x82,
		K_SCROLLLOCK = 0x83,
		K_UPARROW = 0x84,
		K_DOWNARROW = 0x85,
		K_LEFTARROW = 0x86,
		K_RIGHTARROW = 0x87,
		K_LALT = 0x88,
		K_RALT = 0x89,
		K_LCTRL = 0x8A,
		K_RCTRL = 0x8B,
		K_LSHIFT = 0x8C,
		K_RSHIFT = 0x8D,
		K_HIRAGANA = 0x8E,
		K_HENKAN = 0x8F,
		K_MUHENKAN = 0x90,
		K_LWIN = 0x91,
		K_RWIN = 0x92,
		K_MENU = 0x93,
		K_INS = 0x94,
		K_DEL = 0x95,
		K_PGDN = 0x96,
		K_PGUP = 0x97,
		K_HOME = 0x98,
		K_END = 0x99,
		K_F1 = 0x9A,
		K_F2 = 0x9B,
		K_F3 = 0x9C,
		K_F4 = 0x9D,
		K_F5 = 0x9E,
		K_F6 = 0x9F,
		K_F7 = 0xA0,
		K_F8 = 0xA1,
		K_F9 = 0xA2,
		K_F10 = 0xA3,
		K_F11 = 0xA4,
		K_F12 = 0xA5,
		K_UNK166 = 0xA6,
		K_UNK167 = 0xA7,
		K_UNK168 = 0xA8,
		K_KP_HOME = 0xA9,
		K_KP_UPARROW = 0xAA,
		K_KP_PGUP = 0xAB,
		K_KP_LEFTARROW = 0xAC,
		K_KP_5 = 0xAD,
		K_KP_RIGHTARROW = 0xAE,
		K_KP_END = 0xAF,
		K_KP_DOWNARROW = 0xB0,
		K_KP_PGDN = 0xB1,
		K_KP_ENTER = 0xB2,
		K_KP_INS = 0xB3,
		K_KP_DEL = 0xB4,
		K_KP_SLASH = 0xB5,
		K_KP_MINUS = 0xB6,
		K_KP_PLUS = 0xB7,
		K_KP_NUMLOCK = 0xB8,
		K_KP_STAR = 0xB9,
		K_MOUSE1 = 0xBA,
		K_MOUSE2 = 0xBB,
		K_MOUSE3 = 0xBC,
		K_MOUSE4 = 0xBD,
		K_MOUSE5 = 0xBE,
		K_MWHEELDOWN = 0xBF,
		K_MWHEELUP = 0xC0
	};

	struct KeyState
	{
		int down;
		int repeats;
		int binding;
		char pad[20];
	}; // size = 32

	struct PlayerKeyState
	{
		bool overstrikeMode;
		int anyKeyDown;
		KeyState keys[256];
	};

	struct AssetCache
	{
		uintptr_t whiteMaterial;
		uintptr_t cursor;
		uintptr_t blur;
		uintptr_t devFont; // TTF
		uintptr_t defaultFont; // TTF
		uintptr_t bigFont;
		uintptr_t smallFont;
		uintptr_t consoleFont;
		uintptr_t boldFont;
		uintptr_t textFont;
		uintptr_t extraBigFont;
		uintptr_t extraSmallFont;
		uintptr_t fxFont;
	};

	struct sharedUiInfo_t
	{
		AssetCache assets;
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

	enum dvarType_t
	{
		DVAR_TYPE_INVALID = 0x0,
		DVAR_TYPE_BOOL = 0x1,
		DVAR_TYPE_FLOAT = 0x2,
		DVAR_TYPE_FLOAT_2 = 0x3,
		DVAR_TYPE_FLOAT_3 = 0x4,
		DVAR_TYPE_FLOAT_4 = 0x5,
		DVAR_TYPE_INT = 0x6,
		DVAR_TYPE_ENUM = 0x7,
		DVAR_TYPE_STRING = 0x8,
		DVAR_TYPE_COLOR = 0x9,
		DVAR_TYPE_INT64 = 0xA,
		DVAR_TYPE_UINT64 = 0xB,
		DVAR_TYPE_LINEAR_COLOR_RGB = 0xC,
		DVAR_TYPE_COLOR_XYZ = 0xD,
		DVAR_TYPE_COLOR_LAB = 0xE,
		DVAR_TYPE_SESSIONMODE_BASE_DVAR = 0xF,
		DVAR_TYPE_COUNT = 0x10,
	};

	enum dvarFlags_e
	{
		DVAR_NONE = 0,
		DVAR_ARCHIVE = 1 << 0,
		DVAR_USERINFO = 1 << 1,
		DVAR_SERVERINFO = 1 << 2,
		DVAR_SYSTEMINFO = 1 << 3,
		DVAR_LATCH = 1 << 4,
		DVAR_ROM = 1 << 5,
		DVAR_SAVED = 1 << 6,
		DVAR_INIT = 1 << 7,
		DVAR_CHEAT = 1 << 8,
		//DVAR_UNKNOWN = 1 << 9,
		DVAR_EXTERNAL = 1 << 10,
		//DVAR_UNKNOWN3x = 1 << 11-13,
		DVAR_SESSIONMODE = 1 << 15
	};

	union DvarLimits
	{
		struct
		{
			int stringCount;
			const char** strings;
		} enumeration;

		struct
		{
			int min;
			int max;
		} integer;

		struct
		{
			int64_t min;
			int64_t max;
		} integer64;

		struct
		{
			uint64_t min;
			uint64_t max;
		} unsignedInt64;

		struct
		{
			float min;
			float max;
		} value;

		struct
		{
			vec_t min;
			vec_t max;
		} vector;
	};

	struct dvar_t;

	struct DvarValue
	{
		union
		{
			bool enabled;
			int integer;
			uint32_t unsignedInt;
			int64_t integer64;
			uint64_t unsignedInt64;
			float value;
			vec4_t vector;
			const char* string;
			byte color[4];
			const dvar_t* indirect[3];
		} naked;

		uint64_t encrypted;
	};

	struct DvarData
	{
		DvarValue current;
		DvarValue latched;
		DvarValue reset;
	};

	struct dvar_t
	{
		BO4_AssetRef_t name;
		char padding_unk1[8];
		DvarData* value;
		dvarType_t type;
		unsigned int flags;
		DvarLimits domain;
		char padding_unk2[8];
	};
	
	enum eModes : int32_t
	{
		MODE_ZOMBIES = 0x0,
		MODE_MULTIPLAYER = 0x1,
		MODE_CAMPAIGN = 0x2,
		MODE_WARZONE = 0x3,
		MODE_COUNT = 0x4,
		MODE_INVALID = 0x4,
		MODE_FIRST = 0x0,
	};


	//////////////////////////////////////////////////////////////////////////
	//                               SYMBOLS                                //
	//////////////////////////////////////////////////////////////////////////
	
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

	// CMD
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{ 0x143CDE880_g };

	// Dvar
	WEAK symbol<void* (const char* dvarName)> Dvar_FindVar{ 0x143CEBE40_g };
	WEAK symbol<void* (void* dvarHash)> Dvar_FindVar_Hash{ 0x143CEBED0_g };

	// Live Functions
	WEAK symbol<bool(uint64_t, int*)> Live_GetConnectivityInformation{ 0x1437FA460_g };

	// Rendering Functions
	WEAK symbol<void(const char* text, int maxChars, void* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style, int cursorPos, char cursor, float padding)> T8_AddBaseDrawTextCmd{ 0x143616B60_g };
	WEAK symbol<void(float x, float y, float w, float h, float, float, float, float, float* color, void* material)> R_AddCmdDrawStretchPic{ 0x143616790_g };

	WEAK symbol<int(void* font)> R_TextHeight{ 0x1435B2350_g };
	WEAK symbol<int(void* font, float scale)> UI_TextHeight{ 0x143CD6560_g };

	WEAK symbol<int(int localClientNum, const char* text, int maxChars, void* font)> R_TextWidth{ 0x1435B2530_g };
	WEAK symbol<int(int localClientNum, const char* text, int maxChars, void* font, float scale)> UI_TextWidth{ 0x143CD65B0_g };

	WEAK symbol<ScreenPlacement* (int localClientNum)> ScrPlace_GetView{ 0x142876E70_g };

	WEAK symbol<bool()> Com_IsInGame{ 0x14288FDB0_g };
	WEAK symbol<bool()> Com_IsRunningUILevel{ 0x14288FDF0_g };
	WEAK symbol<eModes()> Com_SessionMode_GetMode{ 0x14289EFF0_g };

	WEAK symbol<int> keyCatchers{ 0x148A53F84_g };
	WEAK symbol<PlayerKeyState> playerKeys{ 0x148A3EF80_g };

	WEAK symbol<sharedUiInfo_t> sharedUiInfo{ 0x14F956850_g };

	// Scr Functions

	WEAK symbol<void(scriptInstance_t inst, int value)> ScrVm_AddBool{ 0x14276E760_g };
	WEAK symbol<void(scriptInstance_t inst, float value)> ScrVm_AddFloat{ 0x14276E9B0_g };
	WEAK symbol<void(scriptInstance_t inst, BO4_AssetRef_t* value)> ScrVm_AddHash{ 0x14276EAB0_g };
	WEAK symbol<void(scriptInstance_t inst, int64_t value)> ScrVm_AddInt{ 0x14276EB80_g };
	WEAK symbol<void(scriptInstance_t inst, const char* value) > ScrVm_AddString{ 0x14276EE30_g };
	WEAK symbol<void(scriptInstance_t inst)> ScrVm_AddUndefined{ 0x14276F3C0_g };
	WEAK symbol<void(scriptInstance_t inst, int32_t value)> ScrVm_AddConstString{ 0x14276E5F0_g };
	WEAK symbol<bool(scriptInstance_t inst, unsigned int index)> ScrVm_GetBool{ 0x142772AB0_g };
	WEAK symbol<float(scriptInstance_t inst, unsigned int index)> ScrVm_GetFloat{ 0x1427733F0_g };
	WEAK symbol<BO4_AssetRef_t* (BO4_AssetRef_t* hash, scriptInstance_t inst, unsigned int index)> ScrVm_GetHash{ 0x1427738E0_g };
	WEAK symbol<int64_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetInt{ 0x142773B50_g };
	WEAK symbol<const char*(scriptInstance_t inst, unsigned int index)> ScrVm_GetString{ 0x142774840_g };
	WEAK symbol<void(scriptInstance_t inst, unsigned int index, vec3_t* vector)> ScrVm_GetVector{ 0x142774E40_g };
	WEAK symbol<int32_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetConstString{ 0x142772E10_g };
	WEAK symbol<uint32_t(scriptInstance_t inst)> ScrVm_GetNumParam{ 0x142774440_g };
	WEAK symbol<ScrVarType_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetPointerType{ 0x1427746E0_g };
	WEAK symbol<ScrVarType_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetType{ 0x142774A20_g };

	WEAK symbol<BuiltinFunction(uint32_t canonId, int* type, int* min_args, int* max_args)> CScr_GetFunction{ 0x141F13140_g };
	WEAK symbol<BuiltinFunction(uint32_t canonId, int* type, int* min_args, int* max_args)> Scr_GetFunction{ 0x1433AF840_g };
	WEAK symbol<void*(uint32_t canonId, int* type, int* min_args, int* max_args)> CScr_GetMethod{ 0x141F13650_g };
	WEAK symbol<void*(uint32_t canonId, int* type, int* min_args, int* max_args)> Scr_GetMethod{ 0x1433AFC20_g };

	WEAK symbol<void(uint64_t code, scriptInstance_t inst, char* unused, bool terminal)> ScrVm_Error{ 0x142770330_g };
	WEAK symbol<BO4_scrVarPub> scrVarPub{ 0x148307880_g };

	WEAK symbol<VM_OP_FUNC> gVmOpJumpTable{ 0x144EED340_g };
	WEAK symbol<uint32_t> gObjFileInfoCount{ 0x1482F76B0_g };
	WEAK symbol<objFileInfo_t[SCRIPTINSTANCE_MAX][650]> gObjFileInfo{ 0x1482EFCD0_g };
	     
	WEAK symbol<void(BO4_AssetRef_t* cmdName, xcommand_t function, cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{0x143CDEE80_g};
	
#define Cmd_AddCommand(name, function) \
    static game::cmd_function_t __cmd_func_##function;  \
    game::BO4_AssetRef_t __cmd_func_name_##function { (int64_t)fnv1a::generate_hash(name), 0 }; \
    game::Cmd_AddCommandInternal(&__cmd_func_name_##function, function, &__cmd_func_##function)

#define R_AddCmdDrawText(TXT, MC, F, X, Y, XS, YS, R, C, S) \
	T8_AddBaseDrawTextCmd(TXT, MC, F, X, Y, XS, YS, R, C, S, -1, 0, 0)

#define R_AddCmdDrawTextWithCursor(TXT, MC, F, X, Y, XS, YS, R, C, S, CP, CC) \
	T8_AddBaseDrawTextCmd(TXT, MC, F, X, Y, XS, YS, R, C, S, CP, CC, 0)
	
#define Com_Error(code, fmt, ...) \
		Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)
}