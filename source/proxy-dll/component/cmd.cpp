#include <std_include.hpp>
#include "definitions/game.hpp"
#include <utilities/string.hpp>
#include "component/dvars.hpp"
#include "definitions/variables.hpp"
#include "loader/component_loader.hpp"

namespace cmd
{
    namespace
    {
        bool find_matches(const std::string& input, bool exact)
        {
            double required_ratio = exact ? 1.00 : 0.01;

            for (const auto& dvar : variables::dvars_record)
            {
                if (dvars::find_dvar(dvar.fnv1a) && utilities::string::match(input, dvar.name) >= required_ratio)
                {
                    return true;
                }
            }

            for (const auto& cmd : variables::commands_record)
            {
                if (utilities::string::match(input, cmd.name) >= required_ratio)
                {
                    return true;
                }
            }

            if (dvars::find_dvar(input))
            {
                return true;
            }

            return false;
        }

        void console_input_thread()
        {
            HANDLE console_input_handle = GetStdHandle(STD_INPUT_HANDLE);
            DWORD chars_read;
            CHAR input_buffer[256];

            while (true)
            {
                if (ReadConsole(console_input_handle, input_buffer, sizeof(input_buffer), &chars_read, nullptr))
                {
                    input_buffer[chars_read - 2] = '\0';
#ifdef DEBUG
                    logger::write(logger::LOG_TYPE_DEBUG, "[ INFO ]: User console input >>> %s", input_buffer);
#endif
                    game::Cbuf_AddText(0, utilities::string::va("%s \n", input_buffer));
                }

                Sleep(100);
            }
        }

        void start_listen_thread()
        {
            std::thread listen_thread(console_input_thread);
            listen_thread.detach();
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            start_listen_thread();
        }
    };
}

REGISTER_COMPONENT(cmd::component)