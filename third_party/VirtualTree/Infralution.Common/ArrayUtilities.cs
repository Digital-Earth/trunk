//
//      FILE:   ArrayUtilities.cs.
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
	/// Provides utilities for manipulation of arrays.
	/// </summary>
	public static class ArrayUtilities
	{

        /// <summary>
        /// Concatenate two arrays
        /// </summary>
        /// <param name="a1">The prefix array</param>
        /// <param name="a2">The suffix array </param>
        /// <returns>A concatenated array built from the two input arrays</returns>
        public static Array Concat(Array a1, Array a2)
        {
            Array result = Array.CreateInstance(a1.GetType().GetElementType(), a1.Length + a2.Length);
            Array.Copy(a1, 0, result, 0, a1.Length);
            Array.Copy(a2, 0, result, a1.Length, a2.Length);
            return result;
        }

        /// <summary>
        /// Return a slice of the given input array
        /// </summary>
        /// <param name="source">The input array to return a slice of</param>
        /// <param name="startIndex">The index at which the slice begins</param>
        /// <param name="length">The length of the slice</param>
        /// <returns>A slice of the given input array</returns>
        public static Array Slice(Array source, long startIndex, long length)
        {
            Array result = Array.CreateInstance(source.GetType().GetElementType(), length);
            Array.Copy(source, startIndex, result, 0, length);
            return result;
        }

        /// <summary>
        /// Return a new array with contents in the reverse order
        /// </summary>
        /// <param name="a">The array to reverse</param>
        /// <returns>An array with contents in the reverse order</returns>
        public static Array Reverse(Array a)
        {
            Array result = Array.CreateInstance(a.GetType().GetElementType(), a.Length);
            for (int i=0, j=a.Length - 1; i < a.Length; i++, j--)
            {
                result.SetValue(a.GetValue(j), i);
            }
            return result;
        }

        /// <summary>
        /// Concatenate two byte arrays
        /// </summary>
        /// <param name="a1">The prefix array</param>
        /// <param name="a2">The suffix array </param>
        /// <returns>A concatenated array built from the two input arrays</returns>
        public static byte[] Concat(byte[] a1, byte[] a2)
        {
            byte[] result = new byte[a1.Length + a2.Length];
            Array.Copy(a1, 0, result, 0, a1.Length);
            Array.Copy(a2, 0, result, a1.Length, a2.Length);
            return result;
        }

        /// <summary>
        /// Return a slice of the given input byte array
        /// </summary>
        /// <param name="source">The input array to return a slice of</param>
        /// <param name="startIndex">The index at which the slice begins</param>
        /// <param name="length">The length of the slice</param>
        /// <returns>A slice of the given input array</returns>
        public static byte[] Slice(byte[] source, long startIndex, long length)
        {
            byte[] result = new byte[length];
            Array.Copy(source, startIndex, result, 0, length);
            return result;
        }

        /// <summary>
        /// Are the contents of the two byte arrays equal
        /// </summary>
        /// <param name="a1">The first array</param>
        /// <param name="a2">The second array </param>
        /// <returns>True if the contents of the arrays is equal</returns>
        public static bool Equals(byte[] a1, byte[] a2)
        {
            if (a1 == a2) return true;
            if ((a1 == null) || (a2 == null)) return false;
            if (a1.Length != a2.Length) return false;
            for (int i=0; i < a1.Length; i++)
            {
                if (a1[i] != a2[i]) return false;
            }
            return true;
        }

	}
}
