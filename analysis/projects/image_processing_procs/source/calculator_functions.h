#ifndef CALCULATOR_FUNCTIONS_H
#define CALCULATOR_FUNCTIONS_H

/******************************************************************************
calculator_functions.h

begin		: 2013-12-20
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "../../third_party/expreval34/expreval.h"
#include "../../third_party/expreval34/funclist.h"
#include <math.h>

class Dist_FunctionNode : public ExprEval::FunctionNode
{
public:
	Dist_FunctionNode::Dist_FunctionNode(ExprEval::Expression *expr) : FunctionNode(expr)
	{
		SetArgumentCount(4, 4, 0, 0);
	}
	double DoEvaluate();
};

template<class T>
class FunctionFactory : public ExprEval::FunctionFactory
{
public:
	FunctionFactory(std::string name) :  ExprEval::FunctionFactory(), m_name(name)
	{
	}
	std::string GetName() const
	{
		return m_name;
	}

	ExprEval::FunctionNode *DoCreate(ExprEval::Expression *expr)
	{
		return new T(expr);
	}
private:
	std::string m_name;
};

#endif // guard
