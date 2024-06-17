#pragma once
#include <fmt/format.h>
#include <fmt/core.h>
#include <map>
//#include "../config/config_management.hpp"

/* Interface for action helpers */
namespace gpkih::help {

class IHelper{
public:
	IHelper() = default;
	virtual void usage(bool brief) = 0;
};

class BuildHelper : public IHelper{
	public:
		BuildHelper() = default;
		void usage(bool brief = false) override;
	};

class InitHelper : public IHelper{
	public:
		InitHelper() = default;
		void usage(bool brief = false) override;
};

class RenameHelper : public IHelper{
	public:
		RenameHelper() = default;
		void usage(bool brief = false) override;
};

class RevokeHelper : public IHelper{
	public:
		RevokeHelper() = default;
		void usage(bool brief = false) override;
};

class GencrlHelper : public IHelper{
	public:
		GencrlHelper() = default;
		void usage(bool brief = false) override;
};

class ListHelper : public IHelper{
	public:
		ListHelper() = default;
		void usage(bool brief = false) override;
};

class RemoveHelper : public IHelper{
	public:
		RemoveHelper() = default;
		void usage(bool brief = false) override;
};

class ResetHelper : public IHelper{
	public:
		ResetHelper() = default;
		void usage(bool brief = false) override;
};

class GetHelper : public IHelper{
	public:
		GetHelper() = default;
		void usage(bool brief = false) override;
};

class SetHelper : public IHelper{
	public:
		SetHelper() = default;
		void usage(bool brief = false) override;
};

extern const std::map<std::string, IHelper> &HelperIfaces();

}