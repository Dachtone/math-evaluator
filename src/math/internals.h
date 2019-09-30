#pragma once

#include <cstdint>
#include <stack>
#include <functional>

#include "mathevaluator.h"

namespace MathInternals
{

	constexpr uint8_t FunctionPrecedence = 5u;

	class Token
	{

	public:
		virtual bool IsOperator() = 0;
		virtual bool IsVariable() = 0;
	
	};

	class Operand : public Token
	{

	public:
		// Warning: To be used for "empty" operand
		Operand()
			: m_value(0)
		{
		}

		Operand(NumberType value)
			: m_value(value)
		{
		}

		virtual bool IsOperator() override { return false; }

		virtual bool IsVariable() override { return false; }

		virtual NumberType GetValue() { return m_value; }

		operator NumberType() { return GetValue(); }

	private:
		// ToDo: Implement integer operands
		NumberType m_value;

	};

	class Variable : public Operand
	{

	public:
		Variable(std::string_view name)
			: Operand(), m_sName(name), m_bInitialized(false)
		{
		}

		Variable(std::string_view name, NumberType value)
			: Operand(), m_sName(name), m_value(value), m_bInitialized(true)
		{
		}

		virtual bool IsVariable() override { return true; }

		std::string GetName() { return m_sName; }

		bool IsInitialized() { return m_bInitialized; }

		NumberType GetValue() { return m_value; }

		operator NumberType() { return GetValue(); }

	private:
		std::string m_sName;
		bool m_bInitialized;
		NumberType m_value;

	};

	using OperandStack = std::stack<Operand*>;
	using OperatorFunction = NumberType(*)(OperandStack&);

	class Operator : public Token
	{

	public:
		Operator(std::string op, uint8_t num, uint8_t precedence, bool leftAssociate, OperatorFunction fn)
			: m_sOperatorName(op), m_numOperands(num), m_nPrecedence(precedence), m_bLeftAssociate(leftAssociate), m_fnOperation(fn)
		{
		}

		Operator()
		{
		}

		virtual bool IsOperator() override { return true; }

		virtual bool IsVariable() override { return false; }

		std::string &GetName() { return m_sOperatorName; }

		uint8_t GetNumOperands() { return m_numOperands; }

		uint8_t GetPrecedence() { return m_nPrecedence; }

		uint8_t IsLeftAssociate() { return m_bLeftAssociate; }

		Operand Evaluate(OperandStack &args) { return Operand(m_fnOperation(args)); }

	private:
		std::string m_sOperatorName;
		uint8_t m_numOperands;
		uint8_t m_nPrecedence;
		bool m_bLeftAssociate;
		OperatorFunction m_fnOperation;

	};

	using AssignmentOperatorFunction = NumberType(*)(MathInternals::State &state, std::string name, Operand *operand);

	class AssignmentOperator : public Operator
	{

	public:
		AssignmentOperator(std::string op, AssignmentOperatorFunction fn)
			: Operator(), m_sOperatorName(op), m_fnOperation(fn)
		{
		}

		/*
		virtual bool IsOperator() override { return true; }

		std::string &GetName() { return m_sOperatorName; }

		uint8_t GetNumOperands() { return m_numOperands; }

		uint8_t GetPrecedence() { return m_nPrecedence; }

		uint8_t IsLeftAssociate() { return m_bLeftAssociate; }
		*/

		Operand Evaluate(MathInternals::State &state, std::string name, Operand *operand) { return Operand(m_fnOperation(state, name, operand)); }

	private:
		std::string m_sOperatorName;
		const uint8_t m_numOperands = 2u;
		const uint8_t m_nPrecedence = FunctionPrecedence;
		const bool m_bLeftAssociate = false;
		AssignmentOperatorFunction m_fnOperation;

	};

	extern Operator g_leftParen;
	extern Operator g_negation;
	extern AssignmentOperator g_assignment;
	// extern Operator g_rightParen;
	extern std::vector<Operator> g_vOperators;
	extern std::vector<std::pair<std::string, NumberType>> g_vConstants;

}