#include <iostream>
#include <string>

#include "math/mathevaluator.h"

int main(int argc, char *argv[])
{
	MathExpressions::State state;

	std::cout << "Enter a mathematical expression:" << std::endl;

	std::string expression;
	while (std::getline(std::cin, expression))
	{
		if (expression == "exit")
			break;

		MathExpressions::Result res = state.Evaluate(expression);
		if (res.Error())
			std::cout << "> Malformed expression";
		else
			std::cout << "> " << res.GetString();

		std::cout << std::endl << std::endl;
	}

	return 0;
}