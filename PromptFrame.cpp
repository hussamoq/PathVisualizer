#include "PromptFrame.h"
#include "MainFrame.h"

wxBEGIN_EVENT_TABLE(PromptFrame, wxFrame)
EVT_BUTTON(5001, OnBoxButtonClicked)
wxEND_EVENT_TABLE()

PromptFrame::PromptFrame(MainFrame* parentClass)
	: wxFrame(parentClass, wxID_ANY, "Prompt input", wxPoint(500, 500), wxSize(420, 100))
{
	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(420, 100));
	panel->SetBackgroundColour(wxColour("white"));

	button = new wxButton(panel, 5001, "Set values", wxPoint(0, 30), wxSize(80, 30));

	promptText = new wxStaticText(panel, wxID_ANY, "Enter start and end square numbers seperated by space:", wxPoint(0, 0));
	promptBox = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxPoint(295, 0));

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(panel, 1, wxEXPAND | wxALL);;

	this->SetSizerAndFit(sizer);
}

PromptFrame::~PromptFrame()
{
	
}

void PromptFrame::OnBoxButtonClicked(wxCommandEvent& evt)
{
	//Extract string for processing 
	wxString str = promptBox->GetValue();

	//Make sure input is not empty
	if (str.empty())
		return;

	//Find the position of the delimeter (space) and check if there are multiple delimeters
	size_t pos = str.find(' ');

	wxString start = str.substr(0, pos);
	wxString end = str.substr(pos + 1, wxString::npos);

	//Strip the second string from trailing spaces
	end.Strip(wxString::trailing);

	//Try and find another space in the second string
	pos = end.find(' ');
	//Make sure there exists only one space
	if (pos != wxString::npos)
	    return;

	//Make sure both numbers do not contain anything other than digits
	for (int i{ 0 }; i < (start.size() > end.size() ? start.size() : end.size()); ++i)
	{
		if (i < start.size() && !std::isdigit(start[i]))
			return;

		if (i < end.size() && !std::isdigit(end[i]))
			return;
	}

	//Get the integer values of the string version of start and end points 
	long startCoordinate;
	long endCoordinate;
	start.ToLong(&startCoordinate);
	end.ToLong(&endCoordinate);

	//Store instance of parent class in a temporary vairable
	MainFrame* parentClass{ dynamic_cast<MainFrame*>(this->GetParent()) };

	//Make sure start and end are within bounds
	if (!parentClass->SetStart(startCoordinate) || !parentClass->SetEnd(endCoordinate))
		return;

	//Make sure start location does not equal end location
	if (startCoordinate == endCoordinate)
		return;	

	//Display message for ensuring the user validity 
	wxMessageBox("Start and end location set successfully.", "Success", wxICON_INFORMATION);

	//Destroy the secondary window
	this->Destroy();

	//Event already handled, so no need to send event up the hierarchy to be handled
	evt.Skip();
}

bool PromptFrame::Destroy()
{
	bool returnValue{ wxTopLevelWindowMSW::Destroy() };
	//Set value for secondary window as inactive
	dynamic_cast<MainFrame*>(this->GetParent())->SetWindowActive(false);
	return returnValue;
}