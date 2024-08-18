#include "SQLiteConnection.hpp"

#include "../../../db/profiles.hpp"
#include "../../../db/entities.hpp"

#include "../../../entities/conv.hpp"
#include "../../../consts.hpp"

namespace gpkih::sqlite
{

constexpr const char *STMT_CREATE_PROFILE_TABLE = R"(CREATE TABLE entities (
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

constexpr const char *STMT_CREATE_ENTITY_TABLE = R"(CREATE TABLE profiles (
    		profile_id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
    		name TEXT UNIQUE NOT NULL,
    		source TEXT COLLATE NOCASE CHECK(length(source) <= 4096) NOT NULL
		);)";

int exportDB(std::string_view dbPath){
	auto ProfileMap = gpkih::db::profiles::get();
	
	if(ProfileMap->empty()){
		return GPKIH_FAIL;
	}

	try{
		SQLiteConnection conn(dbPath);
		
		/* Create profiles and entities tables */
		for(const auto &statement : {STMT_CREATE_PROFILE_TABLE, STMT_CREATE_ENTITY_TABLE}){
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
					||sqlite3_bind_text(stmt, 4, gpkih::entity::conversion::toString(entity.meta.type).c_str(), -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 5, entity.subject.country, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 6, entity.subject.state, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 7, entity.subject.location, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 8, entity.subject.organisation, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 9, entity.subject.email, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 10, entity.keyPath, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 11, entity.crtPath, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 12, entity.csrPath, -1, SQLITE_TRANSIENT)
					||sqlite3_bind_text(stmt, 13, gpkih::entity::conversion::toString(entity.meta.status).c_str(), -1, SQLITE_TRANSIENT)
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

}