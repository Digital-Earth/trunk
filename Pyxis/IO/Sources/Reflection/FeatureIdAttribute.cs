using System;

namespace Pyxis.IO.Sources.Reflection
{
    /// <summary>
    /// Define a property to be used as Feature Id.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property,AllowMultiple=false)]
    public class FeatureIdAttribute : Attribute
    {
    }
}
