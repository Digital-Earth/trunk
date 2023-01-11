/******************************************************************************
FilePublisher.cs

begin      : 06/02/2007 2:02:32 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using System.IO;
using Pyxis.Utilities;
using PyxNet.DataHandling;

namespace PyxNet.Publishing.Files
{
    /// <summary>
    /// This class is responsible for publishing files and folders
    /// and to watch the stack for QueryHit events and then check the query and send
    /// a positive reply if the query matches one of the files in the published list.
    ///
    /// This class will also hook into the DataChunkRequest message and look for
    /// requests for data from things that it is publishing, and transfer the requested data
    /// back in a data chunk message.
    /// </summary>
    public class FilePublisher
    {
        #region Fields And Properties

        /// <summary>
        /// The stack that we are using to publish/monitor.
        /// </summary>
        private readonly Stack m_stack;

        /// <summary>
        /// Used for debug purposes only to simulate a node that doesn't send data.
        /// </summary>
        private bool m_doSendData = true;

        /// <summary>
        /// Keep track of each of the files we are publishing.
        /// </summary>
        private FileSystemPublisher m_fileSystemPublisher;

        /// <summary>
        /// A Pyxis.Utilities.TraceTool that keeps a log of where we got all our pieces from.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<FilePublisher> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<FilePublisher>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// A list of published fileInfos
        /// </summary>

        public IEnumerable<FileInfo> PublishedFiles
        {
            get
            {
                return m_fileSystemPublisher.PublishedFiles;
            }
        }

        /// <summary>
        /// Used for debug purposes only to simulate a node that doesn't send data.
        /// </summary>
        /// <remarks>
        /// Debug helper that will stop this publisher from sending data if the property
        /// is set to false.
        /// It will always give query results and DataInfo.  This should simulate a
        /// connection that becomes unresponsive on the network.
        /// </remarks>
        internal bool SendData
        {
            get { return m_doSendData; }
            set { m_doSendData = value; }
        }

        #endregion Fields And Properties

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack that you want to publish on.</param>
        public FilePublisher(Stack stack)
        {
            m_stack = stack;
            m_fileSystemPublisher = new FileSystemPublisher();
            m_stack.Publisher.PublishItem(m_fileSystemPublisher.FileRepository);
            m_stack.RegisterHandler(DataChunkRequest.MessageID, HandleChunkRequest);
            m_stack.RegisterHandler(DataInfoRequest.MessageID, HandleInfoRequest);
        }

        #region Message Handlers

        /// <summary>
        /// Handle DataChunkRequest messages.  Look for any data that we are publishing that
        /// matches the chunk request and return the chunk of data if we have it.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleChunkRequest(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            if (!m_doSendData)
            {
                return;
            }

            DataChunkRequest request = new DataChunkRequest(args.Message);

            if (request.Certificate == null)
            {
                m_tracer.WriteLine("Received a ChunkRequest without any Certificate.  Ignoring.");
                return;
            }

            DataChunk chunk = null;

            var pfi = m_fileSystemPublisher.GetPFIByDataSetID(request.DataSetID);

            if (pfi != null)
            {
                chunk = new DataChunk(pfi.FileInformation.ToFileInfo(), request.Offset, request.ChunkSize);
                chunk.DataSetID = pfi.DataInfo.DataSetID;
            }

            if (chunk != null)
            {
                args.Context.Sender.SendMessage(chunk.ToMessage());
                Pyxis.Utilities.GlobalPerformanceCounters.Counters.FileBytesUploaded.IncrementBy(
                    chunk.ChunkSize);
            }
        }

        /// <summary>
        /// Handle InfoRequest messages.  Look for any data we are publishing and
        /// return the info that is pertinent to that data.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleInfoRequest(object sender,
            PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            m_tracer.DebugWriteLine("Handling Info request from {0}", args.Context.Sender);
            DataInfoRequest request = new DataInfoRequest(args.Message);
            Message message = null;

            var pfi = m_fileSystemPublisher.GetPFIByDataSetID(request.DataSetID);
            if (pfi != null)
            {
                message = pfi.DataInfo.ToMessage();
            }

            if (message != null)
            {
                args.Context.Sender.SendMessage(message);
            }
        }

        #endregion Message Handlers

        /// <summary>
        /// Publishes the specified file.
        /// </summary>
        /// <param name="file">The file.</param>
        /// <returns></returns>
        public DataInfo Publish(FileInfo file)
        {
            return m_fileSystemPublisher.Publish(file);
        }

        /// <summary>
        /// Publish a file on PyxNet.  This form of publish file should only be used if
        /// you are publishing a file that you think is unique and never been seen on PyxNet
        /// before.
        /// </summary>
        /// <param name="file">The file that you want to publish.</param>
        /// <param name="description">A text description that will be used for PyxNet query matching.</param>
        /// <param name="chunkSize">The size of chunks that the data will be transferred in.</param>
        /// <returns>The DataInfo that was created for this file which includes the DataID.</returns>
        public DataInfo Publish(FileInfo file, string description, int chunkSize)
        {
            return m_fileSystemPublisher.Publish(file, description, chunkSize);
        }

        /// <summary>
        /// Publish a file so that it will be available to other nodes across PyxNet.  This
        /// form of publish should only be used if you already know the guid (DataID) for the
        /// file.  For instance, you recieved the file over PyxNet and thus know the DataID
        /// that the sender was publishing under.  Otherwise, use another form of PublishFile.
        /// </summary>
        /// <param name="file">The file that you want to publish.</param>
        /// <param name="description">A text description that will be used for PyxNet query matching.</param>
        /// <param name="dataInfo">The DataInfo for this file.  Contains the guid for this file.</param>
        public DataInfo Publish(FileInfo file, string description, DataInfo dataInfo)
        {
            return m_fileSystemPublisher.Publish(new FileInformation(file), description, dataInfo);
        }

        /// <summary>
        /// Publishes all the files in the given directory and then watches the
        /// directory for changes so that new files will also be published, and
        /// deleted files will cease to be published.
        ///
        /// </summary>
        /// <param name="directory">The directory that you want to publish.</param>
        public DelayedFileSystemWatcher PublishDirectory(DirectoryInfo directory)
        {
            return PublishDirectory(directory, false);
        }

        /// <summary>
        /// Publishes all the files in the given directory and then watches the
        /// directory for changes so that new files will also be published, and
        /// deleted files will cease to be published.
        /// </summary>
        /// <param name="directory">The directory that you want to publish.</param>
        /// <param name="includeSubdirectories">if set to <c>true</c> [recurse].</param>
        /// <returns></returns>
        public DelayedFileSystemWatcher PublishDirectory(DirectoryInfo directory, bool includeSubdirectories)
        {
            return m_fileSystemPublisher.PublishDirectory(directory, includeSubdirectories);
        }

        /// <summary>
        /// Unhook from the stack and stop publishing.
        /// </summary>
        public void StopPublishing()
        {
            // unregister the messages that we want to monitor.
            m_stack.UnregisterHandler(DataChunkRequest.MessageID, HandleChunkRequest);
            m_stack.UnregisterHandler(DataInfoRequest.MessageID, HandleInfoRequest);
            m_fileSystemPublisher.StopPublishing();
        }

        public bool Unpublish(FileInfo fileInfo)
        {
            return m_fileSystemPublisher.Unpublish(fileInfo);
        }

        /// <summary>
        /// Stop publishing a directory.  Remove all of the files in this
        /// directory from the list of published files, and stop watching
        /// for changes in the directory.
        /// </summary>
        /// <param name="directory"></param>
        public void UnpublishDirectory(DirectoryInfo directory)
        {
            m_fileSystemPublisher.UnpublishDirectory(directory);
        }
    }
}