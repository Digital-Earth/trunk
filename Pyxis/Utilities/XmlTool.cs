using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Provides some helper functions for XML streaming.
    /// </summary>
    public static class XmlTool
    {
        /// <summary>
        /// Parses a "ReturnType" object from the given xml string.
        /// </summary>
        /// <typeparam name="ReturnType"></typeparam>
        /// <param name="xml"></param>
        /// <returns></returns>
        public static ReturnType FromXml<ReturnType>(string xml) where ReturnType : class
        {
            System.IO.TextReader input = new System.IO.StringReader(xml);
            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(typeof(ReturnType));

            try
            {
                ReturnType result = s.Deserialize(input) as ReturnType;
                return result;
            }
            catch (Exception)
            {
                // There may have been an exception thrown because the 
                //  record is from a "derived" type.  Try again with a type map.
                List<Type> typeMap = new List<Type>();
                foreach (var assembly in System.AppDomain.CurrentDomain.GetAssemblies())
                {
                    foreach (var t in assembly.GetTypes())
                    {
                        var baseType = t;
                        while ((baseType != null) && (baseType != typeof(object)))
                        {
                            if (baseType == typeof(ReturnType))
                            {
                                input = new System.IO.StringReader(xml);
                                s = new System.Xml.Serialization.XmlSerializer(t);
                                try
                                {
                                    return s.Deserialize(input) as ReturnType;
                                }
                                catch (Exception)
                                {
                                    // Eat internal exceptions.
                                }
                                break;
                            }
                            baseType = baseType.BaseType;
                        }
                    }
                }
                s = new System.Xml.Serialization.XmlSerializer(typeof(ReturnType), typeMap.ToArray());
                ReturnType result = s.Deserialize(input) as ReturnType;
                return result;
            }
        }

        /// <summary>
        /// Parses a "ReturnType" object from the given stream.
        /// </summary>
        /// <typeparam name="ReturnType">The type to return.</typeparam>
        /// <param name="input">The input.</param>
        /// <returns></returns>
        public static ReturnType FromXml<ReturnType>(System.IO.TextReader input) 
            where ReturnType : class
        {
            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(typeof(ReturnType));
            
            ReturnType result = s.Deserialize(input) as ReturnType;
            return result;
        }

        /// <summary>
        /// Convert the given object to XML.
        /// </summary>
        /// <param name="o">The object.</param>
        /// <returns></returns>
        public static string ToXml(object o)
        {
            System.IO.StringWriter outputStream = new System.IO.StringWriter(
                new System.Text.StringBuilder());

            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(o.GetType());
            s.Serialize(outputStream, o);

            // To cut down on the size of the xml, we'll strip extraneous stuff.
            string result = outputStream.ToString().Replace("xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"", "");
            result = result.Replace("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"", "");
            result = result.Replace("<?xml version=\"1.0\" encoding=\"utf-16\"?>", "").Trim();

            return result;
        }
    }
}
