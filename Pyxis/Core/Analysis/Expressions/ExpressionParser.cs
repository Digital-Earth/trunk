using System;
using System.Collections.Generic;
using System.Drawing.Text;
using System.Linq;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis.Expressions
{
    public class CompiledGeoSource
    {
        public bool Published { get; set; }
        public GeoSource GeoSource { get; set; }
        public string FieldName { get; set; }

        public CompiledGeoSource(GeoSource geoSource)
        {
            GeoSource = geoSource;
            Published = false;
        }

        public CompiledGeoSource(GeoSource geoSource, string fieldName)
        {
            GeoSource = geoSource;
            FieldName = fieldName;
            Published = false;
        }

        public CompiledGeoSource(GeoSource geoSource, string fieldName, bool published)
        {
            GeoSource = geoSource;
            FieldName = fieldName;
            Published = published;
        }

        public static implicit operator CompiledGeoSource(GeoSource geoSource)
        {
            return new CompiledGeoSource(geoSource);
        }

        public static implicit operator GeoSource(CompiledGeoSource compiledGeoSource)
        {
            return compiledGeoSource.GeoSource;
        }
    }

    public interface IExpressionNodeCompiler
    {
        CompiledGeoSource Compile(ExpressionNode node, ExpressionContext context, string outputType);
    }

    public class ExpressionParser
    {
        public string Expression { get; private set; }
        public ExpressionContext Context { get; private set; }
        
        public ExpressionNode ExpressionTree { get; private set; }
        
        public ExpressionParser(string expression, ExpressionContext context)
        {
            Expression = expression;
            Context = context;
        }

        public bool Parse()
        {
            ExpressionTree = new ExpressionNode(Context, Expression, ExpressionNodeType.Caclulation);

            ExpressionTree.ReduceExpression();

            return true;
        }
    }
}