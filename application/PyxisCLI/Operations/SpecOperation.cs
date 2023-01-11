using System;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core.Analysis;
using Pyxis.Core.Measurements;

namespace PyxisCLI.Operations
{
    class SpecOperation : IOperationMode
    {
        public string Command
        {
            get { return "spec"; }
        }

        public string Description
        {
            get { return "show geosource specification"; }
        }

        public void Run(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("usage: pyx {0} geosource", Command);
                return;
            }

            var reference = new Reference(args[1]);

            if (Program.Workspaces.WorkspaceExists(reference.Workspace))
            {
                var geoSource = Program.Workspaces.ResolveGeoSource(reference);

                DisplaySpec(geoSource);
            }
            else
            {
                Console.WriteLine("Can't find: {0}", args[1]);
            }
        }

        public void DisplaySpec(GeoSource geoSource)
        {
            var process = Program.Engine.GetProcess(geoSource);
            var specification = Program.Engine.GetSpecification(geoSource);
            var feature = pyxlib.QueryInterface_IFeature(process.getOutput());
            var boundingCircle = feature.getGeometry().getBoundingCircle();
            var center = PointLocation.fromXYZ(boundingCircle.getCenter()).asWGS84();

            Console.WriteLine("Type: {0}", specification.OutputType);
            if (specification.OutputType == PipelineSpecification.PipelineOutputType.Feature)
            {
                Console.WriteLine("Number of Features: {0}", Program.Engine.GetAsFeature(geoSource).GetFeaturesCount());
            }
            Console.WriteLine("Number of Fields: {0}", specification.Fields.Count);
            Console.WriteLine("Native resolution: {0}", feature.getGeometry().getCellResolution());
            Console.WriteLine("Bounding Sphere: latitude={0},longitude={1},radius={2}[m]", center.y(), center.x(), (boundingCircle.getRadius() * SphericalDistance.Radian).InMeters );

            var i = 1;
            foreach (var field in specification.Fields)
            {
                if (field.Name != field.Metadata.Name)
                {
                    Console.WriteLine("Field #{0}: {1} ({2}) - display name: {3}", i, field.Name, field.FieldType,field.Metadata.Name);
                }
                else
                {
                    Console.WriteLine("Field #{0}: {1} ({2})", i, field.Name, field.FieldType);
                }
                i++;
            }
        }
    }
}
