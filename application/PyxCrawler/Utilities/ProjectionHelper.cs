using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using ProjNet.Converters.WellKnownText;
using ProjNet.CoordinateSystems;
using ProjNet.CoordinateSystems.Transformations;
using PyxCrawler.Models;

namespace PyxCrawler.Utilities
{
    public static class ProjectionHelper
    {
        private static readonly Dictionary<int, string> s_srids = new Dictionary<int, string>();
        private static readonly CoordinateTransformationFactory s_ctFactory = new CoordinateTransformationFactory();
        private static readonly CoordinateSystem s_wgs84 = ProjNet.CoordinateSystems.GeographicCoordinateSystem.WGS84;

        public static void Initialize()
        {
            foreach (var wkt in SridReader.GetSRIDs())
            {
                s_srids[wkt.WKID] = wkt.WKT;
            }
        }

        public static double[] MercatorToWgs84(double mercatorX, double mercatorY)
        {
            return WkidToWgs84(3785, mercatorX, mercatorY);
        }

        public static double[] WkidToWgs84(int wkid, double x, double y)
        {
            return WktToWgs84(s_srids[wkid], x, y);
        }

        // From: https://projnet.codeplex.com/wikipage?title=CreateProjection&referringTitle=FAQ
        // *Note that no datum transformation is applied if the coordinate systems doesn't have a ToWGS84 parameter defined.
        public static double[] WktToWgs84(string wkt, double x, double y)
        {
            ICoordinateSystem cs = CoordinateSystemWktReader.Parse(wkt) as ICoordinateSystem;
            var transformation = s_ctFactory.CreateFromCoordinateSystems(cs, s_wgs84);
            return transformation.MathTransform.Transform(new[] {x, y});
        }

        // adapted from https://projnet.codeplex.com/SourceControl/latest#DemoWebApp/App_Code/SRIDReader.cs
        public static class SridReader
        {
            private static string filename = System.Web.HttpRuntime.AppDomainAppPath + "\\App_Data\\SRID.csv";

            public struct WKTstring
            {
                /// <summary>
                /// Well-known ID
                /// </summary>
                public int WKID;

                /// <summary>
                /// Well-known Text
                /// </summary>
                public string WKT;
            }

            /// <summary>
            /// Enumerates all SRID's in the SRID.csv file.
            /// </summary>
            /// <returns>Enumerator</returns>
            public static IEnumerable<WKTstring> GetSRIDs()
            {
                using (System.IO.StreamReader sr = System.IO.File.OpenText(filename))
                {
                    while (!sr.EndOfStream)
                    {
                        string line = sr.ReadLine();
                        int split = line.IndexOf(';');
                        if (split > -1)
                        {
                            WKTstring wkt = new WKTstring();
                            wkt.WKID = int.Parse(line.Substring(0, split));
                            wkt.WKT = line.Substring(split + 1);
                            yield return wkt;
                        }
                    }
                    sr.Close();
                }
            }

            /// <summary>
            /// Gets a coordinate system from the SRID.csv file
            /// </summary>
            /// <param name="id">EPSG ID</param>
            /// <returns>Coordinate system, or null if SRID was not found.</returns>
            public static ICoordinateSystem GetCSbyID(int id)
            {
                //TODO: Enhance this with an index so we don't have to loop all the lines
                ICoordinateSystemFactory fac = new CoordinateSystemFactory();
                foreach (SridReader.WKTstring wkt in SridReader.GetSRIDs())
                {
                    if (wkt.WKID == id)
                    {
                        return ProjNet.Converters.WellKnownText.CoordinateSystemWktReader.Parse(wkt.WKT) as ICoordinateSystem;
                    }
                }
                return null;
            }
        }

        public static void ProjectToWgs84(Wgs84BoundingBox bbox, int wkid)
        {
            var lowerLeft = WkidToWgs84(wkid, bbox.West, bbox.South);
            var upperRight = WkidToWgs84(wkid, bbox.East, bbox.North);
            bbox.North = upperRight[1];
            bbox.East = upperRight[0];
            bbox.South = lowerLeft[1];
            bbox.West = lowerLeft[0];
        }
    }
}