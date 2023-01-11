using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DLM
{
    /// <summary>
    /// The client makes a publication request, passing its ID, plus the 
    /// metadata on the dataset.  (The metadata will typically include a 
    /// checksum on the raw datafile, or the dataset ID of a source dataset, 
    /// plus operation id on any operation that generated the dataset 
    /// (sampling algorithm, etc)
    /// </summary>
    class PublicationRequestMessage: Message 
    {
        public PublicationRequestMessage()
            : base(MessageTypes.PublicationRequest)
        {
        }

        private NodeIdentity m_PublishingNode;
        public NodeIdentity PublishingNode
        {
            get { return m_PublishingNode; }
            set { m_PublishingNode = value; }
        }

        private DataSetIdentity m_PublishedDataSet;
        public DataSetIdentity PublishedDataSet
        {
            get { return m_PublishedDataSet; }
            set { m_PublishedDataSet = value; }
        }

        #region IMessage Members

        public override byte[] Contents 
        {
            get
            {
                return null;
                //byte[] results = new byte[ m_PublishedDataSet.Contents.GetLength +
                //    m_PublishingNode]();
                //m_PublishedDataSet.Contents.CopyTo( results, results.Length);
                //return results;
            }
            set
            {
                throw new Exception("The method or operation is not implemented.");
            }
        }

        #endregion
    }
}
