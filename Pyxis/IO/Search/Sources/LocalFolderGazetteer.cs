using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Core;

namespace Pyxis.IO.Search.Sources
{
    public class LocalFolderGazetteer
    {
        public static IGazetteer CreateFromFolder(string path, string filePattern = "*.json", int segments = 0, int segmentsId = 0)
        {
            var gazetteer = new Gazetteer();

            if (Directory.Exists(path))
            {
                var dataSets = new List<DataSet>();

                var files = Directory.GetFiles(path, filePattern);

                if (segments > 0)
                {
                    files = files.Where((file, index) => index % segments == segmentsId).ToArray();
                }

                Parallel.ForEach(files, (file) =>
                {
                    Console.WriteLine(file);

                    var dataSetsInFile =
                        JsonConvert.DeserializeObject<List<DataSetWithResource>>(File.ReadAllText(file))
                            .RemoveDuplicates();

                    lock (dataSets)
                    {
                        dataSets.AddRange(dataSetsInFile);
                    }
                });

                Console.WriteLine("Loaded {0} files with {1} dataSets",files.Length,dataSets.Count);

                gazetteer.AddRange(dataSets.RemoveDuplicates());
            }

            return gazetteer;
        }
    }
}
