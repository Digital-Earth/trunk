using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.DataDiscovery;
using Pyxis.Utilities;
using ApplicationUtility;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// Class that represent an import process of a url into GeoSource.
    /// </summary>
    public class ImportGeoSourceProgress
    {
        /// <summary>
        /// Data set to import from
        /// </summary>
        public DataSet DataSet { get; private set; }

        private IDataSetImportService DataSetImportService { get; set; }

        /// <summary>
        /// SettingProvider been used for the import action
        /// </summary>
        public IImportSettingProvider SettingProvider { get; private set; }

        /// <summary>
        /// Pyxis.Core.Engine that been used to generate the GeoSource
        /// </summary>
        public Engine Engine { get; private set; }

        // Holds the result callback
        private readonly TaskCompletionSource<GeoSource> m_taskSource;

        /// <summary>
        /// Get the task of GeoSource that can be waited for getting the GeoSource
        /// </summary>
        public Task<GeoSource> Task
        {
            get
            {
                return m_taskSource.Task;
            }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        internal ImportGeoSourceProgress(
            Engine engine,
            DataSet dataSet,
            IDataSetImportService importService,
            IImportSettingProvider settingProvider
            )
        {
            Engine = engine;
            DataSet = dataSet;
            DataSetImportService = importService;
            SettingProvider = settingProvider;

            m_taskSource = new TaskCompletionSource<GeoSource>();

            System.Threading.Tasks.Task.Factory.StartNew(BeginImport);
        }

        private void BeginImport()
        {
            try
            {
                // Build a pipeline and verify the result
                var process = DataSetImportService.BuildPipeline(Engine, SettingProvider);
                if (process == null)
                {
                    m_taskSource.SetException(new Exception("Failed to create a pipeline for URI: " + DataSet.Uri));
                    return;
                }

                // Perform actual importing
                InitializeResult(process);
            }
            catch (Exception ex)
            {
                m_taskSource.SetException(ex);
            }
        }

        private void InitializeResult(IProcess_SPtr process)
        {
            try
            {
                if (process.isNull())
                {
                    m_taskSource.SetException(new Exception("Failed to create a pipeline for URI: " + DataSet.Uri));
                    return;
                }

                if (process.initProc(true) == IProcess.eInitStatus.knInitialized)
                {
                    VerifyOutput(process);
                    return;
                }

                var errorProc = PipeUtils.findFirstError(process);
                var error = errorProc.getInitError();

                //allow the process error fixer to fix the first error.
                var task = ProcessErrorFixer.Fix(process, errorProc, SettingProvider);

                if (task == null)
                {
                    //error fixer can't fix that error
                    m_taskSource.SetException(new Exception(error.getError()));
                    return;
                }

                task.ContinueWith(t =>
                {
                    //when error fixer made a progress...

                    if (t.IsFaulted)
                    {
                        //if it has failed...
                        m_taskSource.SetException(t.Exception);
                    }
                    else
                    {
                        //try to initialize the pipeline again and fix the next error if we found one.
                        InitializeResult(t.Result);
                    }
                });
            }
            catch (Exception ex)
            {
                m_taskSource.SetException(ex);
            }
        }

        /// <summary>
        /// Verify that the process output has geo-spatial meaning
        /// </summary>
        /// <param name="process">the process to verify the output has geospatial meaning</param>
        private void VerifyOutput(IProcess_SPtr process)
        {
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

            if (coverage.isNotNull() || featureCollection.isNotNull())
            {
                var geoSource = CreateGeoSourceFromProcess(process);
                DataSetImportService.EnrichGeoSource(Engine, geoSource);
                
                m_taskSource.SetResult(geoSource);
                return;
            }

            var recordCollection = pyxlib.QueryInterface_IRecordCollection(process.getOutput());

            if (recordCollection.isNotNull())
            {
                var task = SettingProvider.ProvideSetting(
                    typeof(GeoTagImportSetting),
                    new ProvideGeoTagImportSettingArgs()
                    {
                        RecordCollection = process
                    });

                if (task != null)
                {
                    task.ContinueWith(t =>
                    {
                        if (t.IsFaulted)
                        {
                            m_taskSource.SetException(t.Exception);
                            return;
                        }

                        try
                        {
                            var geoTagImportSetting = t.Result as GeoTagImportSetting;

                            if (geoTagImportSetting == null || geoTagImportSetting.Method == null)
                            {
                                m_taskSource.SetException(new Exception("No GeoTag method was provided"));
                                return;
                            }

                            var geoTaggedProcess = geoTagImportSetting.Method.GeoTag(process);

                            InitializeResult(geoTaggedProcess);
                        }
                        catch (Exception e)
                        {
                            m_taskSource.SetException(e);
                        }
                    });
                    return;
                }
                else
                {
                    m_taskSource.SetException(new Exception("To import a Record Collection pipeline please provide GeoTagImportSetting"));
                }
                
            }

            m_taskSource.SetException(new Exception("Unsupported Pipeline output type "));
        }

        private GeoSource CreateGeoSourceFromProcess(IProcess_SPtr process)
        {
            //make sure we have a stable identity
            PipeUtils.waitUntilPipelineIdentityStable(process);

            var geoPacketSource = process.GeoPacketSources().First();
            var manifests = new List<Manifest>(geoPacketSource.WalkPipelinesExcludeGeoPacketSourcesAfterParent().ExtractManifests());
            long datasize = manifests.SelectMany(m => m.Entries).Sum(e => e.FileSize);

            var geoSource = new GeoSource(
                id: Guid.NewGuid(),
                licenses: new List<LicenseReference>(),
                metadata: new Metadata(
                    name: DataSet.Metadata.Name ?? process.getProcName(),
                    description: DataSet.Metadata.Description ?? process.getProcDescription(),
                    user: Engine.GetUserInfo(),
                    providers: new List<Provider>(),
                    category: "",
                    tags: DataSet.Metadata.Tags ?? new List<string>(),
                    systemTags: new List<string>(),
                    created: DateTime.Now,
                    updated: DateTime.Now,
                    basedOnVersion: null,
                    externalUrls: GetExternalUrls(),
                    visibility: VisibilityType.Public,
                    comments: new LinkedList<AggregateComment>(),
                    ratings: new AggregateRatings()),
                version: Guid.NewGuid(),
                procRef: pyxlib.procRefToStr(new ProcRef(process)),
                definition: PipeManager.writePipelineToNewString(process),
                basedOn: new List<ResourceReference>(),
                state: null,
                dataSize: datasize,
                styles: new List<ResourceReference>(),
                usedBy: new List<Guid>(),
                related: new List<Guid>());

            geoSource.Specification = process.CreatePipelineSpecification();
            geoSource.Style = process.ExtractStyle();

            return geoSource;
        }

        private List<ExternalUrl> GetExternalUrls()
        {
            // Check if the URI is an HTTP or HTTPS URL
            Uri buffer;
            if (
                System.Uri.TryCreate(DataSet.Uri, UriKind.Absolute, out buffer)
                && (buffer.Scheme == System.Uri.UriSchemeHttp || buffer.Scheme == System.Uri.UriSchemeHttps)
                )
            {
                return new List<ExternalUrl>
                {
                    new ExternalUrl
                    {
                        Type = ExternalUrlType.Reference,
                        Url = DataSet.Uri
                    }
                };
            }
            // Otherwise don't set a reference URL and return an empty collection
            return new List<ExternalUrl>();
        }
    }
}
