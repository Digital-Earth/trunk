using Pyxis.Contract.Publishing;
using Pyxis.Publishing;
using Pyxis.Publishing.Permits;
using Pyxis.IO.Publish;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Services
{
    static class PublishGeoSourceHelper
    {
        public static GeoSource PublishGeoSourceForUser(string userToken, GeoSource geoSource, Style style)
        {
            var channel = new Channel(ApiUrl.ProductionLicenseServerRestAPI);
            var tokenDetails = new AccessToken.TokenDetails
            {
                Token = userToken
            };
            var originalUser = channel.Authenticate(tokenDetails).AsUser();

            var styledGeoSource = Pyxis.UI.Layers.Globe.StyledGeoSource.Create(GeoSourceInitializer.Engine, geoSource);

            if (style != null)
            {
                styledGeoSource.ApplyStyle(style);
            }

            var publishProgress = GeoSourceInitializer.Engine.BeginPublish(geoSource, styledGeoSource.Style, new Pyxis.IO.Publish.PublishSettingProvider(), originalUser);

            return publishProgress.Task.Result.PublishedGeoSource;
        }

        public static GeoSource UpdateGeoSourceMetadataForUser(string userToken, GeoSource geoSource)
        {
            var channel = new Channel(ApiUrl.ProductionLicenseServerRestAPI);
            var tokenDetails = new AccessToken.TokenDetails
            {
                Token = userToken
            };
            var originalUser = channel.Authenticate(tokenDetails).AsUser();

            var updates = new GeoSource()
            {
                Type = geoSource.Type,
                Id = geoSource.Id,
                Version = geoSource.Version,
                Metadata = new Metadata()
                {
                    Name = geoSource.Metadata.Name,
                    Description = geoSource.Metadata.Description,
                    ExternalUrls = geoSource.Metadata.ExternalUrls,
                    Tags = geoSource.Metadata.Tags,
                    Visibility = geoSource.Metadata.Visibility
                }
            };

            originalUser.PutResource(updates);

            return Channel.Authenticate(originalUser).GeoSources.GetById(geoSource.Id);
        }
    }
}
