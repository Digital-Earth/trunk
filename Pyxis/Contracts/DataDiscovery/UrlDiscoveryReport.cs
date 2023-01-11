using System;
using System.ComponentModel;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.DataDiscovery
{
    /// <summary>
    /// Url Discovery Report is generated to give statsitics on how many datasets a url discovery operation has return. 
    /// </summary>
    public class UrlDiscoveryReport
    {
        public string Uri { get; set; }

        public string Host { get; set; }

        public string Status { get; set; }

        public string ServiceIdentifier { get; set; }

        public int DataSetCount { get; set; }
        
        public int VerifiedDataSetCount { get; set; }
        
        public int BrokenDataSetCount { get; set; }

        public int UnknownDataSetCount { get; set; }

        public DateTime LastDiscovered { get; set; }

        public DateTime LastVerified { get; set; }

        public SimpleMetadata Metadata { get; set; }

        [DefaultValue(typeof(TimeSpan), "1.00:00:00")]
        public TimeSpan RefreshRate { get; set; }

        public static UrlDiscoveryReport FromUri(string uri)
        {
            string host = new Uri(uri).Host;
            return new UrlDiscoveryReport()
            {
                Uri = uri,
                Host = host,
                ServiceIdentifier = string.Empty,
                LastDiscovered = DateTime.MinValue,
                LastVerified = DateTime.MinValue,
                DataSetCount = 0,
                VerifiedDataSetCount = 0,
                BrokenDataSetCount = 0,
                UnknownDataSetCount = 0,
                RefreshRate = TimeSpan.FromDays(7),
                Status = "New",
                Metadata = new SimpleMetadata()
                {
                    Name = new Uri(uri).Host,
                    Description = ""
                }
            };
        }
    }
}