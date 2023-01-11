using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Utilities;
using PyxisCLI.Server.Models;

namespace PyxisCLI.Workspaces
{
    /// <summary>
    /// Create a stauts dictionary from Workspace items.
    /// </summary>
    internal static class StatusFactory
    {
        public static Dictionary<string, object> Create(Endpoint endpoint)
        {
            var status = new Dictionary<string, object>();

            var endPointStatus = LocalPersistance.AttachData(endpoint).Get<UrlDiscoveryReport>("status");

            if (endPointStatus == null)
            {
                status["Status"] = "New";
            }
            else
            {
                status["Status"] = endPointStatus.Status;
                status["Service"] = endPointStatus.ServiceIdentifier;
                status["Last Discovered [date]"] = endPointStatus.LastDiscovered.ToString("s");
                status["Datasets"] = endPointStatus.DataSetCount;
            }

            return status;
        }

        public static Dictionary<string, object> Create(IImport import)
        {
            var status = new Dictionary<string, object>();

            if (import.Type == "DataSet")
            {
                var geoSource = LocalPersistance.AttachData(import).Get<GeoSource>("geosource");

                if (geoSource != null)
                {
                    status["Status"] = "Imported";
                    status["Last Imported [date]"] = geoSource.Metadata.Created;
                    status["Type"] = geoSource.Specification.OutputType.ToString();
                    status["Data Size [bytes]"] = geoSource.DataSize;
                }
                else
                {
                    status["Status"] = "New";
                }
            }
            else
            {
                status["Status"] = "Template";
            }

            return status;
        }

        public static Dictionary<string, object> Create(GlobeTemplate globe)
        {
            var status = new Dictionary<string, object>();

            status["Layers"] = globe.Layers.Count;

            return status;
        }
    }
}
