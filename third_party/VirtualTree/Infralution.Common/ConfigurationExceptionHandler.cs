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
using System.Configuration;
namespace Infralution.Common
{
    /// <summary>
    /// Defines a method used to handle configuraton error while reading Application Settings 
    /// </summary>
    /// <param name="e">The exception</param>
    public delegate void ConfigurationExceptionHandler(ConfigurationException e);

}
