#include "help2.hpp"

using namespace gpkih::help;

const std::map<std::string, IHelper>& HelperIfaces(){
	static std::map<std::string,IHelper> helpMap;
	
	if(helpMap.size()){
		return helpMap;
	}

	helpMap["init"]   = InitHelper();
	helpMap["build"]  = BuildHelper();
	helpMap["revoke"] = RevokeHelper();
	helpMap["gencrl"] = GencrlHelper();
	helpMap["rename"] = RenameHelper();
	helpMap["list"]   = ListHelper();
	helpMap["get"]    = GetHelper();
	helpMap["set"]    = SetHelper();
	helpMap["reset"]  = ResetHelper();
	helpMap["remove"] = RemoveHelper();
	
	return helpMap;
}