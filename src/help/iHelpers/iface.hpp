#pragma once
#include <fmt/format.h>

class IHelper{
public:
	IHelper() = default;
	virtual ~IHelper() = default;
	virtual void usage(bool brief = false) = 0;
};