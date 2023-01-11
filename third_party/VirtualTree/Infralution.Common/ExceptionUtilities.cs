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
using System.Reflection;
using System.ComponentModel;
using System.Configuration;
using Infralution.Common.Properties;
namespace Infralution.Common
{
	/// <summary>
	/// Provides utilities for handling .NET Exceptions.
	/// </summary>
	public static class ExceptionUtilities
	{
        /// <summary>
        /// Has the configuration error message already been shown
        /// </summary>
        private static bool _configurationErrorMessageShown = false;

        /// <summary>
        /// Return the innermost exception from an exception.
        /// </summary>
        /// <param name="e">The exception to get the innermost exception from</param>
        /// <returns>The innermost exception</returns>
        public static Exception GetInnermostException(Exception e)
        {
            Exception result = e;
            while (result.InnerException != null)
            {
                result = result.InnerException;
            }
            return result;
        }

        /// <summary>
        /// Fired by <see cref="HandleConfigurationException"/> to allow application to handle configuration errors. 
        /// </summary>
        public static event ConfigurationExceptionHandler ConfigurationException;

        /// <summary>
        /// The default handler for configuration exceptions.
        /// </summary>
        /// <param name="e">The exception being handled</param>
        /// <param name="showErrorMessage">Should an error message be displayed to the user</param>
        /// <remarks>Deletes the corrupted user.config file and optionally displays an error message to the
        /// user the first time the error occurs.  
        /// </remarks>
        public static void DefaultConfigurationExceptionHandler(ConfigurationException e, bool showErrorMessage)
        {
            if (showErrorMessage && !_configurationErrorMessageShown)
            {
                MessageBoxEx.ShowError(Resources.ConfigurationErrorTitle, Resources.ConfigurationErrorMessage);
                _configurationErrorMessageShown = true;
            }

            ConfigurationException innerEx = e;
            while (string.IsNullOrEmpty(innerEx.Filename) && innerEx != null)
            {
                innerEx = innerEx.InnerException as ConfigurationException;
            }
            if (innerEx != null)
            {
                if (System.IO.File.Exists(innerEx.Filename))
                {
                    try
                    {
                        System.IO.File.Delete(innerEx.Filename);
                    }
                    catch
                    {
                    }
                }
            }
        }

        /// <summary>
        /// Called by code using Application Settings to handle a configuration exception due to a
        /// corrupted user.config file
        /// </summary>
        /// <param name="e">The configuration exception</param>
        /// <remarks>
        /// Fires the <see cref="ExceptionUtilities.ConfigurationException"/> event to handle the exception.
        /// If there is no handler registered then it calls <see cref="DefaultConfigurationExceptionHandler"/>.
        /// </remarks>
        public static void HandleConfigurationException(ConfigurationException e)
        {
            if (ConfigurationException == null)
            {
                DefaultConfigurationExceptionHandler(e, true);
            }
            else
            {
                ConfigurationException(e);
            }

        }
    }
}
