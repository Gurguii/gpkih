#include "../iface.hpp"

namespace gpkih::help::iHelper
{
	class set : public IHelper
	{
	public:
		set() = default;
		void usage(bool brief = false){
			fmt::print(R"(== set ==
PARTIALLY IMPLEMENTED
set generic/profile-specific configuration

[ syntax ]
  ./gpkih set <profile> @vpn|pki.<section> key1=nval1 [...] @vpn|pki.<section> key1=nval1 [...] 
  ./gpkih set @<section> key1=nval2 [...] @<section> key1=nval1 [...]

note: when not giving a profile, 'gpkih' is assumed as the context so valid
sections will be sections in gpkih.conf.

[ subopts ]
	** no subopts **

[ examples ]
	./gpkih set @behaviour autoanswer=no headers=yes
	./gpkih set myProfile @pki.output create_pfx=no create_inline=yes
	./gpkih set myProfile @vpn.client remote="192.168.1.14 9999" @vpn.common cipher=AES-256-GCM
)");
		}
	};
} // namespace