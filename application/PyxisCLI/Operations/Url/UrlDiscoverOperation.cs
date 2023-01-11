using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations.Url
{
    class UrlDiscoverOperation : IOperationMode
    {
        public string Command
        {
            get { return "url discover"; }
        }

        public string Description
        {
            get { return "discover datasets on url"; }
        }

        public void Run(string[] args)
        {
            int numberOfProcess = 1;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("p|process", (value) => numberOfProcess = int.Parse(value))
                );


            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} {{action}}", Command);
                return;
            }

            DiscoverUrl(args.Skip(2).ToArray(), numberOfProcess);
        }

        private void DiscoverUrl(string[] urls, int numberOfProcess)
        {
            foreach (var url in urls)
            {
                var state = MongoPersistance.GetRoot(url);

                if (state != null)
                {
                    DiscoverUrl(state);
                }
                else
                {
                    Console.WriteLine("Could not find " + url);

                    List<UrlDiscoveryReport> suggestionList = MongoPersistance.GetRootSuggestion(url);
                    if (suggestionList.Count > 0)
                    {
                        Console.WriteLine("Possible similar urls are:");
                        foreach (var line in suggestionList)
                        {
                            Console.WriteLine(line.Uri);
                        }
                    }
                }
            }

            if (urls.Length == 0)
            {
                Parallel.ForEach(
                    MongoPersistance.GetRootsToDiscover(TimeSpan.FromDays(7)).ToList(),
                    new ParallelOptions() { MaxDegreeOfParallelism = numberOfProcess },
                    (state) => DiscoverUrl(state));
            }

        }

        private void DiscoverUrl(UrlDiscoveryReport state)
        {
            Console.WriteLine("Discovering uri: " + state.Uri);

            var discoverySummary = LocalGazetteer.DiscoverRecursively(Program.Engine, state.Uri, state.DataSetCount);
            var dataSets = discoverySummary.DataSets;
            var datasSetsChangeInfo = MongoPersistance.PrepareChangesForRoot(state, dataSets);

            RecoverDataSetVerificationFromOldDataSets(datasSetsChangeInfo);

            state.DataSetCount = dataSets.Count;
            state.BrokenDataSetCount =
                dataSets.Count(
                    x => x.DiscoveryReport != null && x.DiscoveryReport.Status == DataSetDiscoveryStatus.Failed);
            state.VerifiedDataSetCount =
                dataSets.Count(
                    x =>
                        x.DiscoveryReport != null &&
                        x.DiscoveryReport.Status == DataSetDiscoveryStatus.Successful);
            state.UnknownDataSetCount = state.DataSetCount - state.VerifiedDataSetCount - state.BrokenDataSetCount;
            state.LastDiscovered = DateTime.Now;
            state.Status = "Discovered";
            state.ServiceIdentifier = discoverySummary.ServiceIdentifier;

            var commitResult = MongoPersistance.CommitChangesForRoot(state, datasSetsChangeInfo);
            Console.WriteLine("Changes commited to database: inserted:{0}, replaced:{1}, deleted:{2}, no change:{3}", commitResult.InsertCount, commitResult.ReplaceCount, commitResult.DeleteCount, commitResult.NothingChangedCount);

            var rootsAddes = 0;
            foreach (var root in discoverySummary.AdditionalRoots)
            {
                
                var conflictedRoot = MongoPersistance.GetConflictedRoot(root.Uri);
                
                if (conflictedRoot == null)
                {
                    Console.WriteLine("Discovered new root : {0}", root.Uri);
                    MongoPersistance.AddRoot(root.Uri, root.ServiceIdentifier);
                    rootsAddes++;
                }
                else
                {
                    if (root.Uri != conflictedRoot.Uri)
                    {
                        Console.WriteLine("Root already exists : " + root.Uri + " same as " + conflictedRoot.Uri);
                    }
                    else
                    {
                        Console.WriteLine("Root already exists : " + root.Uri);
                    }
                }
            }

            AutomationLog.UpdateInfo("newRoots", rootsAddes);

            AutomationLog.PushInfo("root", state);
        }

        /// <summary>
        /// Try to find Verified Datasets status from old discovery operations and update the DiscoveryReport to new found datasets
        /// </summary>
        /// <param name="url">Root url</param>
        /// <param name="dataSetsChangeInfo">new discovered dataSets to populate discovery status uppon</param>
        private void RecoverDataSetVerificationFromOldDataSets(List<MongoDataSetChangeInfo> dataSetsChangeInfo)
        {
            foreach (var changeInfo in dataSetsChangeInfo.Where(changeInfo => changeInfo.ExistingDataset != null && changeInfo.NewDataset != null))
            {
                if (changeInfo.ExistingDataset.DiscoveryReport != null && changeInfo.ExistingDataset.DiscoveryReport.Status != DataSetDiscoveryStatus.Unknown)
                {
                    var existingBBox = JsonConvert.SerializeObject(changeInfo.ExistingDataset.BBox);
                    var newBBox = JsonConvert.SerializeObject(changeInfo.NewDataset.BBox);

                    //short-handed way to by pass the error where NewDataset.DiscoveryReport is null
                    var existingFeatureCount = changeInfo.ExistingDataset.DiscoveryReport.FeaturesCount;
                    var newFeatureCount = changeInfo.NewDataset.DiscoveryReport != null ? changeInfo.NewDataset.DiscoveryReport.FeaturesCount : 0;

                    if (existingFeatureCount == newFeatureCount && existingBBox == newBBox)
                    {
                        changeInfo.NewDataset.DiscoveryReport = changeInfo.ExistingDataset.DiscoveryReport;
                    }
                }
            }
        }
    }
}