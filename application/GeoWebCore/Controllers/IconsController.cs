using System;
using System.Collections.Generic;
using System.Web.Http;
using GeoWebCore.Models;
using GeoWebCore.Utilities;
using Pyxis.Core.IO;
using GeoWebCore.WebConfig;
using GeoWebCore.Services;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Controller that generate icon model for a vector GeoSource
    /// 
    /// In order to support GeoSources with many (100K+) features on the globe this API
    /// uses GeoPackets tree to allow the client receive aggregate view of the data.
    /// </summary>
    [RoutePrefix("api/v1/Local")]
    public class IconsController : ApiController
    {
        /// <summary>
        /// Return Icons model for a given source and field. if groupId is specificed the IconsModel would be for that group
        /// </summary>
        /// <param name="geoSource">GeoSource id to fetch icon data from</param>
        /// <param name="field">field name. can be "*" for all fields or several field names splited by ","</param>
        /// <param name="groupId">optional groupId, if not specific the root group of GeoSource is used</param>
        /// <returns>Icons Model for the given group.</returns>
        [HttpGet]
        [Route("Icons")]
        [ApiCache]
        [AuthorizeGeoSource]
        [TimeTrace("geoSource,field,groupId")]
        public IconsModel Icons(Guid geoSource, string field, string groupId = "")
        {
            var model = new IconsModel
            {
                GeoSource = geoSource,
                GroupId = groupId,
                Groups = new List<IconsModel.GroupIcon>(),
                Icons = new List<IconsModel.FeatureIcon>()
            };

            var process = GeoSourceInitializer.Initialize(geoSource);

            if (process.isNull())
            {
                return model;
            }

            var featureGroup = pyxlib.QueryInterface_IFeatureGroup(process.getOutput());

            if (!string.IsNullOrEmpty(groupId) && featureGroup.isNotNull())
            {
                featureGroup = featureGroup.getFeatureGroup(groupId);
            }

            if (featureGroup.isNull())
            {
                return model;
            }

            var definition = featureGroup.getFeatureDefinition();

            var fieldsExtractor = (field == "*") ?
                //select all fields
                FeatureFieldsExtractor.Create(definition) :
                //select specific fields
                FeatureFieldsExtractor.Create(definition, field.Split(','));

            foreach (var feature in featureGroup.GetGroupEnumerator())
            {
                var group = pyxlib.QueryInterface_IFeatureGroup(feature);

                var circle = feature.getGeometry().getBoundingCircle();
                var center = circle.getCenter();

                if (group.isNotNull())
                {
                    var iconModel = new IconsModel.GroupIcon
                    {
                        Id = group.getID(),
                        X = center.x(),
                        Y = center.y(),
                        Z = center.z(),
                        Radius = circle.getRadius(),
                        Count = group.getFeaturesCount().max,
                        Values = fieldsExtractor.ExtractFields(group)
                    };

                    model.Groups.Add(iconModel);
                }
                else
                {
                    var iconModel = new IconsModel.FeatureIcon()
                    {
                        Id = feature.getID(),
                        X = center.x(),
                        Y = center.y(),
                        Z = center.z(),
                        Radius = circle.getRadius(),
                        Values = fieldsExtractor.ExtractFields(feature)
                    };

                    model.Icons.Add(iconModel);
                }
            }

            return model;
        }
    }
}