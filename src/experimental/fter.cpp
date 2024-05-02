#include "fter.hpp"

void Fter::pentry(Entity &entity, STYLE st){
	Subject &sub = entity.subject;
	
	std::stringstream ss{};
	ss << "{:^10}";
	ss << "{:^" << sub.cnlen << "}";
	ss << "{:^" << sub.statelen << "}";
	ss << "{:^4}";
	ss << "{:^" << sub.emaillen << "}";
	ss << "{:^" << sub.locationlen << "}";
	ss << "{:^" << sub.organisationlen << "}";

	fmt::print("{}{}{}{}{}{}", entity.serial, sub.cn, sub.state, sub.email, sub.location, sub.organisation);
}