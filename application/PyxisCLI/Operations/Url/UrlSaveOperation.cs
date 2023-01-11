using System;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.DataDiscovery;
using PyxisCLI.State;
using File = System.IO.File;

namespace PyxisCLI.Operations.Url
{
    class UrlSaveOperation : IOperationMode
    {
        public string Command
        {
            get { return "url save"; }
        }

        public string Description
        {
            get { return "export url datasets into local files"; }
        }

        public void Run(string[] args)
        {
            args = args.Skip(2).ToArray();


            var roots = args.Length > 0
                
                // convert input args into roots
                ? args.Select(MongoPersistance.GetRoot).ToList()

                // get all roots
                : MongoPersistance.GetRoots().ToList();

            //force initialize the engine.
            Program.Engine.GetChannel();

            var index = 0;
            foreach (var root in roots)
            {
                index++;
                try
                {
                    SaveUrl(root, index);
                }
                catch (Exception e)
                {
                    Console.WriteLine("Failed to write {0} due to: {1}",root.Uri,e.Message);
                    SaveUrl(root, index);
                }
            }
            
        }

        private void SaveUrl(UrlDiscoveryReport root, int index)
        {
            Console.WriteLine(root.Uri + " with " + root.VerifiedDataSetCount + " verified datasets");
            var verifiedDataSets = MongoPersistance.GetVerifiedDataSetsForRoot(root.Uri).ToList();

            //make sure we have a valid bbox
            verifiedDataSets.ForEach(ConvertBBoxToWgs84);

            verifiedDataSets = verifiedDataSets.Where(dataSet => dataSet.BBox != null).ToList();

            Console.WriteLine("after fixing bbox, we have " + verifiedDataSets.Count + " datasets");

            var uniqueDataSets = LocalGazetteer.DetectDuplicatedDataSets(verifiedDataSets);

            Console.WriteLine("after removing duplicate datasets, we have " + uniqueDataSets.Count + " datasets");

            File.WriteAllText("DataSets." + index + "." + new Uri(root.Uri).Host + ".json",
                JsonConvert.SerializeObject(uniqueDataSets));
        }

        private void ConvertBBoxToWgs84(DataSet dataSet)
        {
            if (dataSet.BBox == null)
            {
                return;
            }

            try
            {
                dataSet.BBox = dataSet.BBox.ConvertBBoxToWgs84();
            }
            catch (Exception ex)
            {
                //bboxs are broken
                dataSet.BBox = null;
            }
        }
    }
}