using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations.Url
{
    class UrlValidateOperation : IOperationMode
    {
        public string Command
        {
            get { return "url validate"; }
        }

        public string Description
        {
            get { return "validate datasets on a url"; }
        }

        public void Run(string[] args)
        {
            int numberOfDataSets = 10;
            int numberOfFeatures = 100000;
            int skipDatasets = 0;
            bool cleanValidatingFiles = false;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("n|number", (value) => numberOfDataSets = int.Parse(value)),
                new ArgsParser.Option("s|skip", (value) => skipDatasets = int.Parse(value)),
                new ArgsParser.Option("f|features", (value) => numberOfFeatures = int.Parse(value)),
                new ArgsParser.Option("clean", (value) => cleanValidatingFiles = true)
                );

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} {{action}}", Command);
                return;
            }

            ValidateUrl(args.Skip(2).ToArray(), numberOfFeatures, numberOfDataSets, skipDatasets, cleanValidatingFiles);
        }

        private void ValidateUrl(string[] urls, int featuresLimit = 10000, int dataSetCount = 10, int skipDatasets = 0, bool cleanValidatingFiles = false)
        {
            foreach (var url in urls)
            {
                ValidateUrl(url, featuresLimit, dataSetCount, skipDatasets, cleanValidatingFiles);
            }
        }

        private void ValidateUrl(string url, int featuresLimit = 10000, int dataSetCount = 10, int skipDatasets = 0, bool cleanValidatingFiles = false)
        {
            var urlDiscoveryStatus = MongoPersistance.GetRoot(url);

            if (urlDiscoveryStatus != null)
            {
                var dataSets = MongoPersistance.GetUnverifiedDataSetsForRoot(url).OrderBy(x => x.DiscoveryReport != null ? x.DiscoveryReport.FeaturesCount : 0).Where(x => x.DiscoveryReport == null || x.DiscoveryReport.FeaturesCount < featuresLimit).Skip(skipDatasets).Take(dataSetCount).ToList();

                AutomationLog.UpdateInfo("datasets", dataSets.Count);

                var index = 0;
                var importCount = 0;
                var validated = 0;
                var broken = 0;
                string localPath = LocalGazetteer.GetLocalDirForServer(new Uri(url));
                Program.EngineConfig.CacheDirectory = Path.Combine(localPath, "PYXCache");

                foreach (var dataSet in dataSets)
                {
                    index++;

                    if (dataSet.DiscoveryReport == null)
                    {
                        dataSet.DiscoveryReport = new DataSetDiscoveryReport();
                    }

                    if (dataSet.DiscoveryReport.Status == DataSetDiscoveryStatus.Successful)
                    {
                        validated++;
                        continue;
                    }

                    if (dataSet.DiscoveryReport.Status == DataSetDiscoveryStatus.Failed || dataSet.DiscoveryReport.Issues != null && dataSet.DiscoveryReport.Issues.Count > 0)
                    {
                        broken++;
                        continue;
                    }

                    if (dataSet.DiscoveryReport.FeaturesCount > featuresLimit)
                    {
                        continue;
                    }

                    importCount++;

                    dataSet.DiscoveryReport.Issues = null;
                    dataSet.DiscoveryReport.Status = DataSetDiscoveryStatus.Unknown;
                    dataSet.DiscoveryReport.DiscoveredTime = DateTime.UtcNow;

                    var start = DateTime.Now;

                    Console.WriteLine(index + ": " + dataSet.Uri);

                    try
                    {
                        var settingProvider = new ImportSettingProvider();
                        settingProvider.Register(typeof(DownloadLocallySetting), new DownloadLocallySetting()
                        {
                            Path = LocalGazetteer.GetLocalDir(dataSet)
                        });

                        var geoSource = Program.Engine.BeginImport(dataSet, settingProvider).Task.Result;

                        dataSet.DiscoveryReport.ImportTime = (DateTime.Now - start);
                        dataSet.DiscoveryReport.DataSize = geoSource.DataSize ?? 0;
                        dataSet.DiscoveryReport.Status = DataSetDiscoveryStatus.Successful;
                        validated++;
                    }
                    catch (Exception ex)
                    {
                        broken++;
                        dataSet.DiscoveryReport.Status = DataSetDiscoveryStatus.Failed;
                        var aggregateException = ex as AggregateException;
                        if (aggregateException != null)
                        {
                            foreach (var innerEx in aggregateException.InnerExceptions)
                            {
                                dataSet.DiscoveryReport.AddError(innerEx.Message);
                            }
                        }
                        else
                        {
                            dataSet.DiscoveryReport.AddError(ex.Message);
                        }

                    }

                    MongoPersistance.UpdateDataSet(dataSet);

                    AutomationLog.UpdateInfo("progress", 100 * index / dataSets.Count);
                    Console.WriteLine(JsonConvert.SerializeObject(dataSet.DiscoveryReport, Formatting.Indented));

                    GC.Collect();
                    var mb = (Process.GetCurrentProcess().PagedMemorySize64 / 1024 / 1024);
                    Console.WriteLine("Memory Usage: " + mb + " MB");

                    if (mb > 700)
                    {
                        Console.WriteLine("It seems we are too heavy on memory.");
                        break;
                    }
                }

                if (cleanValidatingFiles)
                {
                    CleanValidationFiles(localPath);
                }

                AutomationLog.UpdateInfo("progress", 100);
                AutomationLog.UpdateInfo("validated", validated);

                Console.WriteLine("Validated " + validated + " From " + index + " DataSets under " + url);

                var status = MongoPersistance.CountDataSetsStatusForRoot(url);
                urlDiscoveryStatus.VerifiedDataSetCount = status.ContainsKey(DataSetDiscoveryStatus.Successful) ? (int) status[DataSetDiscoveryStatus.Successful] : 0;
                urlDiscoveryStatus.BrokenDataSetCount = status.ContainsKey(DataSetDiscoveryStatus.Failed) ? (int)status[DataSetDiscoveryStatus.Failed] : 0;
                urlDiscoveryStatus.UnknownDataSetCount = status.ContainsKey(DataSetDiscoveryStatus.Unknown) ? (int)status[DataSetDiscoveryStatus.Unknown] : 0;
                urlDiscoveryStatus.LastVerified = DateTime.UtcNow;

                MongoPersistance.UpdateRoot(urlDiscoveryStatus);
                AutomationLog.UpdateInfo("root", urlDiscoveryStatus);
            }
        }

        private static void CleanValidationFiles(string localPath)
        {
            Program.BeforeExit += (sender, args) =>
            {
                const int MaxRetry = 3;
                const int SecondsBetweenRetries = 3;
                GC.Collect();

                for (var retry = 0; retry < MaxRetry; retry++)
                {
                    try
                    {
                        var localDir = new DirectoryInfo(localPath);
                        localDir.Delete(true);
                        return;
                    }
                    catch (Exception ex)
                    {
                        if (retry == MaxRetry)
                        {
                            Console.WriteLine("Failed to delete validation directory:" + ex.Message);
                        }
                        else
                        {
                            Console.WriteLine("Failed to delete validation directory. Try again in " + SecondsBetweenRetries + " sec");
                            System.Threading.Thread.Sleep(TimeSpan.FromSeconds(SecondsBetweenRetries));
                        }
                    }
                }
            };
        }
    }
}