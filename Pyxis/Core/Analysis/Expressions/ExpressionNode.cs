using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using ApplicationUtility;

namespace Pyxis.Core.Analysis.Expressions
{
    public enum ExpressionNodeType
    {
        Caclulation,
        ComplexFunction,
        Reference
    }

    public class ExpressionNode
    {
        private readonly ExpressionContext m_context;

        public string OriginalExpression { get; private set; }
        public string Expression { get; set; }
        public ExpressionNodeType Type { get; set; }
        public List<ExpressionNode> SubExpressions { get; set; }

        public IEnumerable<string> References
        {
            get
            {
                if (Type == ExpressionNodeType.Reference)
                {
                    yield return Expression;
                }
                foreach (var reference in SubExpressions.SelectMany(sub => sub.References))
                {
                    yield return reference;
                }
            }
        }

        public ExpressionNode(ExpressionContext context, string expression, ExpressionNodeType type)
        {
            m_context = context;
            OriginalExpression = Expression = expression;
            Type = type;
            SubExpressions = new List<ExpressionNode>();
        }

        private string DetectComplexFunctionName(int pos)
        {
            while (pos > 0 && Expression[pos] == ' ')
            {
                pos--;
            }

            if (pos <= 0)
            {
                return null;
            }

            foreach (var name in m_context.ComplexFunctions.Keys)
            {
                if (pos >= name.Length - 1 && Expression.Substring(pos - (name.Length - 1), name.Length) == name)
                {
                    return name;
                }
            }

            return null;
        }

        private ExpressionNode FindComplexFunctionCall()
        {
            var tokenStart = new Stack<int>();

            int functionDepth = -1;
            var insideRefToken = false;

            for (var i = 0; i < Expression.Length; i++)
            {
                var c = Expression[i];
                switch (c)
                {
                    case '(':
                        if (insideRefToken)
                        {
                            break;
                        }

                        tokenStart.Push(i);


                        if (functionDepth == -1)
                        {
                            var functionName = DetectComplexFunctionName(i - 1);
                            if (functionName.HasContent())
                            {
                                functionDepth = tokenStart.Count;
                            }
                        }
                        break;
                    case ')':
                        if (insideRefToken)
                        {
                            break;
                        }
                        if (tokenStart.Count == 0)
                        {
                            throw new Exception("Expression has to many ')'");
                        }


                        if (functionDepth == tokenStart.Count)
                        {
                            var start = tokenStart.Pop();
                            var functionName = DetectComplexFunctionName(start - 1);
                            start = Expression.LastIndexOf(functionName, start - 1, StringComparison.OrdinalIgnoreCase);

                            var result = new ExpressionNode(m_context,
                                Expression.Substring(start, i - start + 1),
                                ExpressionNodeType.ComplexFunction);

                            return result;
                        }
                        else
                        {
                            tokenStart.Pop();
                        }
                        break;
                    case '[':
                        insideRefToken = true;
                        break;
                    case ']':
                        insideRefToken = false;
                        break;
                }
            }
            return null;
        }

        private bool IsSimpleReferenceExpression()
        {
            return Regex.IsMatch(Expression.Trim(), @"^[a-z]\d+$");
        }

        public void ReduceExpression()
        {
            var index = 0;
            switch (Type)
            {
                case ExpressionNodeType.Caclulation:
                    var complexFunctionCall = FindComplexFunctionCall();

                    while (complexFunctionCall != null)
                    {
                        SubExpressions.Add(complexFunctionCall);
                        Expression = Expression.Replace(complexFunctionCall.Expression, GenerateVariableName(index));
                        index++;
                        complexFunctionCall = FindComplexFunctionCall();
                    }

                    SubExpressions.ForEach(x => x.ReduceExpression());

                    ResolveReferences();

                    if (IsSimpleReferenceExpression())
                    {
                        if (SubExpressions.Count == 1)
                        {
                            var target = SubExpressions[0];
                            Type = target.Type;
                            Expression = target.Expression;
                            SubExpressions = target.SubExpressions;
                        }
                        else
                        {
                            Type = ExpressionNodeType.Reference;
                        }
                    }
                    break;

                case ExpressionNodeType.ComplexFunction:
                    SubExpressions = ExtractFunctionArguments();

                    foreach (var expression in SubExpressions)
                    {
                        Expression = Expression.Replace(expression.Expression, GenerateVariableName(index));
                        index++;
                    }

                    SubExpressions.ForEach(x => x.ReduceExpression());
                    break;

                case ExpressionNodeType.Reference:
                    break;
            }

            ResolveReferences();
        }

        private string GenerateVariableName(int variableIndex, int fieldIndex = 0)
        {
            return string.Format("{0}{1}", (char) ('a' + variableIndex), fieldIndex);
        }

        /// <summary>
        /// Resolve GeoSource References and convert them into a valid calculation expressions.
        /// </summary>
        private void ResolveReferences()
        {
            if (Type == ExpressionNodeType.Reference)
            {
                //expresion is already a reference
                return;
            }

            var reference = ExtractReference();

            while (reference != null)
            {
                Expression = Expression.Replace(reference.Expression, GenerateVariableName(SubExpressions.Count));
                SubExpressions.Add(reference);
                reference = ExtractReference();
            }
        }

        /// <summary>
        /// Extract reference sub-expression from Expression. 
        /// Reference synatx is "[field@geoSourceIdOrName]".
        /// "field@" is optional. if omited, the first field in the geo-source will be selected.
        /// geoSourceIdOrName can be the geosource id, id:version, or substring of the name.
        /// In case it a name of a geo-source, the parser will try to match it to the given list of geosources
        /// </summary>
        /// <returns></returns>
        private ExpressionNode ExtractReference()
        {
            var start = Expression.IndexOf('[');

            if (start == -1)
            {
                return null;
            }

            var end = Expression.IndexOf(']', start);

            if (end == -1)
            {
                throw new Exception("reference expression is not closed: " + Expression.Substring(start));
            }

            var result = new ExpressionNode(m_context,
                Expression.Substring(start, end - start + 1), 
                ExpressionNodeType.Reference);

            return result;
        }

        private List<ExpressionNode> ExtractFunctionArguments()
        {
            var arguments = new List<ExpressionNode>();

            var tokenStart = new Stack<int>();

            for (var i = 0; i < Expression.Length; i++)
            {
                var c = Expression[i];
                switch (c)
                {
                    case '(':
                    case '[':
                    case '{':
                        tokenStart.Push(i);
                        break;
                    case ')':
                    case ']':
                    case '}':
                        {
                            var start = tokenStart.Pop();
                            if (tokenStart.Count == 0)
                            {
                                arguments.Add(new ExpressionNode(m_context,
                                    Expression.Substring(start + 1, i - start - 1),
                                    ExpressionNodeType.Caclulation));
                            }
                        }
                        break;
                    case ',':
                        if (tokenStart.Count == 1)
                        {
                            var start = tokenStart.Pop();

                            arguments.Add(new ExpressionNode(m_context,
                                Expression.Substring(start + 1, i - start - 1),
                                ExpressionNodeType.Caclulation));

                            tokenStart.Push(i);
                        }
                        break;
                }
            }
            return arguments;
        }
    }
}