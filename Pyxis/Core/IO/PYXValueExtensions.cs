using System;
using System.Linq;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.IO
{
    /// <summary>
    /// Provides methods to convert PYXIS values into externally usable types.
    /// </summary>
    public static class PYXValueExtensions
    {
        /// <summary>
        /// Returns the Pyxis.Core.IO.PipelineDefinition.FieldType corresponding to the PYXIS field definition.
        /// </summary>
        /// <param name="field">The PYXIS field definition to determine the Pyxis.Core.IO.PipelineDefinition.FieldType of.</param>
        /// <returns>The Pyxis.Core.IO.PipelineDefinition.FieldType of the PYXIS field definition.</returns>
        public static PipelineSpecification.FieldType ToFieldType(this PYXFieldDefinition field)
        {
            var count = field.getCount();
            var type = field.getType();
            if (type == PYXValue.eType.knUInt8 && (count == 3 || count == 4))
            {
                return PipelineSpecification.FieldType.Color;
            }
            return type.ToFieldType();
        }

        /// <summary>
        /// Returns the Pyxis.Core.IO.PipelineDefinition.FieldType corresponding to the PYXIS value type.
        /// </summary>
        /// <param name="type">The PYXIS value type to determine the Pyxis.Core.IO.PipelineDefinition.FieldType of.</param>
        /// <returns>The Pyxis.Core.IO.PipelineDefinition.FieldType of the PYXIS value type.</returns>
        public static PipelineSpecification.FieldType ToFieldType(this PYXValue.eType type)
        {
            switch (type)
            { 
                case PYXValue.eType.knString:
                case PYXValue.eType.knChar:
                    return PipelineSpecification.FieldType.String;
                case PYXValue.eType.knBool:
                    return PipelineSpecification.FieldType.Boolean;
                default:
                    return PipelineSpecification.FieldType.Number;
            }
        }

        /// <summary>
        /// Returns a System.Object corresponding to the PYXIS value.
        /// </summary>
        /// <param name="value">The PYXIS value to convert into a System.Object.</param>
        /// <returns>null if the <paramref name="value"/> is null; otherwise a System.Object containing the corresponding System representation of the PYXIS value.</returns>
        public static object ToDotNetObject(this PYXValue value)
        {
            if (value.isNull())
            {
                return null;
            }

            var pyxType = value.getType();
            var type = pyxType.ToFieldType();
            var length = value.getArraySize();

            if (length > 1)
            {
                switch (type)
                {
                    case PipelineSpecification.FieldType.Boolean:
                        return Enumerable.Range(0, length).Select(value.getBool).ToArray();
                    case PipelineSpecification.FieldType.Number:
                        return Enumerable.Range(0, length).Select(value.getDouble).ToArray();
                    default:
                        if (pyxType == PYXValue.eType.knChar)
                        {
                            //PYXValue knChar getString convert the char into a number - and we want to preserve it string fullness 
                            return Enumerable.Range(0, length).Select(i=>value.getChar(i).ToString()).ToArray();
                        }
                        else
                        {
                            return Enumerable.Range(0, length).Select(value.getString).ToArray();
                        }
                        
                }
            }
            else
            {
                switch (type)
                {
                    case PipelineSpecification.FieldType.Boolean:
                        return value.getBool();
                    case PipelineSpecification.FieldType.Number:
                        return value.getDouble();
                    default:
                        if (pyxType == PYXValue.eType.knChar)
                        {
                            //PYXValue knChar getString convert the char into a number - and we want to preserve it string fullness 
                            return value.getChar().ToString();
                        }
                        else
                        {
                            return value.getString();
                        }
                }
            }            
        }

        /// <summary>
        /// Returns a PYXIS value corresponding to the System.Object.
        /// </summary>
        /// <param name="obj">System.Object to convert into PYXValue.</param>
        /// <returns>PYXValue of matching type of System.Object or type null if System.Object is of unsupported type</returns>
        public static PYXValue CreatePYXValueFrom(object obj)
        {
            if (obj is bool)
            {
                return new PYXValue((bool)obj);
            } 
            else if (obj is int)
            {
                return new PYXValue((int)obj);
            }
            else if (obj is uint)
            {
                return new PYXValue((uint)obj);
            }
            //json convert integer numbers into int64
            else if (obj is Int64)
            {
                return new PYXValue((int)(Int64)obj);
            }
            //json convert integer numbers into int64
            else if (obj is UInt64)
            {
                return new PYXValue((uint)(UInt64)obj);
            }
            else if (obj is float)
            {
                return new PYXValue((float)obj);
            }            
            else if (obj is double)
            {
                return new PYXValue((double)obj);
            }
            else if (obj is char)
            {
                return new PYXValue((char)obj);
            }
            else if (obj is string)
            {
                return new PYXValue((string)obj);
            }
            return new PYXValue();
        }
    }
}
