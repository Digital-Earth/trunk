using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;

namespace Pyxis.Contract.Publishing
{
    public class Camera
    {
        public Camera()
        {
        }

        public Camera(Camera camera)
        {
            Latitude = camera.Latitude;
            Longitude = camera.Longitude;
            Heading = camera.Heading;
            Altitude = camera.Altitude;
            Tilt = camera.Tilt;
            Range = camera.Range;
        }
        
        /// <summary>
        /// Gets or sets the latitude.
        /// </summary>
        public double Latitude { get; set; }

        /// <summary>
        /// Gets or sets the longitude.
        /// </summary>        
        public double Longitude { get; set; }

        /// <summary>
        /// Gets or sets the heading.
        /// </summary>
        public double Heading { get; set; }

        /// <summary>
        /// Gets or sets the altitude.
        /// </summary>
        public double Altitude { get; set; }

        /// <summary>
        /// Gets or sets the tilt.
        /// </summary>
        public double Tilt { get; set; }
        
        /// <summary>
        /// Gets or sets the range.
        /// </summary>        
        public double Range { get; set; }

        /// <summary>
        /// Determines if the position of the Pyxis.Contract.Publishing.Camera is the same as the camera position of the specified System.Object.
        /// </summary>
        /// <param name="obj">The System.Object whose underlying camera position is to be compared with the position of the current Pyxis.Contract.Publishing.Camera.</param>
        /// <returns>true if the underlying camera position of <paramref name="obj"/> is the same as the position of the current Pyxis.Contract.Publishing.Camera; otherwise, false.</returns>
        public override bool Equals(object obj)
        {
            if (obj is Camera)
            {
                var other = obj as Camera;

                return
                    Latitude == other.Latitude &&
                    Longitude == other.Longitude &&
                    Altitude == other.Altitude &&
                    Tilt == other.Tilt &&
                    Heading == other.Heading &&
                    Range == other.Range;
            }
            return base.Equals(obj);
        }

        /// <summary>
        /// Returns the hash code for this instance.
        /// </summary>
        /// <returns>A 32-bit signed integer hash code.</returns>
        public override int GetHashCode()
        {
            return
                Latitude.GetHashCode() ^
                Longitude.GetHashCode() ^
                Altitude.GetHashCode() ^
                Tilt.GetHashCode() ^
                Heading.GetHashCode() ^
                Range.GetHashCode();
        }

        /// <summary>
        /// Indicates whether the positions of two specified Pyxis.Contract.Publishing.Camera objects are equal.
        /// </summary>
        /// <param name="a">The first object to compare.</param>
        /// <param name="b">The second object to compare.</param>
        /// <returns>true if a and b are equal; otherwise, false.</returns>
        public static bool operator ==(Camera a, Camera b)
        {
            // If both are null, or both are same instance, return true.
            if (ReferenceEquals(a, b))
            {
                return true;
            }

            // If one is null, but not both, return false.
            if (((object)a == null) || ((object)b == null))
            {
                return false;
            }

            // Return true if the fields match:
            return a.Equals(b);
        }

        /// <summary>
        /// Indicates whether the positions of two specified Pyxis.Contract.Publishing.Camera objects are not equal.
        /// </summary>
        /// <param name="a">The first object to compare.</param>
        /// <param name="b">The second object to compare.</param>
        /// <returns>true if a and b are not equal; otherwise, false.</returns>
        public static bool operator !=(Camera a, Camera b)
        {
            return !(a == b);
        }
    }

    public class Dashboard
    {
        public class StyledSelection
        {
            public JObject Geometry { get; set; }
            public Style Style { get; set; }
        }

        public class Widget
        {
            public SimpleMetadata Metadata { get; set; }
            
            public string Type { get; set; }
            
            /// <summary>
            /// Define the inputs that are needed to calculate the widget results
            /// </summary>
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public JObject Inputs { get; set; }

            /// <summary>
            /// General settings related to the widget, like color and size
            /// </summary>
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public JObject Settings { get; set; }
        }

        public SimpleMetadata Metadata { get; set; }
        public StyledSelection Selection { get; set; }
        public List<Widget> Widgets { get; set; }
    }

    public class Map : Pipeline
    {
        public class Group
        {
            public SimpleMetadata Metadata { get; set; }
            public List<Item> Items { get; set; }
        }

        public class Item
        {
            public Pyxis.Contract.Publishing.ResourceReference Resource { get; set; }
            public Pyxis.Contract.Publishing.Metadata Metadata { get; set; }
            public Pyxis.Contract.Publishing.PipelineSpecification Specification { get; set; }

            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public List<Pyxis.Contract.Publishing.Domain> Domains { get; set; }

            public Style Style { get; set; }
            public bool Active { get; set; }
            public bool ShowDetails { get; set; }
        }

        [JsonConverter(typeof(StringEnumConverter))]
        public PipelineDefinitionState? State { get; set; }
        public Camera Camera { get; set; }
        public DateTime? Time { get; set; }
        public List<Guid> Related { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<Group> Groups { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<Dashboard> Dashboards { get; set; }

        public ResourceReference Theme { get; set; }

        // for deserializing from string
        public Map()
        {
        }

        public Map(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn, 
            PipelineDefinitionState state, Camera camera, DateTime? time, List<Guid> related, ResourceReference theme)
            : base(licenses, metadata, version, procRef, definition, basedOn)
        {
            State = state;
            Type = ResourceType.Map;
            Camera = new Camera(camera);
            Time = time;
            Related = new List<Guid>(related);
            Theme = theme;
        }

        public Map(Map basedOnMap)
            : base(basedOnMap)
        {
            State = basedOnMap.State;
            Type = ResourceType.Map;
            Camera = new Camera(basedOnMap.Camera);
            Time = basedOnMap.Time;
            Related = new List<Guid>(basedOnMap.Related);
            Groups = JsonConvert.DeserializeObject<List<Group>>(JsonConvert.SerializeObject(basedOnMap.Groups));
            Dashboards = JsonConvert.DeserializeObject<List<Dashboard>>(JsonConvert.SerializeObject(basedOnMap.Dashboards));
            Theme = basedOnMap.Theme;
        }

        // For unit testing
        public Map(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState state, Camera camera, DateTime time, List<Guid> related, ResourceReference theme)
            : this(licenses, metadata, version, procRef, definition, basedOn, state, camera, time, related, theme)
        {
            Id = id;
        }
    }
}
