// StartApp.cs
// Copyright (C) 2002 Matt Zyzik (www.FileScope.com)
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using System.Threading;
using System.Reflection;
using FileScopeCore;

namespace FileScope
{
    /// <summary>
    /// This is where the application begins and ends.
    /// </summary>
    public class StartApp
    {
        //instance of the main window
        public static MainDlg main;
        public static Splash splash;
        //instance of the whole assembly
        public static Assembly assembly;
        public static Settings settings = new Settings();

        [STAThread]
        public static void Main(string[] args)
        {
            assembly = Assembly.GetExecutingAssembly();

            //CEH ui settings only
            //settings = new Settings();
            //CEH core settings in Stats
            //Configuration.settings.startupPath = Application.StartupPath;
            //CEH GUIBridge
            //Configuration.guiBridge = new GUIBridge();

            Configuration.settings.startupPath = Application.StartupPath;
            Application.ThreadException += new ThreadExceptionEventHandler(AppError);
            if (args.Length > 0)
            {
                bool needToSetPaths = false;
                // look for a path argument
                foreach (string arg in args)
                {
                    if (arg.StartsWith("path:"))
                    {
                        Configuration.settings.startupPath = arg.Substring(5);
                        if (!Directory.Exists(Configuration.settings.startupPath))
                        {
                            Directory.CreateDirectory(Configuration.settings.startupPath);
                        }
                        Directory.CreateDirectory(System.IO.Path.Combine(Configuration.settings.startupPath, "Share"));
                        Directory.CreateDirectory(System.IO.Path.Combine(Configuration.settings.startupPath, "Downloads"));
                        needToSetPaths = true;
                    }
                }

                bool foundSettings = File.Exists(Utils.GetCurrentPath("settings.fscp"));

                // load the setting file.
                LoadSaveSettings.LoadSettings();
                Configuration.Initialize(new GUIBridge());

                if (needToSetPaths)
                {
                    Configuration.shareList.Clear();
                    Configuration.shareList.Add(System.IO.Path.Combine(Configuration.settings.startupPath, "Share"));
                    Configuration.UpdateShares();
                    Configuration.settings.dlDirectory = System.IO.Path.Combine(Configuration.settings.startupPath, "Downloads");
                }

                // look for arguments that we will use if we had no loaded settings.
                if (!foundSettings)
                {
                    Themes.SetColors("Default Colors");
                    settings.closeNormal = true;
                    Configuration.settings.allowUltrapeer = true;
                    Configuration.settings.ultrapeerCapable = true;
                    Configuration.settings.allowChats = true;
                    StartApp.settings.autoGnutella2 = true;
                    Configuration.State.Gnutella2Stats.ultrapeer = true;
                }

                foreach (string arg in args)
                {
                    if (arg.StartsWith("port:"))
                    {
                        Configuration.settings.port = Convert.ToInt32(arg.Substring(5));
                    }
                }
                // if we are using command line arguments then allow multiple connections
                //  from a single IP address because most likely we are testing.
                Configuration.AllowMultiConnections = true;
                StartApp.CreateMainWindow();
            }
            else
            {
                //do we have a settings file?
                if (File.Exists(Utils.GetCurrentPath("settings.fscp")))
                {
                    //splash screen first
                    splash = new Splash();
                    splash.Show();
                }
                else
                {
                    //no settings; start the wizard
                    WizardDlg wizard = new WizardDlg();
                    wizard.Show();
                }
            }

            Application.Run();
        }

        /// <summary>
        /// Error somewhere in our app.
        /// </summary>
        public static void AppError(object sender, ThreadExceptionEventArgs args)
        {
            string errmsg = args.Exception.Message + "\r\n" +
                args.Exception.Source + "\r\n" +
                args.Exception.StackTrace;

            if (Configuration.State.closing)
            {
                try
                {
                    FileStream fsClose = new FileStream(Utils.GetCurrentPath("Closing_Errors.txt"), FileMode.OpenOrCreate);
                    byte[] elError = System.Text.Encoding.ASCII.GetBytes(errmsg);
                    fsClose.Write(elError, 0, elError.Length);
                    fsClose.Close();
                }
                catch
                {
                    System.Diagnostics.Debug.WriteLine("AppError");
                }
            }

            System.Diagnostics.Debug.WriteLine(errmsg);
            MessageBox.Show(errmsg);
        }

        public static void CreateMainWindow()
        {
            main = new MainDlg();
            main.Show();
        }

        /// <summary>
        /// Exit FileScope.
        /// </summary>
        public static void ExitApp()
        {

            //let everyone know we're closing the app
            Configuration.State.closing = true;
            Configuration.StopFSWs();
            main.search.SerializeSearches();
            main.trayIcon.Visible = false;
            Listener.Abort();
            ChatManager.DisconnectAll();
            while (DownloadManager.countFinishes != 0)
                System.Threading.Thread.Sleep(50);
            DownloadManager.StopAllDownloads();
            UploadManager.DisconnectAll();
            FileScopeCore.Gnutella2.StartStop.Stop();
            HashEngine.Stop();
            while (HashEngine.IsAlive())
                System.Threading.Thread.Sleep(20);

            LoadSaveSettings.SaveSettings();

            Configuration.SaveCoreSettings();
            Configuration.SaveShares();
            Configuration.SaveHosts();
            Configuration.SaveWebCache();
            Configuration.SaveLastFileSet();
            System.Diagnostics.Debug.WriteLine("---exiting---");
            Application.Exit();
        }
    }
}
