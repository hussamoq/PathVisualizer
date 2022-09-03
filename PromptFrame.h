#pragma once
#include "wx/wx.h"

class PromptFrame : public wxFrame
{
public:
	PromptFrame(class MainFrame* parentClass);
	~PromptFrame();

public:
	wxBoxSizer* sizer{ nullptr };
	wxPanel* panel{ nullptr };

	//Displays read-only infromation
	wxStaticText* promptText{ nullptr };

	//A little box for the user to enter information inside
	wxTextCtrl* promptBox{ nullptr };

	//A button to handle the information entered insid the textbox
	wxButton* button{ nullptr };

	//override destory function 
	virtual bool Destroy() override;

	wxDECLARE_EVENT_TABLE();
private:
	void OnBoxButtonClicked(wxCommandEvent& evt);
};
