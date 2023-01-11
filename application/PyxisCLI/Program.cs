using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using ApplicationUtility;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.Utilities;
using PyxisCLI.Operations;
using PyxisCLI.Server.Cluster;
using PyxisCLI.Workspaces;

namespace PyxisCLI
{
    class Program
    {
        private static readonly List<IOperationMode> s_operations = new List<IOperationMode>();

        private static Dictionary<string, List<IOperationMode>> s_rootOperations = new Dictionary<string, List<IOperationMode>>();

        private static readonly string CacheFolder = ".pyx";

        static void Main(string[] args)
        {

            try
            {
                var start = DateTime.Now;
                args = ExtractConfigFromArgs(args);

                DiscoverActions();

                if (!System.IO.Directory.Exists(CacheFolder))
                {
                    System.IO.Directory.CreateDirectory(CacheFolder);
                }

                State.LocalDirPersistance.Root = Directory.GetCurrentDirectory();
                Workspaces = new LocalWorkspaces(Directory.GetCurrentDirectory());
                
                EngineConfig = EngineConfig.WorldViewDefault;
                EngineConfig.UsePyxnet = false;
                EngineConfig.CacheDirectory = Path.GetFullPath(Path.Combine(CacheFolder, "PYXCache"));
                EngineConfig.ReferenceResolvers.Add(reference => Workspaces.ResolveGeoSource(reference));
                
                RunOperation(args);

                if (s_engine != null)
                {
                    s_engine.Stop();
                    s_engine = null;
                }

                if (BeforeExit != null)
                {
                    BeforeExit.Invoke(null, null);
                }
                
                if (Program.MeasureTime)
                {
                    var duration = DateTime.Now - start;
                    Console.WriteLine("Time: {0}[sec]", duration.TotalSeconds);
                }
            }
            catch (Exception ex)
            {
                LogExceptions(ex);
            }
        }

        internal static void LogExceptions(Exception ex)
        {
            var defaultColor = Console.ForegroundColor;

            if (ex is AggregateException)
            {
                foreach (var innerEx in ((AggregateException) ex).InnerExceptions)
                {
                    LogExceptions(innerEx);
                }
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;

                Console.WriteLine("[ERROR] {0}", ex.Message);

                Console.ForegroundColor = defaultColor;

                if (Verbose)
                {
                    Console.WriteLine(ex.StackTrace);

                    if (ex.InnerException != null)
                    {
                        LogExceptions(ex.InnerException);
                    }
                }    
            }
        }

        private static string[] ExtractConfigFromArgs(string[] args)
        {
            string configPath = null;
            var cwd = Environment.CurrentDirectory;
            var verbose = false;
            var measureTime = false;

            args = ArgsParser.Parse(args, 
                new ArgsParser.Option("config", value => configPath = value),
                new ArgsParser.Option("cwd", value => cwd = value),
                new ArgsParser.Option("measure-time", value => measureTime = true),
                new ArgsParser.Option("verbose", value => verbose  = true)
                );

            // ReSharper disable once RedundantCheckBeforeAssignment
            if (cwd != Environment.CurrentDirectory)
            {
                Environment.CurrentDirectory = cwd;
            }

            if (configPath.HasContent())
            {
                PyxisCliConfig.Load(configPath);
            }

            Program.Verbose = verbose;
            Program.MeasureTime = measureTime;

            return args;
        }

        public static bool Verbose
        {
            get; private set;
        }

        public static bool MeasureTime
        {
            get;
            private set;
        }

        public static EngineConfig EngineConfig
        {
            get; set;
        }

        private static readonly object s_engineLock = new object();
        private static Engine s_engine = null;

        public static Engine Engine
        {
            get
            {
                lock (s_engineLock)
                {
                    if (s_engine == null)
                    {
                        AppServices.setConfiguration(AppServicesConfiguration.localStorageFormat,AppServicesConfiguration.localStorageFormat_files);
                        AppServices.setConfiguration(AppServicesConfiguration.importMemoryLimit, "200");

                        s_engine = Engine.Create(EngineConfig);

                        EngineCleanup.AttachToEngine(s_engine);

                        s_engine.Start();
                        s_engine.EnableAllImports();
                    }
                    return s_engine;
                }
            }
        }

        public static LocalWorkspaces Workspaces { get; set; }

        public static Cluster Cluster { get; set; }

        public static SharpRaven.RavenClient RavenClient;
        
        public static event EventHandler BeforeExit;

        private static void DiscoverActions()
        {
            var operationType = typeof(IOperationMode);
            var currentAssembly = operationType.Assembly;
            var types = currentAssembly.GetTypes().Where(t =>
                        operationType.IsAssignableFrom(t) && t.GetConstructor(Type.EmptyTypes) != null && !t.IsAbstract)
                .ToList();
            s_operations.AddRange(types.Select(t => currentAssembly.CreateInstance(t.FullName) as IOperationMode).OrderBy(x=>x.Command));

            s_rootOperations = s_operations.GroupBy(op => op.Command.Split(' ').First()).ToDictionary(x => x.Key, x => x.ToList());
        }

        public static void RunOperation(string[] args)
        {
            if (args.Length == 0)
            {
                WriteHelp();
                return;
            }

            if (!s_rootOperations.ContainsKey(args[0]))
            {
                WriteHelp();
                return;
            }

            var operations = s_rootOperations[args[0]];

            if (operations.Count == 0)
            {
                WriteHelp();
                return;
            }

            if (operations.Count > 1)
            {
                if (args.Length > 1)
                {
                    var operation = operations.Where(op => args[1] == op.Command.Split(' ').Skip(1).FirstOrDefault()).FirstOrDefault();

                    if (operation != null)
                    {
                        operation.Run(args);
                        return;
                    }
                }

                WriteCommandsHelp(operations);
                return;
            }
            else
            {
                operations[0].Run(args);
            }
            
        }

        private static void WriteCommandsHelp(IEnumerable<IOperationMode> operations)
        {

            Console.WriteLine("usage: pyx operation [args]");
            foreach (var op in operations)
            {                
                Console.WriteLine("    {0} - {1}", op.Command, op.Description);
            }
        }


        private static void WriteHelp()
        {
            WriteCommandsHelp(s_rootOperations.Select(x => x.Value[0]));
        }
    }
}
