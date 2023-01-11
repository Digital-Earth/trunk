using ApplicationUtility;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.Core.IO
{
    /// <summary>
    /// SpatialReferenceSystem is used to specify which projection to use when importing legacy GIS files.
    /// </summary>
    public static class SpatialReferenceSystemExtensions
    {
       
        /// <summary>
        /// Create SRSProcess based on the SpatialReferenceSystem settings
        /// </summary>
        /// <returns>IProcess_SPtr that can be use as ISRS</returns>
        public static IProcess_SPtr CreateSRSProcess(this SpatialReferenceSystem srs)
        {
            if (!srs.Projection.HasValue && srs.CoordinateSystem == SpatialReferenceSystem.Systems.Projected)
            {
                throw new Exception("Projected System is required to define projection method.");
            }

            var processInfo = new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.SRS)
                    .AddAttribute("system", srs.CoordinateSystem.ToString().ToLower())
                    .AddAttribute("datum", srs.Datum.ToString().ToLower());                   

            if (srs.Projection == SpatialReferenceSystem.Projections.UTM)
            {
                if (!srs.UtmNorth.HasValue)
                {
                    throw new Exception("UTM projection requires north or south to be set.");
                }

                if (!srs.UtmZone.HasValue)
                {
                    throw new Exception("UTM projection requires UTM zone to be set.");
                }

                processInfo
                    .AddAttribute("projection", srs.Projection.Value.ToString().ToLower())
                    .AddAttribute("utm_hemi", srs.UtmNorth.Value ? "north" : "south")
                    .AddAttribute("utm_zone", srs.UtmZone.Value.ToString());
            }
            else if (srs.Projection == SpatialReferenceSystem.Projections.Custom)
            {
                if (String.IsNullOrEmpty(srs.CustomProjection))
                {
                    throw new Exception("Custom projection require set the CustomProjection field.");
                }

                processInfo
                    .AddAttribute("projection", srs.Projection.Value.ToString().ToLower())
                    .AddAttribute("custom_projection", srs.CustomProjection ?? "");
            }

            return PYXCOMFactory.CreateProcess(processInfo);
        }

        /// <summary>
        /// Create ICoordConverter_SPtr based on the SpatialReferenceSystem settings
        /// </summary>
        /// <returns>created ICoordConverter_SPtr</returns>
        public static ICoordConverter_SPtr CreateCoordConverter(this SpatialReferenceSystem srs)
        {
            var pyxSrs = new PYXSpatialReferenceSystem();

            switch (srs.CoordinateSystem)
            {
                case SpatialReferenceSystem.Systems.Geographical:
                    pyxSrs.setSystem(PYXSpatialReferenceSystem.eSystem.knSystemGeographical);
                    break;
                case SpatialReferenceSystem.Systems.Projected:
                    pyxSrs.setSystem(PYXSpatialReferenceSystem.eSystem.knSystemProjected);
                    break;
            }

            switch (srs.Datum)
            {
                case SpatialReferenceSystem.Datums.NAD27:
                    pyxSrs.setDatum(PYXSpatialReferenceSystem.eDatum.knDatumNAD27);
                    break;
                case SpatialReferenceSystem.Datums.NAD83:
                    pyxSrs.setDatum(PYXSpatialReferenceSystem.eDatum.knDatumNAD83);
                    break;
                case SpatialReferenceSystem.Datums.WGS72:
                    pyxSrs.setDatum(PYXSpatialReferenceSystem.eDatum.knDatumWGS72);
                    break;
                case SpatialReferenceSystem.Datums.WGS84:
                    pyxSrs.setDatum(PYXSpatialReferenceSystem.eDatum.knDatumWGS84);
                    break;
            }

            if (srs.Projection.HasValue)
            {
                switch (srs.Projection.Value)
                {
                    case SpatialReferenceSystem.Projections.Custom:
                        pyxSrs.setProjection(PYXSpatialReferenceSystem.eProjection.knCustomProjection);
                        pyxSrs.setWKT(srs.CustomProjection);
                        break;
                    case SpatialReferenceSystem.Projections.UTM:
                        pyxSrs.setProjection(PYXSpatialReferenceSystem.eProjection.knProjectionUTM);

                        if (srs.UtmNorth.HasValue)
                        {
                            pyxSrs.setIsUTMNorth(srs.UtmNorth.Value);
                        }

                        if (srs.UtmZone.HasValue)
                        {
                            pyxSrs.setZone(srs.UtmZone.Value);
                        }
                        break;
                }
            }
            else
            {
                pyxSrs.setProjection(PYXSpatialReferenceSystem.eProjection.knProjectionNone);                
            }

            return PYXCOMFactory.CreateCoordConvertorFromSRS(pyxSrs.clone());
        }

        /// <summary>
        /// Get a normalized version of the spatial reference system.
        /// </summary>
        /// <returns>The normalized spatial reference system.</returns>
        public static SpatialReferenceSystem Normalize(this SpatialReferenceSystem srs)
        {
            if (srs.Projection.HasValue && srs.Projection.Value == SpatialReferenceSystem.Projections.Custom)
            {
                BuildWellKnownWktCache();
                if (s_wellKnownTextCache.ContainsKey(srs.CustomProjection))
                {
                    return s_wellKnownTextCache[srs.CustomProjection].Clone();
                }
            }

            return srs.Clone();
        }

        private static object s_wellKnownWktCacheCache = new object();
        private static Dictionary<string, SpatialReferenceSystem> s_wellKnownTextCache;

        private static void BuildWellKnownWktCache()
        {
            lock (s_wellKnownWktCacheCache)
            {
                if (s_wellKnownTextCache != null)
                {
                    return;
                }

                s_wellKnownTextCache = new Dictionary<string, SpatialReferenceSystem>();
                foreach (SpatialReferenceSystem.Datums datum in Enum.GetValues(typeof(SpatialReferenceSystem.Datums)))
                {
                    var srs = SpatialReferenceSystem.CreateGeographical(datum);
                    var wkt = PYXCOMFactory.CreateSRSFromCoordConverter(srs.CreateCoordConverter()).getWKT();
                    s_wellKnownTextCache[wkt] = srs;
                }

                for (int zone=1; zone<=60; zone++) 
                {
                    var srs = SpatialReferenceSystem.CreateUtmNorth(SpatialReferenceSystem.Datums.NAD83, zone);
                    var wkt = PYXCOMFactory.CreateSRSFromCoordConverter(srs.CreateCoordConverter()).getWKT();
                    s_wellKnownTextCache[wkt] = srs;
                }

                for (int zone = 1; zone <= 60; zone++)
                {
                    var srsNorth = SpatialReferenceSystem.CreateUtmNorth(SpatialReferenceSystem.Datums.WGS84, zone);
                    var wktNorth = PYXCOMFactory.CreateSRSFromCoordConverter(srsNorth.CreateCoordConverter()).getWKT();
                    s_wellKnownTextCache[wktNorth] = srsNorth;

                    var srsSouth = SpatialReferenceSystem.CreateUtmSouth(SpatialReferenceSystem.Datums.WGS84, zone);
                    var wktSouth = PYXCOMFactory.CreateSRSFromCoordConverter(srsSouth.CreateCoordConverter()).getWKT();
                    s_wellKnownTextCache[wktSouth] = srsSouth;
                }
            }
        }
    }
}
