#include "export.hpp"

#include <exception>
#include <libpq-fe.h>

#include "../../../libs/printing/printing.hpp"

#include "postgreSQLconnection.hpp"

#include "../../../db/profiles.hpp"
#include "../../../db/entities.hpp"

#include "../../../entities/conv.hpp"
#include "../../../consts.hpp"

#include "../../../libs/printing/printing.hpp"

#include <memory>

constexpr const char *STMT_CREATE_ENTITY_TABLE = R"(CREATE TABLE entities (
    profile_id INTEGER NOT NULL,
    serial INTEGER NOT NULL,
    common_name VARCHAR(255) NOT NULL,
    type VARCHAR(255) NOT NULL,
    country VARCHAR(255),
    state VARCHAR(255),
    location VARCHAR(255),
    organisation VARCHAR(255),
    email VARCHAR(255),
    keypath VARCHAR(4096) NOT NULL,
    csrpath VARCHAR(4096),
    crtpath VARCHAR(4096) NOT NULL,
    status VARCHAR(255) NOT NULL,
    creation_date VARCHAR(255) NOT NULL,
    expiration_date VARCHAR(255) NOT NULL,
    CONSTRAINT keypath_length_chk CHECK (length(keypath) <= 4096),
    CONSTRAINT csrpath_length_chk CHECK (length(csrpath) <= 4096),
    CONSTRAINT crtpath_length_chk CHECK (length(crtpath) <= 4096)
);)";

constexpr const char *STMT_CREATE_PROFILE_TABLE = R"(CREATE TABLE profiles (
    profile_id SERIAL PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    source TEXT COLLATE "C" CHECK (length(source) <= 4096) NOT NULL
);)";

constexpr const char *TMPL_INSERT_PROFILE = R"(INSERT INTO profiles (profile_id, name, source) VALUES ({},'{}','{}'))";
constexpr const char *TMPL_INSERT_ENTITY = R"(INSERT INTO entities (profile_id, serial, common_name, type, country, state, location, organisation, email, keypath, csrpath, crtpath, status, creation_date, expiration_date)
VALUES ({},{},'{}','{}','{}','{}','{}','{}','{}','{}','{}','{}','{}','{}','{}'))";

using namespace gpkih;

int postgres::exportDB(std::string_view outDir, std::vector<std::string> &args){
	/* BEG - Parse */
	std::string_view connectionURL = "";
	std::string_view user, pass, host, port = "5432";

    for(int i = 0; i < args.size(); ++i){
        const std::string &opt = args[i];
        if(opt == "-h" || opt == "--host"){
            host = args[++i];
        }else if(opt == "-u" || opt == "--user"){
            user = args[++i];
        }else if(opt == "-pass" || opt == "--password"){
            pass = args[++i];
        }else if(opt == "-p" || opt == "--port"){
            port = args[++i];
        }else if(opt == "-u" || opt == "--url"){
			connectionURL = args[++i];
		}
    }
	/* END - Parse */	

    /* BEG - Create connection */
	std::unique_ptr<PostgreSQLConnection> connection;
	try{
		if(!connectionURL.empty()){
			// Attempt connecting
			connection = std::make_unique<PostgreSQLConnection>(connectionURL);
		}else{
			if(user.empty() || pass.empty() || host.empty()){
				PERROR("Can't run with given options, connection url or host,user,password must be given\n");
				return GPKIH_FAIL;
			}
			connection = std::make_unique<PostgreSQLConnection>(host, user, pass, port);
		}
	}catch(std::exception &ex){
		PERROR(ex.what());
		return GPKIH_FAIL;
	}
	/* END - Create connection */

	/* BEG - create database + tables */
	if(connection->exec(STMT_CREATE_PROFILE_TABLE) != GPKIH_OK
	 ||connection->exec(STMT_CREATE_ENTITY_TABLE) != GPKIH_OK
	){
		PERROR(connection->getError());
		return GPKIH_FAIL;
	}
	/* END - create database + tables */

	/* BEG - Insert gpkih data into database */
	auto profiles = db::profiles::get();
	for(const auto &[profileName, profile] : *profiles){
		if(connection->exec(fmt::format(TMPL_INSERT_PROFILE, profile.meta.id, profile.name, profile.source)) != GPKIH_OK){
			PERROR(connection->getError());
			return GPKIH_FAIL;
		};
		EntityManager entities(profileName);
		for(const auto &[cn, entity] : *entities.retrieve()){
			connection->exec(fmt::format(TMPL_INSERT_ENTITY, profile.meta.id, entity.meta.serial, entity.subject.cn, 
				entity::conversion::toString(entity.meta.type), entity.subject.country, entity.subject.state, entity.subject.location, entity.subject.organisation,
				entity.subject.email, entity.keyPath, entity.csrPath, entity.crtPath, entity::conversion::toString(entity.meta.status), fmt::format("{:%d-%m-%Y @ %H:%M}",entity.meta.creationDate), fmt::format("{:%d-%m-%Y @ %H:%M}",entity.meta.expirationDate))
			);
		}
	}
	/* END - Insert gpkih data into database */
	return GPKIH_OK;
}