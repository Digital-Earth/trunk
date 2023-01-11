using System;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace GeoWebCore.Services.Cluster
{
    public class GeoSourceClusterResolver : IClusterResolver
    {
        private Cluster m_cluster;
        private Guid m_geoSourceId;
        private string m_host;
        private GeoSource m_geoSource;
        private IProcess_SPtr m_localProcess;
        private Task m_resolveTask;

        public GeoSourceClusterResolver(Cluster cluster, Guid geoSourceId)
        {
            m_cluster = cluster;
            m_geoSourceId = geoSourceId;
            Key = m_geoSourceId.ToString();
            m_host = cluster.GetEndpointForGeoSource(Key);

            m_resolveTask = Resolve().ContinueWith(task =>
            {
                if (task.IsFaulted)
                {
                    IsFaulted = true;
                    throw new Exception("Failed to resolved GeoSource", task.Exception.InnerException);
                }
            });
        }

        public string Key { get; private set; }

        public bool IsLocal
        {
            get { return m_cluster.IsLocal(m_cluster.ServersRing, m_host); }
        }

        public bool IsReady { get; private set; }
        public bool IsFaulted { get; private set; }

        private async Task Resolve()
        {
            if (IsLocal)
            {
                m_geoSource = GeoSourceInitializer.GetGeoSource(m_geoSourceId);
                m_localProcess = GeoSourceInitializer.Initialize(m_geoSource);
                
                IsReady = true;
            }
            else
            {
                var uri = new UriBuilder(m_host).AddPath("api/v1/GeoSource", Key).ToString();
                var result = await ClusterUtils.SendAsync(uri, "GET", TimeSpan.FromSeconds(30));
                m_geoSource = JsonConvert.DeserializeObject<GeoSource>(result.Content);

                IsReady = true;
            }
        }

        public PYXValueTile_SPtr GetValueTile(PYXTile tile)
        {
            WaitUntilReady();

            if (IsLocal)
            {
                var coverage = pyxlib.QueryInterface_ICoverage(m_localProcess.getOutput());

                if (coverage.isNull())
                {
                    throw new Exception("GeoSource is not a coverage");
                }

                return coverage.getCoverageTile(tile);
            }
            else
            {
                var uri = new UriBuilder(m_host).AddPath("api/v1/GeoSource", Key, "Tile").AddQuery("root",tile.getRootIndex().toString()).AddQuery("depth",tile.getDepth()).ToString();
                var result = ClusterUtils.SendAsync(uri, "GET", TimeSpan.FromMinutes(2)).Result;
                var valueTile = PYXValueTile.createFromBase64String(Convert.ToBase64String(result.Body));
                return valueTile;
            }
        }

        public PYXTileCollection_SPtr GetWhereTile(PYXTile tile)
        {
            if (IsLocal)
            {
                var coverage = pyxlib.QueryInterface_ICoverage(m_localProcess.getOutput());
                if (coverage.isNotNull())
                {
                    var condition = PYXWhereCondition.coverageHasValues(coverage);
                    return condition.match(tile);
                }
                var features = pyxlib.QueryInterface_IFeatureCollection(m_localProcess.getOutput());
                if (features.isNotNull())
                {
                    var condition = PYXWhereCondition.featuresHasValues(features);
                    return condition.match(tile);
                }
                throw new Exception("Unsupported process output");
            }
            else
            {
                var uri = new UriBuilder(m_host).AddPath("api/v1/GeoSource", Key, "Tile/Where")
                    .AddQuery("root", tile.getRootIndex().toString())
                    .AddQuery("depth", tile.getDepth())
                    .AddQuery("format","raw")
                    .ToString();

                var result = ClusterUtils.SendAsync(uri, "GET", TimeSpan.FromMinutes(2)).Result;
                return pyxlib.DynamicPointerCast_PYXTileCollection(PYXGeometrySerializer.deserializeFromBase64(Convert.ToBase64String(result.Body)));
            }
        }

        public PYXTileCollection_SPtr GetWhereTile(PYXTile tile, string field, string min, string max)
        {
            if (IsLocal)
            {
                var coverage = pyxlib.QueryInterface_ICoverage(m_localProcess.getOutput());
                if (coverage.isNotNull())
                {
                    var condition = PYXWhereCondition.coverageHasValues(coverage);
                    if (!String.IsNullOrEmpty(min) && !String.IsNullOrEmpty(max))
                    {
                        var minNumeric = Double.Parse(min);
                        var maxNumeric = Double.Parse(max);

                        condition = condition.range(new PYXValue(minNumeric), new PYXValue(maxNumeric));
                    }

                    return condition.match(tile);
                }
                var features = pyxlib.QueryInterface_IFeatureCollection(m_localProcess.getOutput());
                if (features.isNotNull())
                {
                    var condition = PYXWhereCondition.featuresHasValues(features);
                    if (!String.IsNullOrEmpty(field))
                    {
                        var index = features.getFeatureDefinition().getFieldIndex(field);
                        if (index == -1)
                        {
                            throw new Exception("Can't find field with name: " + field);
                        }
                        condition = condition.field(field);

                        var numeric = features.getFeatureDefinition().getFieldDefinition(index).isNumeric();

                        if (!String.IsNullOrEmpty(min) && !String.IsNullOrEmpty(max))
                        {
                            if (numeric)
                            {
                                //use numeric values
                                var minNumeric = Double.Parse(min);
                                var maxNumeric = Double.Parse(max);
                                condition = condition.range(new PYXValue(minNumeric), new PYXValue(maxNumeric));
                            }
                            else
                            {
                                //use string values
                                condition = condition.range(new PYXValue(min), new PYXValue(max));
                            }
                        }
                    }
                    return condition.match(tile);
                }
                throw new Exception("Unsupported process output");
            }
            else
            {
                var uri = new UriBuilder(m_host).AddPath("api/v1/GeoSource", Key, "Tile/Where")
                    .AddQuery("root", tile.getRootIndex().toString())
                    .AddQuery("depth", tile.getDepth())
                    .AddQuery("field", field)
                    .AddQuery("min", min)
                    .AddQuery("max", max)
                    .AddQuery("format", "raw")
                    .ToString();

                var result = ClusterUtils.SendAsync(uri, "GET", TimeSpan.FromMinutes(2)).Result;
                return pyxlib.DynamicPointerCast_PYXTileCollection(PYXGeometrySerializer.deserializeFromBase64(Convert.ToBase64String(result.Body)));
            }
        }

        private void WaitUntilReady()
        {
            m_resolveTask.Wait();
        }
    }
}