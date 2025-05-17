#include <sstream> // For parsing strings
#include <unordered_set> // For efficient friend lookups
#include <queue> // For BFS traversal
#include <algorithm> // For sorting
#include "Solution.h" // Header for the Solution class
#include "User.h" // file is under external dependencies when opened in visual studio

// Constructor: opens the output file for writing
Solution::Solution() : _outFile("Output.txt") {}

// Loads users and friendships from the given files
bool Solution::buildNetwork(const std::string& fileNameUsers, const std::string& fileNameFriendships) {
	std::ifstream finUsers(fileNameUsers), finFriends(fileNameFriendships);
	if (finUsers.fail() || finFriends.fail()) return false; // Return false if either file can't be opened

	std::string line;
	while (std::getline(finUsers, line)) { // Read each user line
		std::istringstream ss(line);
		std::string id, name, ageStr, country, activityStr;
		std::getline(ss, id, ',');
		std::getline(ss, name, ',');
		std::getline(ss, ageStr, ',');
		std::getline(ss, country, ',');
		std::getline(ss, activityStr, ',');

		// Convert age and activity strings to numbers
		int age = std::stoi(ageStr);
		double activity = std::stod(activityStr);

		// Create and store the user
		users[id] = std::make_shared<User>(id, name, age, country, activity);
	}

	// Load friendships and update each user's friend set
	while (std::getline(finFriends, line)) {
		std::istringstream ss(line);
		std::string id1, id2;
		std::getline(ss, id1, ',');
		std::getline(ss, id2, ',');

		// Add bidirectional friendship if both users exist
		if (users.count(id1) && users.count(id2)) {
			users[id1]->friends.insert(users[id2].get());
			users[id2]->friends.insert(users[id1].get());
		}
	}

	return true;
}

// Parses and processes a command
bool Solution::processCommand(const std::string& commandString) {
	std::istringstream in(commandString);
	std::string command;
	in >> command;

	_outFile << commandString << '\n';

	// ViewProfile
	if (command == "ViewProfile") {
		// Finds user of the given ID
		std::string id;
		in >> id;
		if (!users.count(id)) {
			_outFile << "Error: User not found.\n\n";
			return true;
		}
		User& u = *users[id];
		// Output user profile details
		_outFile << "Name: " << u.name << '\n';
		_outFile << "Age: " << u.age << '\n';
		_outFile << "Country: " << countryCodeToName(u.countryCode) << '\n';
		_outFile << "Activity Rate: " << int(u.activityRate * 100 + 0.5) << "%\n";
		_outFile << "Friends: " << u.friends.size() << "\n\n";
		return true;
	}

	// ListFriends
	if (command == "ListFriends") {
		std::string id;
		in >> id;
		if (!users.count(id)) {
			_outFile << "Error: User not found.\n\n";
			return true;
		}
		User& u = *users[id];
		_outFile << u.friends.size() << " friend(s) found.\n";
		// Iterate through friends and print names with IDs
		for (std::unordered_set<User*>::iterator it = u.friends.begin(); it != u.friends.end(); ++it) {
			_outFile << (*it)->name << " [ID:" << (*it)->id << "]\n";
		}
		_outFile << '\n';
		return true;
	}

	// ListMutuals
	if (command == "ListMutuals") {
		std::string id1, id2;
		in >> id1 >> id2;
		if (!users.count(id1) || !users.count(id2)) {
			_outFile << "Error: User not found.\n\n";
			return true;
		}
		User& a = *users[id1];
		User& b = *users[id2];

		std::vector<std::string> mutuals;
		// Check each of a's friends to see if they are also in b's friend set
		for (std::unordered_set<User*>::iterator it = a.friends.begin(); it != a.friends.end(); ++it) {
			if (b.friends.count(*it)) mutuals.push_back((*it)->id);
		}

		_outFile << mutuals.size() << " mutual friend(s) found.\n";
		// Print each mutual friend’s name and ID
		for (std::vector<std::string>::iterator it = mutuals.begin(); it != mutuals.end(); ++it) {
			_outFile << users[*it]->name << " [ID:" << *it << "]\n";
		}
		_outFile << '\n';
		return true;
	}

	// FindSeparation
	if (command == "FindSeparation") {
		std::string id1, id2;
		in >> id1 >> id2;
		if (!users.count(id1) || !users.count(id2)) {
			_outFile << "Error: User not found.\n\n";
			return true;
		}
		if (id1 == id2) {
			_outFile << "Separation between " << id1 << " and " << id2 << " is 0.\n\n";
			return true;
		}

		// BFS to find shortest path
		std::queue<std::pair<std::string, int>> q;
		std::unordered_set<std::string> visited;
		q.push(std::make_pair(id1, 0));
		visited.insert(id1);

		while (!q.empty()) {
			std::pair<std::string, int> front = q.front(); q.pop();
			std::string currId = front.first;
			int dist = front.second;

			User* curr = users[currId].get();
			// Explore neighbours
			for (std::unordered_set<User*>::iterator it = curr->friends.begin(); it != curr->friends.end(); ++it) {
				if ((*it)->id == id2) {
					_outFile << "Separation between " << id1 << " and " << id2 << " is " << dist + 1 << ".\n\n";
					return true;
				}
				// Add unvisited friends to the queue
				if (visited.insert((*it)->id).second) {
					q.push(std::make_pair((*it)->id, dist + 1));
				}
			}
		}

		_outFile << "No connection found.\n\n";
		return true;
	}

	// SuggestFriends
	if (command == "SuggestFriends") {
		std::string id;
		in >> id;
		if (!users.count(id)) {
			_outFile << "Error: User not found.\n\n";
			return true;
		}

		User& source = *users[id];
		const std::unordered_set<User*>& friendSet = source.friends;

		struct Suggestion {
			std::shared_ptr<User> user;
			int mutuals;
			double score;
		};

		std::unordered_set<std::string> visited;
		std::unordered_map<std::string, int> level;
		std::queue<std::pair<std::string, int>> q;
		std::vector<Suggestion> suggestions;

		// BFS up to a depth of 2 (as it's mathmatically improbable to have a higher friend score with further separation)
		q.push(std::make_pair(id, 0));
		visited.insert(id);
		level[id] = 0;

		while (!q.empty()) {
			std::string currId = q.front().first;
			int dist = q.front().second;
			q.pop();

			if (dist >= 2) continue;

			User* curr = users[currId].get();
			for (User* neighbour : curr->friends) {
				std::string neighbourId = neighbour->id;
				if (visited.insert(neighbourId).second) {
					level[neighbourId] = dist + 1;
					q.push(std::make_pair(neighbourId, dist + 1));
				}
			}
		}

		// Evaluate each person who's not already a friend
		for (std::unordered_map<std::string, int>::iterator it = level.begin(); it != level.end(); ++it) {
			std::string candidateId = it->first;
			int dist = it->second;

			if (candidateId == id) continue;

			std::shared_ptr<User> candidate = users[candidateId];
			if (friendSet.count(candidate.get())) continue;

			int mutualCount = 0;
			for (User* f : candidate->friends) {
				if (friendSet.count(f)) mutualCount++;
			}
			if (mutualCount == 0) continue;

			double rA = source.activityRate, rB = candidate->activityRate;
			double score = (mutualCount * rA * rB) + (720.0 / dist); // dist is either 1 or 2

			Suggestion s;
			s.user = candidate;
			s.mutuals = mutualCount;
			s.score = score;
			suggestions.push_back(s);
		}

		// Sort by descending score, then by name
		std::sort(suggestions.begin(), suggestions.end(), [](const Suggestion& a, const Suggestion& b) {
			if (a.score != b.score) return a.score > b.score;
			return a.user->name < b.user->name;
			});

		_outFile << suggestions.size() << " suggestion(s) found.\n";
		int printed = 0;
		for (std::vector<Suggestion>::iterator it = suggestions.begin(); it != suggestions.end() && printed < 5; ++it, ++printed) {
			_outFile << it->user->name << " [ID:" << it->user->id << "], " << it->mutuals << " mutual friend(s)\n";
		}
		_outFile << '\n';
		_outFile.flush(); // Ensure data is written
		return true;
	}



	// FriendScore
	if (command == "FriendScore") {
		std::string id1, id2;
		in >> id1 >> id2;
		if (!users.count(id1) || !users.count(id2)) {
			_outFile << "Error: User not found.\n\n";
			return true;
		}

		if (id1 == id2) {
			// Max mutual score for self
			double r = users[id1]->activityRate;
			double score = (users[id1]->friends.size() * r * r) + 720.0;
			char buf[64];
			std::snprintf(buf, sizeof(buf), "Friend Score: %.3f\n\n", score);
			_outFile << buf;
			return true;
		}

		User& a = *users[id1];
		User& b = *users[id2];

		// Optimised mutual friend count (iterate over smaller set)
		const std::unordered_set<User*>& friendsA = a.friends;
		const std::unordered_set<User*>& friendsB = b.friends;

		int mutuals = 0;
		if (friendsA.size() < friendsB.size()) {
			for (User* f : friendsA) {
				if (friendsB.count(f)) mutuals++;
			}
		}
		else {
			for (User* f : friendsB) {
				if (friendsA.count(f)) mutuals++;
			}
		}

		// BFS to find separation (limit depth to 6 for optimising)
		std::queue<std::pair<std::string, int>> q;
		std::unordered_set<std::string> visited;
		q.push(std::make_pair(id1, 0));
		visited.insert(id1);
		int separation = 6; // Default if not found

		while (!q.empty()) {
			std::string currId = q.front().first;
			int dist = q.front().second;
			q.pop();

			if (dist >= 6) continue;

			if (currId == id2) {
				separation = dist;
				break;
			}

			User* curr = users[currId].get();
			for (User* neighbour : curr->friends) {
				const std::string& neighbourId = neighbour->id;
				if (visited.insert(neighbourId).second) {
					if (neighbourId == id2) {
						separation = dist + 1;
						goto SCORE; // Immediate escape (as it's faster than break from both loops)
					}
					q.push(std::make_pair(neighbourId, dist + 1));
				}
			}
		}

	SCORE: //goto label
		double score = (mutuals * a.activityRate * b.activityRate) + (720.0 / separation);
		char buf[64];
		std::snprintf(buf, sizeof(buf), "Friend Score: %.3f\n\n", score);
		_outFile << buf;
		return true;
	}


	// TotalUsers command
	if (command == "TotalUsers") {
		std::unordered_set<std::string> filter;
		std::string code;
		while (in >> code) filter.insert(code); // Read country code filters

		int count = 0;
		for (std::unordered_map<std::string, std::shared_ptr<User> >::iterator it = users.begin(); it != users.end(); ++it) {
			if (filter.empty() || filter.count(it->second->countryCode)) count++;
		}
		_outFile << "Total Users: " << count << "\n\n";
		_outFile.flush(); // Ensure data is written
		return true;
	}

	// Unknown command
	_outFile << "Error: Unrecognised command.\n\n";
	return true;
}

// Converts country code to full country name
std::string Solution::countryCodeToName(const std::string& code) const {
	static std::unordered_map<std::string, std::string> countryNames;
	if (countryNames.empty()) {
		countryNames["UK"] = "United Kingdom";
		countryNames["US"] = "United States";
		countryNames["FR"] = "France";
		countryNames["DE"] = "Germany";
		countryNames["IN"] = "India";
		countryNames["CN"] = "China";
		countryNames["JP"] = "Japan";
	}
	std::unordered_map<std::string, std::string>::const_iterator it = countryNames.find(code);
	return (it != countryNames.end()) ? it->second : code;
}
