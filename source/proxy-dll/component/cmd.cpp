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

            if ( dvars::find_dvar(input))
            {
                return true;
            }

            return false;
        }

        void ConsoleInputThread() 
        {
            HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
            DWORD charsRead;
            CHAR inputBuffer[256];

            while (true) 
            {
                ReadConsole(hConsoleInput, inputBuffer, sizeof(inputBuffer), &charsRead, nullptr);
                inputBuffer[charsRead - 2] = '\0';
#ifdef DEBUG
                logger::write(logger::LOG_TYPE_DEBUG, "[ INFO ]: User console input >>> %s", inputBuffer);
#endif
                std::istringstream iss(inputBuffer);
                std::string dvar;
                iss >> dvar;

                if (find_matches(dvar, true))
                {
#ifdef DEBUG
                    logger::write(logger::LOG_TYPE_DEBUG, "[ INFO ]: Dvar / Command found. %s", inputBuffer);
#endif
                    game::Cbuf_AddText(0, utilities::string::va("%s \n", inputBuffer));
                }
                else
                {
#ifdef DEBUG
                    logger::write(logger::LOG_TYPE_DEBUG, "[ WARNING ]: Dvar / Command not found. %s", inputBuffer);
#endif
                }

                Sleep(100);
            }
        }

        void start_listen_thread()
        {
            std::thread listen_thread(ConsoleInputThread);
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