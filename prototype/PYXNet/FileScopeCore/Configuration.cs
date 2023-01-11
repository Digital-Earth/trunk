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
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;
using System.Collections;
using System.Net;
using System.Timers;
using System.Xml.Serialization;
using System.Xml;

namespace FileScopeCore
{
    //connection type
    public enum EnumConnectionType
    {
        dialUp, cable, t1, t3
    }

    /// <summary>
    /// Class for access to the states, stats, and settings for FileScope.
    /// </summary>
    public class Configuration
    {
        // The following three members are where the real configuration 
        // is stored and then accessed through static members below.
        // This is to make using this class easier as an outside client
        // so that all configuration settings can be made with one call.
        private static ConfigurationData m_config = new ConfigurationData();
        public static ConfigurationData Config
        {
            get { return Configuration.m_config; }
            set { Configuration.m_config = value; }
        }
        private static StateData m_state = new StateData();
        public static StateData State
        {
            get { return Configuration.m_state; }
            set { Configuration.m_state = value; }
        }

        private static bool m_allowMultiConnections = false;
        public static bool AllowMultiConnections
        {
            get { return m_allowMultiConnections; }
            set { m_allowMultiConnections = value; }
        }

        public static string userAgent
        {
            get { return Config.userAgent; }
            set { Config.userAgent = value; }
        }

        public static string userAgentCode
        {
            get { return Config.userAgentCode; }
            set { Config.userAgentCode = value; }
        }

        public static IGUIBridge guiBridge
        {
            get { return Config.guiBridge; }
            set { Config.guiBridge = value; }
        }

        //public static bool ultrapeer = false;
        public static IPAddress myIPA				//our ip address
        {
            get { return Config.myIPA; }
            set { Config.myIPA = value; }
        }

        //FileScope version
        public static string version
        {
            get { return Config.version; }
            set { Config.version = value; }
        }

        //stores all of FileScope settings
        public static FileScopeCoreSettings settings = new FileScopeCoreSettings();

        //list of all shared directories
        public static ArrayList shareList
        {
            get { return Config.shareList; }
            set { Config.shareList = value; }
        }

        //file system watchers for each shared directory
        public static FileSystemWatcher[] fsws
        {
            get { return Config.fsws; }
            set { Config.fsws = value; }
        }

        //timer to check to see if all shared directories are still existent
        public static GoodTimer tmrCheckDirs
        {
            get { return Config.tmrCheckDirs; }
            set { Config.tmrCheckDirs = value; }
        }

        //carry out any 1 second chores
        public static GoodTimer tmrChores
        {
            get { return Config.tmrChores; }
            set { Config.tmrChores = value; }
        }

        //list of all shared files
        public static ArrayList fileList
        {
            get { return Config.fileList; }
            set { Config.fileList = value; }
        }

        //store the last filelist's sha1 values in this list to speed up the background hashing thread
        public static Hashtable lastFileSet
        {
            get { return Config.lastFileSet; }
            set { Config.lastFileSet = value; }
        }

        //list of all blocked chat hosts
        public static ArrayList blockedChatHosts
        {
            get { return Config.blockedChatHosts; }
            set { Config.blockedChatHosts = value; }
        }

        //gwc2 servers
        public static ArrayList gnutella2WebCache
        {
            get { return Config.gnutella2WebCache; }
            set { Config.gnutella2WebCache = value; }
        }

        /*
        * flag used to prevent copying our filelist into lastFileSet during UpdateShares
        * it's set to true because the first time we UpdateShares, we load lastFileSet
        */
        public static bool suppressCopy
        {
            get { return Config.suppressCopy; }
            set { Config.suppressCopy = value; }
        }

        /*
         * when the HashEngine doesn't finish, hashEngineAborted is set to true
         * when true, this variable tells us that all the values in lastFileSet weren't copied
         * because we'll clear them normally in CopyHashes, those hash values will have to be recalculated
         * this boolean will allow the CopyHashes function to keep those hash values and reduce overhead later on
         */
        public static bool hashEngineAborted
        {
            get { return Config.hashEngineAborted; }
            set { Config.hashEngineAborted = value; }
        }

        #region Methods
        public static void Initialize(ConfigurationData config, StateData state, FileScopeCoreSettings inSettings)
        {
            Configuration.m_config = config;
            Configuration.m_state = state;
            Configuration.settings = inSettings;
            State.Reset();
            InitializeFinal();
        }

        public static void Initialize(IGUIBridge guibridge)
        {
            Configuration.guiBridge = guibridge;
            Configuration.Initialize();
        }

        public static void Initialize()
        {
            State.Reset();
            LoadConfig();
            InitializeFinal();
        }

        public static void LoadConfig()
        {
            Configuration.LoadCoreSettings();
            Configuration.LoadBlockedChatHosts();
            Configuration.LoadHosts();
            Configuration.LoadLastFileSet();
            Configuration.LoadShares();
            Configuration.LoadWebCache();
            Configuration.UpdateShares();
        }

        public static void InitializeFinal()
        {
            //CEH - force ultrapeer
            if (settings.allowUltrapeer)
                //if(Configuration.settings.ultrapeerCapable && !Configuration.settings.firewall && Configuration.settings.allowUltrapeer)
                State.Gnutella2Stats.ultrapeer = true;

            FileType.FillExt();

            if (Configuration.settings.ipAddress.Length == 0)
            {
                try
                {

                    myIPA = Endian.GetIPAddress(Endian.BigEndianIP(settings.ipAddress), 0);
                }
                catch
                {
                    System.Diagnostics.Debug.WriteLine("couldn't set myIPA in Configuration.State.InitializeVariables");
                    myIPA = null;
                }
            }
            else
            {
                myIPA = System.Net.IPAddress.Parse(Configuration.settings.ipAddress);
            }
            // Configuration.gnutella2WebCache.Add("http://localhost/gwebcache/lynnx.asp");

            //Gnutella2.G2Data.SetupFuncTable(); //done in .Start
            tmrCheckDirs.AddEvent(new ElapsedEventHandler(tmrCheckDirs_Tick));
            tmrCheckDirs.Start();
            tmrChores.AddEvent(new ElapsedEventHandler(tmrChores_Tick));
            tmrChores.Start();
        }

        public static void LoadCoreSettings()
        {
            if (File.Exists(Utils.GetCurrentPath("coresettings.config")))
            {
                FileStream fStream = new FileStream(Utils.GetCurrentPath("coresettings.config"), FileMode.Open, FileAccess.Read);
                try
                {
                    XmlSerializer x = new XmlSerializer(typeof(FileScopeCoreSettings));
                    XmlReader reader = new XmlTextReader(fStream);
                    settings = (FileScopeCoreSettings)x.Deserialize(reader);
                    fStream.Close();
                }
                catch
                {
                    fStream.Close();
                    SaveCoreSettings(); //create a default file that can be edited at least
                    System.Diagnostics.Debug.WriteLine("LoadCoreSettings");
                    //try{File.Delete(Utils.GetCurrentPath("coresettings.fscp"));}
                    //catch{System.Diagnostics.Debug.WriteLine("Deleting coresettings.fscp");}
                    //System.Windows.Forms.Application.Exit();
                }
            }
        }

        public static void SaveCoreSettings()
        {
            XmlSerializer x = new XmlSerializer(typeof(FileScopeCoreSettings));
            TextWriter writer = new StreamWriter(Utils.GetCurrentPath("coresettings.config"));
            x.Serialize(writer, settings);
            writer.Close();
        }

        public static void LoadShares()
        {
            if (File.Exists(Utils.GetCurrentPath("shares.fscp")))
            {
                FileStream fStream = new FileStream(Utils.GetCurrentPath("shares.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    shareList = (ArrayList)crip.Deserialize(fStream);
                    fStream.Close();
                    UpdateShares();
                }
                catch
                {
                    fStream.Close();
                    System.Diagnostics.Debug.WriteLine("LoadShares");
                    try { File.Delete(Utils.GetCurrentPath("shares.fscp")); }
                    catch { System.Diagnostics.Debug.WriteLine("Deleting shares.fscp"); }
                    //System.Windows.Forms.Application.Exit();
                }
            }
        }

        public static void SaveShares()
        {
            BinaryFormatter crip = new BinaryFormatter();
            FileStream fStream = new FileStream(Utils.GetCurrentPath("shares.fscp"), FileMode.Create, FileAccess.Write);
            crip.Serialize(fStream, shareList);
            fStream.Close();
        }

        public static void LoadHosts()
        {
            if (File.Exists(Utils.GetCurrentPath("gnut2hosts.fscp")))
            {
                FileStream fStream2 = new FileStream(Utils.GetCurrentPath("gnut2hosts.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();

                    //gnutella2
                    lock (Gnutella2.HostCache.recentHubs)
                    {
                        Gnutella2.HostCache.recentHubs = (SortedList)crip.Deserialize(fStream2);
                        fStream2.Close();
                        //we've been gone probably for a while
                        foreach (Gnutella2.HubInfo hi in Gnutella2.HostCache.recentHubs.Values)
                            hi.timeKnown += 650;
                    }
                }
                catch (Exception e)
                {
                    fStream2.Close();
                    System.Diagnostics.Debug.WriteLine("LoadHosts " + e.Message);
                    System.Diagnostics.Debug.WriteLine(e.StackTrace);
                }
            }

            if (File.Exists(Utils.GetCurrentPath("gnut2cache.fscp")))
            {
                FileStream fStream3 = new FileStream(Utils.GetCurrentPath("gnut2cache.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    lock (Gnutella2.HostCache.hubCache)
                    {
                        Gnutella2.HostCache.hubCache = (Hashtable)crip.Deserialize(fStream3);
                        fStream3.Close();
                        foreach (Gnutella2.HubInfo hi in Gnutella2.HostCache.hubCache.Values)
                            hi.timeKnown += 650;
                    }
                }
                catch (Exception e)
                {
                    fStream3.Close();
                    System.Diagnostics.Debug.WriteLine("LoadHosts " + e.Message);
                    System.Diagnostics.Debug.WriteLine(e.StackTrace);
                }
            }

            if (File.Exists(Utils.GetCurrentPath("gnut2keys.fscp")))
            {
                FileStream fStream4 = new FileStream(Utils.GetCurrentPath("gnut2keys.fscp"), FileMode.Open, FileAccess.Read);

                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    lock (Gnutella2.HostCache.sentQueryKeys)
                    {
                        Gnutella2.HostCache.sentQueryKeys = (Hashtable)crip.Deserialize(fStream4);
                        fStream4.Close();
                        foreach (Gnutella2.QKeyInfo qki in Gnutella2.HostCache.sentQueryKeys.Values)
                            qki.timeKnown += 650;
                    }
                }
                catch (Exception e)
                {
                    fStream4.Close();
                    System.Diagnostics.Debug.WriteLine("LoadHosts " + e.Message);
                    System.Diagnostics.Debug.WriteLine(e.StackTrace);
                }

            }
        }

        public static void SaveHosts()
        {
            BinaryFormatter crip = new BinaryFormatter();

            //g2
            lock (Gnutella2.HostCache.recentHubs)
            {
                FileStream fStream2 = new FileStream(Utils.GetCurrentPath("gnut2hosts.fscp"), FileMode.Create, FileAccess.Write);
                crip.Serialize(fStream2, Gnutella2.HostCache.recentHubs);
                fStream2.Close();
            }
            lock (Gnutella2.HostCache.hubCache)
            {
                FileStream fStream3 = new FileStream(Utils.GetCurrentPath("gnut2cache.fscp"), FileMode.Create, FileAccess.Write);
                crip.Serialize(fStream3, Gnutella2.HostCache.hubCache);
                fStream3.Close();
            }
            lock (Gnutella2.HostCache.sentQueryKeys)
            {
                FileStream fStream4 = new FileStream(Utils.GetCurrentPath("gnut2keys.fscp"), FileMode.Create, FileAccess.Write);
                crip.Serialize(fStream4, Gnutella2.HostCache.sentQueryKeys);
                fStream4.Close();
            }

        }

        public static void LoadWebCache()
        {
            if (File.Exists(Utils.GetCurrentPath("webcache2.fscp")))
            {
                FileStream fStream2 = new FileStream(Utils.GetCurrentPath("webcache2.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    gnutella2WebCache = (ArrayList)crip.Deserialize(fStream2);
                    fStream2.Close();
                }
                catch
                {
                    fStream2.Close();
                    System.Diagnostics.Debug.WriteLine("LoadWebCache");
                    try { File.Delete(Utils.GetCurrentPath("webcache2.fscp")); }
                    catch { System.Diagnostics.Debug.WriteLine("Deleting webcache2.fscp"); }
                    HashEngine.Stop();
                    //System.Windows.Forms.Application.Exit();
                }
            }
        }

        public static void SaveWebCache()
        {
            BinaryFormatter crip = new BinaryFormatter();
            FileStream fStream2 = new FileStream(Utils.GetCurrentPath("webcache2.fscp"), FileMode.Create, FileAccess.Write);
            crip.Serialize(fStream2, gnutella2WebCache);
            fStream2.Close();
        }

        public static void LoadBlockedChatHosts()
        {
            if (File.Exists(Utils.GetCurrentPath("chatblocks.fscp")))
            {
                FileStream fStream = new FileStream(Utils.GetCurrentPath("chatblocks.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    blockedChatHosts = (ArrayList)crip.Deserialize(fStream);
                    fStream.Close();
                }
                catch
                {
                    fStream.Close();
                    System.Diagnostics.Debug.WriteLine("LoadBlockedChatHosts");
                    try { File.Delete(Utils.GetCurrentPath("chatblocks.fscp")); }
                    catch { System.Diagnostics.Debug.WriteLine("Deleting chatblocks.fscp"); }
                    HashEngine.Stop();
                }
            }
        }

        public static void SaveBlockedChatHosts()
        {
            BinaryFormatter crip = new BinaryFormatter();
            FileStream fStream = new FileStream(Utils.GetCurrentPath("chatblocks.fscp"), FileMode.Create, FileAccess.Write);
            crip.Serialize(fStream, blockedChatHosts);
            fStream.Close();
        }

        public static void LoadLastFileSet()
        {
            if (File.Exists(Utils.GetCurrentPath("hashlist.fscp")))
            {
                FileStream fStream = new FileStream(Utils.GetCurrentPath("hashlist.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    lastFileSet = (Hashtable)crip.Deserialize(fStream);
                    fStream.Close();
                }
                catch
                {
                    fStream.Close();
                    System.Diagnostics.Debug.WriteLine("LoadLastFileSet");
                    try { File.Delete(Utils.GetCurrentPath("hashlist.fscp")); }
                    catch { System.Diagnostics.Debug.WriteLine("Deleting hashlist.fscp"); }
                    HashEngine.Stop();
                    //System.Windows.Forms.Application.Exit();
                }
            }
        }

        public static void SaveLastFileSet()
        {
            //copy everything first
            CopyHashes();

            //normal procedure
            BinaryFormatter crip = new BinaryFormatter();
            FileStream fStream = new FileStream(Utils.GetCurrentPath("hashlist.fscp"), FileMode.Create, FileAccess.Write);
            crip.Serialize(fStream, lastFileSet);
            fStream.Close();
        }

        //this is used to stop running UpdateShares for an amount of time
        private static bool supUp = false;
        public static bool suppressUpdate
        {
            get
            {
                return supUp;
            }
            set
            {
                supUp = value;
                if (value)
                    StopFSWs();
                else
                    StartFSWs();
            }
        }

        /// <summary>
        /// This function should be called any time the share list is changed.
        /// It should also be called if a change in files occurs (deletion, creation, etc.).
        /// It will scan all of the shared directories and record filenames.
        /// Then it will create an appropriate query routing table.
        /// </summary>
        public static void UpdateShares()
        {
            if (suppressUpdate)
                return;

            //dispose/disable any current file system watchers
            StopFSWs();

            //stop hashing for now
            HashEngine.Stop();
            while (HashEngine.IsAlive())
                System.Threading.Thread.Sleep(20);

            CopyHashes();

            //run the rest of the routine
            Gnutella2.QueryRouteTable.PrepareTable();
            lock (fileList)
                fileList.Clear();
            State.filesShared = 0;
            State.kbShared = 0;
            if (shareList.Count > 0)
                lock (shareList)
                {
                    //loop through each shared directory
                    foreach (object item in shareList)
                    {
                        try
                        {
                            //add all of the files in the directory to fileList
                            string[] files = Directory.GetFiles(item.ToString());
                            //loop through each file
                            foreach (object file in files)
                            {
                                FileInfo inf = new FileInfo(file.ToString());
                                FileObject fileProps = new FileObject();
                                //record path and filesize
                                fileProps.location = file.ToString();
                                fileProps.lcaseFileName = Path.GetFileName(fileProps.location).ToLower();
                                fileProps.b = (uint)inf.Length;
                                fileProps.fileIndex = Configuration.State.filesShared;
                                fileProps.md4 = null;
                                fileProps.sha1bytes = null;
                                fileProps.sha1 = "";
                                //update variables
                                State.filesShared++;
                                State.kbShared += Convert.ToInt32(Math.Round((double)fileProps.b / 1024));
                                //add the class object to the ArrayList
                                lock (fileList)
                                    fileList.Add(fileProps);
                                Gnutella2.QueryRouteTable.AddWords(Path.GetFileName(fileProps.location));
                            }
                        }
                        catch
                        {
                            System.Diagnostics.Debug.WriteLine("directory doesn't exist");
                        }
                    }
                }
            Gnutella2.QueryRouteTable.SendQRT();

            //let the GUI know that we altered the shared directories / files
            guiBridge.ChangeShared();

            //start file system watchers
            StartFSWs();

            //restart what we ended before
            HashEngine.Start();

            //update the HomePage
            guiBridge.RefreshHomePageLibrary();
        }

        /// <summary>
        /// Here we're going to take a shortcut.
        /// We don't want to redundantly generate sha1 values again and waste cpu.
        /// We'll copy the hashes we already have and then HashEngine will take care of them.
        /// </summary>
        public static void CopyHashes()
        {
            if (!suppressCopy)
            {
                try
                {
                    lock (fileList)
                    {
                        if (!hashEngineAborted)
                            lastFileSet.Clear();
                        //find files that have hashes already calculated
                        foreach (FileObject fo in fileList)
                            if (fo.sha1 != "")
                            {
                                FileObject2 fo2 = new FileObject2();
                                fo2.bytes = fo.b;
                                fo2.md4 = fo.md4;
                                fo2.sha1 = fo.sha1;
                                fo2.sha1bytes = fo.sha1bytes;
                                if (hashEngineAborted)
                                {
                                    if (lastFileSet.ContainsKey(fo.location))
                                        lastFileSet[fo.location] = fo2;
                                    else
                                        lastFileSet.Add(fo.location, fo2);
                                }
                                else
                                    lastFileSet.Add(fo.location, fo2);
                            }
                    }
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine("Stats CopyHashes: " + e.Message);
                }
                if (hashEngineAborted)
                    hashEngineAborted = false;
            }
            else
                suppressCopy = false;
        }


        /// <summary>
        /// Turn on the file system watchers.
        /// </summary>
        static void StartFSWs()
        {
            if (shareList.Count > 0)
            {
                //instantiate each watcher
                fsws = new FileSystemWatcher[shareList.Count];
                //setup everything accordingly
                for (int x = 0; x < shareList.Count; x++)
                {
                    try
                    {
                        fsws[x] = new FileSystemWatcher(shareList[x].ToString());
                        fsws[x].NotifyFilter = NotifyFilters.LastAccess | NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.DirectoryName;
                        fsws[x].Created += new FileSystemEventHandler(OnChanged);
                        fsws[x].Deleted += new FileSystemEventHandler(OnChanged);
                        fsws[x].Renamed += new RenamedEventHandler(OnRenamed);
                        fsws[x].Error += new ErrorEventHandler(OnError);
                        fsws[x].EnableRaisingEvents = true;
                    }
                    catch
                    {
                        System.Diagnostics.Debug.WriteLine("Stats UpdateShares system watchers");
                    }
                }
            }
        }

        /// <summary>
        /// Turn off the file system watchers.
        /// </summary>
        public static void StopFSWs()
        {
            if (fsws != null)
                if (fsws.Length > 0)
                    foreach (FileSystemWatcher watcher in fsws)
                    {
                        try
                        {
                            watcher.EnableRaisingEvents = false;
                            watcher.Created -= new FileSystemEventHandler(OnChanged);
                            watcher.Deleted -= new FileSystemEventHandler(OnChanged);
                            watcher.Renamed -= new RenamedEventHandler(OnRenamed);
                            watcher.Error -= new ErrorEventHandler(OnError);
                            watcher.Dispose();
                        }
                        catch
                        {
                            System.Diagnostics.Debug.WriteLine("Stats UpdateShares watcher dispose");
                        }
                    }
        }

        /// <summary>
        /// A shared directory was probably deleted.
        /// </summary>
        static void OnError(object sender, ErrorEventArgs e)
        {
            UpdateShares();
        }

        /// <summary>
        /// A file/directory was added or deleted.
        /// </summary>
        static void OnChanged(object source, FileSystemEventArgs e)
        {
            UpdateShares();
        }

        /// <summary>
        /// A file/directory was renamed.
        /// </summary>
        static void OnRenamed(object source, RenamedEventArgs e)
        {
            UpdateShares();
        }

        static void tmrCheckDirs_Tick(object sender, ElapsedEventArgs e)
        {
            bool changeMade = false;
            lock (shareList)
            {
                if (shareList.Count > 0)
                    for (int x = shareList.Count - 1; x >= 0; x--)
                        if (!Directory.Exists((string)shareList[x]))
                        {
                            changeMade = true;
                            shareList.RemoveAt(x);
                        }
            }
            if (changeMade)
                UpdateShares();
        }

        static void tmrChores_Tick(object sender, ElapsedEventArgs e)
        {
            if (State.timestamp == int.MaxValue)
            {
                State.timestamp = int.MinValue;
                return;
            }
            State.timestamp++;
        }

        #endregion

    }

    /// <summary>
    /// Class for storing the states, stats, and settings for FileScope.
    /// </summary>
    public class ConfigurationData
    {
        public string userAgent = "PyxNet";
        public string userAgentCode = "PYXN";
        public IGUIBridge guiBridge = new NullGUIBridge();
        //public static bool ultrapeer = false;
        public volatile IPAddress myIPA;				//our ip address
        //FileScope version
        public string version = "0.5.0";
        //stores all of FileScope settings
        public FileScopeCoreSettings settings = new FileScopeCoreSettings();
        //list of all shared directories
        public ArrayList shareList = new ArrayList();
        //file system watchers for each shared directory
        public FileSystemWatcher[] fsws;
        //timer to check to see if all shared directories are still existent
        public GoodTimer tmrCheckDirs = new GoodTimer(5000);
        //carry out any 1 second chores
        public GoodTimer tmrChores = new GoodTimer(1000);
        //list of all shared files
        public ArrayList fileList = new ArrayList();
        //store the last filelist's sha1 values in this list to speed up the background hashing thread
        public Hashtable lastFileSet = new Hashtable();
        //list of all blocked chat hosts
        public ArrayList blockedChatHosts = new ArrayList();
        //gwc2 servers
        public ArrayList gnutella2WebCache = new ArrayList();

        /*
         * flag used to prevent copying our filelist into lastFileSet during UpdateShares
         * it's set to true because the first time we UpdateShares, we load lastFileSet
         */
        public bool suppressCopy = true;
        /*
         * when the HashEngine doesn't finish, hashEngineAborted is set to true
         * when true, this variable tells us that all the values in lastFileSet weren't copied
         * because we'll clear them normally in CopyHashes, those hash values will have to be recalculated
         * this boolean will allow the CopyHashes function to keep those hash values and reduce overhead later on
         */
        public bool hashEngineAborted = false;
    }

    public class Gnutella2StatsData
    {
        public bool enableDeflate = true;
        public volatile bool ultrapeer = false;			//are we an ultrapeer now or not?
        public volatile int lastConnectionCount2 = 0;  //stores the number of connections
        public int lastConnectionCount
        {
            get { return lastConnectionCount2; }
            set
            {
                bool sameCount = (lastConnectionCount2 == value);
                lastConnectionCount2 = value;
                if (!sameCount)
                    Configuration.guiBridge.RefreshHomePageNetworks();
            }
        }
        public volatile int numPI = 0;
        public volatile int numPO = 0;
        public volatile int numQ2 = 0;
        public volatile int numQA = 0;
        public volatile int numQH2 = 0;
        public volatile int numLNI = 0;
        public volatile int numKHL = 0;
        public volatile int numPUSH = 0;
        public volatile int numQHT = 0;
        public volatile int numQKR = 0;
        public volatile int numQKA = 0;

        public void Reset()
        {
            enableDeflate = true;
            lastConnectionCount = 0;
            numPI = 0;
            numPO = 0;
            numLNI = 0;
            numKHL = 0;
            numQ2 = 0;
            numQA = 0;
            numQH2 = 0;
            numPUSH = 0;
            numQHT = 0;
            numQKR = 0;
            numQKA = 0;
        }
    }

    /// <summary>
    /// Class for storing the  stats for FileScope.
    /// </summary>
    public class StateData
    {

        public volatile int timestamp = 50;
        public volatile int trueSpeed = -1;				//"real" speed of connection
        public volatile bool le = BitConverter.IsLittleEndian;				//little or big endian
        public volatile int kbShared = 0;				//kilobytes you're sharing
        public volatile int filesShared = 0;				//# of files you're sharing
        public volatile bool everIncoming = false;			//accepted an incoming tcp connection?
        public volatile bool udpIncoming = false;			//ever received incoming udp packets
        public volatile int uploads = 0;					//# of files you've uploaded
        public volatile int uploadsNow2 = 0;				//# of files currently uploading
        public int uploadsNow
        {
            get { return uploadsNow2; }
            set { uploadsNow2 = value; Configuration.guiBridge.RefreshHomePageTransfers(); }
        }
        public volatile int downloadsNow2 = 0;			//# of files currently downloading
        public int downloadsNow
        {
            get { return downloadsNow2; }
            set { downloadsNow2 = value; Configuration.guiBridge.RefreshHomePageTransfers(); }
        }
        public volatile int numChats = 0;				//number of active chats
        public volatile bool closing = false;				//are we closing FileScope?
        public volatile bool opened = false;					//did we fully open FileScope?
        public volatile int inNetworkBandwidth = 0;		//b/s inward network bandwidth
        public volatile int outNetworkBandwidth = 0;		//b/s outward network bandwidth
        public volatile int inTransferBandwidth = 0;		//b/s inward transfers bandwidth
        public volatile int outTransferBandwidth = 0;	//b/s outward transfers bandwidth

        public Gnutella2StatsData Gnutella2Stats = new Gnutella2StatsData();

        public void Reset()
        {
            trueSpeed = -1;
            uploads = 0;
            uploadsNow = 0;
            downloadsNow = 0;
            numChats = 0;
            closing = false;
            everIncoming = false;
            udpIncoming = false;
            timestamp = 50;
            le = BitConverter.IsLittleEndian;
            Gnutella2Stats.Reset();
        }

        /// <summary>
        /// Return the speed of this connection.
        /// </summary>
        public int GetSpeed()
        {
            if (trueSpeed != -1)
                return trueSpeed;
            else
                switch (Configuration.settings.connectionType)
                {
                    case EnumConnectionType.dialUp:
                        return 5;
                    case EnumConnectionType.cable:
                        return 100;
                    case EnumConnectionType.t1:
                        return 1000;
                    case EnumConnectionType.t3:
                        return 50000;
                    default:
                        return 100;
                }
        }
    }



    /// <summary>
    /// Class for all of the settings in FileScope.
    /// </summary>
    [Serializable]
    public class FileScopeCoreSettings
    {
        public volatile string startupPath = "";
        public volatile int maxUploads = 2;						//the maximum simultaneous uploads
        public volatile string ipAddress = "";					//public IP Address
        public volatile bool ultrapeerCapable = false;				//fulfilled gnutella ultrapeer requirements
        public volatile string dlDirectory;					//download directory
        public volatile EnumConnectionType connectionType = EnumConnectionType.cable;	//EnumConnectionType will take care of this
        public volatile byte[] myGUID = new byte[16];		//permanent GUID for this computer
        public volatile int gConnectionsToKeep = 200;				//gnutella connections to keep when ultrapeer
        public volatile bool firewall = false;						//are we behind a firewall?
        public volatile int port = 6346;							//port for incoming connections
        public volatile bool allowUltrapeer = false;				//allow ultrapeer
        public volatile bool allowChats = false;					//allow incoming chats?

        public FileScopeCoreSettings()
        {
            //CEH set some basic defaults
            myGUID = GUID.newGUID();
            System.Net.IPHostEntry ipEntry = System.Net.Dns.GetHostEntry(System.Net.Dns.GetHostName());
            ipAddress = ipEntry.AddressList[0].ToString();
            dlDirectory = "c:\\temp\\";

        }
    }


}
