#include <string>
#include "postgreSQLconnection.hpp"
#include <libpq-fe.h>
#include "../../../gpkih.hpp"
#include "../../../libs/printing/printing.hpp" // DEBUGF()

PostgreSQLConnection::PostgreSQLConnection(std::string_view dbUri):connection(PQconnectdb(dbUri.data())){
	DEBUG(2, "PostgreSQLConnection()");
	if(connection == nullptr){
		throw(std::string{"Couldn't connect to postgreSQL database - error: "} + PQerrorMessage(connection));
	}
}

PostgreSQLConnection::~PostgreSQLConnection(){
	if(closed){
		return;
	}
	if(connection != nullptr){
		PQfinish(connection);
	}
}

int PostgreSQLConnection::exec(std::string_view query){
	if(connection == nullptr){
		return -1;
	}
	
	PGresult *result = PQexec(connection, query.data());
	
	if(result == nullptr){
		lastError = PQerrorMessage(connection);
		return GPKIH_FAIL;
	}

	return GPKIH_OK;
}

const PGconn* const PostgreSQLConnection::getConnection(){
	return connection;
}

const char *PostgreSQLConnection::getError(){
	return lastError;
}

void PostgreSQLConnection::close(){
	closed = true;
	PQfinish(connection);
}