using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Utilities
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
            System.IO.StringReader input = new System.IO.StringReader(xml);

            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(typeof(ReturnType));

            try
            {
                ReturnType result = s.Deserialize(input) as ReturnType;
                return result;
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine(e.Message);
                throw;
            }
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
            string result = "";

            // Old, slow way of doing serialization.  Forces a new assembly to be generated
            // every time its called.
            //Type[] typeMap = { };
            //System.Xml.Serialization.XmlSerializer s =
            //    new System.Xml.Serialization.XmlSerializer(o.GetType(), extendedTypemap);

            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(o.GetType());
            s.Serialize(outputStream, o);

            // To cut down on the size of the xml, we'll strip extraneous stuff.
            result = outputStream.ToString().Replace("xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"", "");
            result = result.Replace("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"", "");
            result = result.Replace("<?xml version=\"1.0\" encoding=\"utf-16\"?>", "").Trim();

            return result;
        }
    }
}
