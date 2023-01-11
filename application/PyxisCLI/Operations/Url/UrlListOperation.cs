using System;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations.Url
{
    class UrlListOperation : IOperationMode
    {
        public string Command
        {
            get { return "url list"; }
        }


        public string Description
        {
            get { return "list urls"; }
        }

        public void Run(string[] args)
        {
            bool jsonOutput = false;
            bool exportToCsv = false;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("json", (value) => jsonOutput = true),
                new ArgsParser.Option("csv", (value) => exportToCsv = true));

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0}", Command);
                return;
            }

            if (exportToCsv)
            {
                ListUrlsAsCsv();
            }
            else
            {
                ListUrls(args, jsonOutput);
            }
        }

        private void ListUrlsAsCsv()
        {
            Console.WriteLine("Url,Status,Last Discovered,DataSets,Verified DataSets,Broken DataSets");

            foreach (var url in MongoPersistance.GetRoots())
            {
                Console.WriteLine("{0},{1},{2},{3},{4},{5}", url.Uri, url.Status, url.LastDiscovered, url.DataSetCount, url.VerifiedDataSetCount, url.BrokenDataSetCount);
            }
        }

        private void ListUrls(string[] args, bool jsonOutput)
        {
            var rootCount = 0;
            var datasetCount = 0;
            var verifiedDatasetCount = 0;

            foreach (var url in MongoPersistance.GetRoots())
            {
                rootCount++;
                datasetCount += url.DataSetCount;
                verifiedDatasetCount += url.VerifiedDataSetCount;

                if (jsonOutput)
                {
                    AutomationLog.PushInfo("roots", url);
                }
                else
                {
                    Console.WriteLine("{0} : {1} ({3}% working of {2} DataSets / {4} Broken)", url.Uri, url.Status, url.DataSetCount, url.DataSetCount > 0 ? 100 * url.VerifiedDataSetCount / url.DataSetCount : 0, url.BrokenDataSetCount);
                }
            }

            if (jsonOutput)
            {
                AutomationLog.UpdateInfo("totalRoots", rootCount);
                AutomationLog.UpdateInfo("totalDataSets", datasetCount);
                AutomationLog.UpdateInfo("totalVerifiedDataSets", verifiedDatasetCount);
            }
            else
            {
                Console.WriteLine("Number of roots: {0}", rootCount);
                Console.WriteLine("Number of dataset: {0}", datasetCount);
                Console.WriteLine("Number of verified dataset: {0}", verifiedDatasetCount);
            }
        }
    }
}