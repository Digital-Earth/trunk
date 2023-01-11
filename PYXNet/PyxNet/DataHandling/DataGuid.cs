/******************************************************************************
DataGuid.cs

begin      : 01/03/2007 12:43:26 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DataHandling
{
    /// <summary>
    /// Wrapper so that Data Guids will not be confused with other 
    /// things that are represented with Guids.
    /// </summary>
    [Serializable]
    public class DataGuid : TypedGuid
    {
        /// <summary>
        /// Default Constructor - will create a new contained Guid ID.
        /// </summary>
        public DataGuid()
            : base(Guid.NewGuid())
        {
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="dg">The DataGuid that you wish to duplicate.</param>
        public DataGuid(DataGuid dg)
            : base(dg.Guid)
        {
            // Empty body.
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public DataGuid(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        #region To/From Message
        /// <summary>
        /// Append the Data Guid to an existing message.  
        /// This does not include any message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
             message.Append(Guid);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a DataGuid.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Guid = reader.ExtractGuid();
        }
        #endregion
    }
}
