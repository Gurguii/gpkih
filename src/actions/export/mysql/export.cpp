#include "export.hpp"
#include "MySqlConnection.hpp"

#include "../../../consts.hpp"

#include "../../../libs/printing/printing.hpp"

#include "../../../db/profiles.hpp"
#include "../../../db/entities.hpp"

#include "../../../entities/conv.hpp"

#include "../../../libs/printing/printing.hpp"

constexpr const char *STMT_CREATE_ENTITY_TABLE = R"(
    CREATE TABLE {}.entities (
        profile_id INT NOT NULL,
        serial INT NOT NULL,
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
        expiration_date VARCHAR(255) NOT NULL
    );
)";

constexpr const char *STMT_CREATE_PROFILE_TABLE = R"(
    CREATE TABLE {}.profiles (
        profile_id INT UNIQUE PRIMARY KEY,
        name VARCHAR(255) UNIQUE NOT NULL,
        source VARCHAR(4096) NOT NULL
    );
)";

constexpr const char *TMPL_INSERT_PROFILE = R"(INSERT INTO {}.profiles (profile_id, name, source) VALUES ({},'{}','{}'))";
constexpr const char *TMPL_INSERT_ENTITY = R"(INSERT INTO {}.entities (profile_id, serial, common_name, type, country, state, location, organisation, email, keypath, csrpath, crtpath, status, creation_date, expiration_date)
VALUES ({},{},'{}','{}','{}','{}','{}','{}','{}','{}','{}','{}','{}','{}','{}'))";

using namespace gpkih;

int mysql::exportDB(std::string_view outDir, std::vector<std::string> &args){
    std::string_view user, pass, host;
    std::string_view db = "gpkih";
    uint32_t port = 3306;

    for(int i = 0; i < args.size(); ++i){
        const std::string &opt = args[i];
        if(opt == "-h" || opt == "--host"){
            host = args[++i];
        }else if(opt == "-u" || opt == "--user"){
            user = args[++i];
        }else if(opt == "-pass" || opt == "--password"){
            pass = args[++i];
        }else if(opt == "-db" || opt == "--database"){
            db = args[++i];
        }else if(opt == "-p" || opt == "--port"){
            port = std::stoi(args[++i]);
        }
    }

    if(user.empty()){
        PERROR("-u | --user option is mandatory\n");
        return GPKIH_FAIL;
    }else if(pass.empty()){
        PERROR("-pass | --password option is mandatory\n");
        return GPKIH_FAIL;
    }else if(host.empty()){
        PERROR("-h | --host option is mandatory\n");
        return GPKIH_FAIL;
    }

    try{
        MySqlConnection connection(host, user, pass, port);
        /* BEG - create database + tables */
        std::string databaseSTMT = std::string{"CREATE DATABASE "} += db;
        if(connection.exec(databaseSTMT) != GPKIH_OK
        || connection.exec(fmt::format(STMT_CREATE_PROFILE_TABLE, db)) != GPKIH_OK  
        || connection.exec(fmt::format(STMT_CREATE_ENTITY_TABLE, db)) != GPKIH_OK){
            PERROR(connection.getError());
            return GPKIH_FAIL;            
        }
        /* END - create database + tables */

        /* BEG - Insert gpkih data into database */
        auto profiles = db::profiles::get();
        
        for(const auto &[profileName, profile] : *profiles){
            if(connection.exec(fmt::format(TMPL_INSERT_PROFILE, db, profile.meta.id, profile.name, profile.source)) != GPKIH_OK){
                PERROR(connection.getError());
                return GPKIH_FAIL;
            };
            EntityManager entities(profileName);
            for(const auto &[cn, entity] : *entities.retrieve()){
                if(connection.exec(fmt::format(TMPL_INSERT_ENTITY, db, profile.meta.id, entity.meta.serial, entity.subject.cn, 
                    entity::conversion::toString(entity.meta.type), entity.subject.country, entity.subject.state, entity.subject.location, entity.subject.organisation,
                    entity.subject.email, entity.keyPath, entity.csrPath, entity.crtPath, entity::conversion::toString(entity.meta.status), fmt::format("{:%d-%m-%Y @ %H:%M}",entity.meta.creationDate), fmt::format("{:%d-%m-%Y @ %H:%M}",entity.meta.expirationDate))
                ) != GPKIH_OK){
                    PERROR(connection.getError());
                    return GPKIH_FAIL;
                };
            }
        }
        /* END - Insert gpkih data into database */
    }catch(const std::exception &ex){
        PERROR(ex.what());
        return GPKIH_FAIL;
    }
    
    return GPKIH_OK;
};