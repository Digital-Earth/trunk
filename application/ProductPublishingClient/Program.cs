using System;
using System.Collections.Generic;
using System.Net;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing;
using Pyxis.Publishing.Protocol;
using Pyxis.Utilities;

namespace ProductPublishingClient
{
    /// <summary>
    /// The command line application performs publishing of Resources of the type Product to the backend.
    /// It is intended to be used after the artifact of a product was uploaded to the Pyxis storage
    /// and a storage key was received.
    /// If an error occurs it is printed to the standard output.
    /// Otherwise, the program returns code 0.
    /// 
    /// In the early version only the POST Resource command is supported
    /// 
    /// Sample usage:
    /// 
    /// ProductPublishingClient.exe -help
    /// ProductPublishingClient.exe -POST -u=<username> -p=<password> -env=Live -ProductType=WorldViewStudio -ProductVersion=0.10.0.875 -key=<storage_key> -SystemTag=Development
    /// 
    /// </summary>
    internal class Program
    {
        // Product information
        private static string s_productType;
        private static string s_productVersion;
        private static string s_productKey;
        private static string s_productSystemTag;

        // Credentials for authentication to the License Server
        private static string s_user;
        private static string s_password;

        // Backend Server information
        private static string s_serverURL;

        // Flags that define the action to perform
        private static bool s_post = false;

        private static int Main(string[] args)
        {
            // Parse the input arguments
            if (!ParseCommandLineArguments(args))
            {
                return -1;
            }

            // This version only supports POST commands (posts a Product resource)
            if (!s_post)
            {
                Console.WriteLine("The current version only supports POST");
                return -2;
            }

            // Create a product to publish
            var product = CreateProduct();
            if (null == product)
            {
                return -3;
            }

            // Create a web client that will perform publishing
            var client = CreateClient();
            if (null == client)
            {
                return -4;
            }

            // Publish the product
            try
            {
                client.PostResource<Product>(product);
            }
            catch (Exception e)
            {
                // Just print out the error message
                Console.WriteLine(e.Message);
                return -5;
            }

            return 0;
        }

        private static bool ParseCommandLineArguments(string[] args)
        {
            // If the user needs help, just print the list of arguments and exit
            bool help = false;
            var extra = ArgsParser.Parse(args,
                new ArgsParser.Option("help|h", (value) =>
                    {
                        if (help)
                        {
                            return;
                        }
                        Console.WriteLine("Usage:");
                        Console.WriteLine();
                        Console.WriteLine("-POST | -PUT       Specify the command");
                        Console.WriteLine();
                        Console.WriteLine("Specify the command arguments");
                        Console.WriteLine();
                        Console.WriteLine("-u               Username to connect to License Server");
                        Console.WriteLine("-p               Password");
                        Console.WriteLine("-env             Dev | Test | Live (backend environment)");
                        Console.WriteLine("-ProductType     WorldViewStudio | GeoWebStreamServer");
                        Console.WriteLine("-ProductVersion  Version of the product");
                        Console.WriteLine("-key             Pyxis Storage key to the artifact");
                        Console.WriteLine("-SystemTag       Production | Development");
                        Console.WriteLine();
                        Console.WriteLine("All arguments are mandatory. Use '=' to assign a value to an argument");
                        help = true;
                    }
                    )
                );
            if (help)
            {
                return false;
            }

            // Allow only the expected count of arguments
            if (8 > args.Length)
            {                
                Console.WriteLine("Some program arguments are missing. Use -help for information");
                return false;
            }

            // Backend environment argument
            string environment = "";

            // Perform parsing
            extra = ArgsParser.Parse(args,
                new ArgsParser.Option("POST", (value) => s_post = "" == value),
                new ArgsParser.Option("PUT", (value) => { }),
                new ArgsParser.Option("u", (value) => s_user = value),
                new ArgsParser.Option("p", (value) => s_password = value),
                new ArgsParser.Option("env", (value) => environment = value),
                new ArgsParser.Option("ProductType", (value) => s_productType = value),
                new ArgsParser.Option("ProductVersion", (value) => s_productVersion = value),
                new ArgsParser.Option("key", (value) => s_productKey = value),
                new ArgsParser.Option("SystemTag", (value) => s_productSystemTag = value)
                );

            if (0 == args.Length || 0 != extra.Length)
            {
                Console.WriteLine("Wrong argument list. Use -help for information");
                return false;
            }

            // Retrieve the publishing server URL
            switch (environment.ToLower())
            {
                case "dev":
                    s_serverURL = ApiUrl.DevelopmentLicenseServerRestAPI;
                    break;

                case "test":
                    s_serverURL = ApiUrl.TestLicenseServerRestAPI;
                    break;

                case "live":
                    s_serverURL = ApiUrl.ProductionLicenseServerRestAPI;
                    break;

                default:
                    Console.WriteLine(String.Format("Unknown environment: {0}", environment));
                    return false;
            }

            return true;
        }

        private static Product CreateProduct()
        {
            // Create a product object that'll be sent
            var product = new Product();

            try
            {
                // Set the product type
                switch (s_productType.ToLower())
                {
                    case "worldviewstudio":
                        product.ProductType = ProductType.WorldViewStudio;
                        break;

                    case "geowebstreamserver":
                        product.ProductType = ProductType.GeoWebStreamServer;
                        break;

                    default:
                        Console.WriteLine(String.Format(
                            "Unknown/unsupported product type: {0}", s_productType));
                        return null;
                }

                // Set the product version
                product.ProductVersion = new Version(s_productVersion);

                // Pyxis Storage URL
                product.Url = "http://storage-pyxis.azurewebsites.net";

                // Set the transfer type
                product.TransferType = TransferType.BlobClientV1;

                // Set the key corresponding to the artifact uploaded to the Pyxis Storage
                product.Key = s_productKey;

                // The system tag defines whether the product is for public or internal use
                // The other metadata fields here are just mandatory to provide
                product.Metadata = new Metadata();
                product.Metadata.Name = s_productType + "." + s_productVersion;
                product.Metadata.Description = "New build";
                product.Metadata.Visibility = VisibilityType.Public;
                product.Metadata.SystemTags = new List<string>();
                switch (s_productSystemTag.ToLower())
                {
                    case "production":
                        product.Metadata.SystemTags.Add(ProductSystemTags.Production);
                        break;

                    case "development":
                        product.Metadata.SystemTags.Add(ProductSystemTags.Development);
                        break;

                    default:
                        Console.WriteLine(String.Format(
                            "Unknown/unsupported product metadata system tag: {0}", s_productSystemTag));
                        return null;
                }
                
                return product;
            }
            catch (Exception e)
            {
                // Print out the error message (accessing null is expected)
                Console.WriteLine(e.Message);
                return null;
            }
        }
        
        private static AuthenticatedPipelineClient CreateClient()
        {
            try
            {
                // Create a communication channel
                var channel = new Channel(s_serverURL);
                // Login as admin
                var auth = channel.Authenticate(new NetworkCredential(s_user, s_password));
                // Retrieve the access token retainer and create an authenticated client with it
                return new AuthenticatedPipelineClient(s_serverURL, auth.AsUser().TokenRetainer);
            }
            catch (Exception e)
            {
                // Print out the error message
                Console.WriteLine(e.Message);
                return null;
            }
        }
    }
}
