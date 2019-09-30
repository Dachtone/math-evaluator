#include "mathevaluator.h"

#include <cctype>
#include <clocale>
#include <string>
#include <sstream>
#include <stack>
#include <queue>

#include "internals.h"

static bool isNumber(unsigned char token);
static bool isDelimiter(unsigned char token);
static bool isWhitespace(unsigned char token);
static bool isArgumentSeparator(unsigned char token);
static bool isArbitraryChar(unsigned char token);
static bool isNegation(unsigned char token);

void MathExpressions::Result::SetResult(MathInternals::NumberType result)
{
	m_bError = false;
	m_result = result;
}

/*
MathInternals::NumberType MathExpressions::Result::Get()
{
	if (m_bError)
		return 0;

	return m_result;
}
*/

std::string MathExpressions::Result::GetString(std::size_t precision)
{
	std::string res;

	if (m_bError)
	{
		res = "Error";
	}
	else
	{
		std::ostringstream ss;
		ss.precision(precision);
		ss << m_result;
		res = ss.str();
	}

	return res;
}

std::ostream& MathExpressions::operator<<(std::ostream &os, const MathExpressions::Result& obj)
{
	if (obj.m_bError)
		os << "Error";
	else
		os << obj.m_result;

	return os;
}

MathExpressions::Result MathExpressions::Evaluate(std::string input, MathInternals::State *state)
{
	if (input.size() == 0)
		return MathExpressions::Result();

	// Fraction delimiter should be a dot
	// Save the current locale and set it to the "C" locale
	char *lastLocale;
	{
		const char *pLastLocale = std::setlocale(LC_NUMERIC, nullptr);
		std::size_t length = std::strlen(pLastLocale) + 1;
		lastLocale = new char[length];
		strcpy_s(lastLocale, length, pLastLocale);
	}
	std::setlocale(LC_NUMERIC, "C");

/*
	const std::size_t opsLength = MathInternals::g_vOperators.size();
	const std::size_t constsLength = MathInternals::g_vConstants.size();
*/

	MathExpressions::Result res;

	bool bMalformed = false;

	// Rewrite the expression in reverse Polish notation
	// Shunting-yard algorithm

	std::stack<MathInternals::Operator*> ops;
	std::queue<MathInternals::Token*> output;

	std::size_t offset = 0;
	bool inNumber = false;
	bool isFractional = false;
	bool inCharSequence = false;
	bool lastOperator = true;
	std::stack<MathInternals::NumberType> negations;
	bool assigningVar = false;
	
	const std::size_t length = input.length();
	unsigned char token;
	for (std::size_t n = 0; n < length; n++)
	{
		token = input[n];

		if (inNumber)
		{
			if (!isNumber(token))
			{
				if (isDelimiter(token))
				{
					if (isFractional)
					{
						// Cannot have multiple fraction delimiters inside the same number
						bMalformed = true;
						break;
					}

					isFractional = true;
					continue;
				}

				inNumber = false;
				isFractional = false;
				lastOperator = false;
				
				MathInternals::NumberType value;
				try
				{
					value = static_cast<MathInternals::NumberType>(std::stod(input.substr(offset, n - offset)));
				}
				catch (...)
				{
					bMalformed = true;
					break;
				}

				MathInternals::Operand *arg = new MathInternals::Operand(value);
				output.push(arg);

				if (!negations.empty() && negations.top() == 1)
				{
					output.push(&MathInternals::g_negation);
					negations.pop();
				}

				offset = n;

				goto skip_number_check;
			}
			else
			{
				continue;
			}
		}

		if (isNumber(token))
		{
			if (assigningVar)
			{
				bMalformed = true;
				break;
			}

			inNumber = true;
			offset = n;
			continue;
		}

		if (!inCharSequence)
			offset = n;

skip_number_check:
		if (isWhitespace(token) || isArgumentSeparator(token))
		{
			if (inCharSequence)
			{
				// Sequence interrupted, no match found
				bMalformed = true;
				break;
			}

			if (isArgumentSeparator(token))
				lastOperator = true;

			offset = n;
			continue;
		}

		// Assumes ASCII
		// ToDo: Implement other encodings
//		if (assigningVar)
//		{
		if (token == '=')
		{
			lastOperator = true;
			assigningVar = false;

			while (!ops.empty() && (
				(ops.top()->GetPrecedence() > MathInternals::g_assignment.GetPrecedence()) ||
				((ops.top()->GetPrecedence() == MathInternals::g_assignment.GetPrecedence()) && ops.top()->IsLeftAssociate())
			))
			{
				output.push(ops.top());
				ops.pop();
			}
			ops.push(&MathInternals::g_assignment);

			// output.push(&MathInternals::g_assignment);

			offset = n;
			continue;
		}
/*
		else
		{
			bMalformed = true;
			break;
		}
*/
//		}
		else if (token == '(')
		{
			ops.push(&MathInternals::g_leftParen);

			inCharSequence = false;
			lastOperator = true;

			if (!negations.empty())
				negations.top()++;

			offset = n;
			continue;
		}
		else if (token == ')')
		{
			bool bFoundParen = false;
			while (!ops.empty() && ops.top()->GetName() != "(")
			{
				output.push(ops.top());
				ops.pop();
			}
			if (ops.empty() || ops.top()->GetName() != "(")
			{
				// Non-matching parentheses
				bMalformed = true;
				break;
			}
			ops.pop();

			if (!negations.empty())
			{
				if (negations.top() == 2)
				{
					// Warning: Might be incorrect, not a part of the algorithm
					if (!ops.empty() && ops.top()->GetPrecedence() == MathInternals::FunctionPrecedence)
					{
						output.push(ops.top());
						ops.pop();
					}

					output.push(&MathInternals::g_negation);
					negations.pop();
				}
				else if (negations.top() > 0)
				{
					negations.top()--;
				}
			}

			inCharSequence = false;
			// lastOperator = true;

			offset = n;
			continue;
		}

		// Not a number, searching for matching character sequences

		if (!inCharSequence)
		{
			inCharSequence = true;
			offset = n;
		}

		// ToDo: Optimize
		if (isArbitraryChar(token) && (n != (length - 1)) && isArbitraryChar(input[n + 1]))
			continue;

		if (lastOperator && isNegation(token))
		{
			negations.push(1);
			inCharSequence = false;
			offset = n;
			continue;
		}

		std::string_view substring(input.c_str() + offset, n - offset + 1);

		MathInternals::Operator* opMatch = nullptr;
		for (MathInternals::Operator& op : MathInternals::g_vOperators)
		{
			if (op.GetName() == substring)
			{
				opMatch = &op;
				inCharSequence = false;
				lastOperator = true;
				break;
			}
		}

		if (opMatch == nullptr)
		{
			MathInternals::Token* variableMatch = nullptr;
			for (std::pair<std::string, MathInternals::NumberType>& item : MathInternals::g_vConstants)
			{
				if (item.first == substring)
				{
					variableMatch = new MathInternals::Operand(item.second);
					output.push(variableMatch);

					inCharSequence = false;
					lastOperator = false;

					if (!negations.empty() && negations.top() == 1)
					{
						output.push(&MathInternals::g_negation);
						negations.pop();
					}

					offset = n;
					break;
				}
			}

			if (variableMatch != nullptr)
				continue;

			// ToDo: Refactor code 
			if (state != nullptr && !state->empty())
			{
				for (std::pair<std::string, MathInternals::NumberType>& item : *state)
				{
					if (item.first == substring)
					{
						variableMatch = new MathInternals::Variable(substring, item.second);
						output.push(variableMatch);

						inCharSequence = false;
						lastOperator = false;

						if (!negations.empty() && negations.top() == 1)
						{
							output.push(&MathInternals::g_negation);
							negations.pop();
						}

						offset = n;
						break;
					}
				}
			}

			if (variableMatch == nullptr)
			{
				inCharSequence = false;
				assigningVar = true;

				MathInternals::Variable *var = new MathInternals::Variable(substring);
				output.push(var);
			}

			continue;
		}

		while (!ops.empty() && (
			(ops.top()->GetPrecedence() > opMatch->GetPrecedence()) ||
			((ops.top()->GetPrecedence() == opMatch->GetPrecedence()) && ops.top()->IsLeftAssociate())
		))
		{
			output.push(ops.top());
			ops.pop();
		}
		ops.push(opMatch);

		offset = n;
	}

	if (bMalformed)
		goto postfix_done;

	// ToDo: Refactor code duplication
	if (inNumber)
	{
		if (!isNumber(token))
		{
			bMalformed = true;
			goto postfix_done;
		}

		MathInternals::NumberType value;
		try
		{
			value = static_cast<MathInternals::NumberType>(std::stod(input.substr(offset, length - offset + 1)));

			MathInternals::Operand *arg = new MathInternals::Operand(value);
			output.push(arg);

			if (!negations.empty() && negations.top() == 1)
			{
				output.push(&MathInternals::g_negation);
				negations.pop();
			}
		}
		catch (...)
		{
			bMalformed = true;
		}
	}

	if (inCharSequence || !negations.empty() || assigningVar)
	{
		bMalformed = true;
		goto postfix_done;
	}

postfix_done:
	{
		MathInternals::Operator *op;
		while (!ops.empty())
		{
			op = ops.top();
			if (op == &MathInternals::g_leftParen)
				bMalformed = true;
			if (!bMalformed)
				output.push(op);
			ops.pop();
		}
	}

	// Reverse Polish evaluation
	std::stack<MathInternals::Operand*> evalStack;

	if (bMalformed)
		goto function_end;

	while (output.size() != 0)
	{
		union
		{
			MathInternals::Token *tk;
			MathInternals::Operator *op;
			MathInternals::Operand *arg;
			MathInternals::Variable *var;
		};
		tk = output.front();
		
		if (tk->IsOperator())
		{
			op = static_cast<MathInternals::Operator*>(tk);

			if (op == &MathInternals::g_assignment)
			{
				if (2u > evalStack.size())
				{
					bMalformed = true;
					goto function_end;
				}

				MathInternals::Operand *operand = evalStack.top();
				/*
				if (operand->IsVariable())
				{
					bMalformed = true;
					goto function_end;
				}
				*/
				evalStack.pop();

				MathInternals::Operand *variable = evalStack.top();
				if (!variable->IsVariable())
				{
					bMalformed = true;
					goto function_end;
				}
				evalStack.pop();
				std::string name = static_cast<MathInternals::Variable*>(variable)->GetName();
				delete variable;

				MathInternals::Operand *evaluated = new MathInternals::Operand(MathInternals::g_assignment.Evaluate(*state, name, operand).GetValue());
				evalStack.push(evaluated);
			}
			else
			{
				std::stack<MathInternals::Operand*> args;
				uint8_t numArgs = op->GetNumOperands();
				if (numArgs > evalStack.size())
				{
					bMalformed = true;
					goto function_end;
				}

				for (uint8_t i = 0; i < numArgs; i++)
				{
					if (evalStack.top()->IsVariable())
					{
						MathInternals::Variable* var = static_cast<MathInternals::Variable*>(evalStack.top());
						if (!var->IsInitialized())
						{
							bMalformed = true;
							delete var;
							goto function_end;
						}

						args.push(new MathInternals::Operand(var->GetValue()));
						delete var;
					}
					else
					{
						args.push(evalStack.top());
					}
					evalStack.pop();
				}
				evalStack.push(new MathInternals::Operand(op->Evaluate(args).GetValue()));
			}
		}
		else
		{
			arg = static_cast<MathInternals::Operand*>(tk);
			evalStack.push(arg);
		}

		output.pop();
	}
	
	if (evalStack.size() != 1)
	{
		bMalformed = true;

		MathInternals::Operand* arg;
		do
		{
			arg = evalStack.top();
			delete arg;

			evalStack.pop();
		}
		while (evalStack.size() != 0);

		goto function_end;
	}

	// Set the result value
	{
		MathInternals::Operand* arg;
		arg = evalStack.top();

		if (arg->IsVariable())
		{
			MathInternals::Variable* var = static_cast<MathInternals::Variable*>(evalStack.top());
			if (!var->IsInitialized())
			{
				bMalformed = true;
				delete var;
				goto function_end;
			}

			res.SetResult(var->GetValue());
			delete var;
		}
		else
		{
			res.SetResult(arg->GetValue());
			delete arg;
		}
	}

function_end:
	// Free all allocated memory
	{
		MathInternals::Token* tk;

		while (output.size() != 0)
		{
			tk = output.front();
			if (!tk->IsOperator())
				delete tk;

			output.pop();
		}
	}

	// Restore the original locale
	std::setlocale(LC_NUMERIC, lastLocale);
	delete[] lastLocale;

	return res;
}

static bool isNumber(unsigned char token)
{
	// Assumes ASCII
	// ToDo: Implement other encodings

	// return token >= 0x30 && token < 0x40;
	return std::isdigit(token);
}

static bool isDelimiter(unsigned char token)
{
	// Assumes ASCII
	// ToDo: Implement other encodings

	// static unsigned char delimiter = std::use_facet<std::numpunct<char>>(std::locale()).decimal_point();
	// return token == delimiter;
	return token == '.';
}

static bool isWhitespace(unsigned char token)
{
	// Assumes ASCII
	// ToDo: Implement other encodings

	return std::isspace(token);
}

static bool isArgumentSeparator(unsigned char token)
{
	// Assumes ASCII
	// ToDo: Implement other encodings

	return token == ',';
}

static bool isArbitraryChar(unsigned char token)
{
	// Assumes ASCII
	// ToDo: Implement other encodings
	static char aPredefinedChars[] =
	{
		'(',
		')',
		'+',
		'-',
		'*',
		'/',
		'^',
		'%'
	};

	for (char tk : aPredefinedChars)
	{
		if (token == tk)
			return false;
	}

	return !isNumber(token) && !isDelimiter(token) && !isWhitespace(token) && !isArgumentSeparator(token);
}

static bool isNegation(unsigned char token)
{
	// Assumes ASCII
	// ToDo: Implement other encodings

	return token == '-';
}