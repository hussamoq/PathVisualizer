#pragma once
#include "wx\wx.h"
#include "MainFrame.h"

class Main : public wxApp
{
public:
	Main();
	~Main();

public:
	virtual bool OnInit();

private:
	MainFrame* mainFrame{ nullptr };
};

