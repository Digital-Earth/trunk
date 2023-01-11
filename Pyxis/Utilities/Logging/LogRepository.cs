using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Utilities.Logging
{
    /// <summary>
    /// LogRespository is used to record LogRecords (usally by using LogCategory object)
    /// The LogRepository class with cache the message and will submit them every 30 sec or every 1000 messages.    
    /// </summary>
    public static class LogRepository
    {
        private static List<LogRecord> CurrentLogChunk { get; set; }
        private static object CurrentLogChunkLock = new object();
        private static System.Threading.Timer uploadTimer;

        private static Guid NodeID = Guid.Empty;
        private static string NodeName = "";

        static LogRepository()
        {
            CurrentLogChunk = new List<LogRecord>();

            //default name
            NodeName = System.AppDomain.CurrentDomain.FriendlyName;

            //start sending pending messages timer (every 30 sec)
            uploadTimer = new System.Threading.Timer(UploadTimeTick, null, TimeSpan.FromSeconds(30), TimeSpan.FromSeconds(30));
        }

        public static void SetLocalNodeInfo(string Name, Guid ID)
        {
            NodeID = ID;
            NodeName = Name;
        }

        /// <summary>
        /// Add record to the log repository.
        /// </summary>
        /// <param name="record">log record to append</param>
        public static void Append(LogRecord record)
        {
            if (record == null)
            {
                return;
            }

            if (record.NodeID == Guid.Empty)
            {
                record.NodeID = NodeID;
            }

            if (String.IsNullOrEmpty(record.NodeName))
            {
                record.NodeName = NodeName;
            }

            lock (CurrentLogChunkLock)
            {
                CurrentLogChunk.Add(record);
                if (CurrentLogChunk.Count > 1000)
                {
                    ForceUpdate();
                }
            }
        }

        /// <summary>
        /// Force update of the pending log records.
        /// </summary>
        public static void ForceUpdate()
        {
            Task.Factory.StartNew(() => Flush());
        }

        /// <summary>
        /// Force update of the pending log records.
        /// </summary>
        public static void Flush()
        {
            List<LogRecord> oldChunk = new List<LogRecord>();
            lock (CurrentLogChunkLock)
            {
                if (CurrentLogChunk.Count > 0)
                {
                    oldChunk = CurrentLogChunk;
                    CurrentLogChunk = new List<LogRecord>();
                }
            }
            if (oldChunk.Count > 0)
            {
                PublishLog(oldChunk);
            }
        }

        private static void PublishLog(List<LogRecord> logChunk)
        {
            if (!System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                System.Diagnostics.Trace.WriteLine("No network, can't publish log with " + logChunk.Count + " messages");
                return;
            }

            var json = JsonConvert.SerializeObject(logChunk, new IsoDateTimeConverter());
            var client = new WebClient();

            int tryCount = 3;

            while (tryCount > 0)
            {
                try
                {
                    client.UploadString("http://www.pyxisinnovation.com/data/logging/report.php", "POST", json);
                    break;
                }
                catch (Exception e)
                {
                    tryCount--;
                    System.Diagnostics.Trace.WriteLine("Failed to uploaded log with error : " + e.Message);
                    System.Threading.Thread.Sleep(TimeSpan.FromSeconds(10));
                }
            }
        }

        private static void UploadTimeTick(object state)
        {
            ForceUpdate();
        }

    }
}
