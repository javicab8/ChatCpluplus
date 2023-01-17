#include <iostream>
#include <string>
#include <sstream>
#include <sqlite3.h>

#pragma once
class SQLiteChatDB
{
public:
	SQLiteChatDB();
	~SQLiteChatDB();
	int open(std::string path);
	void close();
	int insertRecord(std::string datetime, std::string client, std::string message);
private:
	sqlite3* DB;
};

