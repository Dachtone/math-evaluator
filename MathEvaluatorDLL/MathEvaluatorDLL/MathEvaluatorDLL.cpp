// MathEvaluatorDLL.cpp : Defines the exported functions for the DLL application.
//

#include "exports.h"

#include <map>

#include "math/mathevaluator.h"

std::map<int, MathExpressions::State> g_mStates;

int evaluate(const char *expression, char *result, int length)
{
	MathExpressions::Result res = MathExpressions::Evaluate(expression);
	if (res.Error())
	{
		if (length > 0)
			result[0] = '\0';

		return 0;
	}

	if (length > 0)
		strncpy_s(result, length, res.GetString().c_str(), _TRUNCATE);

	return 1;
}

int evaluate_state(const char *expression, char *result, int length, int id)
{
	MathExpressions::Result res = g_mStates[id].Evaluate(expression);
	if (res.Error())
	{
		if (length > 0)
			result[0] = '\0';

		return 0;
	}

	if (length > 0)
		strncpy_s(result, length, res.GetString().c_str(), _TRUNCATE);

	return 1;
}