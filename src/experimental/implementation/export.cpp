#include "../export.hpp"
#include "../../db/profiles.hpp"
#include "../../db/entities.hpp"
#include "../../entities/conv.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../gpkih.hpp"
#include <sqlite3.h>

ExportException::ExportException(const char *msg):msg(msg){};
ExportException::ExportException(std::string &&msg):msg(msg){};

const char *ExportException::what()const noexcept{
	return msg.c_str();
}

namespace gpkih_sqlite3{
	namespace statements {
		constexpr const char *profileTableCreation = R"(CREATE TABLE profiles (
    		profile_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
    		name TEXT UNIQUE NOT NULL,
    		source TEXT COLLATE NOCASE CHECK(length(source) <= 4096) NOT NULL
		);)";

		constexpr const char *entityTableCreation = R"(CREATE TABLE entities (
    		profile_id INTEGER NOT NULL,
    		serial INTEGER NOT NULL,
    		common_name VARCHAR(255) NOT NULL,
    		type VARCHAR(255) NOT NULL,
    		country VARCHAR(255),
    		state VARCHAR(255),
    		location VARCHAR(255),
    		organisation VARCHAR(255),
    		email VARCHAR(255),
    		keypath CHECK(length(keypath) <= 4096) NOT NULL,
    		crtpath CHECK(length(crtpath) <= 4096) NOT NULL,
    		csrpath CHECK(length(csrpath) <= 4096),
    		status VARCHAR(255) NOT NULL,
    		creation_date VARCHAR(255) NOT NULL,
    		expiration_date VARCHAR(255) NOT NULL  		
    	);)";
	}
}

namespace gpkih_mariadb{
	constexpr const char *tableCreationSTMT = R"(CREATE TABLE profiles (
  		id INT AUTO_INCREMENT PRIMARY KEY,
  		name VARCHAR(255) UNIQUE NOT NULL,
  		source VARCHAR(4096) COLLATE utf8mb4_unicode_ci NOT NULL
	) ENGINE=InnoDB;)";
}

namespace gpkih_psql{
	constexpr const char *tableCreationSTMT = R"(CREATE TABLE profiles (
  		id SERIAL PRIMARY KEY,
  		name TEXT UNIQUE NOT NULL,
  		source TEXT COLLATE pg_catalog."en_US.utf8" CHECK(length(source) <= 4096) NOT NULL
	);)";
}

/* ACTUALLY UNUSED */ 
class DB_ConnectionIface{
public:
	// Execute a query and return 0:success 1:error
	virtual int exec() = 0;
};

class SQLiteConnection{
private:
	std::string dbPath;
	sqlite3 *conn = nullptr;
public:
	SQLiteConnection(std::string_view _dbPath):dbPath(_dbPath){
		DEBUGF(3, "SQLiteConnection({})", dbPath);
		if(sqlite3_open(dbPath.c_str(), &conn)){
			throw ExportException("Couldn't create sqlite3 db connection");
		};
	};

	~SQLiteConnection(){
		if(conn != nullptr){
			sqlite3_close_v2(conn);
		}
	};

	int exec(const char *query, int(*callback)(void *,int,char**,char**) = nullptr, void *cbFirstArg = nullptr, char **errmsg = nullptr){
		if(sqlite3_exec(conn, query, callback, cbFirstArg, errmsg)){
			return 1;
		};
		return 0;
	}

	int close(){
		return sqlite3_close(conn);
	}

	sqlite3* const getConnection(){
		return conn;
	}

	const char *getError(){
		return sqlite3_errmsg(conn);
	}
};

int gpkih::db::exportSQLITE(std::string_view dbPath){
	auto ProfileMap = db::profiles::get();
	
	if(ProfileMap->empty()){
		return GPKIH_FAIL;
	}

	try{
		SQLiteConnection conn(dbPath);
		
		/* Create profiles and entities tables */
		for(const auto &statement : {gpkih_sqlite3::statements::profileTableCreation, gpkih_sqlite3::statements::entityTableCreation}){
			if(conn.exec(statement)){
				return GPKIH_FAIL;
			}	
		}

		/* BEG - Create and populate profiles table */
		const char *insertStmt = "INSERT INTO profiles (profile_id, name, source) VALUES (?, ?, ?)";
		sqlite3_stmt *stmt = nullptr;
		auto dbconn = conn.getConnection();

		if(sqlite3_prepare_v2(dbconn, insertStmt, -1, &stmt, nullptr) != SQLITE_OK){
			fprintf(stderr, "sqlite3_prepare_v2() failed - %s\n", sqlite3_errstr(sqlite3_errcode(dbconn)));
			return GPKIH_FAIL;
		};

		/* Populate profiles' table */
		for(const auto &[pname, profile] : *ProfileMap){
			/* Bind profile ID */
			if(sqlite3_bind_int64(stmt, 1, profile.meta.id) != SQLITE_OK){
				fprintf(stderr, "binding profile id failed\n");
				return GPKIH_FAIL;
			}
			
			/* Bind profile NAME */
			if(sqlite3_bind_text(stmt, 2, profile.name, -1, SQLITE_TRANSIENT)){
				fprintf(stderr, "binding profile name failed\n");
				return GPKIH_FAIL;
			}

			/* Bind profile SOURCE */
			if(sqlite3_bind_text(stmt, 3, profile.source, -1, SQLITE_TRANSIENT)){
				fprintf(stderr, "binding profile source failed\n");
				return GPKIH_FAIL;
			}

			/* Execute statement */
			if(sqlite3_step(stmt) != SQLITE_DONE){
				fprintf(stderr, "executing statement failed\n");
				return GPKIH_FAIL;
			};

			/* Reset statement */
			sqlite3_reset(stmt);
		}
		/* END - Create and populate profiles table */

		/* BEG - populate entities' table */
		const char *entityInsertStmt = R"(INSERT INTO entities (profile_id, serial, common_name, type, country, state, location, organisation, email, keyPath, csrPath, crtPath, status, creation_date, expiration_date)
			VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
		)";

		if(sqlite3_prepare_v2(dbconn, entityInsertStmt, -1, &stmt, nullptr) != SQLITE_OK){
			fprintf(stderr, "sqlite3_prepare_v2() failed - %s\n", sqlite3_errstr(sqlite3_errcode(dbconn)));
			return GPKIH_FAIL;
		}

		for(const auto &[pname, profile] : *ProfileMap){
			EntityManager entities(pname);
			auto entityMap = entities.retrieve();

			for(const auto &[cn, entity] : *entityMap){

				if(sqlite3_bind_int64(stmt, 1, profile.meta.id)
					||sqlite3_bind_int64(stmt, 2, entity.meta.serial)
					||sqlite3_bind_text(stmt, 3, entity.subject.cn, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 4, entity::conversion::toString(entity.meta.type).c_str(), -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 5, entity.subject.country, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 6, entity.subject.state, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 7, entity.subject.location, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 8, entity.subject.organisation, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 9, entity.subject.email, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 10, entity.keyPath, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 11, entity.crtPath, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 12, entity.csrPath, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 13, entity::conversion::toString(entity.meta.status).c_str(), -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 14, fmt::format("{:%d-%m-%Y @ %H:%M}", entity.meta.creationDate).c_str(), -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 15, fmt::format("{:%d-%m-%Y @ %H:%M}", entity.meta.expirationDate).c_str(), -1, SQLITE_TRANSIENT)
				){
					fprintf(stderr, "ERROR binding");
				}

				if(sqlite3_step(stmt) != SQLITE_DONE){
					fprintf(stderr, "ERROR executing INSERT on entities - %s\n", conn.getError());
					return GPKIH_FAIL;
				}

				sqlite3_reset(stmt);
			}

		}
		/* END - populate entities' table */
		conn.close();
	}catch(const std::exception &ex){
		fprintf(stderr, "EXCEPTION: %s\n", ex.what());
		return GPKIH_FATAL;
	}

	return GPKIH_OK;
}

int gpkih::db::exportMYSQL(std::string_view dbPath){
	return GPKIH_OK;
}

int gpkih::db::exportPSQL(std::string_view dbPath){
	return GPKIH_OK;
}

int gpkih::db::exportCSV(std::string_view dbPath){
	return GPKIH_OK;
}

int gpkih::db::exportJSON(std::string_view dbPath){
	return GPKIH_OK;
}