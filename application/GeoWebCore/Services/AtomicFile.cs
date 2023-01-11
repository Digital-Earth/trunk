using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Services
{
    /// <summary>
    /// Provide a utility function for writing and reading files to disk in "atomic" way
    /// 
    /// Current limitations:
    /// (x) calling ReadAllText and WriteAllText can cause ReadAllText return empty value.
    /// </summary>
    internal static class AtomicFile
    {
        public static string ReadAllText(string path)
        {
            //we detected that the file exists
            if (File.Exists(path))
            {

                //try to read first time
                try
                {
                    return File.ReadAllText(path);
                }
                catch (Exception)
                {
                    //we failed.. wait for 1 sec and try again
                    System.Threading.Thread.Sleep(1000);

                    if (File.Exists(path))
                    {
                        return File.ReadAllText(path);
                    }
                }
            }

            return null;
        }

        public static void WriteAllText(string path, string value)
        {
            var tempFile = path + "." + DateTime.Now.Ticks;

            try
            {
                File.WriteAllText(tempFile, value);
                
                //try to delete original file if exists
                if (File.Exists(path))
                {
                    try
                    {
                        File.Delete(path);
                    }
                    catch (Exception)
                    {
                        //fall through
                    }
                }

                File.Move(tempFile, path);
            }
            catch (Exception ex)
            {
                if (!File.Exists(path))
                {
                    throw new Exception("failed to atomic write " + path, ex);
                }
            }
            finally
            {
                if (File.Exists(tempFile))
                {
                    File.Delete(tempFile);
                }
            }
        }
    }
}
