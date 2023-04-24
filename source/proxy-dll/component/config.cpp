#include <std_include.hpp>
#include "config.hpp"
#include "loader/component_loader.hpp"
#include <filesystem> 

namespace config
{
    struct defaultconfig
    {
        std::string description;
        std::string default_value;
    };

    static std::map<const std::string, const std::string> configs{};
    static std::map<const std::string, defaultconfig> default_configs{};

    void load_configs()
    {
        static bool loaded = false;

        if (loaded) {
            return;
        }
        loaded = true;

        std::ifstream stream;
        stream.open("project-bo4.cfg", std::fstream::in);

        if (stream.fail())
        {
            // no config?
            return;
        }

        std::string line;

        while (std::getline(stream, line))
        {
            if (line.empty() || line[0] == '#')
            {
                continue; // ignore comment
            }

            size_t idx = line.find('=');


            if (idx == std::string::npos)
            {
                continue; // no data
            }

            const std::string key = line.substr(0, idx);
            const std::string val = line.substr(idx + 1, line.length());

            configs.insert(std::pair(key, val));
        }

        stream.close();
    }
    void sync_configs()
    {
        if (std::filesystem::exists("project-bo4.cfg")) return; // no need to recreate it

        std::ofstream stream;
        stream.open("project-bo4.cfg", std::fstream::out);

        if (stream.fail())
        {
            logger::write(logger::LOG_TYPE_WARN, "Error while syncing config file.");
            return;
        }

        for (auto& [key, val] : configs)
        {
            auto dval = default_configs.find(key);
            if (dval != default_configs.end())
            {
                defaultconfig& d = dval->second;
                stream << "# " << d.description << ", default: " << d.default_value << '\n';
            }
            stream << key << '=' << val << "\n\n";
        }

        stream.close();
    }

    const std::string& noconfig()
    {
        static const std::string no_config = "";
        return no_config;
    }

    void register_config_value(const std::string& key, const std::string& default_value, const char* description)
    {
        load_configs();
        defaultconfig& d = default_configs[key];
        d.default_value = default_value;
        d.description = description;

        // will insert the data if required
        if (configs.find(key) == configs.end())
        {
            configs.insert(std::pair(key, default_value));
        }
    }

    const std::string& get_config_value(const std::string &cfg)
    {
        load_configs();
        if (configs.find(cfg) == configs.end())
        {
            return "";
        }
        return configs.at(cfg);
    }


    class component final : public component_interface
    {
    public:

        void post_unpack() override
        {
            sync_configs();
        }

        int priority() override
        {
            return 0;
        }
    };
}

REGISTER_COMPONENT(config::component)