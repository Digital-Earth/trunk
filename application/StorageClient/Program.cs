using ManyConsole;
using Microsoft.Practices.Unity;
using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using Pyxis.Storage.FileSystemStorage;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace StorageClient
{
    /// <summary>
    /// This utility provides an stand alone program to download and upload files and folders on the azure pyxis storage.
    /// The format of the arguments is CommandName -param=something 
    /// eg. UploadDirectory -dir=c:\Test
    /// </summary>
    internal class Program
    {
        private static int Main(string[] args)
        {
            var commands = GetCommands();
            return ConsoleCommandDispatcher.DispatchCommand(commands, args, Console.Out);
        }

        public static IEnumerable<ConsoleCommand> GetCommands()
        {
            var container = new UnityContainer();
            //var blobClient = new CachedBlobProvider(Properties.Settings.Default.ServerURL, Path.Combine(Directory.GetCurrentDirectory(), "Blobstorage"));
            var blobClient = new AzureBlobProvider("DefaultEndpointsProtocol=https;AccountName=pyxisstorage;AccountKey=SaDuqL2iehi3T9IQmwi5dv+cXsKb/KeOTPD1U6Y5Aj2kiAqx1VQWxwjRfUbfuPfDTW8h4BdGub6Hiibi2mEuOw==");
            container.RegisterInstance<IBlobProvider>(blobClient);
            container.RegisterType<IFileSystemStorage, FileSystemStorage>();
            
            var allCommands = System.Reflection.Assembly.GetExecutingAssembly().GetTypes().Where(t => t.BaseType == typeof(ConsoleCommand));
            foreach (var item in allCommands)
            {
                container.RegisterType(typeof(ConsoleCommand), item, item.Name);
            }
            var all = container.ResolveAll<ConsoleCommand>();
            return all;
        }
    }

    public class SetServerCommand : ConsoleCommand
    {
        public string URL;
        public SetServerCommand()
        {
            this.IsCommand("SetServer", "Download the file with the given key to the target directory");
            this.HasRequiredOption("url=", "Key of the file to download", b => URL = b);
        }

        public override int Run(string[] remainingArguments)
        {
            Properties.Settings.Default.ServerURL = URL;
            Properties.Settings.Default.Save();
            return 0;
        }
    }

    public class DownloadFileCommand : ConsoleCommand
    {
        public string Key;
        public string Path;
        [Dependency]
        public IFileSystemStorage FileClient { get; set; }

        public DownloadFileCommand()
        {
            this.IsCommand("DownloadFile", "Download the file with the given key to the target directory");
            this.HasRequiredOption("key=", "Key of the file to download", b => Key = b);
            this.HasRequiredOption("to=", "Destination folder", b => Path = b);
        }

        public override int Run(string[] remainingArguments)
        {
            FileClient.DownloadFile(Key, Path);
            return 0;
        }
    }

    public class DownloadDirectoryCommand : ConsoleCommand
    {
        public string Key;
        public string Path;
        [Dependency]
        public IFileSystemStorage FileClient { get; set; }

        public DownloadDirectoryCommand()
        {
            this.IsCommand("DownloadDirectory", "Download the directory with the given key to the target directory");
            this.HasRequiredOption("key=", "Key of the directory to download", b => Key = b);
            this.HasRequiredOption("to=", "Destination folder", b => Path = b);
        }

        public override int Run(string[] remainingArguments)
        {
            FileClient.DownloadDirectory(Key, Path);
            return 0;
        }
    }

    public class UploadFileCommand : ConsoleCommand
    {
        public string Path;
        [Dependency]
        public IFileSystemStorage FileClient { get; set; }

        public UploadFileCommand()
        {
            this.IsCommand("UploadFile", "Upload the given file");
            this.HasRequiredOption("file=", "Destination folder", b => Path = b);
        }

        public override int Run(string[] remainingArguments)
        {
            Console.WriteLine(FileClient.UploadFile(Path));
            return 0;
        }
    }

    public class UploadDirectoryCommand : ConsoleCommand
    {
        public string Path;
        [Dependency]
        public IFileSystemStorage FileClient { get; set; }

        public UploadDirectoryCommand()
        {
            this.IsCommand("UploadDirectory", "Upload the given directory");
            this.HasRequiredOption("dir=", "Destination folder", b => Path = b);
        }

        public override int Run(string[] remainingArguments)
        {
            Console.WriteLine(FileClient.UploadDirectory(Path));
            return 0;
        }
    }
}