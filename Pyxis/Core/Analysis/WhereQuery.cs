using System;
using System.Collections.Generic;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// WhereQuery find where on earth several conditions/geometries exists.
    /// 
    /// The WhereQuery is a series of Operations done over a list of geometries. an Operation can be "Intersection","Subtraction" or "Disjunction".
    /// Geometries can be any supported GeoJsonGeometry or PYXIS Specific geometries like ConditionGeometry.
    /// </summary>
    public class WhereQuery
    {
        /// <summary>
        /// Engine to be used to run the query.
        /// </summary>
        private Engine Engine { get; set; }

        /// <summary>
        /// List of clauses to apply.
        /// </summary>
        private BooleanGeometry Clauses { get; set; }

        private WhereQuery(Engine engine)
        {
            Engine = engine;
            Clauses = new BooleanGeometry();
        }       

        private WhereQuery(WhereQuery other)
        {
            Engine = other.Engine;
            Clauses = new BooleanGeometry(other.Clauses);
        }

        /// <summary>
        /// Create a where query with a initial geometry condition.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="geometry">The initial geometry.</param>
        /// <returns>A new WhereQuery object.</returns>
        public static WhereQuery Create(Engine engine, IGeometry geometry)
        {
            var booleanGeometry = geometry as BooleanGeometry;
            if (booleanGeometry != null)
            {
                return new WhereQuery(engine)
                {
                    Clauses = new BooleanGeometry(booleanGeometry)
                };
            }

            return new WhereQuery(engine).Disjunct(geometry);
        }

        /// <summary>
        /// Create a WhereQuery from list of BooleanGeometry.Clause.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="clauses">List of BooleanGeometry.Clauses.</param>
        /// <returns>A new WhereQuery object.</returns>
        internal static WhereQuery Create(Engine engine, IEnumerable<BooleanGeometry.Clause> clauses)
        {
            var query = new WhereQuery(engine)
            {
                Clauses = new BooleanGeometry
                {
                    Operations = new List<BooleanGeometry.Clause>(clauses)
                }
            };

            //Fix first Operation to be Disjunction (can be fixed from Intersection)
            //If first Operation is Subtraction there is no way to fix it.
            if (query.Clauses.Operations.Count > 0 &&
                query.Clauses.Operations[0].Operation == BooleanGeometry.Operation.Intersection)
            {
                query.Clauses.Operations[0].Operation = BooleanGeometry.Operation.Disjunction;
            }

            return query;
        }

        private WhereQuery AddClause(IGeometry geometry, BooleanGeometry.Operation operation)
        {
            var newQuery = new WhereQuery(this);
            newQuery.Clauses.Operations.Add(new BooleanGeometry.Clause 
            {
                Operation = operation,
                Geometry = geometry
            });
            return newQuery;
        }

        /// <summary>
        /// Perform intersection with given geometry.
        /// </summary>
        /// <param name="geometry">IGeometry to intersect.</param>
        /// <returns>A new WhereQuery</returns>
        public WhereQuery Intersect(IGeometry geometry)
        {
            return AddClause(geometry, BooleanGeometry.Operation.Intersection);
        }

        /// <summary>
        /// Disjunct the given geometry with the where query result.
        /// </summary>
        /// <param name="geometry">IGeometry to disjunct.</param>
        /// <returns>A new WhereQuery.</returns>
        public WhereQuery Disjunct(IGeometry geometry)
        {
            return AddClause(geometry, BooleanGeometry.Operation.Disjunction);
        }

        /// <summary>
        /// Subtract the given geometry from the where query result.
        /// </summary>
        /// <param name="geometry">IGeometry to subtract.</param>
        /// <returns>A new WhereQuery.</returns>
        public WhereQuery Subtract(IGeometry geometry)
        {
            return AddClause(geometry, BooleanGeometry.Operation.Subtraction);
        }

        private PYXWhere_SPtr CreatePYXWhereQuery()
        {
            var pyxQuery = PYXWhere.create();

            foreach (var clause in Clauses.Operations)
            {
                var geometry = clause.Geometry as ConditionGeometry;
                if (geometry != null)
                {
                    var condition = geometry;

                    IProcess_SPtr process = null;

                    if (condition.Resource != null)
                    {
                        process = Engine.GetProcess(condition.Resource);
                    } 
                    else if (condition.Reference != null)
                    {
                        var geoSource = Engine.ResolveReference(condition.Reference);
                        process = Engine.GetProcess(geoSource);
                    }

                    if (process == null)
                    {
                        throw new Exception("failed to initialize resource for condition geometry");
                    }

                    var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
                    var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                    if (coverage.isNotNull())
                    {
                        var pyxCondition = PYXWhereCondition.coverageHasValues(coverage);

                        if (condition.Range != null)
                        {
                            pyxCondition = pyxCondition.range(
                                PYXValueExtensions.CreatePYXValueFrom(condition.Range.Min),
                                PYXValueExtensions.CreatePYXValueFrom(condition.Range.Max));
                        }

                        pyxQuery = AddConditionToPyxQuery(pyxQuery, clause.Operation, 
                            pyxlib.DynamicPointerCast_PYXWhereCondition(pyxCondition));
                    }
                    else if (featureCollection.isNotNull())
                    {
                        var pyxCondition = PYXWhereCondition.featuresHasValues(featureCollection);

                        if (!String.IsNullOrEmpty(condition.Property) && condition.Range != null)
                        {
                            pyxCondition = pyxCondition.field(condition.Property).range(
                                PYXValueExtensions.CreatePYXValueFrom(condition.Range.Min),
                                PYXValueExtensions.CreatePYXValueFrom(condition.Range.Max));
                        }

                        pyxQuery = AddConditionToPyxQuery(pyxQuery, clause.Operation,
                            pyxlib.DynamicPointerCast_PYXWhereCondition(pyxCondition));
                    }
                }
                else //clause.Geometry is ConditionGeometry
                {
                    var pyxCondition = PYXWhereCondition.geometry(Engine.ToPyxGeometry(clause.Geometry));

                    pyxQuery = AddConditionToPyxQuery(pyxQuery, clause.Operation,
                        pyxlib.DynamicPointerCast_PYXWhereCondition(pyxCondition));
                }
            }
            return pyxQuery;
        }


        private static PYXWhere_SPtr AddConditionToPyxQuery(
            PYXWhere_SPtr pyxQuery,
            BooleanGeometry.Operation operation,
            PYXWhereCondition_SPtr pyxCondition)
        {
            switch (operation)
            {
                case BooleanGeometry.Operation.Intersection:
                    return pyxQuery.intersect(pyxCondition);
                case BooleanGeometry.Operation.Disjunction:
                    return pyxQuery.disjunct(pyxCondition);
                case BooleanGeometry.Operation.Subtraction:
                    return pyxQuery.subtract(pyxCondition);                    
                default:
                    throw new Exception("unsupported boolean operation type");
            }            
        }

        /// <summary>
        /// Perform the where query over the given geometry.
        /// </summary>
        /// <param name="geometry">IGeometry to perform the query on.</param>
        /// <returns>IGeometry which represent the result of the where query.</returns>
        public IGeometry On(IGeometry geometry)
        {
            var pyxQuery = CreatePYXWhereQuery();

            var result = pyxQuery.on(Engine.ToPyxGeometry(geometry));

            if (result.isNull() || result.isEmpty())
            {
                return null;
            }
            return Geometry.FromPYXGeometry(result);
        }

        /// <summary>
        /// Perform the where query over the given geometry on a specific resolution.
        /// </summary>
        /// <param name="geometry">IGeometry to perform the query on.</param>
        /// <param name="resolution">Resolution to execute query on.</param>
        /// <returns>IGeometry which represent the result of the where query.</returns>
        public IGeometry On(IGeometry geometry, int resolution)
        {
            var pyxQuery = CreatePYXWhereQuery();

            var result = pyxQuery.on(Engine.ToPyxGeometry(geometry, resolution), resolution);

            if (result.isNull() || result.isEmpty())
            {
                return null;
            }
            return Geometry.FromPYXGeometry(result);
        }
    }
}
