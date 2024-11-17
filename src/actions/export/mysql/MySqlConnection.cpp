#include "MySqlConnection.hpp"
#include <mysql/mysql.h>
#include "../../../consts.hpp"
#include "../../../libs/printing/printing.hpp"

MySqlConnection::MySqlConnection(std::string_view host, std::string_view user, std::string_view password, int port)
    : connection(nullptr, mysql_close) // Initialize unique_ptr with custom deleter
{
    DEBUGF(2, "MySqlConnection(host={}, user={}, port={})", host, user, port);

    // Initialize the MYSQL connection
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) {
        throw std::runtime_error("Error initializing MYSQL *");
    }

    // Attempt to connect to the MySQL server
    if (mysql_real_connect(conn, host.data(), user.data(), password.data(), nullptr, port, nullptr, 0) == nullptr) {
        mysql_close(conn); // Clean up if connection fails
        throw std::runtime_error("Error connecting to MySQL server");
    }

    connection.reset(conn); // Transfer ownership to unique_ptr
}

MySqlConnection::~MySqlConnection() {
    DEBUG(3, "~MySqlConnection()");
    // No need to explicitly close connection; unique_ptr will handle it
}

int MySqlConnection::exec(std::string_view query) {
    DEBUGF(3, "MySqlConnection::exec({})", query);

    if (int err = mysql_query(connection.get(), query.data())) {
        switch (err) {
            case 2014:
                lastError = "Commands were executed in an improper order.\n";
                break;
            case 2006:
                lastError = "The MySQL server has gone away.\n";
                break;
            case 2013:
                lastError = "The connection to the server was lost during the query.\n";
                break;
            default:
                lastError = "Unknown error.\n";
                break;
        }
        lastQuery = query;
        PINFO("Last query -> {}\n", query);
        return GPKIH_FAIL;
    }
    return GPKIH_OK;
}

const MYSQL* const MySqlConnection::getConnection() {
    return connection.get();
}

const char *MySqlConnection::getError() {
    return lastError;
}

void MySqlConnection::close() {
    if (connection) {
        connection.reset(); // Close the connection
    }
}