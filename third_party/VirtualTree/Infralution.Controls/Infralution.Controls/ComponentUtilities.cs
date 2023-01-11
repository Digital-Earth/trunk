//
//      FILE:   ComponentUtilities.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
using System;
using System.ComponentModel;
using System.Globalization;
using System.Reflection;
using System.Resources;
using System.Collections;
using System.Diagnostics;
using System.Windows.Forms;
using Infralution.Common;
namespace Infralution.Controls
{
    /// <summary>
    /// Provides utilities for managing components.
    /// </summary>
    public static class ComponentUtilities
    {

        #region Public Interface

        /// <summary>
        /// Add a component to a container using the given name
        /// </summary>
        /// <param name="container">The container to add the component to</param>
        /// <param name="component">The component to add</param>
        /// <param name="name">The name to give the component (if possible)</param>
        /// <remarks>
        /// Unlike <see cref="IContainer.Add(IComponent)"/> this method checks the name for uniqueness
        /// in the given container first and modifies the name if necessary to ensure it is unique.
        /// </remarks>
        static public void AddComponent(IContainer container, IComponent component, string name)
        {
            if (container == null) throw new ArgumentNullException("container");
            if (component == null) throw new ArgumentNullException("component");
            if (name == null) throw new ArgumentNullException("name");

            // sanitize the name
            //
            string newName = StringUtilities.Strip(name, " /\\?.,:;&(){}[]\"'$#%^#!@*+-=<>~");
            int index = 1;
            while (container.Components[newName] != null)
            {
                newName = name + index.ToString();
                index++;
            }
            container.Add(component, newName);
        }

        #endregion

        #region Local Methods

        #endregion
    }
}
