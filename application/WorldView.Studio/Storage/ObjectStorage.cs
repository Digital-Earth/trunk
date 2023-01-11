using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio.Storage
{
    /// <summary>
    /// ObjectStorage provide simple storage of text files
    /// </summary>
    class ObjectStorage
    {
        private object m_lock = new object();

        public string Path { get; private set; }

        /// <summary>
        /// Create an ObjectStorage on a specific directory.        
        /// </summary>
        /// <param name="path">Path to use to store and load files from</param>
        public ObjectStorage(string path)
        {
            Path = path;

            if (!System.IO.Directory.Exists(Path))
            {
                System.IO.Directory.CreateDirectory(Path);
            }
        }

        /// <summary>
        /// Save a string value into a file. the file name would be [name].txt
        /// </summary>
        /// <param name="name">the name of the file. can't contain / or : and can't start with .</param>
        /// <param name="value">the string value to save</param>
        public void Save(string name, string value)
        {
            lock (m_lock)
            {

                ValidateName(name);
                var filename = GetFileName(name, ".txt");
                var backupFilename = GetFileName(name, ".txt.backup");
                var tempFilename = GetFileName(name, ".txt.new");

                //create new file
                if (System.IO.File.Exists(tempFilename))
                {
                    System.IO.File.Delete(tempFilename);
                }
                System.IO.File.WriteAllText(tempFilename, value);
                

                //create backup
                if (System.IO.File.Exists(backupFilename))
                {
                    System.IO.File.Delete(backupFilename);
                }

                if (System.IO.File.Exists(filename))
                {
                    System.IO.File.Copy(filename, backupFilename);                    
                }

                //update the new value
                if (System.IO.File.Exists(filename))
                {
                    System.IO.File.Delete(filename);
                }
                System.IO.File.Move(tempFilename, filename);
            }
        }

        /// <summary>
        ///  load a string value from a file. the file name would be [name].txt
        ///  if file doesn't exists, it would return a default value instead - no file would be created when default value has been returned
        /// </summary>
        /// <param name="name">the name of the file. can't contain / or : and can't start with .</param>
        /// <param name="defaultValue">default value to use in case file doesn't exists</param>
        /// <returns>the content of the file as string or the default value</returns>
        public string Load(string name, string defaultValue)
        {
            lock (m_lock)
            {
                ValidateName(name);
                var filename = GetFileName(name, ".txt");
                var backupFilename = GetFileName(name, ".txt.backup");

                if (System.IO.File.Exists(filename))
                {
                    return System.IO.File.ReadAllText(filename);
                }
                else if (System.IO.File.Exists(backupFilename))
                {
                    System.IO.File.Move(backupFilename, filename);
                    return System.IO.File.ReadAllText(filename);
                }
                else
                {
                    return defaultValue;
                }
            }
        }

        private string GetFileName(string name,string ext)
        {
            return Path + System.IO.Path.DirectorySeparatorChar + name + ext;
        }

        private void ValidateName(string name)
        {
            if (name.IndexOfAny(System.IO.Path.GetInvalidFileNameChars()) != -1)
            {
                throw new ArgumentException("Name can't contain invalid chars", "name");
            }
            if (name.StartsWith(".")) {
                throw new ArgumentException("Name can't start with '.'", "name");
            }
        }
    }
}
