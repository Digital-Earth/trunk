using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// Responsible for creating GeoSources that are mosaics of existing GeoSources, where
    /// a mosaic is a composite of GeoSources with the same PipelineSpecification.
    /// </summary>
    public class GeoSourceMosaic
    {
        private Engine Engine { get; set; }

        /// <summary>
        /// Initializes a new instance of the Pyxis.Core.Analysis.GeoSourceMosaic class.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        public GeoSourceMosaic(Engine engine)
        {
            Engine = engine;
        }

        /// <summary>
        /// Create a GeoSource that is a mosaic of <paramref name="geoSources"/>.
        /// </summary>
        /// <param name="geoSources">The GeoSources to create a mosaic of.</param>
        /// <returns>The created GeoSource that is a mosaic of <paramref name="geoSources"/>.</returns>
        public GeoSource Mosaic(IEnumerable<GeoSource> geoSources)
        {
            var inputs = geoSources.ToList();

            switch (inputs.Count)
            {
                case 0:
                    throw new InvalidOperationException("Can't mosaic no GeoSource");
                case 1:
                    return inputs[0];
            }

            var specs = inputs.Select(x => Engine.GetSpecification(x)).ToList();
            var firstSpec = specs[0];

            if (!firstSpec.IsSameAs(specs))
            {
                throw new InvalidOperationException("All GeoSource must have the same specification in order to mosaic.");
            }

            if (firstSpec.OutputType == PipelineSpecification.PipelineOutputType.Coverage)
            {
                return MakeFirstNotNullGeoSource(inputs);                    
            } 
            if (firstSpec.OutputType == PipelineSpecification.PipelineOutputType.Feature)
            {   
                return MakeFeatureCollectionConcatenation(inputs);
            }

            throw new InvalidOperationException("Failed to find a good method to mosaic the GeoSources.");
        }
       
        private GeoSource MakeFeatureCollectionConcatenation(List<GeoSource> geoSources)
        {
            var processInfo = new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.ConcatFeatures);

            foreach (var geoSource in geoSources)
            {
                var input = Engine.GetProcess(geoSource).StripFeaturesStyleIfPossible();
                
                //try to strip cache only if the GeoSource has not been published yet
                if (geoSource.Metadata != null && 
                    geoSource.Metadata.Providers != null &&
                    geoSource.Metadata.Providers.Count == 0)
                {
                    input = input.StripCacheIfPossible();
                }

                processInfo.AddInput(0, input);
            }

            var process =
                PYXCOMFactory.CreateProcess(new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeaturesSummary)
                    .AddInput(0, PYXCOMFactory.CreateProcess(processInfo))
                    .SetName(geoSources[0].Metadata.Name + String.Format(" (mosaic of {0} GeoSources)", geoSources.Count))
                    .SetDescription(geoSources[0].Metadata.Description)
                    );

            return process.AsGeoSource(Engine);
        }

        private GeoSource MakeFirstNotNullGeoSource(List<GeoSource> geoSources)
        {
            var processInfo = new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageFirstNotNull);

            foreach (var geoSource in geoSources)
            {
                var input = Engine.GetProcess(geoSource).StripCoverageStyleIfPossible();

                //try to strip cache only if the GeoSource has not been published yet
                if (geoSource.Metadata != null &&
                    geoSource.Metadata.Providers != null &&
                    geoSource.Metadata.Providers.Count == 0)
                {
                    input = input.StripCacheIfPossible();
                }

                processInfo.AddInput(0, input);                
            }

            var process =
                PYXCOMFactory.CreateProcess(new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                    .AddInput(0, PYXCOMFactory.CreateProcess(processInfo))
                    .SetName(geoSources[0].Metadata.Name + String.Format(" (mosaic of {0} GeoSources)", geoSources.Count))
                    .SetDescription(geoSources[0].Metadata.Description)
                    );

            if (process.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                Trace.info("failed to init process");
            }

            return process.AsGeoSource(Engine);
        }
    }
}
