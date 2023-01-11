using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using PyxCrawler.Models;
using Pyxis.Contract.DataDiscovery;

namespace PyxCrawler.Utilities
{
    public static class DataSetHelper
    {
        public static Wgs84BoundingBox GetWgs84BoundingBox(DataSet dataSet)
        {
            if (dataSet.BBox != null && dataSet.BBox.Count >= 1)
            {
                return null;
            }

            var bBox = new Wgs84BoundingBox
            {
                North = dataSet.BBox[0].UpperRight.Y,
                East = dataSet.BBox[0].UpperRight.X,
                South = dataSet.BBox[0].LowerLeft.Y,
                West = dataSet.BBox[0].LowerLeft.X,
            };
            if (dataSet.BBox[0].Srs != null)
            {
                bBox = CorrectProjection(bBox, dataSet.BBox[0].Srs);
            }
            return bBox;
        }

        private static Wgs84BoundingBox CorrectProjection(Wgs84BoundingBox bBox, string srs)
        {
            if (srs == null)
            {
                return bBox;
            }
            
            double[] lowerLeft;
            double[] upperRight;
            int wkid;
            if (int.TryParse(srs, out wkid))
            {
                switch (wkid)
                {
                    case 4326: // WGS84
                        break;

                    case 102100: // Web Mercator
                        lowerLeft = ProjectionHelper.MercatorToWgs84(bBox.West, bBox.South);
                        upperRight = ProjectionHelper.MercatorToWgs84(bBox.East, bBox.North);
                        if (!PointsToBbox(lowerLeft, upperRight, bBox))
                        {
                            return null;
                        }
                        break;

                    default:
                        lowerLeft = ProjectionHelper.WkidToWgs84(wkid, bBox.West, bBox.South);
                        upperRight = ProjectionHelper.WkidToWgs84(wkid, bBox.East, bBox.North);
                        if (!PointsToBbox(lowerLeft, upperRight, bBox))
                        {
                            return null;
                        }
                        break;
                }
                return bBox;
            }

            lowerLeft = ProjectionHelper.WktToWgs84(srs, bBox.West, bBox.South);
            upperRight = ProjectionHelper.WktToWgs84(srs, bBox.East, bBox.North);
            if (!PointsToBbox(lowerLeft, upperRight, bBox))
            {
                return null;
            }

            return bBox;
        }

        public static void UpsertDataSet(OnlineGeospatialEndpoint endpoint, OnlineGeospatialService service, DataSet dataSet, Wgs84BoundingBox bbox)
        {
            var onlineGeospatialDataset = OnlineGeospatialDatasetDb.GetOrCreateNoModify(endpoint.Uri,
                new OnlineGeospatialService(service)
                {
                    Status = OnlineGeospatialServiceStatus.NeedsVerifying
                },
                dataSet.Layer);

            var dataSetService = onlineGeospatialDataset.Services.FirstOrDefault(s => s.Protocol == service.Protocol && s.Version == service.Version);
            if (dataSetService == null)
            {
                dataSetService = onlineGeospatialDataset.GetService(service.Protocol, service.Version) as AnnotatedOnlineGeospatialService;
                dataSetService.Status = OnlineGeospatialServiceStatus.NeedsVerifying;
            }
            dataSetService.Uri = new Uri(dataSet.Uri);
            onlineGeospatialDataset.Name = dataSet.Metadata.Name;
            onlineGeospatialDataset.Description = dataSet.Metadata.Description;
            onlineGeospatialDataset.Wgs84BoundingBox = bbox;
            onlineGeospatialDataset.Fields = dataSet.Fields.Any() ? dataSet.Fields : null;
            onlineGeospatialDataset.Domains = dataSet.Domains;
            onlineGeospatialDataset.Tags = dataSet.Metadata.Tags;
            if (onlineGeospatialDataset.Tags != null && onlineGeospatialDataset.Tags.Count > 5)
            {
                onlineGeospatialDataset.Tags = new List<string>(onlineGeospatialDataset.Tags).Take(5).ToList();
            }
        }

        private static bool PointsToBbox(double[] lowerLeft, double[] upperRight, Wgs84BoundingBox bBox)
        {
            if (lowerLeft == null || upperRight == null
                || lowerLeft[0] < -180 || lowerLeft[0] > 180
                || upperRight[0] < -180 || upperRight[0] > 180
                || lowerLeft[1] < -90 || lowerLeft[1] > 90
                || upperRight[1] < -90 || upperRight[1] > 90)
            {
                return false;
            }
            bBox.North = upperRight[1];
            bBox.East = upperRight[0];
            bBox.South = lowerLeft[1];
            bBox.West = lowerLeft[0];
            return true;
        }
    }
}