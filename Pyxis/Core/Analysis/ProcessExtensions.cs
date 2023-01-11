using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;

namespace Pyxis.Core.Analysis
{
    internal static class ProcessExtensions
    {
        internal static GeoSource AsGeoSource(this IProcess_SPtr process, Engine engine)
        {
            return new GeoSource(
                id: Guid.NewGuid(),
                licenses: new List<LicenseReference>(),
                metadata: new Metadata(
                    name: process.getProcName(),
                    description: process.getProcDescription(),
                    user: engine.GetUserInfo(),
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
                version: Guid.NewGuid(),
                procRef: pyxlib.procRefToStr(new ProcRef(process)),
                definition: PipeManager.writePipelineToNewString(process),
                basedOn: new List<ResourceReference>(),
                state: null,
                dataSize: 0,
                styles: new List<ResourceReference>(),
                usedBy: new List<Guid>(),
                related: new List<Guid>(),
                specifiction: process.CreatePipelineSpecification());
        }

        internal static IProcess_SPtr StripFeaturesStyleIfPossible(this IProcess_SPtr process)
        {
            //remove style process if any
            var processClass = process.getSpec().getClass();
            while (
                processClass == PYXCOMFactory.WellKnownProcesses.StyledFeaturesSummary ||
                processClass == PYXCOMFactory.WellKnownProcesses.LineStyledFeatureCollection ||
                processClass == PYXCOMFactory.WellKnownProcesses.IconStyledFeatureCollection ||
                processClass == PYXCOMFactory.WellKnownProcesses.AreaStyledFeatureCollection
                )
            {
                process = process.getParameter(0).getValue(0);
                processClass = process.getSpec().getClass();
            }

            return process;            
        }

        internal static IProcess_SPtr StripCoverageStyleIfPossible(this IProcess_SPtr process)
        {
            //remove style process if any
            while (process.getSpec().getClass() == PYXCOMFactory.WellKnownProcesses.StyledCoverage)
            {
                process = process.getParameter(0).getValue(0);
            }

            return process;
        }

        internal static IProcess_SPtr StripCacheIfPossible(this IProcess_SPtr process)
        {
            //collect the geoSource local supporting files            
            var files =
                process.WalkPipelinesExcludeGeoPacketSourcesAfterParent()
                    .SupportingFiles()
                    .ToList();

            //collect all of the pipeline's local & remote supporting files
            var manifests =
                process.WalkPipelinesExcludeGeoPacketSourcesAfterParent()
                    .ExtractManifests()
                    .SelectMany(x => x.Entries)
                    .ToList();

            //make sure that if there are supporting files - they are all local
            if (files.Count == manifests.Count)
            {
                if (process.getSpec().providesOutputType(IGeoPacketSource.iid))
                {
                    //remove the IGeoPacketSource by returning its first input
                    return process.getParameter(0).getValue(0);
                }
            }

            return process;
        }
    }
}
