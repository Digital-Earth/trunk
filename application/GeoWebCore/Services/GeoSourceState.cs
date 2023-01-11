using System;
using System.Threading.Tasks;
using GeoWebCore.Models;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Core.Measurements;
using Pyxis.UI.Layers.Globe;
using Pyxis.Utilities;

namespace GeoWebCore.Services
{
    internal class GeoSourceState : IGeoSourceState
    {
        public Guid Id { get; private set; }

        public GeoSourceState(Guid id)
        {
            Id = id;
            m_getGeoSource = CachedResult<GeoSource>.CreateNotNullAsync(DoGetGeoSource);
            m_getProcess = new CachedResult<IProcess_SPtr>(DoGetProcess, (process) => process != null && process.isNotNull()).DependsOn(m_getGeoSource);
            m_getStyle = CachedResult<Style>.CreateNotNullAsync(DoGetStyle).DependsOn(m_getProcess);
            m_getCharacterization = CachedResult<GeoSourceDataCharacterization>.CreateNotNullAsync(DoGetCharacterization).DependsOn(m_getProcess);
            m_getSpecification = CachedResult<PipelineSpecification>.CreateNotNullAsync(DoGetSpecification).DependsOn(m_getProcess);
        }

        private readonly object m_lock = new object();
        private readonly CachedResult<GeoSource> m_getGeoSource;
        private readonly CachedResult<IProcess_SPtr> m_getProcess;
        private readonly CachedResult<Style> m_getStyle;
        private readonly CachedResult<GeoSourceDataCharacterization> m_getCharacterization;
        private readonly CachedResult<PipelineSpecification> m_getSpecification;

        private readonly LimitedSizeDictionary<string, CachedResult<Style>> m_styles = new LimitedSizeDictionary<string, CachedResult<Style>>(20);

        private async Task<GeoSource> DoGetGeoSource()
        {
            return await Task.Factory.StartNew(()=>GeoSourceInitializer.GetGeoSource(Id));
        }
        
        private async Task<IProcess_SPtr> DoGetProcess()
        {
            var geoSource = await GetGeoSource();
            return GeoSourceInitializer.Initialize(geoSource);
        }

        private async Task<Style> DoGetStyle()
        {
            return Cache.GeoSourceBlobCacheSingleton.GetOrGenerateMetadata(await GetGeoSource(), (geoSource) =>
            {
                var styledGeoSource = StyledGeoSource.Create(GeoSourceInitializer.Engine, geoSource);

                if (styledGeoSource.Style == null ||
                    (styledGeoSource.Style.Fill == null && styledGeoSource.Style.Icon == null &&
                     styledGeoSource.Style.Line == null))
                {
                    return styledGeoSource.CreateDefaultStyle(RandomStyleGenerator.Create(geoSource.Id));
                }
                else
                {
                    return styledGeoSource.Style;
                }
            });
        }

        private async Task<Style> DoGetStyle(Style basedOn)
        {
            var geoSource = await GetGeoSource();
            var styledGeoSource = StyledGeoSource.Create(GeoSourceInitializer.Engine, geoSource);

            if (styledGeoSource.Style == null || (styledGeoSource.Style.Fill == null && styledGeoSource.Style.Icon == null && styledGeoSource.Style.Line == null))
            {
                return styledGeoSource.CreateDefaultStyle(basedOn);
            }
            else
            {
                return styledGeoSource.Style;
            }
        }

        private async Task<Style> DoGetStyle(AutoStyleRequest styleRequest)
        {
            var geoSource = await GetGeoSource();
            var styledGeoSource = StyledGeoSource.Create(GeoSourceInitializer.Engine, geoSource, styleRequest.Style);

            if (styleRequest.Geometry != null)
            {
                return styledGeoSource.CreateStyleByField(
                    styleRequest.Field, 
                    styleRequest.Palette,
                    styleRequest.Geometry);
            }
            else
            {
                return styledGeoSource.CreateStyleByField(
                    styleRequest.Field,
                    styleRequest.Palette);
            }
        }

        private async Task<GeoSourceDataCharacterization> DoGetCharacterization()
        {
            var geoSource = await GetGeoSource();

            var result = Cache.GeoSourceBlobCacheSingleton.GetMetadata<GeoSourceDataCharacterization>(geoSource);

            if (result != null)
            {
                return result;
            }

            var process = await GetProcess();

            var feature = pyxlib.QueryInterface_IFeature(process.getOutput());

            if (feature == null)
            {
                throw new Exception("failed to resolve the process as feature with a geometry");
            }

            var boundingCircle = feature.getGeometry().getBoundingCircle();

            result = new GeoSourceDataCharacterization
            {
                NativeResolution = feature.getGeometry().getCellResolution(),
                BoundingCircle = new CircleGeometry
                {
                    Coordinates = new GeographicPosition(PointLocation.fromXYZ(boundingCircle.getCenter())),
                    Radius = boundingCircle.getRadius() * SphericalDistance.Radian
                }
            };

            Cache.GeoSourceBlobCacheSingleton.WriteMetadata<GeoSourceDataCharacterization>(geoSource, result);

            return result;
        }

        private async Task<PipelineSpecification> DoGetSpecification()
        {
            var geoSource = await GetGeoSource();

            return Cache.GeoSourceBlobCacheSingleton.GetOrGenerateMetadata(geoSource,GeoSourceInitializer.Engine.GetSpecification);
        }

        public Task<GeoSource> GetGeoSource()
        {
            return m_getGeoSource.GetTask();
        }

        public Task<IProcess_SPtr> GetProcess()
        {
            return m_getProcess.GetTask();
        }

        public Task<Style> GetStyle()
        {
            return m_getStyle.GetTask();
        }

        public Task<Style> GetStyle(Style basedOn)
        {
            if (basedOn == null)
            {
                return m_getStyle.GetTask();
            }
            var key = UniqueHashGenerator.FromObject(basedOn);
            lock (m_styles)
            {
                if (m_styles.ContainsKey(key))
                {
                    return m_styles[key].GetTask();
                }
                m_styles[key] = CachedResult<Style>.CreateNotNullAsync(() => DoGetStyle(basedOn));
                return m_styles[key].GetTask();
            }
        }

        public Task<Style> GetStyle(AutoStyleRequest styleRequest)
        {
            //no style request specificed
            if (styleRequest == null)
            {
                return m_getStyle.GetTask();
            }

            //no style field specificed
            if (string.IsNullOrEmpty(styleRequest.Field))
            {
                return GetStyle(styleRequest.Style);
            }

            //create new style
            var key = UniqueHashGenerator.FromObject(styleRequest);
            lock (m_styles)
            {
                if (m_styles.ContainsKey(key))
                {
                    return m_styles[key].GetTask();
                }
                m_styles[key] = CachedResult<Style>.CreateNotNullAsync(() => DoGetStyle(styleRequest));
                return m_styles[key].GetTask();
            }
        }

        public Task<GeoSourceDataCharacterization> GetCharacterization()
        {
            return m_getCharacterization.GetTask();
        }

        public Task<PipelineSpecification> GetSpecification()
        {
            return m_getSpecification.GetTask();
        }
    }
}