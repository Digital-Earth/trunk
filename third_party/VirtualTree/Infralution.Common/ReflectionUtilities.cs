//
//      FILE:   ReflectionUtilities.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
using System;
using System.Reflection;
using System.ComponentModel;
using System.Diagnostics;
namespace Infralution.Common
{
	/// <summary>
	/// Provides utilities for using .NET Reflection.
	/// </summary>
	public static class ReflectionUtilities
	{
 
        /// <summary>
        /// Load an assembly of the given name - returning null if not loaded
        /// </summary>
        /// <remarks>
        /// The Assembly.Load methods throw exceptions if the assembly could not be loaded.
        /// This method provide a wrapper which returns null if the assembly could not be loaded.
        /// </remarks>
        /// <param name="name">The name of the assembly to load</param>
        /// <returns>The loaded assembly or null if not loaded</returns>
        static public Assembly LoadAssembly(string name)
        {
            Assembly result = null;
            try
            {
                result = Assembly.Load(name);
            }
            catch
            {
            }
            return result;
        }

        /// <summary>
        /// Resolve the type with the given typename
        /// </summary>
        /// <param name="typeName">The name of the type</param>
        /// <returns>The type (or null) if it could not be resolved</returns>
        /// <remarks>
        /// Provides a more robust wrapper around the <see cref="Type.GetType(string)"/>
        /// method which requires assembly qualified type names in some circumstances
        /// and not others.   This method will try resolving first with the assembly qualified
        /// type name and if that fails it will try resolving using the type name only.
        /// </remarks>
        static public Type ResolveType(string typeName)
        {
            if (string.IsNullOrEmpty(typeName)) return null;
            Type type = Type.GetType(typeName, false);
            if (type == null)
            {
                // if resolving the type with the full name did not work
                // then try resolving first with just the assembly qualified name
                // and lastly just the type name
                //
                String[] parts = typeName.Split(',');
                if (parts.Length > 2)
                {
                    typeName = parts[0] + ", " + parts[1];
                    type = Type.GetType(typeName, false);
                }
                if (type == null && parts.Length > 1)
                {
                    type = Type.GetType(parts[0]);
                }
            }
            return type;
        }

        /// <summary>
        /// Return the public property of the given type with the given name.
        /// </summary>
        /// <param name="type">The type to get the property of</param>
        /// <param name="propertyName">The name of the property to return</param>
        /// <returns>The property info for the given property - or null</returns>
        /// <remarks>
        /// The <see cref="Type.GetProperty(string)"/> method does not handle the case where a derived
        /// class overloads a property name (using the new operator).  It throws an 
        /// <see cref="AmbiguousMatchException"/> in this case.  This method will return the 
        /// first property found in this instance which corresponds to the outermost property.
        /// </remarks>
        static public PropertyInfo GetProperty(Type type, string propertyName)
        {
            PropertyInfo[] properties = type.GetProperties(BindingFlags.GetProperty | BindingFlags.Public | BindingFlags.Instance);
            for (int i=0; i < properties.Length; i++)
            {
                PropertyInfo property = properties[i];
                if (property.Name == propertyName)
                    return property;
            }
            return null;
        }

        /// <summary>
        /// Copy the public, modified, writable properties of the source object to the destination object
        /// </summary>
        /// <param name="src">The source object</param>
        /// <param name="dst">The target object</param>
        static public void CopyModifiedProperties(object src, object dst)
        {
            if (src == null) throw new ArgumentNullException("src");
            if (dst == null) throw new ArgumentNullException("dst");
            if (src.GetType() != dst.GetType()) throw new ArgumentException("dst must be the same type as src");
            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(src);
            foreach (PropertyDescriptor prop in properties)
            {           
                if (prop.IsBrowsable && !prop.IsReadOnly && prop.ShouldSerializeValue(src))
                {
                    // ignore errors while attempting to set values
                    try
                    {
                        prop.SetValue(dst, prop.GetValue(src));
                    }
                    catch {}
                }
            }
        }

        /// <summary>
        /// When called from within a constructor returns the assembly that instantiated the object
        /// </summary>
        /// <returns>The assembly that instantiated the object</returns>
        /// <remarks>
        /// Handles dynamic instantiations by walking the stack to find the assembly
        /// that actually created the object
        /// </remarks>
        public static Assembly GetInstantiatingAssembly()
        {
            Assembly coreAssembly = typeof(Type).Assembly;
            Assembly systemAssembly = typeof(Component).Assembly;
            StackTrace trace = new StackTrace();
            Assembly result = null;
            for (int i=2; i < trace.FrameCount; i++)
            {
                StackFrame frame = trace.GetFrame(i);
                Assembly assembly = frame.GetMethod().DeclaringType.Assembly;
                if (assembly != coreAssembly && assembly != systemAssembly)
                {
                    result = assembly;
                    break;
                }
            }
            return result;
        }

        /// <summary>
        /// Return true if the given assembly was compiled using VB.NET
        /// </summary>
        /// <param name="assembly">The assembly to check</param>
        /// <returns>True if the assembly was compiled using VB</returns>
        public static bool IsVBAssembly(Assembly assembly)
        {
            AssemblyName[] referencedAssemblies = assembly.GetReferencedAssemblies();
            foreach (AssemblyName refAssembly in referencedAssemblies)
            {
                if (refAssembly.Name == "Microsoft.VisualBasic")
                    return true;
            }
            return false;
        }

    }
}
