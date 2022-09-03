#include "MainFrame.h"
#include "PromptFrame.h"
#include "Node.h"
#include "AStarNode.h"
#include <map>
#include <stack>
#include <queue>
#include <string>
#include <chrono>
#include <queue>
#include <fstream>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(11000, OnResetButtonClicked)
	EVT_BUTTON(11001, OnSetStartAndEndButtonClicked)
	EVT_BUTTON(11002, OnDFSButtonClicked)
	EVT_BUTTON(11003, OnBFSButtonClicked)
	EVT_BUTTON(11004, OnAStarButtonClicked)
	EVT_BUTTON(11005, OnGenerateMazeButtonClicked)
	EVT_BUTTON(11006, OnSaveMazeButtonClicked)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
	: wxFrame(nullptr, wxID_ANY, "Visualizing Search Algorithms", wxDefaultPosition, wxSize(1500, 1500))
{
	initializeGUI();
	initializeWalls();
	initializeFiles();
}

MainFrame::~MainFrame()
{
	delete[] buttonArray;
	delete[] walls;

	if(searchThread.joinable())
	    searchThread.join();
}

void MainFrame::OnGridButtonClicked(wxCommandEvent& evt)
{
	//get coordinates of the pressed button
	int x = (evt.GetId() - 10000) % nFieldWidth;
	int y = (evt.GetId() - 10000) / nFieldWidth;

	//Make button unclickable 
	buttonArray[y * nFieldWidth + x]->Enable(false);

	//Give button unique color for wall identification
	buttonArray[y * nFieldWidth + x]->SetBackgroundColour("black");

	//Make wall value true to indicate that there is a wall in this square
	walls[y * nFieldWidth + x] = true;

	evt.Skip();
}

void MainFrame::OnResetButtonClicked(wxCommandEvent& evt)
{
	using namespace std::chrono_literals;
	if (currentlySearching)
	{
		wxMessageBox("Please wait until the search completes.", "Reset Failure", wxICON_EXCLAMATION);
		return;
	}

	//Set search started to false indicating that another search can now begin
	searchStarted = false;

	//Simply enable all buttons
	for (int x{ 0 }; x < nFieldWidth; ++x)
	{
		for (int y{ 0 }; y < nFieldHeight; ++y)
		{
			buttonArray[y * nFieldWidth + x]->Enable(true);
			buttonArray[y * nFieldWidth + x]->SetBackgroundColour(defaultColor);
		}
	}

	//Reset walls back to false
	for (int y{ 0 }; y < nFieldHeight; ++y)
	{
		for (int x{ 0 }; x < nFieldWidth; ++x)
		{
			walls[y * nFieldWidth + x] = false;
		}
	}

	//Check if the start and end locations were set prior to reset
	if (start != -1)
	{
		//Set location back to its original number
		buttonArray[start]->SetLabel(std::to_string(start));
		//Set original background color
		buttonArray[start]->SetBackgroundColour(defaultColor);
		//Enable button for future input
		buttonArray[start]->Enable(true);
		start = -1;
	}

	if (end != -1)
	{
		buttonArray[end]->SetLabel(std::to_string(end));
		buttonArray[end]->SetBackgroundColour(defaultColor);
		buttonArray[end]->Enable(true);
		end = -1;
	}

	if(searchThread.joinable())
	    searchThread.join();

	evt.Skip();
}

void MainFrame::OnSetStartAndEndButtonClicked(wxCommandEvent& evt)
{
	//Make sure only one secondary window active at one time 
	if (secondaryWindowActive)
		return;

	//Check if start and end locations were already set
	if (start != -1 || end != -1)
	{
		wxMessageBox("Please reset before attempting to enter new locations", "Set Error", wxICON_ERROR);
		return;
	}

	//Create a new instance of the secondary window and show it 
	promptWindow = new PromptFrame(this);
	promptWindow->Show();

	//Set secondary window to be active so no other secondary window can be created
	secondaryWindowActive = true;

	evt.Skip();
}

void MainFrame::OnGenerateMazeButtonClicked(wxCommandEvent& evt)
{
	//Attempt to open file if exists
	std::ifstream inputFile{ "mazes.bin", std::ios::in | std::ios::binary };
	//If file does not open, there is a high probability it does not exist... so create a new one
	if (!inputFile)
	{
		//This indicates that there are no files with that name to open
		wxMessageBox("No saved mazes.", "File Error", wxICON_ERROR);
		return;
	}

	//Get next walls array
	inputFile.seekg(positionInFile * (GetWidth() * GetHeight()));

	//Load walls into the walls array
	inputFile.read(reinterpret_cast<char*>(walls), GetWidth() * GetHeight());

	//change button color to black corresponding to walls array
	for (int y{ 0 }; y < GetHeight(); ++y)
	{
		for (int x{ 0 }; x < GetWidth(); ++x)
		{
			if (walls[y * GetHeight() + x])
			{
				buttonArray[y * GetHeight() + x]->SetBackgroundColour("black");
				buttonArray[y * GetHeight() + x]->Enable(false);
			}
			else
			{
				buttonArray[y * GetHeight() + x]->SetBackgroundColour("white");
				buttonArray[y * GetHeight() + x]->Enable(true);
			}
				
		}
	}

	if (++positionInFile >= fileRecordCount)
	{
		positionInFile = 0;
	}

	inputFile.close();
}

void MainFrame::OnSaveMazeButtonClicked(wxCommandEvent& evt)
{
	//Make sure maze does not already exist inside the file
	std::bitset<nFieldWidth* nFieldHeight> tempBitset;
	
	for (int i{ 0 }; i < nFieldHeight * nFieldWidth; ++i)
	{
		if (walls[i])
		{
			tempBitset[i] = true;
		}
	}

	if (mazeExists[tempBitset])
	{
		wxMessageBox("Maze already exists inside file!", "File error", wxICON_ERROR);
		return;
	}

	//incase it did not exist, save it as existent inside the hash map
	mazeExists[tempBitset] = true;

	//Attempt to open file, if it does not exist then create it
	std::ofstream outputFile{ "mazes.bin", std::ios::out | std::ios::binary | std::ios::app };
	if (!outputFile)
	{
		wxMessageBox("Error creating or finding file.", "File Error", wxICON_ERROR);
		return;
	}

	//Point write position at the end of the file
	outputFile.seekp(0, std::ios::end);

	//Save the current grid with the size of the grid
	outputFile.write(reinterpret_cast<const char*>(walls), GetWidth() * GetHeight());	
	

	//Update the file record count variable inside the text file 
	++fileRecordCount;
	std::ofstream outputRecordsCountFile{ "recordsCount.txt", std::ios::out | std::ios::trunc };
	outputRecordsCountFile.write(std::to_string(fileRecordCount).c_str(), std::to_string(fileRecordCount).size());
	outputRecordsCountFile.close();

	//Output a success message
	wxMessageBox("Maze saved success.", "Success!", wxICON_INFORMATION);

	outputFile.close();
}

void MainFrame::OnBFSButtonClicked(wxCommandEvent& evt)
{
	if (start == -1 || end == -1)
	{
		wxMessageBox("Please set start and end locations", "Set Error", wxICON_ERROR);
		return;
	}

	if (searchStarted)
	{
		wxMessageBox("Please reset grid before trying to search again.", "Search error", wxICON_ERROR);
		return;
	}

	searchStarted = true;
	searchThread = std::thread([this]() { SolveBFS(); });

	evt.Skip();
}

void MainFrame::OnDFSButtonClicked(wxCommandEvent& evt)
{
	if (start == -1 || end == -1)
	{
		wxMessageBox("Please set start and end locations", "Set Error", wxICON_ERROR);
		return;
	}

	if (searchStarted)
	{
		wxMessageBox("Please reset grid before trying to search again.", "Search error", wxICON_ERROR);
		return;
	}

	searchStarted = true;
	searchThread = std::thread([this]() { SolveDFS(); });

	evt.Skip();
}

void MainFrame::OnAStarButtonClicked(wxCommandEvent& evt)
{
	if (start == -1 || end == -1)
	{
		wxMessageBox("Please set start and end locations", "Set Error", wxICON_ERROR);
		return;
	}

	if (searchStarted)
	{
		wxMessageBox("Please reset grid before trying to search again.", "Search error", wxICON_ERROR);
		return;
	}
	
	searchThread = std::thread([this]() { SolveAStar(); });

	evt.Skip();
}

void MainFrame::SolveDFS()
{
	//Used with timings
	using namespace std::chrono_literals;

	//Use a stack to find a path using DFS
	std::stack<Node*> s;

	//keeps track of nodes allocated to the heap
	std::vector<Node*> nodes;

	//Set searching boolean to true to indicate that a search is in progress
	currentlySearching = true;

	//Use a hashmap to store visited cells
	std::map<std::pair<int, int>, bool> visited;
	
	int x = start % nFieldWidth;
	int y = start / nFieldWidth;
	Node *startNode = new Node{ x, y };
	
	//Push start node into stack and start searching for nearby nodes
	s.push(startNode);
	nodes.push_back(startNode);
	visited[{ startNode->x, startNode->y }] = true;

	//Keep looping while the stack has at least one element inside it and the stop searching flag is set to false
	while (!s.empty())
	{
		Node *tempNode = s.top();
		s.pop();
		buttonArray[tempNode->y * nFieldWidth + tempNode->x]->SetBackgroundColour("yellow");
		//Delay for better visualization
		std::this_thread::sleep_for(100ms);

		if ((tempNode->y * nFieldWidth + tempNode->x) == end)
		{
			buttonArray[end]->SetBackgroundColour("green");
			Node *showPath = tempNode->parent;
			//Draw path from end node to start node
			while (showPath != nullptr)
			{
				//Every node has one parent, so if we reach the end node and walk through the parents 
				//of each previous node we will eventually reach the start node
				buttonArray[showPath->y * nFieldWidth + showPath->x]->SetBackgroundColour("green");
				//Delay for better visualization
				std::this_thread::sleep_for(20ms);
				Node* deletedNode = showPath;
				showPath = showPath->parent;
			}
			//Search ended successfuly 
			currentlySearching = false;

			//Delete all nodes from heap
			for (int i{ 0 }; i < nodes.size(); ++i)
			{
				delete nodes[i];
			}

			wxMessageBox("A path from start node to end node was found successfuly!", "Path found", wxICON_EXCLAMATION);
			return;
		}

		if ((tempNode->x + 1) < nFieldWidth && !visited[{ tempNode->x + 1, tempNode->y }] && !walls[tempNode->y * nFieldWidth + (tempNode->x + 1)])
		{
			Node* node{ new Node(tempNode->x + 1, tempNode->y, tempNode) };
			s.push(node);
			visited[{tempNode->x + 1, tempNode->y}] = true;
			nodes.push_back(node);
		}

		if ((tempNode->x - 1) >= 0 && !visited[{ tempNode->x - 1, tempNode->y }] && !walls[tempNode->y * nFieldWidth + (tempNode->x - 1)])
		{
			Node* node{ new Node(tempNode->x - 1, tempNode->y, tempNode) };
			s.push(node);
			visited[{tempNode->x - 1, tempNode->y}] = true;
			nodes.push_back(node);
		}

		if ((tempNode->y - 1) >= 0 && !visited[{tempNode->x, tempNode->y - 1 }] && !walls[(tempNode->y - 1) * nFieldWidth + tempNode->x])
		{
			Node* node{ new Node(tempNode->x, tempNode->y - 1, tempNode)  };
			s.push(node);
			visited[{tempNode->x, tempNode->y - 1}] = true;
			nodes.push_back(node);
		}

		if ((tempNode->y + 1) < nFieldHeight && !visited[{tempNode->x, tempNode->y + 1}] && !walls[(tempNode->y + 1) * nFieldWidth + tempNode->x])
		{
			Node* node{ new Node(tempNode->x, tempNode->y + 1, tempNode) };
			s.push(node);
			visited[{tempNode->x, tempNode->y + 1}] = true;
			nodes.push_back(node);
		}
	}

	//Search ended successfuly
	currentlySearching = false;

	//Delete all nodes from heap
	for (int i{ 0 }; i < nodes.size(); ++i)
	{
		delete nodes[i];
	}

	wxMessageBox("No path from start node to end node was found!", "No path found", wxICON_EXCLAMATION);
}

void MainFrame::SolveBFS()
{
	//Used with timings
	using namespace std::chrono_literals;

	//Use a queue to find a path using BFS
	std::queue<Node*> q;

	//Keeps track of nodes allocated to the heap
	std::vector<Node*> nodes;

	//Set searching boolean to true to indicate that a search is in progress
	currentlySearching = true;

	//Use a hashmap to store visited cells
	std::map<std::pair<int, int>, bool> visited;

	int x = start % nFieldWidth;
	int y = start / nFieldWidth;
	Node* startNode = new Node{ x, y };

	//Push start node into stack and start searching for nearby nodes
	q.push(startNode);
	visited[{ startNode->x, startNode->y }] = true;

	//Keep looping while the stack has at least one element inside it and the stop searching flag is set to false
	while (!q.empty())
	{
		Node* tempNode = q.front();
		q.pop();
		buttonArray[tempNode->y * nFieldWidth + tempNode->x]->SetBackgroundColour("yellow");
		//Delay for better visualization
		std::this_thread::sleep_for(100ms);

		if ((tempNode->y * nFieldWidth + tempNode->x) == end)
		{
			buttonArray[end]->SetBackgroundColour("green");
			Node* showPath = tempNode->parent;
			//Draw path from end node to start node
			while (showPath != nullptr)
			{
				//Every node has one parent, so if we reach the end node and walk through the parents 
				//of each previous node we will eventually reach the start node
				buttonArray[showPath->y * nFieldWidth + showPath->x]->SetBackgroundColour("green");
				//Delay for better visualization
				std::this_thread::sleep_for(20ms);
				Node* deletedNode = showPath;
				showPath = showPath->parent;
			}
			//Search ended successfuly 
			currentlySearching = false;

			//Delete all nodes from heap
			for (int i{ 0 }; i < nodes.size(); ++i)
			{
				delete nodes[i];
			}

			wxMessageBox("A path from start node to end node was found successfuly!", "Path found", wxICON_EXCLAMATION);
			return;
		}

		if ((tempNode->x + 1) < nFieldWidth && !visited[{ tempNode->x + 1, tempNode->y }] && !walls[tempNode->y * nFieldWidth + (tempNode->x + 1)])
		{
			Node* node{ new Node(tempNode->x + 1, tempNode->y, tempNode) };
			q.push(node);
			visited[{tempNode->x + 1, tempNode->y}] = true;
			nodes.push_back(node);
		}

		if ((tempNode->x - 1) >= 0 && !visited[{ tempNode->x - 1, tempNode->y }] && !walls[tempNode->y * nFieldWidth + (tempNode->x - 1)])
		{
			Node* node{ new Node(tempNode->x - 1, tempNode->y, tempNode)  };
			q.push(new Node(tempNode->x - 1, tempNode->y, tempNode));
			visited[{tempNode->x - 1, tempNode->y}] = true;
			nodes.push_back(node);
		}

		if ((tempNode->y + 1) < nFieldHeight && !visited[{tempNode->x, tempNode->y + 1}] && !walls[(tempNode->y + 1) * nFieldWidth + tempNode->x])
		{
			Node* node{ new Node(tempNode->x, tempNode->y + 1, tempNode)  };
			q.push(node);
			visited[{tempNode->x, tempNode->y + 1}] = true;
			nodes.push_back(node);
		}

		if ((tempNode->y - 1) >= 0 && !visited[{tempNode->x, tempNode->y - 1 }] && !walls[(tempNode->y - 1) * nFieldWidth + tempNode->x])
		{
			Node* node{ new Node(tempNode->x, tempNode->y - 1, tempNode)  };
			q.push(node);
			visited[{tempNode->x, tempNode->y - 1}] = true;
			nodes.push_back(node);
		}
	}

	//Delete all nodes from heap
	for (int i{ 0 }; i < nodes.size(); ++i)
	{
		delete nodes[i];
	}

	//Search ended successfuly
	currentlySearching = false;

	wxMessageBox("No path from start node to end node was found!", "No path found", wxICON_EXCLAMATION);
}

void MainFrame::SolveAStar()
{
	//Used with timings 
	using namespace std::chrono_literals;

	//keeps track of nodes allocated to the heap
	std::vector<AStarNode*> AStarNodes;
	//Keeps track of the indices of the nodes that are dynamically allocated inside the vector
	//Useful when tracking back to the start node
	std::map<std::pair<int, int>, int> indexOfVNode;

	//Priority queue to determine the best path with the least global goal
	//Use std::greater so top is the object with the least global goal
	std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> pQ;

	//Find the coordinates of the start node
	int x = start % nFieldWidth;
	int y = start / nFieldWidth;

	//Find the coordinates of the end node
	int x2 = end % nFieldWidth;
	int y2 = end / nFieldWidth;

	//The heuristic value
	int estimated = std::abs(x - x2) + std::abs(y - y2);

	//Keep track of discovered nodes
	std::map<std::pair<int, int>, bool> discovered;

	//Start node
	AStarNode* startNodePtr = new AStarNode(x, y, estimated, 0);
	discovered[{x, y}] = true;
	AStarNodes.push_back(startNodePtr);
	indexOfVNode[{startNodePtr->x, startNodePtr->y}] = AStarNodes.size() - 1;
	pQ.push(*startNodePtr);

	while (!pQ.empty())
	{
		AStarNode tempNode = pQ.top();
		buttonArray[tempNode.y * nFieldWidth + tempNode.x]->SetBackgroundColour(wxColour("yellow"));
		std::this_thread::sleep_for(100ms);

		//Pop the node off of the queue 
		pQ.pop();

		//If goal found, then...
		if ((tempNode.y * nFieldWidth + tempNode.x) == end)
		{
			buttonArray[end]->SetBackgroundColour("green");
			//Get node from the nodes vector to visualize the back track
			AStarNode* showPath = AStarNodes[indexOfVNode[{tempNode.x, tempNode.y}]];
			//Draw path from end node to start node
			while (showPath != nullptr)
			{
				//Every node has one parent, so if we reach the end node and walk through the parents 
				//of each previous node we will eventually reach the start node
				buttonArray[showPath->y * nFieldWidth + showPath->x]->SetBackgroundColour("green");
				//Delay for better visualization
				std::this_thread::sleep_for(20ms);
				AStarNode* deletedNode = showPath;
				showPath = showPath->parent;
			}
			//Search ended successfuly 
			currentlySearching = false;

			//Get rid of dynamically allocated nodes
			for (int i{ 0 }; i < AStarNodes.size(); ++i)
			{
				delete AStarNodes[i];
			}

			wxMessageBox("A path from start node to end node was found successfuly!", "Path found", wxICON_EXCLAMATION);
			return;
		}

		//Try and discover the nodes around current node
		if (tempNode.x + 1 < nFieldWidth && !discovered[{tempNode.x + 1, tempNode.y }] && !walls[tempNode.y * nFieldWidth + (tempNode.x + 1)])
		{
			//Calculate the heuristic value for this new node, the estimated 
			//Then push it into the queue
			int currentStepCount = tempNode.steps;
			//recalculate the heuristic
			int estimated = std::abs(x2 - (tempNode.x + 1)) + std::abs(y2 - tempNode.y);
			//Seperate node objects entering then queue from the objects we use to find our way back to
			AStarNode node{ AStarNode(tempNode.x + 1, tempNode.y, estimated, currentStepCount + 1) };
			AStarNode* nodePtr = new AStarNode(tempNode.x + 1, tempNode.y, estimated, currentStepCount + 1, AStarNodes[indexOfVNode[{tempNode.x, tempNode.y}]]);
			AStarNodes.push_back(nodePtr);
			indexOfVNode[{nodePtr->x, nodePtr->y}] = AStarNodes.size() - 1;
			discovered[{tempNode.x + 1, tempNode.y}] = true;
			pQ.push(node);
		}

		if (tempNode.x - 1 >=0 && !discovered[{tempNode.x - 1 , tempNode.y }] && !walls[tempNode.y * nFieldWidth + (tempNode.x - 1)])
		{
			//Calculate the heuristic value for this new node, the estimated 
			//Then push it into the queue
			int currentStepCount = tempNode.steps;
			int estimated = std::abs(x2 - tempNode.x - 1) + std::abs(y2 - tempNode.y);
			AStarNode node{ AStarNode(tempNode.x - 1, tempNode.y, estimated, currentStepCount + 1) };
			AStarNode* nodePtr = new AStarNode(tempNode.x - 1, tempNode.y, estimated, currentStepCount + 1, AStarNodes[indexOfVNode[{tempNode.x, tempNode.y}]]);
			AStarNodes.push_back(nodePtr);
			indexOfVNode[{nodePtr->x, nodePtr->y}] = AStarNodes.size() - 1;
			discovered[{tempNode.x - 1, tempNode.y}] = true;
			pQ.push(node);
		}

		if (tempNode.y + 1 < nFieldHeight && !discovered[{tempNode.x, tempNode.y + 1}] && !walls[(tempNode.y + 1) * nFieldWidth + (tempNode.x)])
		{
			//Calculate the heuristic value for this new node, the estimated 
			//Then push it into the queue
			int currentStepCount = tempNode.steps;
			int estimated = std::abs(x2 - tempNode.x) + std::abs(y2 - tempNode.y + 1);
			AStarNode node {AStarNode(tempNode.x, tempNode.y + 1, estimated, currentStepCount + 1) };
			AStarNode* nodePtr = new AStarNode(tempNode.x, tempNode.y + 1, estimated, currentStepCount + 1, AStarNodes[indexOfVNode[{tempNode.x, tempNode.y}]]);
			AStarNodes.push_back(nodePtr);
			indexOfVNode[{nodePtr->x, nodePtr->y}] = AStarNodes.size() - 1;
			discovered[{tempNode.x, tempNode.y + 1}] = true;
			pQ.push(node);
		}

		if (tempNode.y - 1 >= 0 && !discovered[{tempNode.x, tempNode.y - 1}] && !walls[(tempNode.y - 1) * nFieldWidth + (tempNode.x)])
		{
			//Calculate the heuristic value for this new node, the estimated 
			//Then push it into the queue
			int currentStepCount = tempNode.steps;
			int estimated = std::abs(x2 - tempNode.x) + std::abs(y2 - tempNode.y - 1);
			AStarNode node{ AStarNode(tempNode.x, tempNode.y - 1, estimated, currentStepCount + 1) };
			AStarNode* nodePtr = new AStarNode(tempNode.x, tempNode.y - 1, estimated, currentStepCount + 1, AStarNodes[indexOfVNode[{tempNode.x, tempNode.y}]]);
			AStarNodes.push_back(nodePtr);
			indexOfVNode[{nodePtr->x, nodePtr->y}] = AStarNodes.size() - 1;
			discovered[{tempNode.x, tempNode.y - 1}] = true;
			pQ.push(node);
		}
	}

	//Search ended successfuly
	currentlySearching = false;

	//Get rid of dynamically allocated nodes
	for (int i{ 0 }; i < AStarNodes.size(); ++i)
	{
		delete AStarNodes[i];
	}

	wxMessageBox("No path from start node to end node was found!", "No path found", wxICON_EXCLAMATION);
}

void MainFrame::initializeGUI()
{
	//sizer for the general GUI components
	generalSizer = new wxBoxSizer(wxVERTICAL);

	//Initilaize the grid button array
	buttonArray = new wxButton * [nFieldWidth * nFieldHeight];

	//Declare a grid sizer and add it to the general sizer
	wxGridSizer* gridSizer = new wxGridSizer(nFieldHeight, nFieldWidth, 0, 0);

	//initialize top panel to control the buttons grid size and add it to the general sizer
	topPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(700, 700));
	generalSizer->Add(topPanel, 1, wxEXPAND | wxALL);

	//Set the grid sizer for the top panel
	topPanel->SetSizer(gridSizer);

	//initialize bottom panel for utility buttons and user input and add it to the general sizer
	bottomPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 70));
	generalSizer->Add(bottomPanel, 0.3, wxEXPAND | wxTOP, 3);


	for (int y{ 0 }; y < nFieldHeight; ++y)
	{
		for (int x{ 0 }; x < nFieldWidth; ++x)
		{
			//Give each button pointer inside the button array a button to point at
			buttonArray[y * nFieldWidth + x] = new wxButton(topPanel, 10000 + (y * nFieldWidth + x));
			//Add each button to the grid sizer
			gridSizer->Add(buttonArray[y * nFieldWidth + x], 1, wxEXPAND | wxALL);
			//Bind each individual button to a function
			buttonArray[y * nFieldWidth + x]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnGridButtonClicked, this);
			//Set button name to a number for the user to choose from
			buttonArray[y * nFieldWidth + x]->SetLabel(std::to_string(y * nFieldWidth + x));
		}
	}

	//Attach the general sizer to the main frame
	this->SetSizerAndFit(generalSizer);
	gridSizer->Layout();

	//set default color
	defaultColor = buttonArray[0]->GetBackgroundColour();

	/*BOTTOM PANEL BUTTONS*/

	//Reset button
	resetButton = new wxButton(bottomPanel, 11000, "Reset", wxPoint(0, 0), wxSize(80, 30));

	//Set start and end buttons
	setCoordinatesButton = new wxButton(bottomPanel, 11001, "Set", wxPoint(90, 0), wxSize(80, 30));

	//Maze generation button
	generateMazeButton = new wxButton(bottomPanel, 11005, "Generate Maze", wxPoint(450, 0), wxSize(90, 30));

	//Save current maze button
	saveMazeButton = new wxButton(bottomPanel, 11006, "Save Maze", wxPoint(550, 0), wxSize(80, 30));

	//Depth first search button
	depthFirstSearch = new wxButton(bottomPanel, 11002, "Depth First", wxPoint(180, 0), wxSize(80, 30));

	//Breadth first search button
	breadthFirstSearch = new wxButton(bottomPanel, 11003, "Breadth First", wxPoint(270, 0), wxSize(80, 30));

	//A*
	aStarSearch = new wxButton(bottomPanel, 11004, "A*", wxPoint(360, 0), wxSize(80, 30));
}

void MainFrame::initializeWalls()
{
	//intialize all walls to false (no walls)
	walls = new bool[nFieldWidth * nFieldHeight];

	for (int y{ 0 }; y < nFieldHeight; ++y)
	{
		for (int x{ 0 }; x < nFieldWidth; ++x)
		{
			walls[y * nFieldWidth + x] = false;
		}
	}
}

void MainFrame::initializeFiles()
{
	//Load in the number of records inside a file from another file into the file record count variable
	std::ifstream inputRecordCountFile{ "recordsCount.txt", std::ios::in };
	if (!inputRecordCountFile)
	{
		std::ofstream outputRecordCountFile{ "recordsCount.txt", std::ios::out };
		fileRecordCount = 0;
		outputRecordCountFile.close();
	}
	else
	{
		std::string count;
		std::getline(inputRecordCountFile, count);
		fileRecordCount = std::stoi(count);
		inputRecordCountFile.close();
	}

	//open binary file and register each and every prior saved maze inside the hash map
	std::ifstream inputFileMaze{ "maze.bin", std::ios::in | std::ios::binary };
	while (inputFileMaze)
	{
		const int mazeSize = nFieldWidth * nFieldHeight;
		bool* tempWalls = new bool[mazeSize];
		inputFileMaze.read(reinterpret_cast<char*>(tempWalls), mazeSize);
		std::bitset<mazeSize> tempBitset;
		for (int i{ 0 }; i < nFieldHeight * nFieldHeight; ++i)
		{
			if (tempWalls[i])
			{
				tempBitset[i] = true;
			}
		}
		mazeExists[tempBitset] = true;
	}

	inputFileMaze.close();
}

//Attempt to set start location
bool MainFrame::SetStart(int start) 
{
	if (start >= 0 && start < (nFieldWidth * nFieldHeight))
	{
		this->start = start;
		buttonArray[start]->SetBackgroundColour("red");
		buttonArray[start]->SetLabel(wxString("S"));
		buttonArray[start]->Enable(false);
		return true;
	}

	return false;
}

//Attempt to set end location
bool MainFrame::SetEnd(int end)
{
	if (end >= 0 && end < (nFieldWidth * nFieldHeight) && end != start)
	{
		this->end = end;
		buttonArray[end]->SetBackgroundColour("red");
		buttonArray[end]->SetLabel(wxString("E"));
		buttonArray[end]->Enable(false);
		return true;
	}

	return false;
}

size_t MainFrame::GetWidth() const
{
	return nFieldWidth;
}

size_t MainFrame::GetHeight() const
{
	return nFieldHeight;
}

//Set whether secondary window is active
void MainFrame::SetWindowActive(bool flag)
{
	secondaryWindowActive = flag;
}
