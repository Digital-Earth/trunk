#region File Header
//
//      FILE:   DesignAssembly.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2007 
//              Infralution
#endregion
using System;
using System.Collections.Generic;
using System.Text;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines the name of the design assembly containing designer classes for this assembly
    /// </summary>
    /// <remarks>
    /// Allows us to maintain this constant in a single location instead of having it spread throughout
    /// the code and updating it in every place each time the version changes.
    /// </remarks>
    public static class DesignAssembly
    {
        /// <summary>
        /// The name of the design assembly containing the designer classes for this assembly
        /// </summary>
        /// <remarks>
        /// The public key token depends on whether this is an Infralution internal build (and hence CHECK_LICENSE is true)
        /// or a customer build - in which case a generic strong name will be used to sign the assembly
        /// </remarks>
#if CHECK_LICENSE
        public const string Name = "Infralution.Controls.VirtualTree.Design, Version=3.13.0.0, Culture=neutral, PublicKeyToken=3e7e8e3744a5c13f";
#else
        public const string Name = "Infralution.Controls.VirtualTree.Design, Version=3.13.0.0, Culture=neutral, PublicKeyToken=47f1d8dad4968fdb";
#endif
    }
}
