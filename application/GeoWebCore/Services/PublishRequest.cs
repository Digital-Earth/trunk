using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace GeoWebCore.Services
{
    /// <summary>
    /// PublishRequest can be attached to ImportDataSetRequest to publish the resulting GeoSource into the user gallery
    /// </summary>
    public class PublishRequest
    {
        /// <summary>
        /// User AccessToken to be used.
        /// </summary>
        public string Token { get; set; }

        /// <summary>
        /// User Id that request the import
        /// </summary>
        public Guid UserId { get; set; }

        /// <summary>
        /// GalleryId to be publish the resource into
        /// </summary>
        public Guid GalleryId { get; set; }

        /// <summary>
        /// Visibility settings (usually it will be set to private)
        /// </summary>
        public VisibilityType Visibility { get; set; }

        /// <summary>
        /// Define if this Publish request should be performed once the process been imported
        /// </summary>
        public bool Enabled { get; set; }

        /// <summary>
        /// Setup a GeoSource details based on this publish request 
        /// </summary>
        /// <param name="geoSource"></param>
        public void SetupGeoSource(GeoSource geoSource)
        {
            geoSource.Metadata.Visibility = Visibility;
            geoSource.Metadata.Providers = new List<Provider> {
                new Provider() { Type = ResourceType.Gallery, Id = GalleryId, Name = "" }
            };
            geoSource.Metadata.User = null;
        }
    }
}