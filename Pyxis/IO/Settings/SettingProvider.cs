using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Pyxis.IO.Settings
{
    /// <summary>
    /// Default Implementation of the ISettingProvider interface.
    /// 
    /// This class would return null settings as default.
    /// However, the Register method can be used to override the result for any <typeparamref name="T"/> type.
    /// </summary>
    /// <typeparam name="T">Type of settings being provided.</typeparam>
    /// <typeparam name="U">Type of setting arguments.</typeparam>
    public class SettingProvider<T, U> : ISettingProvider<T,U>
        where T : ISetting
        where U : ISettingArgs
    {
        /// <summary>
        /// Dictionary where settings are stored.
        /// </summary>
        protected Dictionary<Type, Func<U, Task<T>>> m_registry;

        private ISettingProvider<T, U> m_defaults;

        /// <summary>
        /// Initializes a new instance of the SettingProvider class.
        /// </summary>
        public SettingProvider()
        {
            m_registry = new Dictionary<Type, Func<U, Task<T>>>();
        }

        /// <summary>
        /// Initializes a new instance of the SettingProvider witha default setting .
        /// </summary>
        public SettingProvider(ISettingProvider<T,U> defaults) : this()
        {
            m_defaults = defaults;
        }

        /// <summary>
        /// This function gets called when a <typeparamref name="T"/> setting object is required of type <paramref name="requiredSetting"/>
        /// </summary>
        /// <param name="requiredSetting">The required type of <typeparamref name="T"/> required.</param>
        /// <param name="args">Additional information passed to the settingProvider, can be null.</param>
        /// <returns>A Task to wait on for the returned setting.</returns>
        public Task<T> ProvideSetting(Type requiredSetting, U args)
        {
            //try generate value
            Func<U, Task<T>> func;
            if (m_registry.TryGetValue(requiredSetting, out func))
            {
                return func(args);
            }

            //if we have defaults, ask default to generate value
            if (m_defaults != null)
            {
                return m_defaults.ProvideSetting(requiredSetting, args);
            }

            //no settings provided.
            return null;
        }

        /// <summary>
        /// Register an object for a specific <typeparamref name="T"/> type.
        /// </summary>
        /// <param name="requiredSetting">Type of <typeparamref name="T"/> to register.</param>
        /// <param name="setting">Setting to return as a result.</param>
        /// <exception cref="System.InvalidOperationException">When requiredSetting does not implement <typeparamref name="T"/>.</exception>
        public void Register(Type requiredSetting, T setting)
        {
            Register(requiredSetting, (args) => Task.FromResult(setting));
        }

        /// <summary>
        /// Register an object for a specific <typeparamref name="T"/> type.
        /// </summary>
        /// <param name="setting">setting objet, the typeo of the object will be resolved at run time</param>
        public void Register(T setting)
        {
            if (setting == null)
            {
                throw new ArgumentNullException("setting");
            }
            Register(setting.GetType(), setting);
        }

        /// <summary>
        /// Register a callback for a specific <typeparamref name="T"/> type.
        /// </summary>
        /// <param name="requiredSetting">Type of <typeparamref name="T"/> to register.</param>
        /// <param name="action">Action to invoke for this specific <typeparamref name="T"/>.</param>
        /// <exception cref="System.InvalidOperationException">When requiredSetting does not implement <typeparamref name="T"/>.</exception>
        public void Register(Type requiredSetting, Func<U, Task<T>> action)
        {
            if (!typeof(T).IsAssignableFrom(requiredSetting))
            {
                throw new InvalidOperationException("requiredSetting type " + requiredSetting.Name + " does not implement " + typeof(T).Name);
            }

            m_registry[requiredSetting] = action;
        }
    }
}
