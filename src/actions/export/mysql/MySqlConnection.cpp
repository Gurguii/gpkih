#include "MySqlConnection.hpp"
#include <mysql/mysql.h>

#include "../../../consts.hpp"
#include "../../../libs/printing/printing.hpp"

MySqlConnection::MySqlConnection(std::string_view host, std::string_view user, std::string_view password, int port){
	DEBUGF(2, "MySqlConnection(host={},user={},port={})", host, user, port);

	if((connection = mysql_init(nullptr)) == nullptr){
		throw("Error initializing MYSQL *");
	}

	if(mysql_real_connect(connection, host.data(), user.data(), password.data(), nullptr, port, nullptr, 0) == nullptr){
		throw("Error connecting to mysql server");
	}
}

MySqlConnection::~MySqlConnection(){
	DEBUG(3,"~MySqlConnection()");
	
	if(connection != nullptr){
		mysql_close(connection);
	}
}

int MySqlConnection::exec(std::string_view query){
	DEBUGF(3, "MySqlConnection::exec({})", query);

	if(int err = mysql_query(connection, query.data())){
		switch(err){
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

const MYSQL* const MySqlConnection::getConnection(){
	return connection;
}

const char *MySqlConnection::getError(){
	return lastError;
}

void MySqlConnection::close(){
	mysql_close(connection);
}
