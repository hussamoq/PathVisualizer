#pragma once
struct Node
{
	Node(int x, int y, Node *parent = nullptr)
	{
		this->x = x;
		this->y = y;
		this->parent = parent;
	}

	//Vars
	Node *parent;

	int x;
	int y;
};

