#include "SQLiteChatDB.h"

SQLiteChatDB::SQLiteChatDB()
{
	DB = {};
}

SQLiteChatDB::~SQLiteChatDB()
{
	this->close();
}

int SQLiteChatDB::open(std::string path)
{ 
	int exit = sqlite3_open(path.c_str(), &DB);
	if (exit)
		return 0;

	std::string sqlTable = "CREATE TABLE IF NOT EXISTS historic("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
		"INSTANT CHAR(26) NOT NULL, "
		"CLIENT TEXT NOT NULL, "
		"MESSAGE TEXT NOT NULL);";
	char* messageError;
	exit = sqlite3_exec(DB, sqlTable.c_str(), nullptr, 0, &messageError);

	if (exit != SQLITE_OK) 
	{
		std::cerr << "Error al crear la tabla: " << messageError << "\r\n";
		sqlite3_free(messageError);
	}
	return 1;
}

void SQLiteChatDB::close()
{
	sqlite3_close(DB);
}

int SQLiteChatDB::insertRecord(std::string datetime, std::string client, std::string message)
{
	std::ostringstream sqlInsertStream;
	sqlInsertStream << "INSERT INTO historic(INSTANT, CLIENT, MESSAGE) VALUES('"
		<< datetime << "', '" << client << "', '" << message << "');";
	std::string sqlString = sqlInsertStream.str();
	
	char* messageError;
	int exit = sqlite3_exec(DB, sqlString.c_str(), nullptr, 0, &messageError);
	if (exit != SQLITE_OK)
	{
		std::cerr << "Error al insertar: " << messageError << "\r\n";
		sqlite3_free(messageError);
		return 0;
	}
	return 1;
}