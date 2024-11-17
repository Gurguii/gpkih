#include <mysql/mysql.h>
#include <memory>
#include <string_view>
#include <string>

class MySqlConnection {
private:
    std::unique_ptr<MYSQL, decltype(&mysql_close)> connection;
    const char *lastError = nullptr;
    std::string lastQuery;

public:
    MySqlConnection() = delete;
    MySqlConnection(std::string_view host, std::string_view user, std::string_view password, int port = 3306);
    ~MySqlConnection();
    int exec(std::string_view query);
    const MYSQL* const getConnection();
    const char *getError();
    void close();
};