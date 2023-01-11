/******************************************************************************
CoverageRequestMessage.cs

begin      : May 4, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Pyxis
{
    public class CoverageRequestHelper
    {
        // TODO: find the filenames for the various definition files
        // by a cleaner method.  ie. maybe get it from the DefaultCoverage, or maybe 
        // this and the default coverage get the names form the same place.

        public static string FileName(CoverageRequestMessage.TransferMode tm)
        {
            switch (tm)
            {
                case CoverageRequestMessage.TransferMode.DataSourceDefinition:
                    return "defn.txt";
                case CoverageRequestMessage.TransferMode.DataSourceValues:
                    return "defn_data.txt";
                case CoverageRequestMessage.TransferMode.CoverageDefinition:
                    return "coverage_defn.txt";
                case CoverageRequestMessage.TransferMode.Geometry:
                    return "geometry.pyx";
            }
            throw new ArgumentException("We don't know the filename for the transfer mode.");
        }
    }

    /// <summary>
    /// This message is used as embedded extra information in the normal data transfer
    /// messages to find info and get data from data publishers.
    /// </summary>
    public class CoverageRequestMessage : ITransmissible
    {
        public const string MessageID = "Covr";

        /// <summary>
        /// The transfer mode controls what portion of the data we are transferring.  A Coverage
        /// is actually made up of many tile files, and some definition files.  These files are all 
        /// addresssed individually through this messsage.
        /// </summary>
        public enum TransferMode 
            { DataSourceDefinition, DataSourceValues, CoverageDefinition, Tile, Geometry };

        #region Properties

        /// <summary>
        /// Storage for mode indicator.
        /// </summary>
        private TransferMode m_mode;

        /// <summary>
        /// Indicates what portion of the coverage we are transferring.
        /// </summary>
        public TransferMode Mode
        {
            get { return m_mode; }
            set { m_mode = value; }
        }

        /// <summary>
        /// Storage for the tile index as a string.
        /// </summary>
        private string m_tileIndex;

        /// <summary>
        /// The tile index as a string.  Only used if the transfer mode is Tile.
        /// </summary>
        public string TileIndex
        {
          get { return m_tileIndex; }
          set { m_tileIndex = value; }
        }

        /// <summary>
        /// Storage for the CacheTileDepth property.
        /// </summary>
        private int m_cacheTileDepth;

        /// <summary>
        /// The depth (in Pyxis resolutions) of the tile to transfer.
        /// </summary>
        public int CacheTileDepth
        {
            get { return m_cacheTileDepth; }
            set { m_cacheTileDepth = value; }
        }

        /// <summary>
        /// Storage for the version of the process that we are publishing.
        /// </summary>
        private int m_processVersion;

        /// <summary>
        /// The version of the process that we are publishing.
        /// </summary>
        public int ProcessVersion
        {
            get { return m_processVersion; }
            set { m_processVersion = value; }
        }

        /// <summary>
        /// Storage for the cell resolution of the process that we are publishing.
        /// </summary>
        private int m_cellResolution;

        /// <summary>
        /// The cell resolution of the process that we are publishing.
        /// </summary>
        public int CellResolution
        {
            get { return m_cellResolution; }
            set { m_cellResolution = value; }
        }

        /// <summary>
        /// Storage for the pipeline definition of this coverage.
        /// </summary>
        private string m_pipelineDefinition;

        /// <summary>
        /// The pipeline definition of this coverage.
        /// </summary>
        public string PipelineDefinition
        {
            get { return m_pipelineDefinition; }
            set { m_pipelineDefinition = value; }
        }

        /// <summary>
        /// Storage for the geometry that this coverage request is interested in.
        /// </summary>
        private PYXGeometry_SPtr m_geometry = null;

        /// <summary>
        /// Cache the serialized version of the geometry.
        /// </summary>
        private string m_serializedGeometry = null;

        /// <summary>
        /// The geometry that this coverage request is interested in.
        /// </summary>
        public PYXGeometry_SPtr Geometry
        {
            get { return m_geometry; }
            set 
            { 
                m_geometry = value;
                // clear the cached serialized version of the geometry.
                m_serializedGeometry = null;
            }
        }

        /// <summary>
        /// Gets or sets the usage certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate UsageCertificate
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the Published certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate PublishedCertificate
        {
            get;
            set;
        }

        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public CoverageRequestMessage()
        {
        }

        /// <summary>
        /// Construct a CoverageRequestMessage from a message.  The message must be a 
        /// PyxNet TileRequestMessage.
        /// </summary>
        /// <param name="message"></param>
        public CoverageRequestMessage(Message message)
        {
            FromMessage(message);
        }

        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the CoverageRequestMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the CoverageRequestMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append((int)Mode);
            message.Append(TileIndex);
            message.Append(CacheTileDepth);
            message.Append(ProcessVersion);
            message.Append(CellResolution);
            message.Append(PipelineDefinition);
            if (Geometry != null && Geometry.get() != null)
            {
                // Cache the serialized geometry to eliminate multiple serializations (optimization)
                if (m_serializedGeometry == null)
                {
                    try
                    {
                        m_serializedGeometry = PYXGeometrySerializer.serialize(Geometry.get());
                    }
                    catch (Exception)
                    {
                        // Eat any exceptions here - infer that this is an invalid geometry.
                    }
                }
            }
            if (m_serializedGeometry != null)
            {
                message.Append(true);
                message.Append(m_serializedGeometry);
            }
            else
            {
                message.Append(false);
            }
            message.Append((bool)(UsageCertificate != null));
            if (UsageCertificate != null)
            {
                UsageCertificate.ToMessage(message);
            }
            message.Append((bool)(PublishedCertificate != null));
            if (PublishedCertificate != null)
            {
                PublishedCertificate.ToMessage(message);
            }
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(MessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a CoverageRequestMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a CoverageRequestMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Mode = (TransferMode)reader.ExtractInt();
            TileIndex = reader.ExtractUTF8();
            CacheTileDepth = reader.ExtractInt();
            ProcessVersion = reader.ExtractInt();
            CellResolution = reader.ExtractInt();
            PipelineDefinition = reader.ExtractUTF8();
            bool hasGeometry = reader.ExtractBool();
            Geometry = null;
            if (hasGeometry)
            {
                m_serializedGeometry = reader.ExtractUTF8();
                Geometry = PYXGeometrySerializer.deserialize(m_serializedGeometry);
            }
            // TODO: Get rid of this work-around.
            // Bail if we're at the end.  This is an "old" version of the message.
            UsageCertificate = null;
            PublishedCertificate = null;
            if (!reader.AtEnd)
            {
                bool hasUsageCertificate = reader.ExtractBool();
                if (hasUsageCertificate)
                {
                    UsageCertificate = new PyxNet.Service.Certificate(reader);
                }

                bool hasPublishCertificate = !reader.AtEnd && reader.ExtractBool();
                if (hasPublishCertificate)
                {
                    PublishedCertificate = new PyxNet.Service.Certificate(reader);
                }
            }
        }
        #endregion
    }
}

// for unit test see PyxNet.Pyxis.Test.cs

