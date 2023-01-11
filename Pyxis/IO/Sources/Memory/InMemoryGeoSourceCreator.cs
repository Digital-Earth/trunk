using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO.GeoJson;
using Pyxis.IO.Sources.Reflection;

namespace Pyxis.IO.Sources.Memory
{
    internal static class InMemoryGeoSourceCreator
    {
        internal static GeoSource Create(Engine engine, FeatureCollection featureCollection)
        {
            var featureCollectionProcess = PYXCOMFactory.CreateSimpleFeatureCollection(
                featureCollection.Features.Select(x => CreateSimpleFeature(engine,x))
                );

            if (featureCollectionProcess.initProc(true) == IProcess.eInitStatus.knInitialized)
            {
                return CreateGeoSource(featureCollectionProcess, engine.GetUserInfo());
            }
            return null;
        }

        internal static GeoSource Create<T>(Engine engine, IEnumerable<T> items)
        {
            var featureCollection = new FeatureCollection()
            {
                Features = items.Select(Converter.Convert).ToList()
            };
            return Create(engine, featureCollection);
        }
        
        private static GeoSource CreateGeoSource(IProcess_SPtr featureCollection, UserInfo userInfo)
        {
            var id = Guid.NewGuid();
            return new GeoSource(
                id: id,
                licenses: new List<LicenseReference>(),
                metadata: new Metadata(
                    name: "GeoSource " + id,
                    description: featureCollection.getParameter(0).getValueCount() + " features",
                    user: userInfo,
                    providers: new List<Provider>(),
                    category: "",
                    tags: new List<string>(),
                    systemTags: new List<string>(),
                    created: DateTime.Now,
                    updated: DateTime.Now,
                    basedOnVersion: null,
                    externalUrls: new List<ExternalUrl>(),
                    visibility: VisibilityType.Public,
                    comments: new LinkedList<AggregateComment>(),
                    ratings: new AggregateRatings()),
                version:Guid.NewGuid(),
                procRef: pyxlib.procRefToStr(new ProcRef(featureCollection)),
                definition: PipeManager.writePipelineToNewString(featureCollection),
                basedOn: new List<ResourceReference>(),
                state: null,
                dataSize: 0,
                styles: new List<ResourceReference>(),
                usedBy: new List<Guid>(),
                related: new List<Guid>()); 
        }

        private static IProcess_SPtr CreateSimpleFeature(Engine engine, Feature feature)
        {
            return PYXCOMFactory.CreateSimpleFeature(
                feature.Id, 
                engine.ToPyxGeometry(feature.Geometry),
                ConvertProperties(feature.Properties));
        }

        private static Dictionary<string,PYXValue> ConvertProperties(Dictionary<string,object> properties)
        {
            var result = new Dictionary<string,PYXValue>();

            foreach (var keyValue in properties)
            {
                if (keyValue.Value == null)
                {
                    throw new ArgumentNullException(keyValue.Key, "Failed to resolve type of property '" + keyValue.Key + "' because the value was null");
                }

                double value;
                if (keyValue.Value is bool) {
                    result[keyValue.Key] = new PYXValue((bool)keyValue.Value);
                }
                else if (double.TryParse(keyValue.Value.ToString(),out value)) 
                {
                    result[keyValue.Key] = new PYXValue(value);
                }
                else
                {
                    result[keyValue.Key] = new PYXValue(keyValue.Value.ToString());
                }
            }

            return result;
        }
    }
}
