using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Services;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;

namespace GeoWebCore.CLI
{
    /// <summary>
    /// Display a status of what happen in the last # of days
    /// </summary>
    static class GalleryStatus
    {
        /// <summary>
        /// Simple GeoSource status
        /// </summary>
        private class GeoSourceStatus
        {
            /// <summary>
            /// GeoSource Id
            /// </summary>
            [JsonProperty("id")]
            public Guid Id { get; set; }

            /// <summary>
            /// GeoSource Name
            /// </summary>
            [JsonProperty("name")]
            public string Name { get; set; }

            /// <summary>
            /// Type of the GeoSource pipeline output
            /// </summary>
            [JsonProperty("type")]
            [JsonConverter(typeof(StringEnumConverter))]
            public PipelineSpecification.PipelineOutputType? Type { get; set; }

            /// <summary>
            /// Onwer of the pipeline
            /// </summary>
            [JsonProperty("user")]
            public string User { get; set; }
            
            /// <summary>
            /// The pipeline Visiblity
            /// </summary>
            [JsonProperty("visibility")]
            [JsonConverter(typeof(StringEnumConverter))]
            public VisibilityType? Visiblity { get; set; }
            
            /// <summary>
            /// DataSize of the geoSource
            /// </summary>
            [JsonProperty("dataSize")]
            public long DataSize { get; set; }

            /// <summary>
            /// Does this pipeline has a none empty style
            /// </summary>
            [JsonProperty("hasStyle")]
            public bool HasStyle { get; set; }
            
            /// <summary>
            /// Does this GoeSource is working
            /// </summary>
            [JsonProperty("working")]
            public bool Working { get; set; }
        }

        public static void RunStatusOnTheLastDays(int amountOfDays = 90)
        {
            var lastDays = DateTime.Now.Subtract(TimeSpan.FromDays(amountOfDays));
            foreach (var geosource in GeoSourceInitializer.Engine.GetChannel()
                .GeoSources.Filter("State eq 'Active' and DataSize gt 0 and Metadata/Created gt DateTime'" +
                                   lastDays.ToString("s") + "'"))
            {
                RunStatusOnGeoSource(geosource);
            }
        }


        public static void RunStatusOnGeoSources(List<Guid> geoSources)
        {
            foreach (var id in geoSources)
            {
                var geoSource = GeoSourceInitializer.Engine.GetChannel().GeoSources.GetById(id);

                RunStatusOnGeoSource(geoSource);
            }
        }

        private static void RunStatusOnGeoSource(GeoSource geosource)
        {
            var geoSourceStatus = new GeoSourceStatus()
            {
                Id = geosource.Id,
                Name = geosource.Metadata.Name,
                Type = geosource.Specification.OutputType,
                User = geosource.Metadata.User.Name,
                Visiblity = geosource.Metadata.Visibility,
                DataSize = geosource.DataSize ?? 0,
            };
            Console.WriteLine("GeoSource " + geosource.Id + " : " + geosource.Metadata.Name);
            Console.WriteLine(" --> DataSize: " + geosource.DataSize);
            Console.WriteLine(" --> Type: " + geosource.Specification.OutputType);
            Console.WriteLine(" --> Visibilty: " + geosource.Metadata.Visibility);

            //detect empty style bug
            if (geosource.Style != null && JsonConvert.SerializeObject(geosource.Style) != "{}")
            {
                geoSourceStatus.HasStyle = true;
            }
            else
            {
                Console.WriteLine(" --> No Style");
            }

            try
            {
                if (GeoSourceInitializer.Initialize(geosource) != null)
                {
                    geoSourceStatus.Working = true;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }

            if (geoSourceStatus.Working)
            {
                Console.WriteLine(" --> Initialized Ok");
            }
            else
            {
                Console.WriteLine(" --> Failed to initialized");
            }

            AutomationLog.PushInfo("geoSources", geoSourceStatus);
        }
    }
}