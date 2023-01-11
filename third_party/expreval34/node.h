// File:    node.h
// Author:  Brian Vanderburg II
// Purpose: Expression node
//------------------------------------------------------------------------------


#ifndef __EXPREVAL_NODE_H
#define __EXPREVAL_NODE_H

// Includes
#include <vector>

#include "parser.h"

// Part of expreval namespace
namespace ExprEval
    {
    // Forward declarations
    class Expression;
    class FunctionFactory;
    class DataEntry;
    
    // Node class
    //--------------------------------------------------------------------------
    class Node
        {
        public:
            Node(Expression *expr);
            virtual ~Node();
            
            virtual double DoEvaluate() = 0;
            virtual void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0) = 0;
                    
            double Evaluate(); // Calls Expression::TestAbort, then DoEvaluate
            
        protected:
            Expression *m_expr;    
        };
        
    // General function node class
    //--------------------------------------------------------------------------
    class FunctionNode : public Node
        {
        public:
            FunctionNode(Expression *expr);
            ~FunctionNode();
            
            // Parse nodes and references
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
                    
        private:
            // Function factory
            FunctionFactory *m_factory;
            
            // Argument count
            long m_argMin;
            long m_argMax;
            long m_refMin;
            long m_refMax;
            long m_dataMin;
            long m_dataMax;
                                    
        protected:
            // Set argument count (called in derived constructors)
            void SetArgumentCount(long argMin = 0, long argMax = 0,
                    long refMin = 0, long refMax = 0, long dataMin = 0, long dataMax = 0);

            // Function name (using factory)
            ::std::string GetName() const;
        
            // Normal, reference, and data parameters
            ::std::vector<Node*> m_nodes;
            ::std::vector<double*> m_refs;
            ::std::vector<DataEntry*> m_data;
            
        friend class FunctionFactory;
        };
        
    // Mulit-expression node
    //--------------------------------------------------------------------------
    class MultiNode : public Node
        {
        public:
            MultiNode(Expression *expr);
            ~MultiNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
        private:
            ::std::vector<Node*> m_nodes;
        };
        
    // Assign node
    //--------------------------------------------------------------------------
    class AssignNode : public Node
        {
        public:
            AssignNode(Expression *expr);
            ~AssignNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
        private:
            double *m_var;
            Node *m_rhs;
        };
    
	// BinaryOperand Node
    //--------------------------------------------------------------------------
    class BinaryOperandNode : public Node
        {
        public:
            BinaryOperandNode(Expression *expr);
            ~BinaryOperandNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);

		protected:
			virtual double Perform(double left,double right) = 0;
                    
        private:
            Node *m_lhs;
            Node *m_rhs;
        };

    // Add node
    //--------------------------------------------------------------------------
    class AddNode : public BinaryOperandNode
        {
        public:
			AddNode(Expression *expr) : BinaryOperandNode(expr) {};
            
		protected:
			double Perform(double left,double right);
        };
        
    // Subtract node
    //--------------------------------------------------------------------------
    class SubtractNode : public BinaryOperandNode
        {
        public:
			SubtractNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
        };

    // Multiply node
    //--------------------------------------------------------------------------
    class MultiplyNode : public BinaryOperandNode
        {
		public:
			MultiplyNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
        };
        
    // Divide node
    //--------------------------------------------------------------------------
    class DivideNode : public BinaryOperandNode
        {
        public:
			DivideNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
        };

	// Compare Equal node
    //--------------------------------------------------------------------------
    class CompareEqualNode : public BinaryOperandNode
        {
        public:
			CompareEqualNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};

	// Compare Smaller node
    //--------------------------------------------------------------------------
    class CompareSmallerNode : public BinaryOperandNode
        {
        public:
			CompareSmallerNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};

	// Compare Greater node
    //--------------------------------------------------------------------------
    class CompareGreaterNode : public BinaryOperandNode
        {
        public:
			CompareGreaterNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};

	// Compare Smaller Equal node
    //--------------------------------------------------------------------------
    class CompareSmallerEqualNode : public BinaryOperandNode
        {
        public:
			CompareSmallerEqualNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};

	// Compare Greater Equal node
    //--------------------------------------------------------------------------
    class CompareGreaterEqualNode : public BinaryOperandNode
        {
        public:
			CompareGreaterEqualNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};

	// Logical And node
    //--------------------------------------------------------------------------
    class LogicalAndNode : public BinaryOperandNode
        {
        public:
			LogicalAndNode (Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};

	// Logical Or node
    //--------------------------------------------------------------------------
    class LogicalOrNode : public BinaryOperandNode
        {
        public:
			LogicalOrNode(Expression *expr) : BinaryOperandNode(expr) {};

		protected:
			double Perform(double left,double right);
	};
        
    // Negate node
    //--------------------------------------------------------------------------
    class NegateNode : public Node
        {
        public:
            NegateNode(Expression *expr);
            ~NegateNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
        private:
            Node *m_rhs;
        };
        
    // Exponent node
    //--------------------------------------------------------------------------
    class ExponentNode : public Node
        {
        public:
            ExponentNode(Expression *expr);
            ~ExponentNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
        private:
            Node *m_lhs;
            Node *m_rhs;
        };
        
    // Variable node (also used for constants)
    //--------------------------------------------------------------------------
    class VariableNode : public Node
        {
        public:
            VariableNode(Expression *expr);
            ~VariableNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
        private:
            double *m_var;
        };
        
    // Value node
    //--------------------------------------------------------------------------
    class ValueNode : public Node
        {
        public:
            ValueNode(Expression *expr);
            ~ValueNode();
            
            double DoEvaluate();
            void Parse(Parser &parser, Parser::size_type start, Parser::size_type end,
                    Parser::size_type v1 = 0);
                    
        private:
            double m_val;
        };        
        
    } // namespace ExprEval
    
#endif // __EXPREVAL_NODE_H  

