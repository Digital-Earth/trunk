using System;

namespace PyxNet.FileTransfer
{
    using Microsoft.Win32;
    using PyxNet.Service;

    /// <summary>
    /// Maintains a record of the best manifests for each named resource.
    /// This record is currently stored in the user's registry.
    /// </summary>
    public class ManifestManager
    {
        #region Tracer

        public readonly Pyxis.Utilities.NumberedTraceTool<ManifestManager> Tracer =
            new Pyxis.Utilities.NumberedTraceTool<ManifestManager>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        #endregion

        private const String CorporateRegistryPath = "Software\\PYXIS";

        private const String ValueName = "Resource Instance Fact";

        private String m_manifestKey = "Manifest";

        private readonly Object m_registryLock = new Object();

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestManager"/> class.
        /// </summary>
        public ManifestManager()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestManager"/> class. 
        /// This is used for testing purposes only.
        /// </summary>
        /// <param name="subkeyName">Name of the subkey.</param>
        public ManifestManager(String subkeyName)
        {
            m_manifestKey = subkeyName;
        }

        /// <summary>
        /// Gets or sets the <see cref="PyxNet.FileTransfer.ManifestManager.ManifestDefinition"/> 
        /// corresponding to the specified application name.
        /// Throws if the storage could not be modified.
        /// </summary>
        /// <value></value>
        public ResourceInstanceFact this[String applicationName]
        {
            get
            {
                string currentValue = GetSerializedValue(applicationName);

                // Try to get it as a certificate...
                Certificate certificate = Deserialize<Certificate>(currentValue);
                if (certificate != null)
                {
                    foreach (ICertifiableFact fact in certificate.Facts)
                    {
                        if (fact is ResourceInstanceFact)
                            return fact as ResourceInstanceFact;
                    }
                }

                // Otherwise, this is an old resource on a dev machine.  We could just ignore this.
                return Deserialize<ResourceInstanceFact>(GetSerializedValue(applicationName));
            }
            set
            {
                if (value.Certificate != null)
                {
                    SetSerializedValue(applicationName, Serialize(value.Certificate));
                }
                else
                {
                    // We shouldn't get here!
                    SetSerializedValue(applicationName, Serialize(value));
                }
            }
        }
        
        /// <summary>
        /// Gets the <see cref="PyxNet.Service.ResourceInstanceFact"/> 
        /// corresponding to the specified application name.
        /// </summary>
        /// <value></value>
        public ResourceInstanceFact GetApplicationCurrentVersionManifest(String applicationName)
        {
            return this[GetApplicationKeyValue(applicationName, "current")];
        }

        public ResourceInstanceFact GetApplicationSpecificVersionManifest(Pyxis.Utilities.ProductSpecification product)
        {
            // TODO make this more robust (regex for name and version?)
            return this["PYXIS." + product.Name + "." + product.Version + ".Manifest"];
        }

        public String GetApplicationKeyValue(String applicationName, String key)
        {
            try
            {
                return GetKeyAtPath("Launcher\\Default\\" + applicationName, key);
            }
            catch (Exception e)
            {   // couldn't get the currentApplicationVersion
                Tracer.WriteLine("Failed to get current manifest of " + applicationName + " (" + e.Message + ")");
                return null;
            }
        }

        /// <summary>
        /// Sets the current version  
        /// corresponding to the specified application name.
        /// </summary>
        /// <value></value>
        public void SetApplicationCurrentVersionManifest(String applicationName, String manifestName)
        {
            SetApplicationKeyValue(applicationName, "current", manifestName);
        }

        public void SetApplicationKeyValue(String applicationName, String key, String value)
        {
            SetKeyAtPath("Launcher\\Default\\" + applicationName, key, value);
        }

        public void DeleteApplicationKey(String applicationName)
        {
            RegistryKey baseKey = Registry.CurrentUser.OpenSubKey(CorporateRegistryPath + "\\Launcher\\Default", true);
            baseKey.DeleteSubKeyTree(applicationName, false);
        }

        /// <summary>
        /// Returns null if unsuccessful.
        /// </summary>
        /// <param name="applicationName"></param>
        /// <returns></returns>
        private String GetSerializedValue(String applicationName)
        {
            try
            {
                lock (m_registryLock)
                {
                    RegistryKey applicationKey = GetApplicationKey(applicationName);
                    return applicationKey.GetValue(ValueName) as String;
                }
            }
            catch (Exception exception)
            {
                Tracer.WriteLine("Exception in GetSerializedValue: {0}",
                    exception.ToString());
            }
            return null;
        }

        /// <summary>
        /// Set the serialized value.
        /// Throws.
        /// </summary>
        /// <param name="applicationName"></param>
        /// <param name="newValue"></param>
        private void SetSerializedValue(String applicationName, String newValue)
        {
            try
            {
                lock (m_registryLock)
                {
                    RegistryKey applicationKey = GetApplicationKey(applicationName);
                    applicationKey.SetValue(ValueName, newValue);
                }
            }
            catch (Exception exception)
            {
                Tracer.WriteLine("Exception in SetSerializedValue: {0}",
                    exception.ToString());
                throw;
            }
        }

        /// <summary>
        /// Get the application key for reading or writing, creating if necessary.
        /// Throws.
        /// </summary>
        /// <param name="applicationName"></param>
        /// <returns></returns>
        private RegistryKey GetApplicationKey(String applicationName)
        {
            // TODO: Test that applicationName is a valid key.

            lock (m_registryLock)
            {
                RegistryKey baseKey = Registry.CurrentUser.CreateSubKey(CorporateRegistryPath);
                RegistryKey manifestKey = baseKey.CreateSubKey(m_manifestKey);
                return manifestKey.CreateSubKey(applicationName);
            }
        }

        /// <summary>
        /// Get the data in the value specified by path relative
        /// to the CorporateRegistryPath.
        /// Throws if the path or value are ill-posed.
        /// </summary>
        /// <param name="applicationName"></param>
        /// <returns></returns>
        private string GetKeyAtPath(String path, String value)
        {
            lock (m_registryLock)
            {
                return Registry.CurrentUser.CreateSubKey(CorporateRegistryPath + "\\" + path).GetValue(value).ToString();
            }
        }

        /// <summary>
        /// Set the key in the CorporateRegistryPath specified by path for reading or writing, 
        /// creating if necessary.
        /// Throws if the key or value are ill-posed.
        /// </summary>
        /// <param name="applicationName"></param>
        /// <returns></returns>
        private void SetKeyAtPath(String path, String value, String data)
        {
            lock (m_registryLock)
            {
                Registry.CurrentUser.CreateSubKey(CorporateRegistryPath + "\\" + path).SetValue(value, (Object)data);
            }
        }

        /// <summary>
        /// Deletes all entries.  For testing purposes only.
        /// </summary>
        public void DeleteAllEntries()
        {
            RegistryKey baseKey = Registry.CurrentUser.OpenSubKey(
                CorporateRegistryPath, true);

            try
            {
                baseKey.DeleteSubKeyTree(m_manifestKey);
            }
            catch (Exception)
            {
            }
        }

        #region Serialization

        private static T Deserialize<T>(String s) where T:class
        {
            T result = default(T);
            if (null != s && !s.Equals(String.Empty))
            {
                using (System.IO.StringReader input = new System.IO.StringReader(s))
                {
                    try
                    {
                        System.Xml.Serialization.XmlSerializer serializer =
                            new System.Xml.Serialization.XmlSerializer(
                                typeof(T));
                        result = serializer.Deserialize(input) as T;
                    }
                    catch (System.InvalidOperationException)
                    {
                        // An XML error.  Eat it and return a null.
                    }
                }
            }
            return result;
        }

        private static String Serialize<T>(T manifestFact)
        {
            System.Text.StringBuilder sb = new System.Text.StringBuilder();
            using (System.IO.StringWriter sw = new System.IO.StringWriter(sb))
            {
                System.Xml.Serialization.XmlSerializer serializer =
                    new System.Xml.Serialization.XmlSerializer(
                        typeof(T));
                serializer.Serialize(sw, manifestFact);
            }
            return sb.ToString();
        }

        #endregion
    }
}
