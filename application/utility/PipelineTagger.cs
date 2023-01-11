using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using Pyxis.Utilities;

namespace ApplicationUtility
{
    [DataContract]
    public class PipelineTagsResult
    {
        [DataMember]
        public List<string> Tags = new List<string>();
        public List<string> Errors = new List<string>();

        public bool IsPublishable 
        { 
            get 
            { 
                return !Tags.Contains(PipelineTags.Unpublishable); 
            }
        }
        public string ErrorMessage 
        { 
            get 
            {
                return string.Join("; ", Errors);
            }
        }
    }

    static public class PipelineTags
    {
        static public string Unpublishable { get { return "Unpublishable"; } }
    }

    static public class PipelineTagger
    {
        static private List<IPipelineTagger> s_taggers;

        static PipelineTagger()
        {
            s_taggers = new List<IPipelineTagger>();
            var unorderedTaggers = GlobalTypeFinder.Interfaces<IPipelineTagger>.FindAll().
                Where(t => t.GetConstructor(Type.EmptyTypes) != null).
                Select(t => Activator.CreateInstance(t) as IPipelineTagger).
                ToList();

            // topological sort on dependencies
            var noOrderingIterations = 0;
            while (unorderedTaggers.Count > 0)
            {
                var tagger = unorderedTaggers[0];
                unorderedTaggers.RemoveAt(0);
                if (tagger.Dependencies.All(x => s_taggers.Select(t => t.GetType()).Contains(x)))
                {
                    s_taggers.Add(tagger);
                    noOrderingIterations = 0;
                }
                else
                {
                    unorderedTaggers.Add(tagger);
                    noOrderingIterations++;
                }
                if (unorderedTaggers.Count > 0 && noOrderingIterations == unorderedTaggers.Count)
                {
                    throw new Exception("IPipelineTagger dependency cycle detected");
                }
            }
        }

        static public PipelineTagsResult Tag(IProcess_SPtr process)
        {
            var tagHolder = new PipelineTagsResult();
            if (process.getSpec().getClass() == PYXCOMFactory.WellKnownProcesses.ViewPointProcess)
            {
                // currently no automated Map tags
                return tagHolder;
            }
            if (!publishableCandidate(process))
            {
                tagHolder.Tags.Add(PipelineTags.Unpublishable);
                tagHolder.Errors.Add("Unable to publish because their is a dependency on an unpublished, remote file.");
                return tagHolder;
            }
            foreach (var tagger in s_taggers)
            {
                var newTagHolder = tagger.Tag(process);
                tagHolder.Tags.AddRange(newTagHolder.Tags);
                if(!newTagHolder.IsPublishable)
                {
                    tagHolder.Errors.AddRange(newTagHolder.Errors);
                    break;
                }
            }
            return tagHolder;
        }

        private static bool publishableCandidate(IProcess_SPtr headProcess)
        {
            foreach (var process in headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent())
            {
                if (process.getInitState() != IProcess.eInitStatus.knInitialized)
                {
                    return false;
                }
            }
            return true;
        }
    }

    interface IPipelineTagger
    {
        List<Type> Dependencies { get; }
        PipelineTagsResult Tag(IProcess_SPtr headProcess);
    }

    public class OGCTagger : IPipelineTagger
    {
        private List<Type> m_dependencies = new List<Type>();
        public List<Type> Dependencies { get { return m_dependencies; } }

        public PipelineTagsResult Tag(IProcess_SPtr headProcess)
        {
            var tagHolder = new PipelineTagsResult();
            var processIOWSReference = headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct().FirstOrDefault(p => p.ProvidesOutputType(PYXCOMFactory.WellKnownInterfaces.IOWSReference));
            if (processIOWSReference != null)
            {
                var hasSupportingFiles = headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().SupportingFiles().Any();
                bool multiSource = false;
                if (!hasSupportingFiles)
                {
                    foreach (var process in headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct())
                    {
                        var sourceCount = 0;
                        foreach (var parameter in process.getParameters())
                        {
                            sourceCount += parameter.getValueCount();
                            if (sourceCount > 1)
                            {
                                multiSource = true;
                                break;
                            }
                        }
                    }
                }

                if (hasSupportingFiles || multiSource)
                {
                    tagHolder.Tags.Add(PipelineTags.Unpublishable);
                    tagHolder.Errors.Add("Only single source OGC pipelines may be published.");
                }
                else
                {
                    tagHolder.Tags.Add("OGC");

                    var IOWSReferenceClass = processIOWSReference.getSpec().getClass();
                    if (IOWSReferenceClass == PYXCOMFactory.WellKnownProcesses.GDALWCSProcessV2)
                    {
                        tagHolder.Tags.Add("WCS");
                    }
                    else if (IOWSReferenceClass == PYXCOMFactory.WellKnownProcesses.OGRWFSProcess)
                    {
                        tagHolder.Tags.Add("WFS");
                    }
                    else if (IOWSReferenceClass == PYXCOMFactory.WellKnownProcesses.GDALWMSProcess)
                    {
                        tagHolder.Tags.Add("WMS");
                    }
                }
            }
            return tagHolder;
        }
    }

    public class GeoServicesTagger : IPipelineTagger
    {
        private List<Type> m_dependencies = new List<Type>();
        public List<Type> Dependencies { get { return m_dependencies; } }

        public PipelineTagsResult Tag(IProcess_SPtr headProcess)
        {
            var tagHolder = new PipelineTagsResult();
            var processIGSReference = headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct().FirstOrDefault(p => p.ProvidesOutputType(PYXCOMFactory.WellKnownInterfaces.IGeoServicesReference));
            if (processIGSReference != null)
            {
                var hasSupportingFiles = headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().SupportingFiles().Any();
                bool multiSource = false;
                if (!hasSupportingFiles)
                {
                    foreach (var process in headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct())
                    {
                        var sourceCount = 0;
                        foreach (var parameter in process.getParameters())
                        {
                            sourceCount += parameter.getValueCount();
                            if (sourceCount > 1)
                            {
                                multiSource = true;
                                break;
                            }
                        }
                    }
                }

                if (hasSupportingFiles || multiSource)
                {
                    tagHolder.Tags.Add(PipelineTags.Unpublishable);
                    tagHolder.Errors.Add("Only single source GeoService pipelines may be published.");
                }
                else
                {
                    tagHolder.Tags.Add("GeoServices");

                    var IGSReferenceClass = processIGSReference.getSpec().getClass();
                    if (IGSReferenceClass == PYXCOMFactory.WellKnownProcesses.OGRFeatureServerProcess)
                    {
                        tagHolder.Tags.Add("FeatureServer");
                    }
                }
            }
            return tagHolder;
        }
    }

    public class BingTagger : IPipelineTagger
    {
        private List<Type> m_dependencies = new List<Type>();
        public List<Type> Dependencies { get { return m_dependencies; } }

        public PipelineTagsResult Tag(IProcess_SPtr headProcess)
        {
            var tagHolder = new PipelineTagsResult();
            var processBingReference = headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct().FirstOrDefault(p => p.getSpec().getClass() == PYXCOMFactory.WellKnownProcesses.GDALBingProcess);
            if (processBingReference == null)
            {
                return tagHolder;
            }
            var hasSupportingFiles = headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().SupportingFiles().Any();
            bool multiSource = false;
            if (!hasSupportingFiles)
            {
                foreach (var process in headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct())
                {
                    var sourceCount = 0;
                    foreach (var parameter in process.getParameters())
                    {
                        sourceCount += parameter.getValueCount();
                        if (sourceCount > 1)
                        {
                            multiSource = true;
                            break;
                        }
                    }
                }
            }

            if (hasSupportingFiles || multiSource)
            {
                tagHolder.Tags.Add(PipelineTags.Unpublishable);
                tagHolder.Errors.Add("Only single source Bing pipelines may be published.");
            }
            else
            {
                tagHolder.Tags.Add("WebService");
            }
            return tagHolder;
        }
    }

    public class ExcelTagger : IPipelineTagger
    {
        private List<Type> m_dependencies = new List<Type>();
        public List<Type> Dependencies { get { return m_dependencies; } }

        public PipelineTagsResult Tag(IProcess_SPtr headProcess)
        {
            var tagHolder = new PipelineTagsResult();
            foreach (var process in headProcess.WalkPipelinesExcludeGeoPacketSourcesAfterParent().Distinct())
            {
                if (process.getSpec().getClass() == PYXCOMFactory.WellKnownProcesses.ExcelRecordCollectionProcess)
                {
                    tagHolder.Tags.Add(PipelineTags.Unpublishable);
                    tagHolder.Errors.Add("Pipelines referencing Microsoft Excel files may not be published.");
                    break;
                }
            }
            return tagHolder;
        }
    }
}
