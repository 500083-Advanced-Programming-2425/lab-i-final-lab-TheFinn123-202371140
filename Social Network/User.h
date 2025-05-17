#pragma once

#include <string>
#include <unordered_set>

// Represents a single user in the network
class User {
public:
	std::string id;
	std::string name;
	int age;
	std::string countryCode;
	double activityRate;
	std::unordered_set<User*> friends;

	// Constructor to initialise a user
	User(const std::string& userId, const std::string& fullName, int userAge, const std::string& country, double rate)
		: id(userId), name(fullName), age(userAge), countryCode(country), activityRate(rate) {
	}
};
