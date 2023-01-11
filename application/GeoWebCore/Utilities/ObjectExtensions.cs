using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Allow easy way to get Properites from object using Property name
    /// </summary>
    public static class ObjectExtensions
    {
        /// <summary>
        /// Return a property value as object or null if failed to find matching property name
        /// </summary>
        /// <param name="obj">Object to fetch property from</param>
        /// <param name="name">Name of the property or property path ("Metdata.Name")</param>
        /// <returns>Property value or null</returns>
        public static object GetPropertyValue(this object obj, string name)
        {
            foreach (var part in name.Split('.'))
            {
                if (obj == null)
                {
                    return null;
                }

                var type = obj.GetType();
                var info = type.GetProperty(part);
                if (info == null)
                {
                    return null;
                }

                obj = info.GetValue(obj, null);
            }
            return obj;
        }


        /// <summary>
        /// Return a property value as object or null if failed to find matching property name
        /// </summary>
        /// <param name="obj">Object to fetch property from</param>
        /// <param name="name">Name of the property or property path ("Metdata.Name")</param>
        /// <typeparam name="T">Type of the property. will try to cast the result value to this class if possible.</typeparam>
        /// <returns>Property value or null</returns>
        public static T GetPropertyValue<T>(this object obj, string name)
        {
            var retval = GetPropertyValue(obj, name);
            if (retval == null)
            {
                return default(T);
            }

            // throws InvalidCastException if types are incompatible
            return (T) retval;
        }
    }
}
