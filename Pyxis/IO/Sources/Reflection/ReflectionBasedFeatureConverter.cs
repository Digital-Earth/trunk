using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;

namespace Pyxis.IO.Sources.Reflection
{
    internal class ReflectionBasedFeatureConverter : IFeatureConverter
    {
        private static readonly Type[] s_simpleTypes = new[]{
                typeof(bool),
                
                typeof(byte),
                typeof(int),
                typeof(uint),
                typeof(short),
                typeof(ushort),
                typeof(long),
                typeof(ulong),
                typeof(float),
                typeof(double),
                
                typeof(char),
                typeof(string),
                
                typeof(DateTime),
                typeof(Guid)};

        private static bool IsSimpleType(Type type) 
        {            
            return s_simpleTypes.Contains(type);
        }

        private ReflectionBasedFeatureConverter() {}

        private List<PropertyInfo> m_properties;
        private PropertyInfo m_idProperty;
        private PropertyInfo m_geometryProperty;

        internal static IFeatureConverter Create(Type type)
        {
            var properties = new List<PropertyInfo>();
            PropertyInfo idProperty = null;
            PropertyInfo geometryProperty = null;

            foreach (var property in type.GetProperties())
            {
                if (!property.CanRead)
                {
                    continue;
                }

                if (property.GetIndexParameters().Length > 0)
                {
                    continue;
                }

                var propertyType = property.PropertyType;

                if (typeof(IGeometry).IsAssignableFrom(propertyType))
                {
                    if (geometryProperty == null)
                    {
                        geometryProperty = property;
                    }
                    else
                    {
                        throw new Exception(String.Format("Feature conversion only supports types with a single geometry property. Found 2 possible properties: {0} and {1}", geometryProperty.Name, property.Name));
                    }
                }
                else if (IsSimpleType(propertyType))
                {
                    properties.Add(property);
                }                                
            }

            //find custom attribute
            idProperty = properties.FirstOrDefault(x => x.GetCustomAttributes(true).FirstOrDefault(a=>a is FeatureIdAttribute) != null);

            //if that fails - find property with "id" name
            if (idProperty == null)
            {
                idProperty = properties.FirstOrDefault(x => x.Name.Equals("id", StringComparison.InvariantCultureIgnoreCase));
            }

            if (idProperty == null)
            {
                throw new Exception(String.Format("Feature conversion only supports types with an id property. Please define property name id or user custom attribute FeatureIdAttribute."));
            }

            properties.Remove(idProperty);            
            
            if (geometryProperty == null)
            {
                throw new Exception(String.Format("Feature conversion only supports types with a single geometry property. Can't find public property that support IGeometry interface"));
            }          

            var converter = new ReflectionBasedFeatureConverter();

            converter.m_properties = properties;
            converter.m_idProperty = idProperty;
            converter.m_geometryProperty = geometryProperty;

            return converter;
        }

        public Feature Convert(object obj)
        {
            return new Feature(
                m_idProperty.GetValue(obj,null).ToString(),
                m_geometryProperty.GetValue(obj,null) as Geometry, 
                m_properties.ToDictionary(prop => prop.Name, prop => prop.GetValue(obj,null)));
        }
    }
}
