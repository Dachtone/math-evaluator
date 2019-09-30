#pragma once

#include <string>
#include <vector>

namespace MathInternals
{

	// Primitive data type used internally to represent a number
	// ToDo: Fix precision loss when using types greater than double
	// Implementation specific, should not be relied upon
	// Warning: 8-bit types are treated as characters in GetString()
	using NumberType = double;

	// Default precision of output in GetString()
	constexpr std::size_t OutputPrecision = 12u;

	// Type used to represent a state
	// ToDo: Store defined functions
	using State = std::vector<std::pair<std::string, MathInternals::NumberType>>;

}

namespace MathExpressions
{

	class Result
	{

	public:
		Result()
			: m_bError(true)
		{
		}

		Result(MathInternals::NumberType output)
			: m_result(output), m_bError(false)
		{
		}

		void SetResult(MathInternals::NumberType result);

		bool Error() { return m_bError; }

		operator bool() { return Error(); }

		template<typename T = MathInternals::NumberType>
		T Get()
		{
			if (m_bError)
				return 0;

			return static_cast<T>(m_result);
		}

		std::string GetString(std::size_t precision = MathInternals::OutputPrecision);

		// operator MathInternals::NumberType() { return Get(); }

		friend std::ostream& operator<<(std::ostream &os, const Result& obj);

	private:
		bool m_bError = false;
		MathInternals::NumberType m_result;

	};

	Result Evaluate(std::string expression, MathInternals::State *state = nullptr);

	class State
	{

	public:
		State()
			: m_state({ })
		{
		}

		template<typename T>
		void AddVariable(std::string name, T value)
		{
			m_state.push_back({ name, static_cast<MathInternals::NumberType>(value) });
		}

		Result Evaluate(std::string expression) { return MathExpressions::Evaluate(expression, &m_state); }

	private:
		MathInternals::State m_state;

	};

}