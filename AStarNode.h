#pragma once

struct AStarNode
{
	AStarNode(int x, int y, int estimated, int steps, AStarNode* parent = nullptr)
	{
		this->x = x;
		this->y = y;
		this->estimated = estimated;
		this->steps = steps;
		global = this->estimated + this->steps;
		this->parent = parent;
	}

	int x;
	int y;

	AStarNode* parent{ nullptr };

	int estimated;
	int steps;
	int global;

	friend bool operator>(const AStarNode& thisObj, const AStarNode& otherObj) 
	{
		return thisObj.global > otherObj.global;
	}

	friend bool operator<(const AStarNode& thisObj, const AStarNode& otherObj)
	{
		return thisObj.global < otherObj.global;
	}
};