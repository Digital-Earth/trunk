using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Threading;
using System.Xml;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core;
using Pyxis.Core.Analysis;
using Pyxis.Core.DERM;
using Pyxis.Core.IO;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class ValueOperation : IOperationMode
    {
        public string Command
        {
            get { return "value"; }
        }

        public string Description
        {
            get { return "get a cell value from a geosource"; }
        }

        public void Run(string[] args)
        {
            string inputField = null;
            string index = null;
            var json = false;
            string domainRange = null;

            args = ArgsParser.Parse(args,
               new ArgsParser.Option("j|json", (value) => json = true),
               new ArgsParser.Option("f|field", (value) => inputField = value),
               new ArgsParser.Option("i|index", (value) => index = value),
               new ArgsParser.Option("domain", (value) => domainRange = value));

            if (args.Length < 1)
            {
                Console.WriteLine("usage: pyx {0} reference", Command);
                return;
            }

            var dggs = Program.Engine.DERM();
            Cell cell = null;

            IGeometry geometry = null;
            if (index.HasContent())
            {
                cell = dggs.Cell(index);
            }
            else
            {
                throw new Exception("Please provide cell index to get value from");
            }

            if (domainRange.HasContent())
            {
                var reference = new Reference(args[1]);

                var importTemplate = Program.Workspaces.GetWorkspace(reference.Workspace).Imports[reference.Name] as ImportTemplate;

                if (importTemplate == null)
                {
                    throw new Exception("domain range can only be used with import templates");
                }

                var parts = domainRange.Split(',');
                var name = parts[0];
                var start = parts[1];
                var end = parts[2];
                var step = parts[3];

                var range = new Pyxis.Contract.Workspaces.Domains.DateRangeDomain(
                    DateTime.Parse(start, null, DateTimeStyles.AssumeUniversal).ToUniversalTime(),
                    DateTime.Parse(end, null, DateTimeStyles.AssumeUniversal).ToUniversalTime(),
                    step);

                if (reference.Domains == null)
                {
                    reference.Domains = new Dictionary<string, string>();
                }

                foreach(var value in range.Values)
                {
                    reference.Domains[name] = value;
                    FetchValueFromGeoSource(reference.ToString(), inputField, cell);   
                }
            }
            else
            {
                var reference = new Reference(args[1]);

                FetchValueFromGeoSource(args[1], inputField, cell);    
            }
        }

        private static void FetchValueFromGeoSource(string reference, string inputField, Cell cell)
        {
            var geoSource = Program.Workspaces.ResolveGeoSource(reference);

            var fields = new List<string>();
            if (inputField.HasContent())
            {
                if (geoSource.Specification.HasField(inputField))
                {
                    fields.Add(inputField);
                }
                else
                {
                    throw new Exception(
                        string.Format("Reference {0} has no field named {1}. Availabled fields: {2}", reference, inputField, string.Join(", ", geoSource.Specification.FieldNames())));
                }
            }
            else
            {
                fields.AddRange(geoSource.Specification.FieldNames());
            }

            foreach (var field in fields)
            {
                Console.WriteLine("{0}={1}", field, cell.ValueOf(geoSource, geoSource.Specification.FieldIndex(field)));
            }
        }
    }
}
