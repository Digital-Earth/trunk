using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace LicenseServer.Models.Mongo
{
    public class ExternalData
    {
        public int Id { get; set; }

        public string DatasetId { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }

        public Uri Server { get; set; }

        private List<OgcImportOption> m_services; 
        public List<OgcImportOption> Services
        {
            get
            {
                return m_services;
            }
            set
            {
                m_services = value;
                if (m_services == null || !m_services.Any())
                {
                    return;
                }

                // only include external data that has been marked as working
                m_services = m_services.Where(s => s.Status == OgcServiceStatus.Working).ToList();
                foreach (var service in m_services)
                {
                    service.ImportUri = ConstructServiceImportUri(service);
                }
            }
        }

        public List<double> Center { get; set; }
        public Envelope Boundary { get; set; }

        private string ConstructServiceImportUri(OgcService service)
        {
            var uri = Server.ToString();
            if (service.Protocol == "WMS" || service.Protocol == "WCS" || service.Protocol == "WFS")
            {
                uri += "?service=" + service.Protocol + "&version=" + service.Version;

                switch (service.Protocol)
                {
                    case "WMS":
                        uri += "&request=GetMap&Layers="
                               + DatasetId
                               + "&format=image%2Fpng";
                        if (service.Version == "1.3.0")
                        {
                            uri += "&bbox=<EX_GeographicBoundingBox xmlns%3D\"http:%2F%2Fwww.opengis.net%2Fwms\"><westBoundLongitude>"
                                   + Boundary.Coordinates[0][0]
                                   + "<%2FwestBoundLongitude><eastBoundLongitude>"
                                   + Boundary.Coordinates[1][0]
                                   + "<%2FeastBoundLongitude><southBoundLatitude>"
                                   + Boundary.Coordinates[1][1]
                                   + "<%2FsouthBoundLatitude><northBoundLatitude>"
                                   + Boundary.Coordinates[0][1]
                                   + "<%2FnorthBoundLatitude><%2FEX_GeographicBoundingBox>";
                        }
                        else
                        {
                            uri += "&bbox=<LatLonBoundingBox minx%3D\""
                                   + Boundary.Coordinates[0][0]
                                   + "\" miny%3D\""
                                   + Boundary.Coordinates[1][1]
                                   + "\" maxx%3D\""
                                   + Boundary.Coordinates[1][0]
                                   + "\" maxy%3D\""
                                   + Boundary.Coordinates[0][1]
                                   + "\" %2F>";
                        }
                        break;
                    case "WCS":
                        uri += "&request=GetCoverage";
                        if (service.Version == "1.0.0")
                        {
                            uri += "&identifier=" + DatasetId;
                        }
                        else if (service.Version.StartsWith("1.1"))
                        {
                            uri += "&identifiers=" + DatasetId;
                        }
                        else
                        {
                            uri += "&coverageId=" + DatasetId;
                        }
                        break;
                    case "WFS":
                        uri += "&request=GetFeature&typeNames=" + DatasetId;
                        break;
                }
            } 
            else if (service.Protocol == "AGSF")
            {
                uri += "/" + DatasetId + "/query?where=objectid+%3D+objectid&outfields=*&f=json";
            }
            return uri;
        }

        public class OgcImportOption : OgcService
        {
            public string ImportUri { get; set; }
        }
    }

    public class OgcService
    {
        public string Protocol { get; set; }
        public string Version { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public OgcServiceStatus Status { get; set; }
    }

    public enum OgcServiceStatus
    {
        Unknown,
        Crawled,
        Accessible,
        Working,
        WorkingButNotUsable,
        Offline,
        Broken,
    }

    public abstract class Geometry
    {
        public string Type { get; set; }
    }

    public class Point : Geometry
    {
        public List<double> Coordinates { get; set; }   
    }

    public class Envelope : Geometry
    {
        public Envelope()
        {
        }

        public Envelope(double upperLat, double leftLon, double lowerLat, double rightLon)
        {
            Type = "Envelope";
            Coordinates = new List<List<double>>
            {
                new List<double> {leftLon, upperLat},
                new List<double> {rightLon, lowerLat}
            };
        }

        public List<List<double>> Coordinates { get; set; }
    }
}