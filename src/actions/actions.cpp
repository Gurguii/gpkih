#include "actions.hpp"

#include "add/AAdd.hpp"
#include "export/AExport.hpp"
#include "gencrl/AGencrl.hpp"
#include "get/AGet.hpp"
#include "init/AInit.hpp"
#include "list/AList.hpp"
#include "remove/ARemove.hpp"
#include "reset/AReset.hpp"
#include "revoke/ARevoke.hpp"
#include "rename/ARename.hpp"
#include "set/ASet.hpp"
#include "dhparam/dhparam.hpp"

/* 
- Alternative approach -

#include <unordered_map>
std::unordered_map<std::string, const IAction&> _testing{
	{"get",AInit::get()},
	{"new",ANew::get()},
};

*/
const IAction* gpkih::actions::GetAction(std::string_view name)
{
	if(name == "init" || name == "i"){
		return &AInit::get();
	}
	else if(name == "add" || name == "a"){
		return &AAdd::get();
	}
	else if(name == "export" || name == "e"){
		return &AExport::get();	
	}
	else if(name == "revoke" || name == "rev"){
		return &ARevoke::get();;
	}
	else if(name == "list" || name == "l"){
		return &AList::get();
	}
	else if(name == "rename" || name == "ren"){
		return &ARename::get();
	}
	else if(name == "get" || name == "g"){
		return &AGet::get();
	}
	else if(name == "set" || name == "s"){
		return &ASet::get();
	}
	else if(name == "gencrl" || name == "genc"){
		return &AGencrl::get();
	}
	else if(name == "reset" || name == "res"){
		return &AReset::get();
	}
	else if(name == "remove" || name == "rem"){
		return &ARemove::get();
	}
	else if(name == "dhparam" || name == "dh"){
		return &ADHparam::get();
	}
	else{
		return nullptr;
	}
}