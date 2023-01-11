using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Web;
using AutoMapper;
using Newtonsoft.Json;

namespace PyxCrawler.Models
{
    public static class LonLatHelper
    {
        public static List<double> RoundLonLat(double lon, double lat)
        {
            lon = Math.Max(Math.Min(lon, 180), -180);
            lat = Math.Max(Math.Min(lat, 90), -90);
            return new List<double> { lon, lat };
        }
    }

    public abstract class Geometry
    {
        public string Type { get; set; }
    }

    public class Envelope : Geometry
    {
        public Envelope(Wgs84BoundingBox bbox)
        {
            Type = "envelope";
            // Initialize with upper left and bottom right points
            Coordinates = new List<List<double>> { LonLatHelper.RoundLonLat(bbox.West, bbox.North), LonLatHelper.RoundLonLat(bbox.East, bbox.South) };
        }

        public List<List<double>> Coordinates { get; set; }
    }

    public class Point : Geometry
    {
        public Point(Wgs84BoundingBox bbox)
        {
            Type = "point";
            // Initialize with center of bbox
            Coordinates = LonLatHelper.RoundLonLat((bbox.East + bbox.West) / 2, (bbox.North + bbox.South) / 2);
        }

        public List<double> Coordinates { get; set; }
    }

    public class OnlineGeospatialIndexModel : OnlineGeospatialBase
    {
        public Envelope Boundary { get; set; }

        public List<double> Center { get; set; }
    }

    public static class OnlineGeospatialIndexModelFactory
    {
        private static readonly double s_globalArea = 360*180;
        private static readonly double s_maxAreaFraction = 0.20;
        static OnlineGeospatialIndexModelFactory()
        {
            Mapper.CreateMap<OnlineGeospatialBase, OnlineGeospatialIndexModel>();
        }

        public static OnlineGeospatialIndexModel Create(OnlineGeospatialDataSet dataSet)
        {
            var bbox = dataSet.Wgs84BoundingBox;
            var model = Mapper.Map<OnlineGeospatialIndexModel>(dataSet);
            // Don't index unnecessary fields to save index space. (index can't store null so null fields are removed on index)
            model.Services.ForEach(s =>
            {
                s.Error = null;
                s.Trace = null;
                s.Definition = null;
                s.ProcRef = null;
            });
            if (bbox != null)
            {
                model.Boundary = new Envelope(bbox);
                // don't use center point for large bounding boxes
                if (CanModelAsPoint(bbox))
                {
                    model.Center = LonLatHelper.RoundLonLat((bbox.East + bbox.West) / 2, (bbox.North + bbox.South) / 2);
                }
            }
            return model;
        }

        private static bool CanModelAsPoint(Wgs84BoundingBox bbox)
        {
            var area = Math.Abs(bbox.East - bbox.West)*Math.Abs(bbox.East - bbox.West);
            return area <= s_globalArea*s_maxAreaFraction;
        }
    }
}