//
//      FILE:   TypeUtilities.cs.
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
using System.Collections;
using System.Collections.Generic;
using Infralution.Common;
using System.Diagnostics;
namespace Infralution.Controls.Design
{
    /// <summary>
    /// Provides utilities for manipulating types at design time.
    /// </summary>
    public static class TypeUtilities
    {

        /// <summary>
        /// Class for sorting types by name
        /// </summary>
        private class TypeComparer : IComparer<Type>
        {
            public int Compare(Type x, Type y)
            {
                return x.FullName.CompareTo(y.FullName);
            }
        }

        /// <summary>
        /// Add the assemblies referenced by the project to the given list
        /// </summary>
        /// <remarks>
        /// Uses DTE to find the assemblies referenced by the project. 
        /// </remarks>
        /// <param name="vsProject">
        /// The visual studio project to get the assemblies for
        /// </param>
        /// <param name="assemblies">The list of assemblies</param>
        static private void GetReferencedAssemblies(VSLangProj.VSProject vsProject, List<Assembly> assemblies)
        {
            foreach (VSLangProj.Reference reference in vsProject.References)
            {
                Assembly assembly = ReflectionUtilities.LoadAssembly(reference.Name);
                if (assembly != null)
                {
                    assemblies.Add(assembly);
                }
            }
        }

        /// <summary>
        /// Return a list of assemblies referenced by the project
        /// </summary>
        /// <remarks>
        /// Uses DTE to find the assemblies referenced by the project.  If DTE is
        /// not available then returns an empty list.
        /// </remarks>
        /// <param name="serviceProvider">
        /// The design time service provider for the project containing the control/form
        /// </param>
        /// <returns>A list of the assemblies referenced by the project</returns>
        static public List<Assembly> GetReferencedAssemblies(IServiceProvider serviceProvider)
        {
            List<Assembly> result = new List<Assembly>();
            try
            {
                EnvDTE.ProjectItem projectItem = (EnvDTE.ProjectItem)serviceProvider.GetService(typeof(EnvDTE.ProjectItem));
                EnvDTE.Project project = projectItem.ContainingProject;
                VSLangProj.VSProject vsProject = (VSLangProj.VSProject)project.Object;
                GetReferencedAssemblies(vsProject, result);
            }
            catch
            {
            }
            return result;
        }

        /// <summary>
        /// Return a list of all the public types in the referenced assemblies
        /// </summary>
        /// <param name="serviceProvider">
        /// The design time service provider for the project containing the control/form
        /// </param>
        /// <returns>A list of all public types in the assemblies referenced by the project</returns>
        static public List<Type> GetReferencedTypes(IServiceProvider serviceProvider)
        {
            List<Type> result = new List<Type>();
            List<Assembly> assemblies = GetReferencedAssemblies(serviceProvider);
            foreach (Assembly assembly in assemblies)
            {
                Type[] types = assembly.GetTypes();
                foreach (Type type in types)
                {
                    if (type.IsPublic)
                    {
                        result.Add(type);
                    }
                }
            }
            result.Sort(new TypeComparer());
            return result;
        }

        /// <summary>
        /// Use the VS Code model to get available typenames for the current project
        /// </summary>
        /// <param name="codeElements">The code elements to get type names from</param>
        /// <param name="typeNames">The list of typenames</param>
        /// <param name="assemblyName">The assembly name to qualify types with</param>
        static private void GetCodeModelTypeNames(EnvDTE.CodeElements codeElements,
                                                  List<String> typeNames,
                                                  string assemblyName)
        {
            foreach (EnvDTE.CodeElement celm in codeElements)
            {
                switch (celm.Kind)
                {
                    case EnvDTE.vsCMElement.vsCMElementNamespace:
                        EnvDTE.CodeNamespace ns = celm as EnvDTE.CodeNamespace;
                        GetCodeModelTypeNames(ns.Members, typeNames, assemblyName);
                        break;
                    case EnvDTE.vsCMElement.vsCMElementClass:
                        EnvDTE.CodeClass ccl = celm as EnvDTE.CodeClass;
                        if (ccl != null)
                        {
                            if (celm.InfoLocation == EnvDTE.vsCMInfoLocation.vsCMInfoLocationProject)
                            {
                                string typeName = ccl.FullName;
                                if (assemblyName != null)
                                {
                                    typeName += ", " + assemblyName;
                                }
                                typeNames.Add(typeName);
                                GetCodeModelTypeNames(ccl.Members, typeNames, assemblyName);
                            }
                        }
                        break;
                }
            }
        }

        /// <summary>
        /// Return a list of all the avialable types from the code model
        /// </summary>
        /// <param name="serviceProvider">
        /// The design time service provider for the project containing the control/form
        /// </param>
        /// <returns>A list of all available type names from the code model</returns>
        static public List<String> GetCodeModelTypeNames(IServiceProvider serviceProvider)
        {
            List<String> result = new List<String>();

            EnvDTE.ProjectItem projectItem = (EnvDTE.ProjectItem)serviceProvider.GetService(typeof(EnvDTE.ProjectItem));
            EnvDTE.Project project = projectItem.ContainingProject;
            EnvDTE.CodeModel codeModel = project.CodeModel;

            // get the locally defined types for the project using the code model
            //
            string assemblyName = null;
            try
            {
                // C++ projects don't have this property
                //
                assemblyName = project.Properties.Item("AssemblyName").Value as string;
            }
            catch
            {
            }
            GetCodeModelTypeNames(codeModel.CodeElements, result, assemblyName);

            // Get the type names for referenced assemblies
            //
            List<Assembly> assemblies = GetReferencedAssemblies(serviceProvider);
            foreach (Assembly assembly in assemblies)
            {
                Type[] types = assembly.GetTypes();
                foreach (Type type in types)
                {
                    if (type.IsPublic)
                    {
                        string typeName = string.Format("{0}, {1}", type.FullName, type.Assembly.GetName().Name); 
                        result.Add(typeName);
                    }
                }
            }
            result.Sort();
            return result;
        }

        /// <summary>
        /// Get the names of the public property for the given class
        /// </summary>
        /// <param name="serviceProvider"></param>
        /// <param name="typeName">The name of the class or struct to get the properties for</param>
        /// <returns>A list of property names</returns>
        static public List<String> GetCodeModelPropertyNames(IServiceProvider serviceProvider, string typeName)
        {
            EnvDTE.ProjectItem projectItem = (EnvDTE.ProjectItem)serviceProvider.GetService(typeof(EnvDTE.ProjectItem));
            EnvDTE.Project project = projectItem.ContainingProject;
            EnvDTE.CodeModel codeModel = project.CodeModel;
            EnvDTE.CodeType codeType = codeModel.CodeTypeFromFullName(typeName);
            List<String> result = new List<String>();
            if (codeType != null)
            {
                GetCodeModelPropertyNames(codeType, result);            
            }
            result.Sort();
            return result;
        }

        #region Local Methods

        /// <summary>
        /// Add the property names for the given code type to the given list
        /// </summary>
        /// <param name="codeType">The type to get the properties for</param>
        /// <param name="names">List to add the names to</param>
        private static void GetCodeModelPropertyNames(EnvDTE.CodeType codeType, List<String> names)
        {
            
            // get the base class properties
            //
            foreach (EnvDTE.CodeElement codeElement in codeType.Bases)
            {
                if (codeElement is EnvDTE.CodeType)
                {
                    GetCodeModelPropertyNames(codeElement as EnvDTE.CodeType, names);
                }
                else if (codeElement.Kind == EnvDTE.vsCMElement.vsCMElementVCBase)
                {
                    // C++ is different!  The code element type is defined in Microsoft.VisualStudio.VCCodeModel 
                    // Instead of referencing this (when potentially it may not be present) we use late binding
                    // to extract the code type for the base class
                    //
                    object baseType = codeElement.GetType().InvokeMember("Class", 
                                                                         BindingFlags.GetProperty, 
                                                                         null, codeElement, null);
                    GetCodeModelPropertyNames(baseType as EnvDTE.CodeType, names);
                }
            }

            // get the properties for this class
            //
            foreach (EnvDTE.CodeElement celm in codeType.Members)
            {
                if (celm.Kind == EnvDTE.vsCMElement.vsCMElementProperty)
                {
                    EnvDTE.CodeProperty prop = celm as EnvDTE.CodeProperty;
                    if (prop.Access == EnvDTE.vsCMAccess.vsCMAccessPublic)
                    {
                        // check the name is not already present from a base class
                        //
                        if (!names.Contains(prop.Name))
                        {
                            names.Add(prop.Name);
                        }
                    }
                }
            }
        }

        #endregion
    }
}
