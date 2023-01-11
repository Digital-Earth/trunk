using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Publish;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class PublishOperation : IOperationMode
    {
        public string Command
        {
            get { return "publish"; }
        }

        public string Description
        {
            get { return "publish local geo-source to a gallery"; }
        }

        public void Run(string[] args)
        {
            var username = "";
            var password = "";
            var gallery = "";
            
            args = ArgsParser.Parse(args,
                new ArgsParser.Option("u|user", (value) => username = value),
                new ArgsParser.Option("p|password", (value) => password = value),
                new ArgsParser.Option("g|gallery", (value) => gallery = value));

            if (args.Length < 2)
            {
                Console.WriteLine("usage: pyx {0} [-user=user] [-password=password] [-gallery=gallery] query", Command);
                return;
            }

            if (!username.HasContent() || !password.HasContent())
            {
                Console.WriteLine("no user and password where provided");
                return;
            }

            var channel = Program.Engine.GetChannel();
            var authChannel = channel.Authenticate(new NetworkCredential(username, password));
            var user = authChannel.AsUser();
            Program.Engine.AuthenticateAs(user);
            var galleries = user.GetGalleries();

            Gallery galleryToPublish = null;

            if (galleries.Count > 1)
            {
                if (!gallery.HasContent())
                {
                    Console.WriteLine("Please specify gallery to publish:");
                    foreach (var gal in galleries)
                    {
                        Console.WriteLine(gal.Metadata.Name);
                    }
                    return;
                }
                galleryToPublish =
                    galleries.First(
                        gal =>
                            gal.Metadata.Name.StartsWith(gallery, StringComparison.InvariantCultureIgnoreCase) ||
                            gal.Id.ToString().StartsWith(gallery, StringComparison.InvariantCultureIgnoreCase));
            }
            else
            {
                galleryToPublish = galleries[0];
            }

            foreach (var arg in args.Skip(1))
            {
                Guid guid = Guid.Parse(arg.Replace(".json", ""));

                var geoSource =
                    JsonConvert.DeserializeObject<GeoSource>(
                        System.IO.File.ReadAllText(String.Format("{0}.json", guid)));

                geoSource.Metadata.Visibility = VisibilityType.Private;
                geoSource.Metadata.Providers = new List<Provider>()
                {
                    new Provider()
                    {
                        Type = ResourceType.Gallery,
                        Id = galleryToPublish.Id,
                        Name = galleryToPublish.Metadata.Name
                    }
                };

                if (geoSource.Style == null)
                {
                    var styledGeoSource = Pyxis.IO.Styles.StyledGeoSource.Create(Program.Engine, geoSource);
                    geoSource.Style = styledGeoSource.Style;
                }

                var publishProgress = Program.Engine.BeginPublish(geoSource, geoSource.Style, new Pyxis.IO.Publish.PublishSettingProvider(), user);

                var publishedGeoSource = publishProgress.Task.Result.PublishedGeoSource;

                System.IO.File.WriteAllText(String.Format("{0}.json", publishedGeoSource.Id),
                    JsonConvert.SerializeObject(publishedGeoSource));

                Console.WriteLine("Published {0} ({1})", publishedGeoSource.Metadata.Name, publishedGeoSource.Id);
            }
        }
    }
}