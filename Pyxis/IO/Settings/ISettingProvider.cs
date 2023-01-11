using System;
using System.Threading.Tasks;

namespace Pyxis.IO.Settings
{
    /// <summary>
    /// Represents a required setting for an IO operation.
    /// </summary>
    public interface ISetting
    {
    }

    /// <summary>
    /// Represents additional arguments for the ISettingProvider
    /// </summary>
    public interface ISettingArgs
    {
    }

    /// <summary>
    /// This interface is used to retrieve additional publish settings when required.
    /// </summary>
    /// <typeparam name="T">The type of settings the setting provider can provide.</typeparam>
    /// <typeparam name="U">The type of additional information passed to the ISettingProvider.</typeparam>
    public interface ISettingProvider<T, U> 
        where T : ISetting
        where U : ISettingArgs
    {
        /// <summary>
        /// This function gets called when a Settings object is required from type <paramref name="requiredSetting"/>.
        /// </summary>
        /// <param name="requiredSetting">The required type of ISetting required.</param>
        /// <param name="args">Additional information passed to the ISettingProvider, can be null.</param>
        /// <returns>A Task to wait on for the returned setting.</returns>
        Task<T> ProvideSetting(Type requiredSetting, U args);
    }
}
