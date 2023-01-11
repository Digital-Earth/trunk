using System;
using System.Collections.Generic;

namespace Pyxis.IO.Sources.Reflection
{
    internal static class TypeConversionRepository
    {
        private static readonly Dictionary<Type,IFeatureConverter> m_knownTypes = new Dictionary<Type,IFeatureConverter>();
        private static readonly object m_lockObject = new object();

        public static IFeatureConverter GetConverter<T>()
        {
            var type = typeof(T);
            lock (m_lockObject)
            {
                if (!m_knownTypes.ContainsKey(type))
                {
                    m_knownTypes[type] = BuildConverterForType(type);
                }

                return m_knownTypes[type];
            }
        }

        public static void RegisterType<T>(IFeatureConverter converter)
        {
            var type = typeof(T);
            lock (m_lockObject)
            {
                m_knownTypes[type] = converter;
            }
        }

        private static IFeatureConverter BuildConverterForType(Type type)
        {
           return ReflectionBasedFeatureConverter.Create(type);
        }
    }
}
