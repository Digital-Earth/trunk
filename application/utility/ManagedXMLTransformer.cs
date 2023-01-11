using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using Pyxis.Utilities;

namespace ApplicationUtility
{
    public class ManagedCSharpFunctionProvider : CSharpFunctionProvider, IDirectorReferenceCounter
    {
        private Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(true);

        /// <summary>
        /// Run an XSLT transform on some XML and return the result.
        /// </summary>
        /// <param name="transform">A transform in XSLT format.</param>
        /// <param name="inputXml">The XML to be transformed.</param>
        /// <returns>The transformed XML.</returns>
        public override string applyXsltTransform(string transform, string inputXml)
        {
            UTF8Encoding encoding = new UTF8Encoding();
            System.IO.MemoryStream streamXSLT =
                new System.IO.MemoryStream(encoding.GetBytes(transform));
            XmlTextReader transformReader = new System.Xml.XmlTextReader(streamXSLT);
            System.Xml.Xsl.XslCompiledTransform trans =
                new System.Xml.Xsl.XslCompiledTransform();
            try
            {
                trans.Load(transformReader);
            }
            catch (Exception ex)
            {
                Exception displayException = ex;
                while (displayException.InnerException != null)
                {
                    displayException = displayException.InnerException;
                }
                Trace.WriteLine("XSLT Definition error : {0}", displayException.Message);
                return "";
            }

            System.IO.MemoryStream streamXml =
                new System.IO.MemoryStream(encoding.GetBytes(inputXml));
            XmlTextReader xmlReader = new System.Xml.XmlTextReader(streamXml);

            System.IO.MemoryStream outputStream = new System.IO.MemoryStream();
            XmlTextWriter writer = new XmlTextWriter(outputStream, null);
            writer.Formatting = Formatting.Indented;
            try
            {
                trans.Transform(xmlReader, writer);
            }
            catch (Exception ex)
            {
                Exception displayException = ex;
                while (displayException.InnerException != null)
                {
                    displayException = displayException.InnerException;
                }
                Trace.WriteLine("XSLT execution error : {0}", displayException.Message);
                return "";
            }

            return System.Text.Encoding.UTF8.GetString(outputStream.GetBuffer(), 0, (int)outputStream.Length);
        }

        /// <summary>
        /// Test to see if the xpath expression would result in any
        /// matches in the input XML.
        /// </summary>
        /// <param name="xPathExpresion">the xpath expresion to test with</param>
        /// <param name="inputXml">The XML to be matched.</param>
        /// <returns>True if running the xpath expression on the input XML generates results.</returns>
        public override bool doesXPathMatch(string xPathExpresion, string inputXml)
        {
            try
            {
                UTF8Encoding encoding = new UTF8Encoding();
                System.IO.MemoryStream streamXML =
                    new System.IO.MemoryStream(encoding.GetBytes(inputXml));
                XmlTextReader xmlReader = new System.Xml.XmlTextReader(streamXML);

                XPathDocument doc = new XPathDocument(xmlReader);
                XPathNavigator nav = doc.CreateNavigator();
                return nav.Select(xPathExpresion).Count > 0;
            }
            catch (Exception ex)
            {
                Exception displayException = ex;
                while (displayException.InnerException != null)
                {
                    displayException = displayException.InnerException;
                }
                Trace.WriteLine("XPath error : {0}",  displayException.Message);
            }
            return false;
        }

        /// <summary>
        /// Expose System.Uri.IsWellFormedUriString() to C++
        /// </summary>
        /// <param name="checkUri">The URI you want to check.</param>
        /// <returns>True if the URI is well formed.</returns>
        public override bool isWellFormedURI(string checkUri)
        {
            return System.Uri.IsWellFormedUriString(checkUri, UriKind.RelativeOrAbsolute);
        }

        public override string setDefaultValueForUrlQueryParameter(string baseUri, string key, string value)
        {
            var builder = new UriQueryBuilder(baseUri);

            builder.SetDefaultParameter(key, value);

            return builder.ToString();
        }

        public override string overwriteUrlQueryParameter(string baseUri, string key, string value)
        {
            var builder = new UriQueryBuilder(baseUri);

            builder.OverwriteParameter(key,value);

            return builder.ToString();
        }

        public override string removeUrlQueryParameter(string baseUri, string key)
        {
            var builder = new UriQueryBuilder(baseUri);

            builder.RemoveParameter(key);

            return builder.ToString();
        }

        public override string getUrlQueryParameter(string baseUri, string key)
        {
            var builder = new UriQueryBuilder(baseUri);

            return builder.Parameters[key]??"";
        }

        public override string getUrlHost(string baseUri)
        {
            var uri = new Uri(baseUri);

            return uri.Host;
        }

        /// <summary>
        /// Serialize vector of strings into a string in XML format.
        /// </summary>
        /// <param name="serializeMe">The vector of strings to serialze.</param>
        /// <returns>The XML representation of the vector of strings.</returns>
        public override string XMLSerialize(Vector_String serializeMe)
        {
            return Pyxis.Utilities.XmlTool.ToXml(serializeMe);
        }

        /// <summary>
        /// Deserialize an XML blob into a vector of strings.
        /// </summary>
        /// <param name="target">The vector to populate with strings.</param>
        /// <param name="source">The XML to parse.</param>
        /// <returns>True if the deserialize went OK.</returns>
        public override bool XMLDeserialize(Vector_String target, String source)
        {
            try
            {
                Vector_String result = Pyxis.Utilities.XmlTool.FromXml<Vector_String>(source);
                target.Clear();
                foreach (string entry in result)
                {
                    target.Add(entry);
                }
            }
            catch (Exception)
            {
                target.Clear();
                return false;
            }
            return true;
        }

        /// <summary>
        /// Create a manifest from all the files refered to by the IPath process
        /// and return the manifest in its XML serialized form.
        /// </summary>
        /// <param name="spPathProc">The IPath process to create a manifest for.</param>
        /// <returns>The serialized manifest.</returns>
        public override string getSerializedManifest(IPath_SPtr spPathProc)
        {
            try
            {
                Pyxis.Utilities.Manifest manifest = new Pyxis.Utilities.Manifest();
                manifest.InstallationDirectory = "";

                int fileCount = spPathProc.getLength();
                if (fileCount == 0)
                {
                    return "";
                }
                
                for (int count = 0; count < fileCount; ++count)
                {
                    string path = spPathProc.getPath(count);

                    // First entry is allowed to be a directory
                    if (count == 0 && Directory.Exists(path))
                    {
                        continue;
                    }

                    System.IO.FileInfo info = new System.IO.FileInfo(path);
                    if (!info.Exists)
                    {
                        // Return an empty manifest for a missing.
                        return "";
                    }
                    
                    Pyxis.Utilities.ManifestEntry entry =
                        new Pyxis.Utilities.ManifestEntry(info, "");
                    
                    // If any file does not have a checksum calculated yet, then we can not 
                    // return the manifest.
                    if (entry.FileStamp.Length == 0)
                    {
                        // TODO: Connect the process to the manifest entry, and notify spPathProc when it's updated.
                        return "";
                    }
                    
                    manifest.Entries.Add(entry);
                }

                return manifest.SerializeManifestToString();
            }
            catch (System.IO.FileNotFoundException)
            {
                // Return an empty manifest for a missing.
                return "";
            }
            catch (Exception ex)
            {
                // If there is an exception then return an empty manifest.
                Trace.WriteLine("Unexpected exception in getSerializedManifest: {0}", ex.ToString());
                return "";
            }
        }

        /// <summary>
        /// Create a manifest from the file specified
        /// and return the manifest in its XML serialized form.
        /// </summary>
        /// <param name="filename">The name of the file to create a manifest for.</param>
        /// <returns>The serialized manifest as a string.</returns>
        public override string getSerializedManifestForFile(string filename)
        {
            try
            {
                Pyxis.Utilities.Manifest manifest = new Pyxis.Utilities.Manifest();
                manifest.InstallationDirectory = "";
                System.IO.FileInfo info = new System.IO.FileInfo(filename);
                Pyxis.Utilities.ManifestEntry entry =
                    new Pyxis.Utilities.ManifestEntry(info, "");
                // If the file does not have a checksum calculated yet, then we can not 
                // return the manifest.
                if (entry.FileStamp.Length == 0)
                {
                    return "";
                }
                manifest.Entries.Add(entry);
                return manifest.SerializeManifestToString();
            }
            catch (Exception)
            {
                // If there is an exception then return an empty manifest.
                return "";
            }
        }

        public override string getIdentity(string strManifest)
        {
            StringBuilder result = new StringBuilder();

            Pyxis.Utilities.Manifest manifest = Pyxis.Utilities.XmlTool.FromXml<Pyxis.Utilities.Manifest>(strManifest);

            foreach (Pyxis.Utilities.ManifestEntry entry in manifest.Entries)
            {
                // if any checksum is blank, then we need to return a blank identity
                if (entry.FileStamp == "")
                {
                    return "";
                }

                result.Append(entry.FileStamp);
            }

            return result.ToString();
        }


        #region PYXObject Lifetime Management

        #region IDirectorReferenceCounter Members

        public void setSwigCMemOwn(bool value)
        {
            swigCMemOwn = value;
        }

        public int doAddRef()
        {
            return base.addRef();
        }

        public int doRelease()
        {
            return base.release();
        }

        #endregion

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.addRef(this);
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.release(this);
        }

        #endregion

        /// <summary>
        /// Default constructor.
        /// </summary>
        public ManagedCSharpFunctionProvider()
        {
        }

        #endregion PYXObject Lifetime Management
    }
}
