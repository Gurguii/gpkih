#include "actions.hpp"

#include "new/ANew.hpp"
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

#include <memory>

std::optional<std::unique_ptr<IAction>> gpkih::actions::GetAction(std::string_view name, std::vector<std::string> &args)
{
	if(name == "init" || name == "i"){
		return std::make_unique<AInit>(args);
	}
	else if(name == "new" || name == "n"){
		return std::make_unique<ANew>(args);
	}
	else if(name == "export" || name == "e"){
		return std::make_unique<AExport>(args);	
	}
	else if(name == "revoke" || name == "rev"){
		return std::make_unique<ARevoke>(args);
	}
	else if(name == "list" || name == "l"){
		return std::make_unique<AList>(args);
	}
	else if(name == "rename" || name == "ren"){
		return std::make_unique<ARename>(args);
	}
	else if(name == "get" || name == "g"){
		return std::make_unique<AGet>(args);
	}
	else if(name == "set" || name == "s"){
		return std::make_unique<ASet>(args);
	}
	else if(name == "gencrl" || name == "genc"){
		return std::make_unique<AGencrl>(args);
	}
	else if(name == "reset" || name == "res"){
		return std::make_unique<AReset>(args);
	}
	else if(name == "remove" || name == "rem"){
		return std::make_unique<ARemove>(args);
	}
	else{
		return {};
	}
}