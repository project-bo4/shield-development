#include "progress_ui.hpp"

#include "string.hpp"

namespace utilities
{
	progress_ui::progress_ui()
	{
		this->dialog_ = utilities::com::create_progress_dialog();
		if (!this->dialog_)
		{
			throw std::runtime_error{"Failed to create dialog"};
		}
	}

	progress_ui::~progress_ui()
	{
		this->dialog_->StopProgressDialog();
	}

	void progress_ui::show(const bool marquee, HWND parent) const
	{
		this->dialog_->StartProgressDialog(parent, nullptr, PROGDLG_AUTOTIME | (marquee ? PROGDLG_MARQUEEPROGRESS : 0), nullptr);
	}

	void progress_ui::set_progress(const size_t current, const size_t max) const
	{
		this->dialog_->SetProgress64(current, max);
	}

	void progress_ui::set_line(const int line, const std::string& text) const
	{
		this->dialog_->SetLine(line, utilities::string::convert(text).data(), false, nullptr);
	}

	void progress_ui::set_title(const std::string& title) const
	{
		this->dialog_->SetTitle(utilities::string::convert(title).data());
	}

	bool progress_ui::is_cancelled() const
	{
		return this->dialog_->HasUserCancelled();
	}
}
