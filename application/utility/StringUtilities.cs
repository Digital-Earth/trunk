using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ApplicationUtility
{
    public static class StringUtilities
    {
        // TOOD[rtaylor,02/04/2013]: Remove this string encoding method when SWIG is updated
        /*
         * SWIG is unable to transfer strings with non ASCII encodings from c++
         * to C#, this method decodes a vector of string bytes and encodes it into
         * a C# string using the default system encoding. The pyxlib method 
         * getStringAsByteVector can be used to get a byte array from the C++ side of 
         * the SWIG barrier.
         */
        public static string DecodeByteVectorIntoString(Vector_Byte encodedValue)
        {
            byte[] bytes = new byte[encodedValue.Count];
            int byteCount = 0;
            foreach (byte byteValue in encodedValue)
            {
                bytes[byteCount] = byteValue;
                byteCount++;
            }
            return System.Text.Encoding.Default.GetString(bytes);
        }

        public static bool HasContent(this string str)
        {
            return !string.IsNullOrWhiteSpace(str);
        }

        public static string RemoveAllChars(this string str, char[] chars)
        {
            return chars.Aggregate(str, (s, c) => s.Replace(c.ToString(), ""));
        }
    }
}
