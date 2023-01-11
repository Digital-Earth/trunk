using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities.Logging
{
    /// <summary>
    /// A single line in our LogRepository
    /// </summary>
    [DataContract]
    public class LogRecord
    {
        /// <summary>
        /// When the message occured in UTC timezone
        /// </summary>
        [DataMember(Name = "time")]
        public DateTime TimeStamp { get; set; }

        /// <summary>
        /// the Node GUID (set up by pyxnet automaticly)
        /// </summary>
        [DataMember(Name = "id")]
        public Guid NodeID { get; set; }

        /// <summary>
        /// the Node Name (set up by pyxnet automaticly)
        /// </summary>
        [DataMember(Name = "name")]
        public string NodeName { get; set; }

        /// <summary>
        /// the record category.
        /// </summary>
        [DataMember(Name = "cat")]
        public string Category { get; set; }

        /// <summary>
        /// the recrod Key. cloud be anything or Error/Warning.
        /// </summary>
        [DataMember(Name = "key")]
        public string Key { get; set; }

        /// <summary>
        /// the value or the error/warning message
        /// </summary>
        [DataMember(Name = "val")]
        public string Value { get; set; }

        public LogRecord()
        {
            TimeStamp = DateTime.UtcNow;
        }
    }
}
