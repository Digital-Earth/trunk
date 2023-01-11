using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Core.Analysis.Expressions;

namespace Pyxis.Core.Test.Analysis
{
    [TestFixture]
    public class ExpressionParserTests
    {
        private ExpressionContext Context = new ExpressionContext
        {
            GeoSources = new List<GeoSource>(),
            ComplexFunctions = new Dictionary<string, IExpressionNodeCompiler>()
            {
                {"slope", null},
                {"aspect", null},
                {"transform",null}
            }
        };

        [Test]
        public void SimpleExpression()
        {
            var parser = new ExpressionParser("1+2", Context);

            parser.Parse();

            Assert.AreEqual("1+2",parser.ExpressionTree.Expression);
        }

        [Test]
        public void ExtractComplexFunctionCall()
        {
            var parser = new ExpressionParser("1+slope([gtopo30])", Context);

            parser.Parse();

            Assert.AreEqual("1+a0", parser.ExpressionTree.Expression);
            Assert.AreEqual(1, parser.ExpressionTree.SubExpressions.Count);
            Assert.AreEqual("slope(a0)",parser.ExpressionTree.SubExpressions[0].Expression);
            Assert.AreEqual("[gtopo30]", parser.ExpressionTree.SubExpressions[0].SubExpressions[0].Expression);
            Assert.AreEqual(ExpressionNodeType.Reference, parser.ExpressionTree.SubExpressions[0].SubExpressions[0].Type);
        }

        [Test]
        public void ExtractComplexFunctionArguments()
        {
            var parser = new ExpressionParser("slope([gtopo30]+1,aspect([lidar],12))", Context);

            parser.Parse();

            var complexCall = parser.ExpressionTree;

            Assert.AreEqual(ExpressionNodeType.ComplexFunction, complexCall.Type);
            Assert.AreEqual("slope(a0,b0)", complexCall.Expression);
            Assert.AreEqual("a0+1", complexCall.SubExpressions[0].Expression);
            Assert.AreEqual(ExpressionNodeType.Caclulation, complexCall.SubExpressions[0].Type);
            
            var aspectFunction = complexCall.SubExpressions[1];

            Assert.AreEqual(2, aspectFunction.SubExpressions.Count);
            Assert.AreEqual("aspect(a0,b0)", aspectFunction.Expression);
            Assert.AreEqual(ExpressionNodeType.Reference, aspectFunction.SubExpressions[0].Type);
            Assert.AreEqual("[lidar]", aspectFunction.SubExpressions[0].Expression);
            Assert.AreEqual("12", aspectFunction.SubExpressions[1].Expression);
        }

        [Test]
        public void ExtractComplexFunctionJsonArguments()
        {
            var parser = new ExpressionParser("transform([gtopo30],{0:0,1000:1,2000:2,3000:3},false)", Context);

            parser.Parse();

            var complexCall = parser.ExpressionTree;

            Assert.AreEqual(ExpressionNodeType.ComplexFunction, complexCall.Type);
            Assert.AreEqual("transform(a0,b0,c0)", complexCall.Expression);
            Assert.AreEqual("[gtopo30]", complexCall.SubExpressions[0].Expression);
            Assert.AreEqual(ExpressionNodeType.Reference, complexCall.SubExpressions[0].Type);
            
            Assert.AreEqual("{0:0,1000:1,2000:2,3000:3}", complexCall.SubExpressions[1].Expression);
            Assert.AreEqual("false", complexCall.SubExpressions[2].Expression);
        }

        [Test]
        public void ExtractComplexFunctionButNotSimpleFunctions()
        {
            var parser = new ExpressionParser("avg(slope([gtopo30]),slope([lidar]))", Context);

            parser.Parse();

            var root = parser.ExpressionTree;

            Assert.AreEqual(ExpressionNodeType.Caclulation,root.Type);
            Assert.AreEqual("avg(a0,b0)",root.Expression);
            Assert.AreEqual(2, root.SubExpressions.Count);

            var firstSlope = root.SubExpressions[0];

            Assert.AreEqual(ExpressionNodeType.ComplexFunction, firstSlope.Type);
            Assert.AreEqual("slope(a0)", firstSlope.Expression);
            Assert.AreEqual(ExpressionNodeType.Reference, firstSlope.SubExpressions[0].Type);
            Assert.AreEqual("[gtopo30]", firstSlope.SubExpressions[0].Expression);

            var secondSlope = root.SubExpressions[1];

            Assert.AreEqual(ExpressionNodeType.ComplexFunction, secondSlope.Type);
            Assert.AreEqual("slope(a0)", secondSlope.Expression);
            Assert.AreEqual(ExpressionNodeType.Reference, secondSlope.SubExpressions[0].Type);
            Assert.AreEqual("[lidar]", secondSlope.SubExpressions[0].Expression);
        }
    }
}
