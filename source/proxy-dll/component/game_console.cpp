#include <std_include.hpp>
#include "game_console.hpp"
#include "definitions/game.hpp"
#include "definitions/variables.hpp"
#include "loader/component_loader.hpp"

#include "component/dvars.hpp"
#include "component/scheduler.hpp"
#include "component/keycatchers.hpp"

#include <utilities/hook.hpp>
#include <utilities/string.hpp>
#include <utilities/concurrency.hpp>

#define R_DrawTextFont reinterpret_cast<void*>(game::sharedUiInfo->assets.bigFont)
#define R_WhiteMaterial reinterpret_cast<void*>(game::sharedUiInfo->assets.whiteMaterial)

namespace game_console
{
	namespace
	{
		game::vec4_t con_inputBoxColor = { 0.1f, 0.1f, 0.1f, 0.9f };
		game::vec4_t con_inputHintBoxColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		game::vec4_t con_outputBarColor = { 0.8f, 0.8f, 0.8f, 0.6f };
		game::vec4_t con_outputSliderColor = { 0.8f, 0.8f, 0.8f, 1.0f };
		game::vec4_t con_outputWindowColor = { 0.15f, 0.15f, 0.15f, 0.85f };
		game::vec4_t con_inputWriteDownColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::vec4_t con_inputDvarMatchColor = { 0.1f, 0.8f, 0.8f, 1.0f };
		game::vec4_t con_inputDvarInactiveValueColor = { 0.4f, 0.8f, 0.7f, 1.0f };
		game::vec4_t con_inputCmdMatchColor = { 0.9f, 0.6f, 0.2f, 1.0f };
		game::vec4_t con_inputDescriptionColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::vec4_t con_inputAltDescriptionColor = { 0.9f, 0.6f, 0.2f, 1.0f };
		game::vec4_t con_inputExtraInfoColor = { 1.0f, 0.5f, 0.5f, 1.0f };
		game::vec4_t con_outputVersionStringColor = { 0.92f, 1.0f, 0.65f, 1.0f };

		using suggestion_t = variables::varEntry;
		using output_queue = std::deque<std::string>;

		struct ingame_console
		{
			char buffer[256]{};
			int cursor{};
			float font_scale{};
			float font_height{};
			int max_suggestions{};
			int visible_line_count{};
			float screen_min[2]{};
			float screen_max[2]{};
			struct {
				float x{}, y{};
			} screen_pointer;
			bool may_auto_complete{};
			char auto_complete_choice[64]{};
			bool output_visible{};
			int display_line_offset{};
			int total_line_count{};
			utilities::concurrency::container<output_queue, std::recursive_mutex> output{};
		};

		ingame_console con{};

		std::int32_t history_index = -1;
		std::deque<std::string> history{};

		std::string fixed_input{};
		std::vector<suggestion_t> matches{};

		void clear_input()
		{
			strncpy_s(con.buffer, "", sizeof(con.buffer));
			con.cursor = 0;

			fixed_input = "";
			matches.clear();
		}

		void clear_output()
		{
			con.total_line_count = 0;
			con.display_line_offset = 0;
			con.output.access([](output_queue& output)
				{
					output.clear();
				});
			history_index = -1;
			history.clear();
		}

		void print_internal(const std::string& data)
		{
			con.output.access([&](output_queue& output)
				{
					if (con.visible_line_count > 0
						&& con.display_line_offset == (output.size() - con.visible_line_count))
					{
						con.display_line_offset++;
					}
					output.push_back(data);
					if (output.size() > 512)
					{
						output.pop_front();
					}
				});
		}

		void toggle_console()
		{
			clear_input();

			con.output_visible = false;
			*game::keyCatchers ^= 1;
		}

		void toggle_console_output()
		{
			con.output_visible = con.output_visible == 0;
		}

		bool is_renderer_ready()
		{
			return (R_DrawTextFont && R_WhiteMaterial);
		}

		void calculate_window_size()
		{
			con.screen_min[0] = 6.0f;
			con.screen_min[1] = 6.0f;
			con.screen_max[0] = game::ScrPlace_GetView(0)->realViewportSize[0] - 6.0f;
			con.screen_max[1] = game::ScrPlace_GetView(0)->realViewportSize[1] - 6.0f;

			con.font_height = static_cast<float>(game::UI_TextHeight(R_DrawTextFont, con.font_scale));
			con.visible_line_count = static_cast<int>(((con.screen_max[1] - con.screen_min[1] - (con.font_height * 2)) - 24.0f) / con.font_height);
		}

		void draw_box(const float x, const float y, const float w, const float h, float* color)
		{
			game::vec4_t outline_color;

			outline_color[0] = color[0] * 0.5f;
			outline_color[1] = color[1] * 0.5f;
			outline_color[2] = color[2] * 0.5f;
			outline_color[3] = color[3];

			game::R_AddCmdDrawStretchPic(x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color, R_WhiteMaterial);
			game::R_AddCmdDrawStretchPic(x, y, 2.0f, h, 0.0f, 0.0f, 0.0f, 0.0f, outline_color, R_WhiteMaterial);
			game::R_AddCmdDrawStretchPic((x + w) - 2.0f, y, 2.0f, h, 0.0f, 0.0f, 0.0f, 0.0f, outline_color, R_WhiteMaterial);
			game::R_AddCmdDrawStretchPic(x, y, w, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, outline_color, R_WhiteMaterial);
			game::R_AddCmdDrawStretchPic(x, (y + h) - 2.0f, w, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, outline_color, R_WhiteMaterial);
		}

		void draw_input_box(const int lines, float* color)
		{
			draw_box( con.screen_pointer.x - 6.0f, con.screen_pointer.y - 6.0f,
				(con.screen_max[0] - con.screen_min[0]) - ((con.screen_pointer.x - 6.0f) - con.screen_min[0]),
				(lines * con.font_height) + 12.0f, color);
		}

		void draw_input_text_and_over(const char* str, float* color)
		{
			game::R_AddCmdDrawText(str, 0x7FFFFFFF, R_DrawTextFont, con.screen_pointer.x,
				con.screen_pointer.y + con.font_height, con.font_scale, con.font_scale, 0.0f, color, 0);

			con.screen_pointer.x = game::UI_TextWidth(0, str, 0x7FFFFFFF, R_DrawTextFont, con.font_scale) + con.screen_pointer.x + 6.0f;
		}

		float draw_hint_box(const int lines, float* color, [[maybe_unused]] float offset_x = 0.0f,
			[[maybe_unused]] float offset_y = 0.0f)
		{
			const auto _h = lines * con.font_height + 12.0f;
			const auto _y = con.screen_pointer.y - 3.0f + con.font_height + 12.0f + offset_y;
			const auto _w = (con.screen_max[0] - con.screen_min[0]) - ((con.screen_pointer.x - 6.0f) - con.screen_min[0]);

			draw_box(con.screen_pointer.x - 6.0f, _y, _w, _h, color);
			return _h;
		}

		void draw_hint_text(const int line, const char* text, float* color, const float offset_x = 0.0f, const float offset_y = 0.0f)
		{
			const auto _y = con.font_height + con.screen_pointer.y + (con.font_height * (line + 1)) + 15.0f + offset_y;

			game::R_AddCmdDrawText(text, 0x7FFFFFFF, R_DrawTextFont, con.screen_pointer.x + offset_x, _y, con.font_scale, con.font_scale, 0.0f, color, 0);
		}

		void find_matches(const std::string& input, std::vector<suggestion_t>& suggestions, bool exact)
		{
			double required_ratio = exact ? 1.00 : 0.01;

			for (const auto& dvar : variables::dvars_table)
			{
				if (dvars::find_dvar(dvar.fnv1a) && utilities::string::match(input, dvar.name) >= required_ratio)
				{
					suggestions.push_back(dvar);
				}

				if (exact && suggestions.size() > 1)
				{
					return;
				}
			}

			if (suggestions.size() == 0 && dvars::find_dvar(input))
			{
				suggestions.push_back({ input, "", fnv1a::generate_hash(input.data()), reinterpret_cast<uintptr_t>(dvars::find_dvar(input)) });
			}

			for (const auto& cmd : variables::commands_table)
			{
				if (utilities::string::match(input, cmd.name) >= required_ratio)
				{
					suggestions.push_back(cmd);
				}

				if (exact && suggestions.size() > 1)
				{
					return;
				}
			}
		}

		void draw_input()
		{
			con.screen_pointer.x = con.screen_min[0] + 6.0f;
			con.screen_pointer.y = con.screen_min[1] + 6.0f;

			draw_input_box(1, con_inputBoxColor);
			draw_input_text_and_over("PROJECT-BO4 >", con_inputWriteDownColor);

			con.auto_complete_choice[0] = 0;

			game::R_AddCmdDrawTextWithCursor(con.buffer, 0x7FFFFFFF, R_DrawTextFont, con.screen_pointer.x, con.screen_pointer.y + con.font_height, con.font_scale, con.font_scale, 0, con_inputWriteDownColor, 0, con.cursor, '|');

			// check if using a prefixed '/' or not
			const auto input = con.buffer[1] && (con.buffer[0] == '/' || con.buffer[0] == '\\')
				? std::string(con.buffer).substr(1) : std::string(con.buffer);

			if (!input.length())
			{
				return;
			}

			if (input != fixed_input)
			{
				matches.clear();

				if (input.find(" ") != std::string::npos)
				{
					find_matches(input.substr(0, input.find(" ")), matches, true);
				}
				else
				{
					find_matches(input, matches, false);

					if (matches.size() <= con.max_suggestions)
					{
						std::sort(matches.begin(), matches.end(), [&input](suggestion_t& lhs, suggestion_t& rhs) {
								return utilities::string::match(input, lhs.name) > utilities::string::match(input, rhs.name);
							});
					}

				}

				fixed_input = input;
			}

			con.may_auto_complete = false;

			if (matches.size() > con.max_suggestions)
			{
				draw_hint_box(1, con_inputHintBoxColor);
				draw_hint_text(0, utilities::string::va("%i matches (too many to show here)", matches.size()), con_inputDvarMatchColor);
			}
			else if (matches.size() == 1)
			{
				auto* dvar = dvars::find_dvar(matches[0].fnv1a);
				auto line_count = dvar ? 3 : 1;

				auto height = draw_hint_box(line_count, con_inputHintBoxColor);
				draw_hint_text(0, matches[0].name.data(), dvar ? con_inputDvarMatchColor : con_inputCmdMatchColor);

				if (dvar)
				{
					auto offset_x = (con.screen_max[0] - con.screen_pointer.x) / 4.f;

					draw_hint_text(0, dvars::get_value_string(dvar, &dvar->value->current).data(), con_inputDvarMatchColor, offset_x);
					draw_hint_text(1, "  default", con_inputDvarInactiveValueColor);
					draw_hint_text(1, dvars::get_value_string(dvar, &dvar->value->reset).data(), con_inputDvarInactiveValueColor, offset_x);
					draw_hint_text(2, matches[0].desc, con_inputDescriptionColor, 0);

					auto offset_y = height + 3.f;
					auto domain_lines = 1;
					if (dvar->type == game::DVAR_TYPE_ENUM) 
						domain_lines = dvar->domain.enumeration.stringCount + 1;

					draw_hint_box(domain_lines, con_inputHintBoxColor, 0, offset_y);
					draw_hint_text(0, dvars::get_domain_string(dvar->type, dvar->domain).data(), con_inputAltDescriptionColor, 0, offset_y);
				}
				else
				{
					auto offset_x = (con.screen_max[0] - con.screen_pointer.x) / 4.f;

					draw_hint_text(0, matches[0].desc, con_inputCmdMatchColor, offset_x);
				}

				strncpy_s(con.auto_complete_choice, matches[0].name.data(), 64);
				con.may_auto_complete = true;
			}
			else if (matches.size() > 1)
			{
				draw_hint_box(static_cast<int>(matches.size()), con_inputHintBoxColor);

				auto offset_x = (con.screen_max[0] - con.screen_pointer.x) / 4.f;

				for (size_t i = 0; i < matches.size(); i++)
				{
					auto* const dvar = dvars::find_dvar(matches[i].fnv1a);

					draw_hint_text(static_cast<int>(i), matches[i].name.data(), dvar ? con_inputDvarMatchColor : con_inputCmdMatchColor);
					draw_hint_text(static_cast<int>(i), matches[i].desc, dvar ? con_inputDvarMatchColor : con_inputCmdMatchColor, offset_x * 1.5f);

					if (dvar)
					{
						draw_hint_text(static_cast<int>(i), dvars::get_value_string(dvar, &dvar->value->current).data(), con_inputDvarMatchColor, offset_x);
					}
				}

				strncpy_s(con.auto_complete_choice, matches[0].name.data(), 64);
				con.may_auto_complete = true;
			}
		}

		void draw_output_scrollbar(const float x, float y, const float width, const float height, output_queue& output)
		{
			auto _x = (x + width) - 10.0f;
			draw_box(_x, y, 10.0f, height, con_outputBarColor);

			auto _height = height;
			if (output.size() > con.visible_line_count)
			{
				auto percentage = static_cast<float>(con.visible_line_count) / output.size();
				_height *= percentage;

				auto remainingSpace = height - _height;
				auto percentageAbove = static_cast<float>(con.display_line_offset) / (output.size() - con.visible_line_count);

				y = y + (remainingSpace * percentageAbove);
			}

			draw_box(_x, y, 10.0f, _height, con_outputSliderColor);
		}

		void draw_output_text(const float x, float y, output_queue& output)
		{
			auto offset = output.size() >= con.visible_line_count ? 0.0f : (con.font_height * (con.visible_line_count - output.size()));

			for (auto i = 0; i < con.visible_line_count; i++)
			{
				auto index = i + con.display_line_offset;
				if (index >= output.size())
				{
					break;
				}

				game::R_AddCmdDrawText(output.at(index).data(), 0x400, R_DrawTextFont, x, y + offset + ((i + 1) * con.font_height), con.font_scale, con.font_scale, 0.0f, con_inputWriteDownColor, 0);
			}
		}

		void draw_output_window()
		{
			con.output.access([](output_queue& output)
				{
					draw_box(con.screen_min[0], con.screen_min[1] + 32.0f, con.screen_max[0] - con.screen_min[0], (con.screen_max[1] - con.screen_min[1]) - 32.0f, con_outputWindowColor);

					auto x = con.screen_min[0] + 6.0f;
					auto y = (con.screen_min[1] + 32.0f) + 6.0f;
					auto width = (con.screen_max[0] - con.screen_min[0]) - 12.0f;
					auto height = ((con.screen_max[1] - con.screen_min[1]) - 32.0f) - 12.0f;

					char sysinfo_version[256];
					utilities::hook::invoke<bool>(game::Live_SystemInfo, 0, 0, sysinfo_version, 256);
					char sysinfo_livebits[256];
					utilities::hook::invoke<bool>(game::Live_SystemInfo, 0, 1, sysinfo_livebits, 256);
					const char* info = utilities::string::va("Project-BO4 1.0.0, Engine Version: %s, LiveBits: %s", sysinfo_version, sysinfo_livebits);

					game::R_AddCmdDrawText(info, 0x7FFFFFFF, R_DrawTextFont, x, ((height - 16.0f) + y) + con.font_height, con.font_scale, con.font_scale, 0.0f, con_outputVersionStringColor, 0);

					draw_output_scrollbar(x, y, width, height, output);
					draw_output_text(x, y, output);
				});
		}

		void draw_console()
		{
			if (!is_renderer_ready()) return;

			calculate_window_size();

			if (*game::keyCatchers & 1)
			{
				if (!(*game::keyCatchers & 1))
				{
					con.output_visible = false;
				}

				if (con.output_visible)
				{
					draw_output_window();
				}

				draw_input();
			}
		}
	}

	void print(const char* fmt, ...)
	{
		char va_buffer[0x200] = { 0 };

		va_list ap;
		va_start(ap, fmt);
		vsprintf_s(va_buffer, fmt, ap);
		va_end(ap);

		const auto formatted = std::string(va_buffer);
		const auto lines = utilities::string::split(formatted, '\n');

		for (const auto& line : lines)
		{
			print_internal(line);
		}
	}

	void print(const std::string& data)
	{
		const auto lines = utilities::string::split(data, '\n');
		for (const auto& line : lines)
		{
			print_internal(line);
		}
	}

	bool console_char_event(const int local_client_num, const int key, const bool is_repeated)
	{
		if (key == game::keyNum_t::K_GRAVE ||
			key == game::keyNum_t::K_TILDE ||
			key == '|' ||
			key == '\\')
		{
			return false;
		}

		if (key > 127)
		{
			return true;
		}

		if (*game::keyCatchers & 1)
		{
			if (key == game::keyNum_t::K_TAB) // tab (auto complete) 
			{
				if (con.may_auto_complete)
				{
					const auto first_char = con.buffer[0];

					clear_input();

					if (first_char == '\\' || first_char == '/')
					{
						con.buffer[0] = first_char;
						con.buffer[1] = '\0';
					}

					strncat_s(con.buffer, con.auto_complete_choice, 64);
					con.cursor = static_cast<int>(std::string(con.buffer).length());

					if (con.cursor != 254)
					{
						con.buffer[con.cursor++] = ' ';
						con.buffer[con.cursor] = '\0';
					}
				}
			}

			if (key == 'v' - 'a' + 1) // paste
			{
				const auto clipboard = utilities::string::get_clipboard_data();
				if (clipboard.empty())
				{
					return false;
				}

				for (size_t i = 0; i < clipboard.length(); i++)
				{
					console_char_event(local_client_num, clipboard[i], false);
				}

				return false;
			}

			if (key == 'c' - 'a' + 1) // clear
			{
				clear_input();
				con.total_line_count = 0;
				con.display_line_offset = 0;
				con.output.access([](output_queue& output)
					{
						output.clear();
					});
				history_index = -1;
				history.clear();

				return false;
			}

			if (key == 'h' - 'a' + 1) // backspace
			{
				if (con.cursor > 0)
				{
					memmove(con.buffer + con.cursor - 1, con.buffer + con.cursor,
						strlen(con.buffer) + 1 - con.cursor);
					con.cursor--;
				}

				return false;
			}

			if (key < 32)
			{
				return false;
			}

			if (con.cursor == 256 - 1)
			{
				return false;
			}

			memmove(con.buffer + con.cursor + 1, con.buffer + con.cursor, strlen(con.buffer) + 1 - con.cursor);
			con.buffer[con.cursor] = static_cast<char>(key);
			con.cursor++;

			if (con.cursor == strlen(con.buffer) + 1)
			{
				con.buffer[con.cursor] = 0;
			}
		}

		return true;
	}

	bool console_key_event(const int local_client_num, const int key, const bool down, const unsigned int time)
	{
		if (key == game::keyNum_t::K_GRAVE || key == game::keyNum_t::K_TILDE)
		{
			if (!down)
			{
				return false;
			}

			const auto shift_down = game::playerKeys[local_client_num].keys[game::keyNum_t::K_LSHIFT].down;
			if (shift_down)
			{
				if (!(*game::keyCatchers & 1))
				{
					toggle_console();
				}

				toggle_console_output();
				return false;
			}

			toggle_console();

			return false;
		}

		if (*game::keyCatchers & 1)
		{
			if (down)
			{
				if (key == game::keyNum_t::K_UPARROW)
				{
					if (++history_index >= history.size())
					{
						history_index = static_cast<int>(history.size()) - 1;
					}

					clear_input();

					if (history_index != -1)
					{
						strncpy_s(con.buffer, history.at(history_index).c_str(), 0x100);
						con.cursor = static_cast<int>(strlen(con.buffer));
					}
				}
				else if (key == game::keyNum_t::K_DOWNARROW)
				{
					if (--history_index < -1)
					{
						history_index = -1;
					}

					clear_input();

					if (history_index != -1)
					{
						strncpy_s(con.buffer, history.at(history_index).c_str(), 0x100);
						con.cursor = static_cast<int>(strlen(con.buffer));
					}
				}

				if (key == game::keyNum_t::K_RIGHTARROW)
				{
					if (con.cursor < strlen(con.buffer))
					{
						con.cursor++;
					}

					return false;
				}

				if (key == game::keyNum_t::K_LEFTARROW)
				{
					if (con.cursor > 0)
					{
						con.cursor--;
					}

					return false;
				}

				//scroll through output
				if (key == game::keyNum_t::K_MWHEELUP || key == game::keyNum_t::K_PGUP)
				{
					con.output.access([](output_queue& output)
						{
							if (output.size() > con.visible_line_count && con.display_line_offset > 0)
							{
								con.display_line_offset--;
							}
						});
				}
				else if (key == game::keyNum_t::K_MWHEELDOWN || key == game::keyNum_t::K_PGDN)
				{
					con.output.access([](output_queue& output)
						{
							if (output.size() > con.visible_line_count
								&& con.display_line_offset < (output.size() - con.visible_line_count))
							{
								con.display_line_offset++;
							}
						});
				}

				if (key == game::keyNum_t::K_ENTER && !utilities::string::is_truely_empty(con.buffer))
				{
					if (history_index != -1)
					{
						const auto itr = history.begin() + history_index;

						if (*itr == con.buffer)
						{
							history.erase(history.begin() + history_index);
						}
					}

					history.push_front(con.buffer);

					print("]%s\n", con.buffer);
					game::Cbuf_AddText(0, utilities::string::va("%s \n", fixed_input.data()));

					if (history.size() > 10)
					{
						history.erase(history.begin() + 10);
					}

					history_index = -1;

					clear_input();
				}
			}
		}

		return true;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scheduler::loop(draw_console, scheduler::renderer);

			keycatchers::add_key_event(console_key_event);
			keycatchers::add_char_event(console_char_event);

			// initialize our structs
			con.cursor = 0;
			con.visible_line_count = 0;
			con.output_visible = false;
			con.display_line_offset = 0;
			con.total_line_count = 0;
			strncpy_s(con.buffer, "", 256);

			con.screen_pointer.x = 0.0f;
			con.screen_pointer.y = 0.0f;
			con.may_auto_complete = false;
			strncpy_s(con.auto_complete_choice, "", 64);

			con.font_scale = 0.38f;
			con.max_suggestions = 24;
		}
	};
}

REGISTER_COMPONENT(game_console::component) 