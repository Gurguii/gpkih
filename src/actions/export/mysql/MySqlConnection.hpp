#include <mysql/mysql.h>

#include <string_view>
#include <string>

class MySqlConnection{
private:
	MYSQL *connection = nullptr;
	const char *lastError = nullptr;
	std::string lastQuery;
	bool closed = false;
	
public:
	//MySqlConnection(mysql://foo:foo@10.10.10.10:4444);
	//MySqlConnection(std::string_view host, std::string_view user, int port = 3306);
	MySqlConnection() = delete;
	MySqlConnection(std::string_view host, std::string_view user, std::string_view password, int port = 3306);
	~MySqlConnection();
	int exec(std::string_view query);
	const MYSQL* const getConnection();
	const char *getError();
	void close();
};