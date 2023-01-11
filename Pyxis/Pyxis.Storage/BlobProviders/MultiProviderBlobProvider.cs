using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace Pyxis.Storage.BlobProviders
{
    public class MultiProviderBlobProvider : AbstractBlobProvider
    {
        public List<IBlobProvider> BlobProviders;

        public MultiProviderBlobProvider()
        {
            BlobProviders = new List<IBlobProvider>();
        }

        public override IDictionary<string, string> GetBlobs(IEnumerable<string> keys)
        {
            var updateList = new List<IBlobProvider>();
            var result = new List<IDictionary<string, string>>();
            var missingKeys = keys;

            foreach (var bprovider in BlobProviders)
            {
                result.Add(bprovider.GetBlobs(missingKeys));
                missingKeys = bprovider.MissingBlobs(missingKeys.ToArray());
                if (missingKeys.Count() == 0)
                {
                    break;
                }
            }

            // Update the other providers.
            for (int i = 1; i < result.Count; i++)
            {
                for (int j = 0; j < i; j++)
                {
                    foreach (var item in result[i])
                    {
                        using (var stream = GenerateStreamFromString(item.Value))
                        {
                            BlobProviders[j].AddBlob(item.Key, stream);
                        }
                    }
                }
            }

            return result.SelectMany(dict => dict)
                         .ToDictionary(pair => pair.Key, pair => pair.Value);
        }

        public Stream GenerateStreamFromString(string s)
        {
            MemoryStream stream = new MemoryStream();
            StreamWriter writer = new StreamWriter(stream);
            writer.Write(s);
            writer.Flush();
            stream.Position = 0;
            return stream;
        }

        public override bool GetBlob(string key, Stream data)
        {
            var updateList = new List<IBlobProvider>();
            bool result = false;
            var stream = new MemoryStream();

            foreach (var provider in BlobProviders)
            {
                if (provider.BlobExists(key))
                {
                    provider.GetBlob(key, stream);
                    result = true;
                    break;
                }
                else
                {
                    updateList.Add(provider);
                }
            }

            if (result)
            {
                stream.Position = 0;
                stream.CopyTo(data);

                //Add to faster providers (cache)
                Task.Factory.StartNew(() =>
                {
                    foreach (var provider in updateList)
                    {
                        stream.Position = 0;
                        provider.AddBlob(key, stream);
                    }
                    stream.Dispose();
                }
                );
            }
            return result;
        }

        public override bool AddBlob(string key, Stream data)
        {
            bool result = true;
            using (var stream = new MemoryStream())
            {
                data.CopyTo(stream);
                foreach (var provider in BlobProviders)
                {
                    stream.Position = 0;
                    result &= provider.AddBlob(key, stream);
                }
            }
            return result;
        }

        public override bool RemoveBlob(string key)
        {
            bool result = true;
            foreach (var provider in BlobProviders)
            {
                result &= provider.RemoveBlob(key);
            }
            return result;
        }

        public override bool BlobExists(string key)
        {
            bool result = true;
            foreach (var provider in BlobProviders)
            {
                result &= provider.BlobExists(key);
            }
            return result;
        }
    }
}