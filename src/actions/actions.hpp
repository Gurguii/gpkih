#pragma once
#include <vector>
#include "../profiles/profiles.hpp"
#include "../config/ProfileConfig.hpp"
#include "../db/entities.hpp"
#include "../gpkih.hpp"
#include "../libs/printing/printing.hpp"
#include "../help/help.hpp"

namespace gpkih::actions {
extern int init(std::string_view &profile_name, std::string_view &profile_source);

extern int revoke(Profile &profile, std::vector<std::string> &common_names, std::vector<std::string> &serials);

extern int gencrl(Profile &profile);

extern int list(uint16_t fields);
extern int list(std::string_view profile_name, uint16_t fields);

extern int remove(std::vector<std::string> &profiles_to_remove, int remove_all = 0);

extern int build_ca(Profile &profile, ProfileConfig &pkiconf, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize);
extern int build(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize);

extern int exportdb(std::string_view outPath, std::string_view type);
} // namespace gpkih::actions

namespace gpkih::actions::experimental{

class Action {
public:
	virtual int parse() = 0; 
	virtual Helper helpMessages() = 0;
};

class BuildAction : public Action {
  private:
  	int build_CA();
	int build_SV();
	int build_CL();

  public:
	int parse() override;
	Helper helpMessages() override;
};

}