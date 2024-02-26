#include "formatter.hpp"
#include "formatter.cpp"
#include <iostream>

using namespace gpkih::experimental;

int main()
{
	Formatter::FormatInfo i{10,C_ALLIGN};
	Formatter formatter(i);
	formatter.printv("hola","gurguito","que","tal");
}