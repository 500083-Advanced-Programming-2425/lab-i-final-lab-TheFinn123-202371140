#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include "User.h"

// Manages the social network and processes commands
class Solution final {
	std::ofstream _outFile; // Output file stream
	std::unordered_map<std::string, std::shared_ptr<User>> users; // All users by ID

	std::string countryCodeToName(const std::string& code) const; // Convert code to country name

public:
	Solution();

	bool buildNetwork(const std::string& fileNameUsers, const std::string& fileNameFriendships);
	bool processCommand(const std::string& commandString);
};
