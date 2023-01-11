namespace PyxisCLI.Server.Cache
{
    /// <summary>
    /// Provide a storage later of PersistentCache
    /// </summary>
    internal interface ICacheStorage
    {
        /// <summary>
        /// Return true if storage has a key
        /// </summary>
        /// <param name="key">key value</param>
        /// <returns>true if key exists in storage</returns>
        bool Has(string key);

        /// <summary>
        /// Read the given value assoicated with key
        /// </summary>
        /// <param name="key">key value</param>
        /// <returns>string value</returns>
        string Read(string key);

        /// <summary>
        /// Read the given value assoicated with key
        /// </summary>
        /// <param name="key">key value</param>
        /// <returns>byte[] value</returns>
        byte[] ReadBytes(string key);

        /// <summary>
        /// add a value to storage with a given key
        /// </summary>
        /// <param name="key">key to write</param>
        /// <param name="value">value to write</param>
        void Write(string key,string value);

        /// <summary>
        /// add a value to storage with a given key
        /// </summary>
        /// <param name="key">key to write</param>
        /// <param name="value">value to write</param>
        void WriteBytes(string key, byte[] value);
    }
}
