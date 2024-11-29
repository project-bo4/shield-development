#include <std_include.hpp>
#include <utilities/nt.hpp>
#include "loader/component_loader.hpp"



namespace plugins
{
	std::filesystem::path plugins_dir = "project-bo4/plugins";
    namespace
    {
		using plugin_handler = void(*)();
		class plugin_info
		{
		public:
			plugin_info(const std::filesystem::path& _lib) : lib{ utilities::nt::library::load(_lib) }
			{
				if (!lib) return;

				const char* (*PBO4_GetPluginName)() = lib.get_proc<const char* (*)()>("PBO4_GetPluginName");

				if (PBO4_GetPluginName)
				{
					name = PBO4_GetPluginName();
				}
			}

			plugin_info(const plugin_info& a) : lib(a.lib), name(a.name)
			{
			}

			operator bool() const
			{
				return lib;
			}

			constexpr const char* get_name() const
			{
				return name;
			}

			void pre_start() const
			{
				plugin_handler pre_start_handler = lib.get_proc<plugin_handler>("PBO4_PreStart");
				if (pre_start_handler) pre_start_handler();
			}

			void post_unpack() const
			{
				plugin_handler post_unpack_handler = lib.get_proc<plugin_handler>("PBO4_PostUnpack");
				if (post_unpack_handler) post_unpack_handler();
			}

			void pre_destroy() const
			{
				plugin_handler pre_destroy_handler = lib.get_proc<plugin_handler>("PBO4_PreDestroy");
				if (pre_destroy_handler) pre_destroy_handler();
			}

		private:
			utilities::nt::library lib;
			const char* name{ "unknown" };
		};
		std::vector<plugin_info> loaded_plugins{};
    }

    class component final : public component_interface
    {
    public:

		void pre_start() override
		{
			std::filesystem::create_directories(plugins_dir);

			for (const std::filesystem::directory_entry& sub : std::filesystem::directory_iterator{ plugins_dir })
			{
				const std::filesystem::path& p{ sub };

				if (p.has_extension() && p.extension() != ".dll") continue;

				plugin_info plugin{ p };

				if (!plugin)
				{
					logger::write(logger::LOG_TYPE_ERROR, std::format("Can't load {}", p.string()));
					continue;
				}

				plugin.pre_start();

				logger::write(logger::LOG_TYPE_INFO, std::format("Loaded plugin '{}'", plugin.get_name()));

				loaded_plugins.emplace_back(plugin);
			}
		}

		void pre_destroy() override
		{
			for (const plugin_info& plugin : loaded_plugins)
			{
				plugin.pre_destroy();
			}
		}

		void post_unpack() override
		{
			for (const plugin_info& plugin : loaded_plugins)
			{
				plugin.post_unpack();
			}
		}

    };
}

REGISTER_COMPONENT(plugins::component)
