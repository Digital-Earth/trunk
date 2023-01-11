using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities
{
    public static class GlobalTypeFinder
    {
        /// <summary>
        /// Finds types within currently loaded assemblies decorated with the given attribute.
        /// Loading assemblies may be required if a desired type is in a not currently loaded assembly.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        public static class Attributes<T> where T : Attribute
        {
            private static IEnumerable<Type> s_attributeProviders;

            static Attributes()
            {
                if (typeof(T).Namespace.StartsWith("System."))
                {
                    s_attributeProviders =
                        (from a in AppDomain.CurrentDomain.GetAssemblies()
                         from t in a.GetTypes()
                         where Attribute.IsDefined(t, typeof(T), true)
                         select t).ToList();
                }
                else
                {
                    s_attributeProviders =
                        (from a in AppDomain.CurrentDomain.GetAssemblies()
                         where 
                            !a.FullName.StartsWith("System") &&
                            !a.FullName.StartsWith("mscorlib") &&
                            !a.FullName.StartsWith("Microsoft.mshtml")
                         from t in a.GetTypes()
                         where Attribute.IsDefined(t, typeof(T), true)
                         select t).ToList();
                }
            }

            public static IEnumerable<Type> FindAll()
            {
                return s_attributeProviders;
            }
        }

        /// <summary>
        /// Finds types within currently loaded assemblies implementing the given interface.
        /// Loading assemblies may be required if a desired type is in a not currently loaded assembly.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        public static class Interfaces<T> where T : class
        {
            private static IEnumerable<Type> s_implementors;

            static Interfaces()
            {
                if (!typeof(T).IsInterface)
                {
                    s_implementors = Enumerable.Empty<Type>();
                    return;
                }

                s_implementors =
                    from a in AppDomain.CurrentDomain.GetAssemblies().AsParallel().AsEnumerable()
                    from t in a.GetLoadableTypes()
                    where t.GetInterfaces().Contains(typeof(T))
                    select t;
            }

            public static IEnumerable<Type> FindAll()
            {
                return s_implementors;
            }
        }

        public static IEnumerable<Type> GetLoadableTypes(this Assembly assembly)
        {
            if (assembly == null) throw new ArgumentNullException();
            try
            {
                return assembly.GetTypes();
            }
            catch (ReflectionTypeLoadException e)
            {
                return e.Types.Where(t => t != null);
            }
        }
    }
}