#include "internals.h"

#include <cmath>
#include <vector>
#include <random>
#include <functional>

static MathInternals::NumberType PopStack(MathInternals::OperandStack &stack);

MathInternals::Operator MathInternals::g_leftParen("(", 0u, 0u, false, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
{
	// Return value should be discarded
	return 0;
});

MathInternals::Operator MathInternals::g_negation("-", 1u, 5u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
{
	return -PopStack(args);
});

MathInternals::AssignmentOperator MathInternals::g_assignment("=", [](MathInternals::State &state, std::string name, MathInternals::Operand *operand) -> MathInternals::NumberType
{
	MathInternals::NumberType value = operand->GetValue();
	delete operand;

	bool bFound = false;
	for (std::pair<std::string, MathInternals::NumberType> &variable : state)
	{
		if (variable.first == name)
		{
			bFound = true;
			variable.second = value;
		}
	}
	
	if (!bFound)
		state.push_back({ name, value });

	return value;
});

/*
MathInternals::Operator MathInternals::g_rightParen(")", 0u, 5u, false, [](std::stack<MathInternals::Operand> args)
{
	return MathInternals::Operand();
});
*/

std::vector<MathInternals::Operator> MathInternals::g_vOperators =
{
	// Operator constructor:
	// name, num of args, precedence (5 for functions), is left associate?, action
	// Action:
	// Lambda function, takes in a reference to a stack of arguments, returns an operand
	// All arguments should be popped
	// Order of evaluation is not guaranteed, values that come first in an operation have to be stored

	/* Basic operators */
	MathInternals::Operator("+", 2u, 1u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return PopStack(args) + PopStack(args);
	}),
	MathInternals::Operator("-", 2u, 1u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return arg - PopStack(args);
	}),
	MathInternals::Operator("*", 2u, 2u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return PopStack(args) * PopStack(args);
	}),
	MathInternals::Operator("/", 2u, 2u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return arg / PopStack(args);
	}),
	MathInternals::Operator("^", 2u, 3u, false, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return static_cast<MathInternals::NumberType>(std::pow(arg, PopStack(args)));
	}),
	MathInternals::Operator("%", 2u, 2u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return static_cast<MathInternals::NumberType>(std::fmod(arg, PopStack(args)));
	}),
	MathInternals::Operator("mod", 2u, 2u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return static_cast<MathInternals::NumberType>(std::fmod(arg, PopStack(args)));
	}),

	/* Bitwise operators */
	MathInternals::Operator("&", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) & static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),
	MathInternals::Operator("and", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) & static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),
	MathInternals::Operator("|", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) | static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),
	MathInternals::Operator("or", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) | static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),
	MathInternals::Operator("xor", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) ^ static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),
	MathInternals::Operator("<<", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) << static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),
	MathInternals::Operator(">>", 2u, 0u, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		if (std::floor(arg1) == arg1 && std::floor(arg2) == arg2)
		{
			return static_cast<MathInternals::NumberType>(static_cast<long long int>(arg1) >> static_cast<long long int>(arg2));
		}
		else
		{
			return 0;
		}
	}),

	/* Power and exponentials */
	MathInternals::Operator("pow", 2u, MathInternals::FunctionPrecedence, false, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return static_cast<MathInternals::NumberType>(std::pow(arg, PopStack(args)));
	}),
	MathInternals::Operator("sqrt", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::sqrt(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("exp", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::exp(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("ln", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::log(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("lg", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::log10(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("log2", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::log2(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("log", 2u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg = PopStack(args);
		return static_cast<MathInternals::NumberType>(
			std::log(static_cast<double>(PopStack(args))) / std::log(static_cast<double>(arg))
		);
	}),

	/* Trigonometry */
	MathInternals::Operator("sin", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::sin(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("cos", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::cos(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("tan", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::tan(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("asin", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::asin(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("acos", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::acos(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("atan", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::atan(static_cast<double>(PopStack(args))));
	}),

	/* Number functions */
	MathInternals::Operator("max", 2u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		return arg1 > arg2 ? arg1 : arg2;
	}),
	MathInternals::Operator("min", 2u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);

		return arg1 < arg2 ? arg1 : arg2;
	}),
	MathInternals::Operator("abs", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::abs(PopStack(args)));
	}),
	MathInternals::Operator("round", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::round(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("ceil", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::ceil(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("floor", 1u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		return static_cast<MathInternals::NumberType>(std::floor(static_cast<double>(PopStack(args))));
	}),
	MathInternals::Operator("rand", 2u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());

		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);
		if (arg1 > arg2)
			return 0;

		std::uniform_int_distribution<long long int> dist(static_cast<long long int>(arg1), static_cast<long long int>(arg2));

		return static_cast<MathInternals::NumberType>(dist(rng));
	}),
	MathInternals::Operator("randf", 2u, MathInternals::FunctionPrecedence, true, [](MathInternals::OperandStack &args) -> MathInternals::NumberType
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());

		MathInternals::NumberType arg1 = PopStack(args);
		MathInternals::NumberType arg2 = PopStack(args);
		if (arg1 > arg2)
			return 0;

		std::uniform_real_distribution<double> dist(static_cast<double>(arg1), static_cast<double>(arg2));

		return static_cast<MathInternals::NumberType>(dist(rng));
	})

};

static MathInternals::NumberType PopStack(MathInternals::OperandStack &stack)
{
	MathInternals::NumberType value = stack.top()->GetValue();

	delete stack.top();
	stack.pop();

	return value;
}