#pragma once

#include <string>

class Player
{
private:
	int userId;
	std::string name;

public:
	Player(int userId, const std::string &name);
	~Player();

	int getUserId();
	const std::string &getName();
};
