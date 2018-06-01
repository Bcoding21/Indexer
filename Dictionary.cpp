#include "Dictionary.h"

Dictionary::Dictionary() : counter(0) { }

void Dictionary::add(const std::string& key) {
	auto it = data.find(key);
	if (it == data.end()) {
		data.emplace(key, counter++);
		currId = counter;
	}
	else {
		currId = it->second;
	}
}

unsigned long Dictionary::getCurrId() const {
	return currId;
}
