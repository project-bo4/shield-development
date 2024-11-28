#pragma once
#include "keys.hpp"
#include "network.hpp"
#include "scripting.hpp"
#include "definitions/variables.hpp"

#define WEAK __declspec(selectany)

namespace game
{
	//////////////////////////////////////////////////////////////////////////
	//                                SHIELD                                //
	//////////////////////////////////////////////////////////////////////////

	void verify_game_version();

	const char* Com_GetVersionString();

	///////////////////////////////////////////////////////////////////////////
	//                                 TYPES                                 //
	///////////////////////////////////////////////////////////////////////////

	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];

	///////////////////////////////////////////////////////////////////////////
	//                                STRUCTS                                //
	///////////////////////////////////////////////////////////////////////////
	
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

	inline BO4_AssetRef_t
		AssetRef(const char* nameRef)
	{
		BO4_AssetRef_t m128i;
		m128i.hash = fnv1a::generate_hash(nameRef);

		return m128i;
	}

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

	typedef void (*xcommand_t)(void);

	struct cmd_function_t
	{
		cmd_function_t* next;
		BO4_AssetRef_t name;
		const char* autoCompleteDir;
		const char* autoCompleteExt;
		xcommand_t function;
		int autoComplete;
	};
	
	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		int argshift[8];
		int argc[8];
		const char** argv[8];
		char textPool[8192];
		const char* argvPool[512];
		int usedTextPool[8];
		int totalUsedArgvPool;
		int totalUsedTextPool;
	};

	struct va_info_t
	{
		char va_string[4][1024];
		int index;
	};

	struct TLSData
	{
		va_info_t* vaInfo;
		jmp_buf* errorJmpBuf;
		void* traceInfo;
		CmdArgs* cmdArgs;
		void* errorData;
	};

	///////////////////////////////////////////////////////////////////////////
	//                                 ENUMS                                 //
	///////////////////////////////////////////////////////////////////////////
	
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

	enum eGameModes
	{
		MODE_GAME_MATCHMAKING_PLAYLIST = 0x0,
		MODE_GAME_MATCHMAKING_MANUAL = 0x1,
		MODE_GAME_DEFAULT = 0x2,
		MODE_GAME_LEAGUE = 0x3,
		MODE_GAME_THEATER = 0x4,
		MODE_GAME_COUNT = 0x5,
		MODE_GAME_INVALID = 0x5,
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

	enum consoleLabel_e : int32_t
	{
		CON_LABEL_TEMP = 0x0,
		CON_LABEL_GFX = 0x2,
		CON_LABEL_TASKMGR2 = 0x3,
		CON_LABEL_LIVE = 0x4,
		CON_LABEL_LIVE_XBOX = 0x5,
		CON_LABEL_LIVE_PS4 = 0x6,
		CON_LABEL_MATCHMAKING = 0x7,
		CON_LABEL_DEMONWARE = 0x8,
		CON_LABEL_LEADERBOARDS = 0x9,
		CON_LABEL_LOBBY = 0x0A,
		CON_LABEL_LOBBYHOST = 0x0B,
		CON_LABEL_LOBBYCLIENT = 0x0C,
		CON_LABEL_LOBBYVM = 0x0D,
		CON_LABEL_MIGRATION = 0x0E,
		CON_LABEL_IG_MIGRATION_Host = 0x0F,
		CON_LABEL_IG_MIGRATION_Client = 0x10,
		CON_LABEL_SCRIPTER = 0x11,
		CON_LABEL_VM = 0x12,
		CON_LABEL_DVAR = 0x13,
		CON_LABEL_TOOL = 0x14,
		CON_LABEL_ANIM = 0x15,
		CON_LABEL_NETCHAN = 0x16,
		CON_LABEL_BGCACHE = 0x17,
		CON_LABEL_PM = 0x18,
		CON_LABEL_MAPSWITCH = 0x19,
		CON_LABEL_AI = 0x1A,
		CON_LABEL_GADGET = 0x1B,
		CON_LABEL_SOUND = 0x1C,
		CON_LABEL_SNAPSHOT = 0x1D,
		CON_LABEL_PLAYGO = 0x1E,
		CON_LABEL_LUI = 0x1F,
		CON_LABEL_LUA = 0x20,
		CON_LABEL_VOIP = 0x21,
		CON_LABEL_DEMO = 0x22,
		CON_LABEL_DB = 0x23,
		CON_LABEL_HTTP = 0x24,
		CON_LABEL_DCACHE = 0x25,
		CON_LABEL_MEM = 0x26,
		CON_LABEL_CINEMATIC = 0x27,
		CON_LABEL_DDL = 0x28,
		CON_LABEL_STORAGE = 0x29,
		CON_LABEL_STEAM = 0x2A,
		CON_LABEL_CHKPTSAVE = 0x2B,
		CON_LABEL_DLOG = 0x2C,
		CON_LABEL_FILESHARE = 0x2D,
		CON_LABEL_LPC = 0x2E,
		CON_LABEL_MARKETING = 0x2F,
		CON_LABEL_STORE = 0x30,
		CON_LABEL_TESTING = 0x31,
		CON_LABEL_LOOT = 0x32,
		CON_LABEL_MATCHRECORDER = 0x33,
		CON_LABEL_EXCHANGE = 0x34,
		CON_LABEL_SCRIPTERROR = 0x35,
		CON_LABEL_PLAYOFTHEMATCH = 0x36,
		CON_LABEL_FILESYS = 0x37,
		CON_LABEL_JSON = 0x38,
		CON_LABEL_CUSTOMGAMES = 0x39,
		CON_LABEL_GAMEPLAY = 0x3A,
		CON_LABEL_STREAM = 0x3B,
		CON_LABEL_XPAK = 0x3C,
		CON_LABEL_AE = 0x3D,
		CON_LABEL_STRINGTABLE = 0x3E,
		CON_LABEL_COUNT = 0x3F
	};

	///////////////////////////////////////////////////////////////////////////
	//                                CLASSES                                //
	///////////////////////////////////////////////////////////////////////////
	
	enum scoped_critical_section_type : int32_t
	{
		SCOPED_CRITSECT_NORMAL = 0x0,
		SCOPED_CRITSECT_DISABLED = 0x1,
		SCOPED_CRITSECT_RELEASE = 0x2,
		SCOPED_CRITSECT_TRY = 0x3,
	};

	class scoped_critical_section
	{
		int32_t _s;
		bool _hasOwnership;
		bool _isScopedRelease;
		scoped_critical_section* _next;
	public:
		scoped_critical_section(int32_t s, scoped_critical_section_type type);
		~scoped_critical_section();
	};

	///////////////////////////////////////////////////////////////////////////
	//                                 HAVOK                                 //
	///////////////////////////////////////////////////////////////////////////
	
	struct hks_global {};
	struct hks_callstack
	{
		void* m_records; // hks::CallStack::ActivationRecord*
		void* m_lastrecord; // hks::CallStack::ActivationRecord*
		void* m_current; // hks::CallStack::ActivationRecord*
		const void* m_current_lua_pc; // const hksInstruction*
		const void* m_hook_return_addr; // const hksInstruction*
		int32_t m_hook_level;
	};
	struct lua_state;
	struct hks_object
	{
		uint32_t t;
		union {
			void* ptr;
			float number;
			int32_t boolean;
			uint32_t native;
			lua_state* thread;
		} v;
	};
	struct hks_api_stack
	{
		hks_object* top;
		hks_object* base;
		hks_object* alloc_top;
		hks_object* bottom;
	};

	struct lua_state
	{
		// hks::GenericChunkHeader
		size_t m_flags;
		// hks::ChunkHeader
		void* m_next;

		hks_global* m_global;
		hks_callstack m_callStack;
		hks_api_stack m_apistack;

		// ...
	};

	///////////////////////////////////////////////////////////////////////////
	//                                SYMBOLS                                //
	///////////////////////////////////////////////////////////////////////////
	
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

	// Main
	WEAK symbol<void(const char* file, int line, int code, const char* fmt, ...)> Com_Error_{ 0x14288B410_g };

	// Sys
	WEAK symbol<int()> Sys_Milliseconds{ 0x143D89E80_g };
	WEAK symbol<TLSData* ()> Sys_GetTLS{ 0x143C56140_g };

	// Mutex
	WEAK symbol<void(scoped_critical_section* sec, int32_t s, scoped_critical_section_type type)> ScopedCriticalSectionConstructor{ 0x14289E3C0_g };
	WEAK symbol<void(scoped_critical_section* sec)> ScopedCriticalSectionDestructor{ 0x14289E440_g };

	// Dvar
	WEAK symbol<void* (const char* dvarName)> Dvar_FindVar{ 0x143CEBE40_g };
	WEAK symbol<void* (void* dvarHash)> Dvar_FindVar_Hash{ 0x143CEBED0_g };

	// Cmd
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{ 0x143CDE880_g };
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* buffer)> Cbuf_ExecuteBuffer{ 0x143CDEBE0_g };
	WEAK symbol<int()> Com_LocalClients_GetPrimary{ 0x142893AF0_g };

	WEAK symbol<void(BO4_AssetRef_t* cmdName, xcommand_t function, cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{ 0x143CDEE80_g };
	WEAK symbol<void()> Cbuf_AddServerText_f{ 0x143CDE870_g };
	WEAK symbol<void(BO4_AssetRef_t* cmdName, xcommand_t function, cmd_function_t* allocedCmd)> Cmd_AddServerCommandInternal{ 0x143CDEEF0_g };
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* text, bool fromRemoteConsole)> Cmd_ExecuteSingleCommand{ 0x143CDF490_g };

	WEAK symbol<void(int localClientNum, int localControllerIndex, const char* text_in,
		int max_tokens, bool evalExpressions, CmdArgs* args)> Cmd_TokenizeStringKernel{ 0x143CE0750_g };

	WEAK symbol<void()> Cmd_EndTokenizedString{ 0x143CDF070_g };
	WEAK symbol<void(const char* text_in)> SV_Cmd_TokenizeString{ 0x143CE0A10_g };
	WEAK symbol<void()> SV_Cmd_EndTokenizedString{ 0x143CE09C0_g };

	// NET
	WEAK symbol<bool(netsrc_t sock, netadr_t* adr, const void* data, int len)> NET_OutOfBandData{ 0x142E06390_g };
	WEAK symbol<bool(netsrc_t sock, int length, const void* data, const netadr_t* to)> Sys_SendPacket{ 0x143D89900_g };
	WEAK symbol<bool(netadr_t* adr, const char* s)> NetAdr_InitFromString{ 0x142E05230_g };
	WEAK symbol<bool(const netadr_t addr1, const netadr_t addr2)> NetAdr_IsTheSameAddr{ 0x142E053A0_g };

	// Live
	WEAK symbol<bool(uint64_t, int*)> Live_GetConnectivityInformation{ 0x1437FA460_g };

	// Renderer
	WEAK symbol<void(const char* text, int maxChars, void* font, float x, float y, float xScale, float yScale, float rotation,
		            float* color, int style, int cursorPos, char cursor, float padding)> T8_AddBaseDrawTextCmd{ 0x143616B60_g };
	WEAK symbol<void(float x, float y, float w, float h, float, float, float, float, float* color, void* material)> R_AddCmdDrawStretchPic{ 0x143616790_g };

	WEAK symbol<int(void* font)> R_TextHeight{ 0x1435B2350_g };
	WEAK symbol<int(void* font, float scale)> UI_TextHeight{ 0x143CD6560_g };

	WEAK symbol<int(int localClientNum, const char* text, int maxChars, void* font)> R_TextWidth{ 0x1435B2530_g };
	WEAK symbol<int(int localClientNum, const char* text, int maxChars, void* font, float scale)> UI_TextWidth{ 0x143CD65B0_g };

	WEAK symbol<ScreenPlacement* (int localClientNum)> ScrPlace_GetView{ 0x142876E70_g };

	// BuildInfo
	WEAK symbol<const char* ()> Com_GetBuildVersion{ 0x142892F40_g };
	WEAK symbol<bool(int controllerIndex, int info, char* outputString, const int outputLen)> Live_SystemInfo{ 0x143804B00_g };

	// ETC
	WEAK symbol<bool()> Com_IsInGame{ 0x14288FDB0_g };
	WEAK symbol<bool()> Com_IsRunningUILevel{ 0x14288FDF0_g };
	WEAK symbol<eModes()> Com_SessionMode_GetMode{ 0x14289EFF0_g };
	WEAK symbol<eModes(const char* str)> Com_SessionMode_GetModeForAbbreviation{ 0x14289F000_g };
	WEAK symbol<const char*(eModes mode)> Com_SessionMode_GetAbbreviationForMode{0x14289EC70_g};

	// SCR
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
	WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetConstString{ 0x142772E10_g };
	WEAK symbol<uint32_t(scriptInstance_t inst)> ScrVm_GetNumParam{ 0x142774440_g };
	WEAK symbol<ScrVarType_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetPointerType{ 0x1427746E0_g };
	WEAK symbol<ScrVarType_t(scriptInstance_t inst, unsigned int index)> ScrVm_GetType{ 0x142774A20_g };
	WEAK symbol<uint32_t(scriptInstance_t inst)> ScrVm_AddStruct{ 0x14276EF00_g };
	WEAK symbol<void(scriptInstance_t inst, uint32_t structId, uint32_t name)> ScrVm_SetStructField{ 0x142778450_g };
	WEAK symbol<void(scriptInstance_t inst)> ScrVm_AddToArray{ 0x14276F1C0_g };
	WEAK symbol<void(scriptInstance_t inst, BO4_AssetRef_t* name)> ScrVm_AddToArrayStringIndexed{ 0x14276F230_g };
	WEAK symbol<void(scriptInstance_t inst, vec3_t* vec)> ScrVm_AddVector{ 0x14276F490_g };
	WEAK symbol<void(scriptInstance_t inst)> ScrVar_PushArray{ 0x142775CF0_g };
	WEAK symbol<const char* (ScrVarIndex_t index)> ScrStr_ConvertToString{ 0x142759030_g };
	WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId, ScrVarNameIndex_t index)> ScrVar_NewVariableByIndex{ 0x142760440_g };
	WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id, ScrVarValue_t* value)> ScrVar_SetValue{ 0x1427616B0_g };
	
	WEAK symbol<BuiltinFunction(uint32_t canonId, int* type, int* min_args, int* max_args)> CScr_GetFunction{ 0x141F13140_g };
	WEAK symbol<BuiltinFunction(uint32_t canonId, int* type, int* min_args, int* max_args)> Scr_GetFunction{ 0x1433AF840_g };
	WEAK symbol<void*(uint32_t canonId, int* type, int* min_args, int* max_args)> CScr_GetMethod{ 0x141F13650_g };
	WEAK symbol<void*(uint32_t canonId, int* type, int* min_args, int* max_args)> Scr_GetMethod{ 0x1433AFC20_g };
	WEAK symbol<void(game::scriptInstance_t inst, byte* codepos, const char** scriptname, int32_t* sloc, int32_t* crc, int32_t* vm)> Scr_GetGscExportInfo{ 0x142748550_g };

	WEAK symbol<void(uint64_t code, scriptInstance_t inst, char* unused, bool terminal)> ScrVm_Error{ 0x142770330_g };
	WEAK symbol<BO4_scrVarPub> scrVarPub{ 0x148307880_g };
	WEAK symbol<BO4_scrVarGlob> scrVarGlob{ 0x148307830_g };
	WEAK symbol<BO4_scrVmPub> scrVmPub{ 0x148307AA0_g };

	WEAK symbol<VM_OP_FUNC> gVmOpJumpTable{ 0x144EED340_g };
	WEAK symbol<uint32_t> gObjFileInfoCount{ 0x1482F76B0_g };
	WEAK symbol<objFileInfo_t[SCRIPTINSTANCE_MAX][650]> gObjFileInfo{ 0x1482EFCD0_g };

	// LUA
	WEAK symbol<bool(lua_state* luaVM, const char* file)> Lua_CoD_LoadLuaFile{ 0x143962DF0_g };
	WEAK symbol<void(int code, const char* error, lua_state* s)> Lua_CoD_LuaStateManager_Error{ 0x14398A860_g };
	WEAK symbol<const char*(lua_state* luaVM, hks_object* obj, size_t* len)> hks_obj_tolstring{ 0x143755730_g };
	WEAK symbol<float(lua_state* luaVM, const hks_object* obj)> hks_obj_tonumber{ 0x143755A90_g };


#define R_AddCmdDrawText(TXT, MC, F, X, Y, XS, YS, R, C, S) \
	T8_AddBaseDrawTextCmd(TXT, MC, F, X, Y, XS, YS, R, C, S, -1, 0, 0)

#define R_AddCmdDrawTextWithCursor(TXT, MC, F, X, Y, XS, YS, R, C, S, CP, CC) \
	T8_AddBaseDrawTextCmd(TXT, MC, F, X, Y, XS, YS, R, C, S, CP, CC, 0)
	
#define Com_Error(code, fmt, ...) \
		Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)


	///////////////////////////////////////////////////////////////////////////
	//                              NAME TABLES                              //
	///////////////////////////////////////////////////////////////////////////	
	WEAK symbol<const char*> var_typename{ 0x144EED240_g }; // GSC Types
		
	WEAK symbol<const char*> builtinLabels{ 0x144F11530_g }; // Console Labels


	///////////////////////////////////////////////////////////////////////////
	//                               VARIABLES                               //
	///////////////////////////////////////////////////////////////////////////
	WEAK symbol<sharedUiInfo_t> sharedUiInfo{ 0x14F956850_g };

	WEAK symbol<cmd_function_t> cmd_functions{ 0x14F99B188_g };
	WEAK symbol<CmdArgs> sv_cmd_args{ 0x14F998070_g };

	WEAK symbol<int> keyCatchers{ 0x148A53F84_g };
	WEAK symbol<PlayerKeyState> playerKeys{ 0x148A3EF80_g };

	// Global game definitions
	constexpr auto CMD_MAX_NESTING = 8;
}