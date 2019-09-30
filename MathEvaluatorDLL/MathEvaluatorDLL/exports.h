#pragma once

#ifdef MATHEVALUATORDLL_EXPORTS
#define MATHEVALUATOR_API __declspec(dllexport)
#else
#define MATHEVALUATOR_API __declspec(dllimport)
#endif

extern "C" MATHEVALUATOR_API int evaluate(const char *expression, char *result, int length);

extern "C" MATHEVALUATOR_API int evaluate_state(const char *expression, char *result, int length, int id);
