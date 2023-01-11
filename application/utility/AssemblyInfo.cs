using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;

namespace ApplicationUtility
{
    /// <summary>
    /// This class provides access methods to generic assembly information.
    /// </summary>
    public class AssemblyInfo
    {
        /// <summary>
        /// Return the copyright string from an assembly from which ever assembly 
        /// called this method.
        /// </summary>
        /// <returns>
        /// The published copyright string (if found) of the calling assembly or an empty 
        /// string if the copyright value is empty or not found.
        /// </returns>
        static public string GetCopyrightString()
        {
            Assembly assembly = System.Reflection.Assembly.GetCallingAssembly();
            System.Attribute unTypedAttr = Attribute.GetCustomAttribute(
                assembly,
                typeof(System.Reflection.AssemblyCopyrightAttribute));
            if (unTypedAttr != null)
            {
                AssemblyCopyrightAttribute copyright =
                    unTypedAttr as AssemblyCopyrightAttribute;
                System.Diagnostics.Debug.Assert(copyright != null);
                return copyright.Copyright;
            }
            return "";
        }
    }
}
