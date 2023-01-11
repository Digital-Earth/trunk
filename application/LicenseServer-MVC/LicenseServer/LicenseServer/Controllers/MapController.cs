using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.Web;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using Map = LicenseServer.Models.Mongo.Map;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Map")]
    public class MapController : BaseResourceController<Map>
    {
        private static readonly UserInfo s_anonymousUserInfo = new UserInfo(Properties.Settings.Default.AnonymousUserId, Properties.Settings.Default.AnonymousUserName);

        private static readonly Provider s_anonymousProvider = new Provider
        {
            Id = Properties.Settings.Default.AnonymousGalleryId,
            Name = Properties.Settings.Default.AnonymousGalleryName,
            Type = ResourceType.Gallery
        };

        private static readonly string s_imageUrl = @"http://www.pyxisinnovation.com/images/pipelines/";
        private static readonly string s_imageUploadUrl = @"http://www.pyxisinnovation.com/data/catalogue/saveImage.php";

        public MapController() 
        { }

        // Inject for test
        public MapController(TestMongoSetup setup) :
            base(setup)
        { }

        // Get api/v1/Map/5/GeoSources
        [HttpGet]
        [Route("{id}/GeoSources")]
        public PageResult<dynamic> GetGeoSources(Guid id, ODataQueryOptions<GeoSource> options)
        {
            var mapResponse = base.Get(id);
            Map map;
            if (mapResponse.TryGetContentValue(out map))
            {
                return db.GetResourcesByIds<GeoSource>(map.BasedOn.Select(r => r.Id).ToList()).ToPageResult(Request, options);
            }
            throw new HttpException((int)HttpStatusCode.NotFound, "No Map with the specified Id.");
        }

        // POST api/v1/Map
        [AllowAnonymous]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(Map map)
        {
            if (map.Camera == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Map Camera must be specified");
            }
            if (map.Metadata == null || map.Metadata.Name == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Map Metadata.Name must be specified");
            }

            if (map.Id == Guid.Empty)
            {
                map.Id = Guid.NewGuid();
            }
            if (map.State == null)
            {
                map.State = PipelineDefinitionState.Active;
            }
            if (map.Metadata.ExternalUrls != null)
            {
                if (map.Metadata.ExternalUrls.Count(e => e.Type == ExternalUrlType.Image) > 1)
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "Only one image may be provided.");
                }
                if (HasDataUri(map.Metadata.ExternalUrls))
                {
                    var image = map.Metadata.ExternalUrls.FirstOrDefault(e => e.Type == ExternalUrlType.Image);
                    var extension = Regex.Match(image.Url, @"data:image/(?<type>.+?);").Groups["type"].Value;
                    var imageName = map.Id + "." + extension;
                    HttpTool.UploadDataUrl(s_imageUploadUrl, image.Url, imageName, new NameValueCollection());
                    image.Url = s_imageUrl + imageName;
                }
            }
            // anonymous Maps are promoted to a shared User and made NonDiscoverable
            if (CurrentUserIdentity == null)
            {
                map.Metadata.User = s_anonymousUserInfo;
                map.Metadata.Visibility = VisibilityType.NonDiscoverable;
                map.Metadata.Providers = new List<Provider> { s_anonymousProvider };
            }

            return base.Post(map);
        }

        private static bool HasDataUri(List<ExternalUrl> externalUrls)
        {
            var image = externalUrls.FirstOrDefault(e => e.Type == ExternalUrlType.Image);
            return image != null && image.Url.StartsWith("data:image/");
        }

        // PUT api/v1/Map/5?Version={version}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Put(Guid id, Guid version, Map map)
        {
            List<Guid> providerGalleryIds = null;
            if (map.State.HasValue && map.State.Value == PipelineDefinitionState.Removed)
            {
                var existingMap = db.GetResourceById<Map>(id);
                if (existingMap != null)
                {
                    providerGalleryIds = RemoveGalleryProviders(existingMap, map);
                }
                else
                {
                    return Request.CreateResponse(HttpStatusCode.BadRequest, "No Map found with the given Id.");
                }
            }

            var response = base.Put(id, version, map);

            if (response.StatusCode == HttpStatusCode.OK && providerGalleryIds != null && providerGalleryIds.Any())
            {
                response = RemoveResourceFromProviderGalleries(providerGalleryIds, id);
            }
            return response;
        }
    }
}
