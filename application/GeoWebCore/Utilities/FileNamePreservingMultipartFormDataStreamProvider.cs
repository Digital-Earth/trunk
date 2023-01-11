using System;
using System.IO;
using System.Net.Http;
using System.Net.Http.Headers;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Generates streams for uploaded files in multipart http content. Preserves the name of the uploaded file and
    /// disallows unsafe file types.
    /// </summary>
    public class FileNamePreservingMultipartFormDataStreamProvider : MultipartFormDataStreamProvider
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="path">Path to file</param>
        public FileNamePreservingMultipartFormDataStreamProvider(string path)
            : base(path)
        {
        }

        /// <summary>
        /// http://www.howtogeek.com/137270/50-file-extensions-that-are-potentially-dangerous-on-windows/
        /// </summary>
        private static readonly string[] s_executableExtensions =
        {
            ".application ",
            ".bat ",
            ".cmd ",
            ".com ",
            ".cpl ",
            ".doc",
            ".docm",
            ".dotm",
            ".exe ",
            ".gadget ",
            ".hta ",
            ".inf ",
            ".jar ",
            ".js ",
            ".jse ",
            ".lnk ",
            ".msc ",
            ".msh",
            ".msh1",
            ".msh1xml",
            ".msh2",
            ".msh2xml",
            ".mshxml",
            ".msi ",
            ".msp ",
            ".pif ",
            ".potm",
            ".ppam",
            ".ppsm",
            ".ppt",
            ".pptm",
            ".ps1",
            ".ps1xml",
            ".ps2",
            ".ps2xml",
            ".psc1",
            ".psc2",
            ".reg ",
            ".scf ",
            ".scr ",
            ".sldm",
            ".vb",
            ".vbe ",
            ".vbs",
            ".ws",
            ".wsc",
            ".wsf",
            ".wsh",
            ".xlam",
            ".xls",
            ".xlsm",
            ".xltm"
        };

        /// <summary>
        /// Overridden to return the same name as the uploaded file. Throws an exception for unsafe files.
        /// </summary>
        /// <param name="headers">The Http headers</param>
        /// <returns></returns>
        public override string GetLocalFileName(HttpContentHeaders headers)
        {
            string oldFileName = headers.ContentDisposition.FileName.Replace("\"", string.Empty);
            var extension = Path.GetExtension(oldFileName).ToLower();

            if (!String.IsNullOrEmpty(extension) && (Array.IndexOf(s_executableExtensions, extension) > -1))
            {
                throw new Exception("Invalid file extension: " + extension);
            }

            // return the original file name
            return oldFileName;
        }
    }
}
