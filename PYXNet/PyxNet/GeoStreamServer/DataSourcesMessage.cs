/******************************************************************************
DataSourcesMessage.cs

begin      : November 11, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    public class DataSourcesMessage : GeoStreamServerMessage
    {
        public const string MessageID = "GSds";

        public override string GetMsgID()
        {
            return MessageID;
        }

        public override string GetMsgName()
        {
            return "GeoStream-DataSources";
        }

        [Serializable]
        public class PublishedItem
        {
            public string ProcRef { get; set; }
            public string Name { get; set; }
            public string Desc { get; set; }

            /// <summary>
            /// Indicates whether the Pipeline has been stored into the Server Library. this mean the pipeline definition is valid.
            /// </summary>
            public bool Imported { get; set; }

            /// <summary>
            /// Indicates whether the Pipeline is been published over the GeoWeb. Note that GeoWeb Streem Server can have local pipelines that are not published over the web
            /// </summary>
            public bool Published { get; set; }

            /// <summary>
            /// Indicates whether the Raw datafiles that used for this pipeline has been downloaded to the local GESS
            /// </summary>
            public bool Downloaded { get; set; }

            /// <summary>
            /// Indicates whether the pipeline has been processed and chached on the local GWSS
            /// </summary>
            public bool Processed { get; set; }

            /// <summary>
            /// Indicates the processing progress of the pipeline
            /// </summary>
            public string ProcessingProgress { get; set; }

            /// <summary>
            /// The PipelineDefinition used to import this pipeline
            /// </summary>
            public string PipelineDefinition { get; set; }
        }

        [Serializable]
        public enum RequestMode
        {
            Summary,
            Full
        }

        [Serializable]
        public class RequestOptions
        {
            public RequestMode Mode { get; set; }

            /// <summary>
            /// if set, the query will only return information about the given pipeline
            /// </summary>
            public string DataSourceProcRef { get; set; }

            static public RequestOptions Basic
            {
                get { return new RequestOptions {DataSourceProcRef = "", Mode = RequestMode.Summary}; }
            }

            static public RequestOptions Full
            {
                get { return new RequestOptions {DataSourceProcRef = "", Mode = RequestMode.Full}; }
            }

            public RequestOptions FullForPipeline(Guid dataset, int version)
            {
                return new RequestOptions { DataSourceProcRef = dataset.ToString() + "[" + version + "]", Mode = RequestMode.Full };
            }
        }

        private List<PublishedItem> m_publishedItems = null;

        public List<PublishedItem> PublishedItems
        {
            get
            {
                if (m_publishedItems == null)
                {
                    m_publishedItems = new List<PublishedItem>();
                }
                return m_publishedItems;
            }
        }

        public RequestOptions Options { get; set; }
        public string Note { get; set; }

        #region Convert to/from message format

        /// <summary>
        /// Append the UsageReportMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.(will be modified)</param>
        /// <returns></returns>
        public override void ToMessage(Message message)
        {
            base.ToMessage(message);

            message.Append(Note);
            message.Append(Pyxis.Utilities.XmlTool.ToXml(PublishedItems) );
            message.Append(Pyxis.Utilities.XmlTool.ToXml(Options));
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a UsageReportMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public override void FromMessageReader(MessageReader reader)
        {
            base.FromMessageReader(reader);

            Note = reader.ExtractUTF8();
            string rawXml = reader.ExtractUTF8();
            m_publishedItems = Pyxis.Utilities.XmlTool.FromXml<List<PublishedItem>>(rawXml);

            rawXml = reader.ExtractUTF8();
            Options = Pyxis.Utilities.XmlTool.FromXml<RequestOptions>(rawXml);
        }

        #endregion

    }
}
