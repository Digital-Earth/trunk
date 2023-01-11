using Microsoft.Owin.Hosting;
using System;

namespace StorageServer
{
    internal class Program
    {
        private static void Main(string[] args)
        {
            string address = Properties.Settings.Default.HostingAddress;
            if (args.Length > 0)
            {
                address = args[0];
            }

            using (WebApp.Start<FileServerOwinStartup>(address))
            {
                Console.WriteLine("FileServer started");
                Console.ReadLine();
                Console.WriteLine("Terminating FileServer");
            }
        }
    }
}