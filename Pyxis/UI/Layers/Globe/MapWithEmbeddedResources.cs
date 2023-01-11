using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;

namespace Pyxis.UI.Layers.Globe
{
    /// <summary>
    /// Represents a Map with embedded GeoSources
    /// </summary>
    public class MapWithEmbeddedResources : Map
    {
        /// <summary>
        /// List of embedded GeoSources. Just to make serialization easier we are guiding it to a 
        /// specific type.
        /// </summary>
        public List<GeoSource> EmbeddedResources { get; set; }

        /// <summary>
        /// Upgrade an old Map model that utilize ProcRef,Definition to Groups and Dashboards
        /// </summary>
        /// <param name="engine">Engine to use to access Definition fields</param>
        public void Upgrade(Engine engine)
        {
            if (Groups != null && Groups.Sum(x => x.Items != null ? x.Items.Count : 0) > 0)
            {
                //map groups are in used already - map already upgrades

                //clear old definition and procRef
                Definition = null;
                ProcRef = null;
                return;
            }
            
            if (String.IsNullOrEmpty(Definition))
            {
                //no definition - there is nothing to upgrade
                return;
            }

            var process = engine.GetProcess(this);            
            
            Groups = new List<Group> {
                new Group {
                    Metadata = new SimpleMetadata {
                        Name = "",
                        Description = ""
                    },
                    Items = new List<Item>()
                }
            };
            Dashboards = new List<Dashboard>();
            EmbeddedResources = new List<GeoSource>();
            Camera = CameraExtensions.Default;            

            var document = application.QueryInterface_IDocument(process.getOutput());
            if (document.isNotNull())
            {
                var camera = new Camera();
                view_model_swig.camFromCookieStr(camera, document.getCameraCookieString());
                Camera = camera.ToCamera();

                //move to view point process
                process = process.getParameter(0).getValue(0);
            }

            //viewpoint process output is a regular coverage. so we need to cast the process and not process.getOutput()
            var viewpoint = pyxlib.QueryInterface_IViewPoint(
                            pyxlib.QueryInterface_PYXCOM_IUnknown(process));

            if (viewpoint.isNull())
            {
                throw new Exception("unsupported process, expected ViewPoint or Document process.");
            }

            foreach (var itemProcess in process.GetChildProcesses())
            {
                var geoSourceProcesses = itemProcess.ImmediateGeoPacketSources().ToList();

                if (geoSourceProcesses.Count == 0)
                {
                    throw new Exception("can't generate map item from input with no GeoPacketSource.");
                }

                if (geoSourceProcesses.Count > 1)
                {
                    throw new Exception("can't generate map item from input with multiple GeoPacketSources.");
                }

                var geoSourceProcess = geoSourceProcesses[0];

                var procref = pyxlib.procRefToStr(new ProcRef(geoSourceProcess));

                try
                {
                    var pipeline = engine.GetChannel().GetResourceByProcRef(procref);

                    Groups[0].Items.Add(new Item()
                    {
                        Metadata = pipeline.Metadata,
                        Resource = ResourceReference.FromResource(pipeline),
                        Specification = engine.GetSpecification(pipeline),
                        Style = itemProcess.ExtractStyle(),
                        Active = true,
                        ShowDetails = true
                    });
                }
                catch (Exception e)
                {
                    throw new Exception("Failed to resolve resource with ProcRef " + procref +".", e);
                }
            }

            //everything went ok - so we can remove the old definition.
            Definition = null;
            ProcRef = null;
        }

        /// <summary>
        /// Downgrade a Map that uses Groups and Dashboard into ProcRef and Definition
        /// </summary>
        /// <param name="engine">Engine to use to access Definition fields</param>
        public void Downgrade(Engine engine)
        {
            var channel = engine.GetChannel();
            var viewPointProcess = PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.ViewPointProcess);

            foreach (var mapItem in Groups.SelectMany(x => x.Items).Where(x => x.Active))
            {
                var geoSource = channel .GeoSources.GetByIdAndVersion(mapItem.Resource.Id,mapItem.Resource.Version);
                var styledGeoSource = StyledGeoSource.Create(engine, geoSource, mapItem.Style);
                styledGeoSource.AddToViewPoint(viewPointProcess);
            }

            var docProc = PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.DocumentProcess);
            var doc = application.QueryInterface_IDocument(docProc);
            doc.setViewPointProcess(viewPointProcess);

            if (Camera != null)
            {
                doc.setCameraCookieString(view_model_swig.camToCookieStr(Camera.ToPYXCamera()));
            }

            if (docProc.initProc() != IProcess.eInitStatus.knInitialized)
            {
                throw new Exception("failed to create document process");
            }

            //downgrade - allow wv browser to show the map from a process instead from groups.
            Definition = PipeManager.writePipelineToNewString(docProc);
            ProcRef = (new ProcRef(docProc)).ToString();
        }
    }
}
