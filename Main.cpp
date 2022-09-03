#include "Main.h"

wxIMPLEMENT_APP(Main);

Main::Main()
{

}

Main::~Main()
{

}

bool Main::OnInit()
{
	mainFrame = new MainFrame();
	mainFrame->Show();

	return true;
}
