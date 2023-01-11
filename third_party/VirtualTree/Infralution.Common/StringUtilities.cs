//
//      FILE:   StringUtilities.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
using System;
using System.Globalization;
using System.Text;
namespace Infralution.Common
{
	/// <summary>
	/// Provides utilities for string manipulation.
	/// </summary>
	public static class StringUtilities
	{
        /// <summary>
        /// Test whether a string is null/empty/whitespace
        /// </summary>
        /// <param name="value">The string to test</param>
        /// <returns>True if the string is null or contains only whitespace</returns>
        static public bool IsBlank(string value)
        {
            return (value == null) || (value.Trim().Length == 0);
        }

        /// <summary>
        /// Convert blank strings to null strings - otherwise
        /// </summary>
        /// <param name="value">The string to convert</param>
        /// <returns>Null if value was blank otherwise returns value</returns>
        static public string BlankToNull(string value)
        {
            if (value != null && (value.Trim().Length == 0)) return null;
            return value;
        }

        /// <summary>
        /// Trim the given string - handling nulls
        /// </summary>
        /// <param name="value">The string to trim</param>
        /// <returns>The trimmed string</returns>
        static public string Trim(string value)
        {
            if (value == null) return null;
            return value.Trim();
        }

        /// <summary>
        /// Return the given input string stripped of the given characters
        /// </summary>
        /// <param name="value">The string to strip</param>
        /// <param name="characters">The characters to strip from the string</param>
        /// <returns>The input string with the given characters removed</returns>
        public static string Strip(string value, string characters)
        {
            if (value == null) return null;
            StringBuilder sb = new StringBuilder();
            foreach (char ch in value)
            {
                if (characters.IndexOf(ch, 0) < 0)
                {
                    sb.Append(ch);
                }
            }
            return sb.ToString();       
        }

        /// <summary>
        /// Return the given input string stripped all white space
        /// </summary>
        /// <param name="value">The string to strip</param>
        /// <returns>The input string with white space removed</returns>
        public static string Strip(string value)
        {
            return Strip(value, "\t\n\r ");
        }

        /// <summary>
        /// Return the first line from the given string
        /// </summary>
        /// <param name="value">The string to get the first line from</param>
        /// <returns>The first line if any of the given multiline string</returns>
        public static string FirstLine(string value)
        {
            if (value == null) return null;
            int i = value.IndexOfAny(new char[] { '\r', '\n' } );
            return (i > 0) ? value.Substring(0, i) : value;
        }

        /// <summary>
        /// Converts a byte array into a hexadecimal representation.
        /// </summary>
        /// <param name="data">The byte data to convert</param>
        /// <param name="hyphenate">Should the output string be hyphenated into groups of 4 characters</param>
        /// <returns>Hexadecimal representation of the data</returns>
        public static string ToHex(byte[] data, bool hyphenate)
        {
            StringBuilder sb = new StringBuilder();
            for (int i=0; i < data.Length; i++)
            {
                if (hyphenate && i > 0 && i % 2 == 0)
                {
                    sb.Append("-");
                }
                sb.Append(data[i].ToString("X2"));
            }
            return sb.ToString();
        }

        /// <summary>
        /// Converts a hexadecimal string into a byte array.
        /// </summary>
        /// <param name="hex">Te hexadecimal string to convert</param>
        /// <returns>The converted byte data</returns>
        public static byte[] FromHex(string hex)
        {
            string strippedHex = Strip(hex, "\t\r\n -");
            if (strippedHex == null || strippedHex.Length % 2 != 0) 
                throw new FormatException("Invalid hexadecimal string");
            byte[] result = new byte[strippedHex.Length / 2];
            for (int i=0, j = 0; i < strippedHex.Length; i += 2, j++)
            {
                string s = strippedHex.Substring(i, 2);
                result[j] = byte.Parse(s, NumberStyles.HexNumber);                 
            }
            return result;
        }

	}
}
