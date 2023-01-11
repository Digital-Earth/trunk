/******************************************************************************
PyxNetFileResolver.cs

begin      : 30/03/2007 2:36:01 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;
using PyxNet;
using PyxNet.DataHandling;
using PyxNet.Publishing.Files;

namespace ApplicationUtility
{
    /// <summary>
    /// A file resolver agent that is capable of getting files from PyxNet.
    /// It also watches files that are resolved and publishes them.
    /// </summary>
    class FileResolver : ResolverAgent 
    {
        /// <summary>
        /// The directory that we will use to hold files that have been transfered
        /// over PyxNet.
        /// </summary>
        private readonly string m_saveToDirectory;

        /// <summary>
        /// A file publisher that is used to publish every file that is resolved.
        /// </summary>
        private readonly FilePublisher m_filePublisher;

        /// <summary>
        /// The stack to publish and query on.
        /// </summary>
        private readonly Stack m_stack;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="stack">The stack to publish and query on.</param>
        /// <param name="saveToDirectory">The directory in which to store transferred files.</param>
        public FileResolver(Stack stack, String saveToDirectory)
        {
            if (saveToDirectory.Length == 0)
            {
                m_saveToDirectory = System.IO.Path.GetTempPath();
            }
            else
            {
                m_saveToDirectory = saveToDirectory;
            }

            m_stack = stack;
            m_filePublisher = new FilePublisher(stack);
        }

        /// <summary>
        /// This function is called when any file has been successfully resolved
        /// and the PyxNet resolver will look at this, and automatically publish
        /// this file so that other PyxNet users can get the file.
        /// </summary>
        /// <param name="filename">The fully qualified name of the file which was resolved.</param>
        public override void OnFileResolved(string filename)
        {
            try
            {
                System.IO.FileInfo theFile = new System.IO.FileInfo(filename);
                if (theFile.Exists)
                {
                    m_filePublisher.Publish(theFile, "Raw_Pyxis_data_file.", 8192);
                }
            }
            catch
            {
            }
        }

        /// <summary>
        /// Look for a file over PyxNet to see if it is available anywhere and get it.
        /// This call may be slow.
        /// </summary>
        /// <param name="filename">The file to look for.</param>
        /// <returns>The fully qualified name of the resolved file.</returns>
        public override string ResolveFile(string filename)
        {
            // query for the file
            QueryResultList results = new QueryResultList(m_stack, filename);
            results.WaitForResults(1, 10);
            results.Stop();

            // no results means no file.
            if (results.Count == 0)
            {
                return "";
            }

            // make a list of providers
            List<NodeInfo> providerList = new List<NodeInfo>(results.Count);
            foreach (QueryResult qr in results)
            {
                // TODO: check the hash code to make sure it is the file that we want?
                // we can't know the hashcode here, so how do we know it is a file or the right file??
                providerList.Add(qr.ResultNode);
            }

            DataDownloader downloader = new DataDownloader(results[0].MatchingDataSetID,
                m_stack, providerList);

            // create the proper file info
            string downloadedFilename = m_saveToDirectory + filename;
            System.IO.FileInfo theFile = new System.IO.FileInfo(downloadedFilename);

            if (!downloader.DownloadFile(theFile))
            {
                return "";
            }

            m_filePublisher.Publish(theFile, "", downloader.DownloadedDataInfo);

            return downloadedFilename;
        }
    }
}
