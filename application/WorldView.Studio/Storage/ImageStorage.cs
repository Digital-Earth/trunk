using Awesomium.Core.Data;
using Pyxis.Utilities;
using Pyxis.WorldView.Studio.Layers.Html;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio.Storage
{
    /// <summary>
    /// Storage of images, used by screen capture API
    /// </summary>
    class ImageStorage
    {
        public string Path { get; private set; }

        public ImageStorage(string path)
        {
            Path = path;

            if (!System.IO.Directory.Exists(Path))
            {
                System.IO.Directory.CreateDirectory(Path);
            }
        }

        public string SaveImage(Bitmap bitmap)
        {
            var now = DateTime.Now;
            var index = 0;

            var name_prefix = "img." + now.ToString("yyyyMMdd.HHmm");

            while (System.IO.File.Exists(GetFileName(name_prefix + "." + index, ".png")))
            {
                index++;
            }

            return SaveImage(name_prefix + "." + index, bitmap);
        }

        public string SaveImage(string name, Bitmap bitmap)
        {
            var filename = GetFileName(name, ".png");
            bitmap.Save(filename);
            return filename;
        }

        public string ToResourceName(string name)
        {
            return name.Replace(Path + System.IO.Path.DirectorySeparatorChar, "asset://imagestorage/");
        }

        public string FromResourceName(string name)
        {
            return name.Replace("asset://imagestorage/", Path + System.IO.Path.DirectorySeparatorChar);
        }

        private string GetFileName(string name, string ext = "")
        {
            return Path + System.IO.Path.DirectorySeparatorChar + name + (ext ?? "");
        }

        internal IAssetProvider CreateWebDataProvider()
        {
            return new ImageDataSource("imagestorage", this);
        }

        private class ImageDataSource : IAssetProvider
        {            
            public string Name { get; private set; }

            public ImageStorage Storage { get; set; }

            public ImageDataSource(string name, ImageStorage storage)
            {
                Storage = storage;
                Name = name;
            }

            public Task<AssetResponse> Provide(DataSourceRequest request)
            {
                return Task<AssetResponse>.Factory.StartNew(() =>
                {
                    var path = request.Url.GetLeftPart(UriPartial.Path).Split('/').Last();

                    var queryBuilder = new UriQueryBuilder(request.Url.ToString());
                    var format = queryBuilder.Parameters["format"];

                    if (System.IO.File.Exists(Storage.GetFileName(path)))
                    {
                        if (format != null && format.ToLower() == "dataurl")
                        {
                            var bitmap = Bitmap.FromFile(Storage.GetFileName(path));
                            var buffer = System.Text.Encoding.UTF8.GetBytes(
                                            Pyxis.Contract.Converters.DataUrlConverter.ToDataUrl(bitmap));

                            return new AssetResponse()
                            {
                                Data = buffer,
                                MimeType = "text/text"
                            };
                        }
                        else
                        {
                            var buffer = System.IO.File.ReadAllBytes(Storage.GetFileName(path));
                            return new AssetResponse()
                            {
                                Data = buffer,
                                MimeType = "image/png"
                            };
                        }
                    }
                    else
                    {
                        throw new Exception("resource not found");
                    }

                });
            }
        }
    }
}
