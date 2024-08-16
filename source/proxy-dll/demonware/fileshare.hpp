#pragma once
#include "data_types.hpp"

namespace demonware::fileshare
{
    const char* get_fileshare_host_name();

    enum fileshareCategory_e : uint16_t
    {
        FILESHARE_CATEGORY_ALL = 0,
        FILESHARE_CATEGORY_CONTENT_SERVER_START = 1,
        FILESHARE_CATEGORY_EMBLEM = 1,
        FILESHARE_CATEGORY_PAINTJOB = 2,
        FILESHARE_CATEGORY_VARIANT = 3,
        FILESHARE_CATEGORY_DECAL = 4,
        FILESHARE_CATEGORY_GUNRENDER = 5,
        FILESHARE_CATEGORY_CLIP = 6,
        FILESHARE_CATEGORY_EMBLEMIMAGE = 7,
        FILESHARE_CATEGORY_FILM = 10,
        FILESHARE_CATEGORY_SCREENSHOT = 11,
        FILESHARE_CATEGORY_CUSTOM_GAME_MODE = 12,
        FILESHARE_CATEGORY_CONTENT_SERVER_END = 12,
        FILESHARE_CATEGORY_CLIP_PRIVATE = 14,
        FILESHARE_CATEGORY_SCREENSHOT_PRIVATE = 15,
        FILESHARE_CATEGORY_CUSTOM_GAME_MODE_PRIVATE = 16,
        FILESHARE_CATEGORY_INGAMESTORE_START = 100,
        FILESHARE_CATEGORY_INGAMESTORE_MAPPACKS = 100,
        FILESHARE_CATEGORY_INGAMESTORE_THEMES = 101,
        FILESHARE_CATEGORY_INGAMESTORE_AVATARS = 102,
        FILESHARE_CATEGORY_INGAMESTORE_WEAPONPACKS = 103,
        FILESHARE_CATEGORY_INGAMESTORE_CALLINGCARDPACKS = 104,
        FILESHARE_CATEGORY_INGAMESTORE_STORAGEPACKS = 105,
        FILESHARE_CATEGORY_INGAMESTORE_END = 105,
        FILESHARE_CATEGORY_MOTD_IMAGES = 120,
        FILESHARE_CATEGORY_VOTE_IMAGES = 130,
        FILESHARE_CATEGORY_MDLC = 1000,
        FILESHARE_CATEGORY_MDLC_DEDICATED = 1001,
        FILESHARE_CATEGORY_MDLC_LAST = 1999,
        FILESHARE_CATEGORY_AVI = 32768,
        FILESHARE_CATEGORY_EXEMONITOR = 32769,
        FILESHARE_CATEGORY_INVALID = 0xFFFF
    };

    const char* get_category_extension(fileshareCategory_e cat);
    fileshareCategory_e get_extension_category(const char* ext);


    std::string get_fileshare_directory();
    std::string get_file_name(const uint64_t fileID, 
        fileshareCategory_e category = FILESHARE_CATEGORY_INVALID);
    std::string get_file_url(const std::string& file);
    std::string get_file_path(const std::string& file);
    std::string get_metadata_path(const std::string& file);


    class FileMetadata final
    {
    public:
        FileMetadata() = default;
        ~FileMetadata() = default;

        enum file_state
        {
            FILE_STATE_UNKNOWN = 0,
            FILE_STATE_INVALID = -1,
            FILE_STATE_UPLOADING = 1,
            FILE_STATE_UPLOADED = 2,
            FILE_STATE_DESCRIBED = 3
        } state{};

        struct file_info
        {
            uint64_t id;
            std::string name;
            uint32_t size;
            uint32_t timestamp;
        } file{};

        struct author_info
        {
            uint64_t xuid;
            std::string name;
        } author{};

        fileshareCategory_e category = FILESHARE_CATEGORY_ALL;
        std::string ioFileName = "";
        uint32_t ioFileSize = 0;
        std::string ddlMetadata = "";
        std::map<uint64_t, uint64_t> tags;

        bool MetadataTaskResult(bdFileMetaData* output, bool download);

        bool ReadMetaDataJson(const std::string& path, file_state expect = FILE_STATE_UNKNOWN);
        bool WriteMetaDataJson(const std::string& path, file_state _state = FILE_STATE_UNKNOWN);

    private:
        std::string SerializeMetaJSON();
        bool ParseMetaJSON(const std::string& input);
    };

    std::vector<uint64_t> fileshare_list_demo_ids();
}
