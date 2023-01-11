/******************************************************************************
QueryHashTable.cs

begin      : 11/01/2007 1:46:24 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using PyxNet.DataHandling;

namespace PyxNet
{
    /// <summary>
    /// Maintain a Query Hash Table.
    /// </summary>
    public class QueryHashTable : ITransmissible
    {
        public readonly Pyxis.Utilities.NumberedTraceTool<QueryHashTable> Tracer =
            new Pyxis.Utilities.NumberedTraceTool<QueryHashTable>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// Size constants for a 20 bit hash table. Set NumberOfBits to 5 bits if you want
        /// a 4 byte long hash table so that the bytes that are being manipulated can be 
        /// easily written down and debugged.
        /// </summary>
        const int NumberOfBits = 20;
        const int SizeInBits = 1 << NumberOfBits;
        const int SizeInBytes = (SizeInBits + 7) >> 3;
        /// <summary>
        /// The storage for which hashes have been added to the table.
        /// </summary>
        private System.Collections.BitArray m_bits = 
            new System.Collections.BitArray(SizeInBits, false);

        /// <summary>
        /// Storage for CompressdMessages property.
        /// </summary>
        private bool m_compressMessages = true;

        /// <summary>
        /// Set to true to use gzip compression for the QHaT messages.
        /// </summary>
        public bool CompressedMessages
        {
            get { return m_compressMessages; }
            set { m_compressMessages = value; }
        }

        /// <summary>
        /// Thread saftey member.
        /// </summary>
        private object m_lockThis = new object();

        /// <summary>
        /// Controls the raising of the OnChangeEvent.  If this is zero
        /// we will raise events otherwise we will not.
        /// </summary>
        private int m_notificationsSuspended = 0;

        /// <summary>
        /// Remembers if we have a OnChangeEvent that needs to be fired
        /// when the Updates are turned back on.
        /// </summary>
        private bool m_notificationPending = false;

        #region Change Event

        /// <summary>
        /// Turns off the OnChange event notification until EndUpdate
        /// is called.
        /// </summary>
        public void BeginUpdate()
        {
            System.Threading.Interlocked.Increment(ref m_notificationsSuspended); 
        }

        /// <summary>
        /// Turns OnChange event notification back on after a BeginUpdate.
        /// </summary>
        public void EndUpdate()
        {
            System.Threading.Interlocked.Decrement(ref m_notificationsSuspended);
            if (m_notificationPending && (m_notificationsSuspended == 0))
            {
                OnChangeRaise();
            }
        }

        /// <summary>
        /// The OnChange event is fired when the query hash table
        /// values have changed.  If one wants to do a series of
        /// changes to the QueryHashTable, the BeginUpdate and
        /// EndUpdate methods can wrapped around the updates so that only
        /// one OnChange event is fired at the end of the changes.
        /// </summary>
        public class ChangeEventArgs : EventArgs
        {
            private QueryHashTable m_QueryHashTable;

            public QueryHashTable QueryHashTable
            {
                get { return m_QueryHashTable; }
                set { m_QueryHashTable = value; }
            }

            internal ChangeEventArgs(QueryHashTable theQueryHashTable)
            {
                m_QueryHashTable = theQueryHashTable;
            }
        }

        public event EventHandler<ChangeEventArgs> OnChange
        {
            add
            {
                m_OnChange.Add(value);
            }
            remove
            {
                m_OnChange.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<ChangeEventArgs> m_OnChange = new Pyxis.Utilities.EventHelper<ChangeEventArgs>();

        protected void OnChangeRaise()
        {
            if (m_notificationsSuspended > 0)
            {
                m_notificationPending = true;
            }
            else
            {
                m_notificationPending = false;
                m_OnChange.Invoke(this, new ChangeEventArgs(this));
            }
        }

        #endregion Change Event

        #region Construction
        /// <summary>
        /// Default constructor
        /// </summary>
        public QueryHashTable()
        {
        }

        /// <summary>
        /// Construct from a PyxNet Message.
        /// </summary>
        /// <param name="message">The message to construct from.</param>
        public QueryHashTable(Message message)
        {
            FromMessage(message);
        }
        #endregion

        #region Operations

        /// <summary>
        /// Add a string to the hash table.  Will trigger the OnChange event
        /// if the hash table changes.
        /// </summary>
        /// <param name="value">The string value to add.</param>
        public void Add(string value)
        {
            Tracer.DebugWriteLine("Adding string '{0}'.", value);

            lock (m_lockThis)
            {
                int hashIndex = HashCode(value);
                if (!m_bits[hashIndex])
                {
                    m_bits[hashIndex] = true;

                    System.Diagnostics.Debug.Assert(MayContain(value));
                    Tracer.DebugWriteLine("String '{0}' added.", value);

                    OnChangeRaise();
                }
                else
                {
                    System.Diagnostics.Debug.Assert(MayContain(value));
                    Tracer.DebugWriteLine("String '{0}' already added.", value);
                }
            }
        }

        /// <summary>
        /// Add all the possibilities from another Query Hash Table into this one.
        /// </summary>
        /// <param name="value">The hash table that you want to add.</param>
        public void Add(QueryHashTable value)
        {
            Tracer.DebugWriteLine("Adding qht.");

            if (value != null)
            {
                lock (m_lockThis)
                {
                    m_bits = m_bits.Or(value.m_bits);
                    Tracer.DebugWriteLine("QHT added.", value);

                    OnChangeRaise();
                }
            }
        }

        /// <summary>
        /// See if a string "hits" the hash table.
        /// </summary>
        /// <param name="value">The string value that you wish to test.</param>
        /// <returns></returns>
        public bool MayContain(string value)
        {
            lock (m_lockThis)
            {
                bool mayContain = true;

                if (m_bits[HashCode(value)])
                {
                    // The whole expression was matched.  This is probably a 
                    // non-text query, or a single word query.
                }
                else
                {
                    // Since we add words individually to the qht, we need to 
                    // check them individually as well.  If any words in the
                    // query are missed, then there can be no hits.
                    foreach (string word in value.Split(null))
                    {
                        if (m_bits[HashCode(word)] == false)
                        {
                            mayContain = false;
                            break;
                        }
                    }
                }

                Tracer.DebugWriteLine("{0} contain '{1}'.",
                    (mayContain ? "May" : "Doesn't"), value);

                return mayContain;
            }
        }

        /// <summary>
        /// Clear the current query hash table (set all bits to false)
        /// </summary>
        private void Clear()
        {
            m_bits = new System.Collections.BitArray(SizeInBits, false);
        }

        /// <summary>
        /// Set the query hash table bit to be equal to the passed in table's
        /// </summary> 
        public void Set(QueryHashTable value)
        {
            Clear();
            Add(value);
        }

        /// <summary>
        /// Convert a string into a hash position.
        /// </summary>
        /// <param name="value">The string value that you wish to hash.</param>
        /// <returns></returns>
        private int HashCode(string value)
        {
            // use a 20 bit G2 hash.
            return G2Hash(value, value.Length, NumberOfBits);
        }

        /// <summary>
        /// The implementation of a hash code taken from Gnutella 2.  You can hash all
        /// or just part of the value passed in by changing the length parameter.
        /// </summary>
        /// <param name="value">The string value that you wish to hash.</param>
        /// <param name="len">The length of the string that you wish to hash.
        /// Can be less than the length of the string.</param>
        /// <param name="bits">The number of bits to use for the returned hash value.</param>
        /// <returns>The hash value.</returns>
        private static int G2Hash(string value, int len, int bits)
        {
            int num = 0;
            int pos = 0;
            for (int i = 0; i < len; i++)
            {
                int val = (int)char.ToLower(value[i]) & 0xFF;
                val <<= (pos * 8);
                pos = (pos + 1) & 3;
                num ^= val;
            }
            UInt64 nProduct = (UInt64)num * (UInt64)0x4F1BBCDC;
            UInt64 nHash = (nProduct << 32) >> (32 + (32 - bits));
            return (int)nHash;
        }
        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the Query Hash Table.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(StackConnection.QueryHashTableMessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the Query Hash Table to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            lock (m_lockThis)
            {
                byte[] byteTable = new byte[SizeInBytes];
                m_bits.CopyTo(byteTable, 0);
                DataPackage messagifier = new DataPackage(byteTable, m_compressMessages, false);
                messagifier.ToMessage(message);
            }
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(StackConnection.QueryHashTableMessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a Query Hash Table message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a QueryHashTable Message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a Query Hash Table.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            lock (m_lockThis)
            {
                DataPackage demessagifier = new DataPackage(reader);
                m_bits = new System.Collections.BitArray(demessagifier.Data);
            }
        }
        #endregion
    }
}

