#include <string>
/*
	This header defines the export action, its' methods, possible exporting options...
	
	There are 2 ways of exporting:
		- To a database (sqlite3, mysql, psql)
		- To a file (csv, json)
*/
class ExportException : public std::exception{
private:
	std::string msg;
public:
	ExportException() = default;
	ExportException(const char *msg);
	ExportException(std::string &&st);
	const char *what() const noexcept override;
};

enum class ExportOption {
	sqlite3,
	mysql,
	psql,
	csv,
};

namespace gpkih::db {
	/* Database options */
	extern int exportSQLITE(std::string_view dbPath);

	extern int exportMYSQL(std::string_view dbPath);

	extern int exportPSQL(std::string_view dbPath);

	/* File options */	
	extern int exportCSV(std::string_view dbPath);

	extern int exportJSON(std::string_view dbPath);
}