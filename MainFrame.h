#pragma once
#include "wx\wx.h"
#include <thread>
#include <unordered_map>
#include <bitset>

class MainFrame : public wxFrame
{
public:
	MainFrame();
	~MainFrame();
public:
	wxButton** buttonArray{ nullptr };

	wxBoxSizer* generalSizer{ nullptr };
	wxPanel* topPanel{ nullptr };
	wxPanel* bottomPanel{ nullptr };

	//Utility buttons
	wxButton* resetButton{ nullptr };
	wxButton* setCoordinatesButton{ nullptr };
	wxButton* generateMazeButton{ nullptr };
	wxButton* saveMazeButton{ nullptr };

	//Search algorithms buttons
	wxButton* depthFirstSearch{ nullptr };
	wxButton* breadthFirstSearch{ nullptr };
	wxButton* aStarSearch{ nullptr };

	//A secondary frame to prompt user to enter start and end locations
	class PromptFrame* promptWindow{ nullptr };

	//This boolean indicates that a search ended successfuly and the reset button was not activated yet by the user
	//Therefore, we cannot start searching again until the reset button was pressed
	bool searchStarted{ false };

	//Current searching boolean to indicate that a search is currently in progress
	bool currentlySearching{ false };

public:
	//setter for secondary window active boolean
	void SetWindowActive(bool flag);

	//setters for start and end coordinates
	bool SetStart(int start);
	bool SetEnd(int end);

	//Getters
	size_t GetWidth() const;
	size_t GetHeight() const;

	wxDECLARE_EVENT_TABLE();
private:
	static const size_t nFieldWidth{ 20 };
	static const size_t nFieldHeight{ 20 };

	bool* walls{ nullptr };

	//End and start squares set by the user
	long start{ -1 };
	long end{ -1 };

	//Store default button color
	wxColour defaultColor;

	//Check if there exist a secondary window active
	bool secondaryWindowActive{ false };

	//Check if there is a search being run
	bool searchRunning{ false };

	//thread for running a search algorithm
	std::thread searchThread;


	/*FILES*/

	//For maze generation, this variable is used to keep track of selected mazes from 
	//a binary file as they will be read sequentially and not randomly.
	int positionInFile{ 0 };

	//File record count to determine the number of records inside one file
	int fileRecordCount{ 0 };

	//A hash map to avoid duplicate boards inside file
	//Initialize this map with the boards inside the file at program start
	std::unordered_map<std::bitset<nFieldHeight * nFieldWidth>, bool> mazeExists;

private:
	/*BUTTONS FUNCTIONS*/
	void OnGridButtonClicked(wxCommandEvent& evt);
	void OnResetButtonClicked(wxCommandEvent& evt);
	void OnGenerateMazeButtonClicked(wxCommandEvent& evt);
	void OnSaveMazeButtonClicked(wxCommandEvent& evt);
	void OnSetStartAndEndButtonClicked(wxCommandEvent& evt);
	void OnDFSButtonClicked(wxCommandEvent& evt);
	void OnBFSButtonClicked(wxCommandEvent& evt);
	void OnAStarButtonClicked(wxCommandEvent& evt);

	/*ALGORITHMS*/
	void SolveBFS();
	void SolveDFS();
	void SolveAStar();

	/*INITIALIZERS*/
	void initializeGUI();
	void initializeWalls();
	void initializeFiles();

};

