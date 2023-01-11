#region Licence...
//-----------------------------------------------------------------------------
// Date:	10/11/04	Time: 3:00p
// Module:	CSScriptLib.cs
// Classes:	CSScript
//			AppInfo
//
// This module contains the definition of the CSScript class. Which implements 
// compiling C# script engine (CSExecutor). Can be used for hosting C# script engine
// from any CLR application
//
// Written by Oleg Shilo (oshilo@gmail.com)
// Copyright (c) 2004. All rights reserved.
//
// Redistribution and use of this code in source and binary forms,  without 
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, 
//	this list of conditions and the following disclaimer. 
// 2. Neither the name of an author nor the names of the contributors may be used 
//	to endorse or promote products derived from this software without specific 
//	prior written permission.
// 3. This code may be used in compiled form in any way you desire. This
//	  file may be redistributed unmodified by any means PROVIDING it is 
//	not sold for profit without the authors written consent, and 
//	providing that this notice and the authors name is included. 
//
// Redistribution and use of this code in source and binary forms, with modification, 
// are permitted provided that all above conditions are met and software is not used 
// or sold for profit.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//	Caution: Bugs are expected!
//----------------------------------------------
#endregion

using System;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using System.Diagnostics;
using System.Xml;
using System.Collections;
#if !net1
using System.Collections.Generic;
#endif
using System.Configuration;
using System.Collections.Specialized;
using csscript;

namespace CSScriptLibrary
{
    /// <summary>
    /// Delegate to handle output from script
    /// </summary>
    public delegate void PrintDelegate(string msg);

    /// <summary>
    /// Class which is implements CS-Script class library interface.
    /// </summary>
    public class CSScript
    {
        static string dummy = "";
        /// <summary>
        /// Default constructor
        /// </summary>
        public CSScript()
        {
            rethrow = false;
        }
        /// <summary>
        /// Force caught exceptions to be rethrown.
        /// </summary>
        static public bool Rethrow
        {
            get { return rethrow; }
            set { rethrow = value; }
        }
        /// <summary>
        /// Enables automatic resolving of unsuccessfull assembly probing on the base of the Settings.SearchDirs.
        /// Default value is true.
        /// 
        /// CLR does assembly probing only in GAC and in the local (with respect to the application) directories. CS-Script
        /// however allows you to specify extra directory(es) for assembly probing by sertting enabeling CS-Script asssmbly resolving 
        /// through setting the AssemblyResolvingEnabled to true and changing the Settings.SearchDirs appropriately.
        /// </summary>
        static public bool AssemblyResolvingEnabled
        {
            get { return assemblyResolvingEnabled; }
            set
            {
                if (value)
                {
                    AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler(OnAssemblyResolve);
                    callingResolveEnabledAssembly = Assembly.GetCallingAssembly();
                }
                else
                    AppDomain.CurrentDomain.AssemblyResolve -= new ResolveEventHandler(OnAssemblyResolve);

                assemblyResolvingEnabled = value;
            }
        }
        static bool assemblyResolvingEnabled = true;
        /// <summary>
        /// Gets or sets the assembly sharing mode. If set to true all assemblies (including the host assembly itself) 
        /// currently loaded to the host application AppDomain are automatically available/accessible from the script code.
        /// Default value is true.
        /// 
        /// Sharing the same assembly set between the host application and the script require AssemblyResolvingEnabled to 
        /// be enabled. Whenever SharesHostRefAssemblies is changed to true it automatically sets AssemblyResolvingEnabled to
        /// true as well. 
        /// </summary>
        static public bool ShareHostRefAssemblies
        {
            get { return shareHostRefAssemblies; }
            set
            {
                if (shareHostRefAssemblies != value)
                {
                    shareHostRefAssemblies = value;
                    if (shareHostRefAssemblies)
                        AssemblyResolvingEnabled = true;
                }
            }

        }
        static private bool shareHostRefAssemblies = true;
        static Assembly callingResolveEnabledAssembly;
        static Assembly OnAssemblyResolve(object sender, ResolveEventArgs args)
        {
            Assembly retval = null;
            if (args.Name == "GetExecutingAssembly()")
                retval = callingResolveEnabledAssembly;//Assembly.GetExecutingAssembly();
            else if (args.Name == "GetEntryAssembly()")
                retval = Assembly.GetEntryAssembly();
            else
            {
                CSExecutor.ExecuteOptions options = InitExecuteOptions(new CSExecutor.ExecuteOptions(), CSScript.GlobalSettings, null, ref dummy);

                foreach (string dir in options.searchDirs)
                {
                    if ((retval = AssemblyResolver.ResolveAssembly(args.Name, dir)) != null)
                        break;
                }
            }
            return retval;
        }
        /// <summary>
        /// Settings object containing runtime settings, which controls script compilation/execution.
        /// Ths is Settings class essentially is a deserialized content of the CS-Script configuration file (css_config.xml). 
        /// </summary>
        public static Settings GlobalSettings = Settings.Load(Environment.ExpandEnvironmentVariables(@"%CSSCRIPT_DIR%\css_config.xml"));
        /// <summary>
        /// Invokes global (static) CSExecutor (C# script engine)
        /// </summary>
        /// <param name="print">Print delegate to be used (if not null) to handle script engine output (eg. compilation errors).</param>
        /// <param name="args">Script arguments.</param>
        static public void Execute(CSScriptLibrary.PrintDelegate print, string[] args)
        {
            lock (CSExecutor.options)
            {
                CSExecutor.ExecuteOptions oldOptions = CSExecutor.options;
                try
                {
                    csscript.AppInfo.appName = new FileInfo(Application.ExecutablePath).Name;
                    csscript.CSExecutor exec = new csscript.CSExecutor();
                    exec.Rethrow = Rethrow;

                    InitExecuteOptions(CSExecutor.options, CSScript.GlobalSettings, null, ref dummy);

                    exec.Execute(args, new csscript.PrintDelegate(print != null ? print : new CSScriptLibrary.PrintDelegate(DefaultPrint)), null);
                }
                finally
                {
                    CSExecutor.options = oldOptions;
                }
            }
        }
        /// <summary>
        /// Invokes CSExecutor (C# script engine)
        /// </summary>
        /// <param name="print">Print delegate to be used (if not null) to handle script engine output (eg. compilation errors).</param>
        /// <param name="args">Script arguments.</param>
        /// <param name="rethrow">Flag, which indicated if script exceptions should be rethrowed by the script engine without any handling.</param>
        public void Execute(CSScriptLibrary.PrintDelegate print, string[] args, bool rethrow)
        {
            lock (CSExecutor.options)
            {
                CSExecutor.ExecuteOptions oldOptions = CSExecutor.options;
                try
                {
                    AppInfo.appName = new FileInfo(Application.ExecutablePath).Name;
                    CSExecutor exec = new CSExecutor();
                    exec.Rethrow = rethrow;

                    InitExecuteOptions(CSExecutor.options, CSScript.GlobalSettings, null, ref dummy);

                    exec.Execute(args, new csscript.PrintDelegate(print != null ? print : new CSScriptLibrary.PrintDelegate(DefaultPrint)), null);
                }
                finally
                {
                    CSExecutor.options = oldOptions;
                }
            }
        }
        /// <summary>
        /// Compiles script code into assembly with CSExecutor
        /// </summary>
        /// <param name="scriptText">The script code to be compiled.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string CompileCode(string scriptText, params string[] refAssemblies)
        {
            return CompileCode(scriptText, null, false, refAssemblies);
        }
        /// <summary>
        /// Compiles script code into assembly with CSExecutor
        /// </summary>
        /// <param name="scriptText">The script code to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string CompileCode(string scriptText, string assemblyFile, bool debugBuild, params string[] refAssemblies)
        {
            string tempFile = Path.GetTempFileName();
            try
            {
                using (StreamWriter sw = new StreamWriter(tempFile))
                {
                    sw.Write(scriptText);
                }
                return Compile(tempFile, assemblyFile, debugBuild, refAssemblies);
            }
            finally
            {
                File.Delete(tempFile);
            }
        }
        /// <summary>
        /// Compiles script file into assembly with CSExecutor
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string Compile(string scriptFile, string assemblyFile, bool debugBuild, params string[] refAssemblies)
        {
            return CompileWithConfig(scriptFile, assemblyFile, debugBuild, CSScript.GlobalSettings, null, refAssemblies);
        }
        /// <summary>
        /// Compiles script file into assembly (temporary file) with CSExecutor.
        /// This method is an equivalent of the CSScript.Compile(scriptFile, null, false);
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string Compile(string scriptFile, params string[] refAssemblies)
        {
            return Compile(scriptFile, null, false, refAssemblies);
        }

        /// <summary>
        /// Compiles script file into assembly with CSExecutor. Uses specified config file to load script engine settings.
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="cssConfigFile">The name of CS-Script configuration file. If null the default config file will be used (appDir/css_config.xml).</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string CompileWithConfig(string scriptFile, string assemblyFile, bool debugBuild, string cssConfigFile)
        {
            return CompileWithConfig(scriptFile, assemblyFile, debugBuild, cssConfigFile, null, null);
        }

        /// <summary>
        /// Compiles script file into assembly with CSExecutor. Uses specified config file to load script engine settings and compiler specific options.
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="cssConfigFile">The name of CS-Script configuration file. If null the default config file will be used (appDir/css_config.xml).</param>
        /// <param name="compilerOptions">The string value to be passed directly to the language compiler. Not used if set to null.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string CompileWithConfig(string scriptFile, string assemblyFile, bool debugBuild, string cssConfigFile, string compilerOptions, params string[] refAssemblies)
        {
            Settings settings = Settings.Load(cssConfigFile != null ? cssConfigFile : Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "css_config.xml"));
            if (settings == null)
                throw new ApplicationException("The configuration file \"" + cssConfigFile + "\" cannot be loaded");

            return CompileWithConfig(scriptFile, assemblyFile, debugBuild, settings, compilerOptions, refAssemblies);
        }
        /// <summary>
        /// Compiles script file into assembly with CSExecutor. Uses script engine settings object and compiler specific options.
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="scriptSettings">The script engine Settings object.</param>
        /// <param name="compilerOptions">The string value to be passed directly to the language compiler. Not used if set to null. </param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly file name.</returns>
        static public string CompileWithConfig(string scriptFile, string assemblyFile, bool debugBuild, Settings scriptSettings, string compilerOptions, params string[] refAssemblies)
        {
            lock (CSExecutor.options)
            {
                CSExecutor.ExecuteOptions oldOptions = CSExecutor.options;
                try
                {
                    CSExecutor exec = new csscript.CSExecutor();
                    exec.Rethrow = true;

                    InitExecuteOptions(CSExecutor.options, scriptSettings, compilerOptions, ref scriptFile);
                    CSExecutor.options.DBG = debugBuild;
                    if (refAssemblies != null && refAssemblies.Length != 0)
                    {
                        string dir;
                        foreach (string file in refAssemblies)
                        {
                            dir = Path.GetDirectoryName(file);
                            CSExecutor.options.AddSearchDir(dir); //settings used by Compiler
                            CSScript.GlobalSettings.AddSearchDir(dir); //settings used by AsmHelper
                        }
                        CSExecutor.options.refAssemblies = refAssemblies;
                    }

                    if (CacheEnabled)
                    {
                        if (assemblyFile != null)
                        {
                            if (!ScriptAsmOutOfDate(scriptFile, assemblyFile))
                                return assemblyFile;
                        }
                    }

                    return exec.Compile(scriptFile, assemblyFile, debugBuild);
                }
                finally
                {
                    CSExecutor.options = oldOptions;
                }
            }
        }
        static CSExecutor.ExecuteOptions InitExecuteOptions(CSExecutor.ExecuteOptions options, Settings scriptSettings, string compilerOptions, ref string scriptFile)
        {
            Settings settings = (scriptSettings == null ? CSScript.GlobalSettings : scriptSettings);

            options.altCompiler = settings.ExpandUseAlternativeCompiler();
            options.compilerOptions = compilerOptions != null ? compilerOptions : "";
            options.apartmentState = settings.DefaultApartmentState;
            options.reportDetailedErrorInfo = settings.ReportDetailedErrorInfo;
            options.cleanupShellCommand = settings.CleanupShellCommand;
            options.doCleanupAfterNumberOfRuns = settings.DoCleanupAfterNumberOfRuns;
            options.useCompiled = CSScript.CacheEnabled;

            ArrayList dirs = new ArrayList();

            options.shareHostRefAssemblies = ShareHostRefAssemblies;
            if (options.shareHostRefAssemblies)
            {
                foreach (Assembly asm in AppDomain.CurrentDomain.GetAssemblies())
                    try
                    {
                        if (asm.IsDynamic)
                            continue;
                        if (!File.Exists(asm.Location))
                            continue;

                        dirs.Add(Path.GetDirectoryName(asm.Location));
                    }
                    catch (NotSupportedException)
                    {
                        //under ASP.NET some assemblies do not have location (e.g. dynamically built/emitted assemblies)
                    }
            }

            string libDir = Environment.ExpandEnvironmentVariables(@"%CSSCRIPT_DIR%\lib");
            if (!libDir.StartsWith("%"))
                dirs.Add(libDir);

            if (settings != null)
                dirs.AddRange(Environment.ExpandEnvironmentVariables(settings.SearchDirs).Split(",;".ToCharArray()));

            if (scriptFile != "")
            {
                scriptFile = FileParser.ResolveFile(scriptFile, (string[])dirs.ToArray(typeof(string))); //to handle the case when the script fiole is specified by file name only
                dirs.Add(Path.GetDirectoryName(scriptFile));
            }

            options.searchDirs = RemovePathDuplicates((string[])dirs.ToArray(typeof(string)));

            options.scriptFileName = scriptFile;

            return options;
        }
        /// <summary>
        /// Surrounds the method implementation code into a class and compiles it code into assembly with CSExecutor and loads it in current AppDomain.
        /// The most convenient way of using dynamic methods is to declare them as static methods. In this case they can be invoked with wild card character as a class name (e.g. asmHelper.Invoke("*.SayHello")). Otherwise you will need to instantiate class "DyamicClass.Script" in order to call dynamic method.
        /// 
        /// You can have multiple methods implementations in the single methodCode. Also you can specify namespaces at the begining of the code:
        /// 
        /// CSScript.LoadMethod(
        ///     @"using System.Windows.Forms;
        /// 
        ///     public static void SayHello(string gritting)
        ///     {
        ///         MessageBoxSayHello(gritting);
        ///         ConsoleSayHello(gritting);
        ///     }
        ///     public static void MessageBoxSayHello(string gritting)
        ///     {
        ///         MessageBox.Show(gritting);
        ///     }
        ///     public static void ConsoleSayHello(string gritting)
        ///     {
        ///         Console.WriteLine(gritting);
        ///     }");
        /// </summary>
        /// <param name="methodCode">The C# code, containing method implementation.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly.</returns>
        static public Assembly LoadMethod(string methodCode, params string[] refAssemblies)
        {
            return LoadMethod(methodCode, null, false, refAssemblies);
        }
        /// <summary>
        /// Surrounds the method implementation code into a class and compiles it code into 
        /// assembly with CSExecutor and loads it in current AppDomain. The most convenient way of 
        /// using dynamic methods is to declare them as static methods. In this case they can be 
        /// invoked with wild card character as a class name (e.g. asmHelper.Invoke("*.SayHello")). 
        /// Otherwise you will need to instantiate class "DyamicClass.Script" in order to call dynamic method.
        /// 
        /// 
        /// You can have multiple methods implementations in the single methodCode. Also you can specify namespaces at the begining of the code:
        /// 
        /// CSScript.LoadMethod(
        ///     @"using System.Windows.Forms;
        /// 
        ///     public static void SayHello(string gritting)
        ///     {
        ///         MessageBoxSayHello(gritting);
        ///         ConsoleSayHello(gritting);
        ///     }
        ///     public static void MessageBoxSayHello(string gritting)
        ///     {
        ///         MessageBox.Show(gritting);
        ///     }
        ///     public static void ConsoleSayHello(string gritting)
        ///     {
        ///         Console.WriteLine(gritting);
        ///     }");
        /// </summary>
        /// <param name="methodCode">The C# code, containing method implementation.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly.</returns>
        static public Assembly LoadMethod(string methodCode, string assemblyFile, bool debugBuild, params string[] refAssemblies)
        {
            StringBuilder code = new StringBuilder(4096);
            code.Append("//Auto-generated file\r\n"); //cannot use AppendLine as it is not available in StringBuilder v1.1
            code.Append("using System;\r\n");

            bool headerProcessed = false;
            string line;
            using (StringReader sr = new StringReader(methodCode))
                while ((line = sr.ReadLine()) != null)
                {
                    if (!headerProcessed && !line.TrimStart().StartsWith("using ")) //not using...; statement of the file header
                        if (!line.StartsWith("//") && line.Trim() != "") //not comments or empty line
                        {
                            headerProcessed = true;
                            code.Append("namespace Scripting\r\n");
                            code.Append("{\r\n");
                            code.Append("   class DynamicClass\r\n");
                            code.Append("   {\r\n");
                        }

                    code.Append(line);
                    code.Append("\r\n");
                }

            code.Append("   }\r\n");
            code.Append("}\r\n");

            return LoadCode(code.ToString(), assemblyFile, debugBuild, refAssemblies);
        }
        /// <summary>
        /// Compiles script code into assembly with CSExecutor and loads it in current AppDomain.
        /// </summary>
        /// <param name="scriptText">The script code to be compiled.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly.</returns>
        static public Assembly LoadCode(string scriptText, params string[] refAssemblies)
        {
            return LoadCode(scriptText, null, false, refAssemblies);
        }
        /// <summary>
        /// Compiles script code into assembly with CSExecutor and loads it in current AppDomain.
        /// </summary>
        /// <param name="scriptText">The script code to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled assembly.</returns>
        static public Assembly LoadCode(string scriptText, string assemblyFile, bool debugBuild, params string[] refAssemblies)
        {
            string tempFile = CSExecutor.GetScriptTempFile();
            try
            {
                using (StreamWriter sw = new StreamWriter(tempFile))
                {
                    sw.Write(scriptText);
                }
                return Load(tempFile, assemblyFile, debugBuild, refAssemblies);
            }
            finally
            {
                if (!debugBuild)
                    File.Delete(tempFile);
                else
                {
                    if (tempFiles == null)
                    {
                        tempFiles = new ArrayList();
                        //Note: ApplicationExit will not be called if this library is hosted by a console application.
                        //Thus CS-Script periodical cleanup will take care of the temop files
                        Application.ApplicationExit += new EventHandler(OnApplicationExit);
                    }
                    tempFiles.Add(tempFile);
                }
            }
        }
        static ArrayList tempFiles;
        static void OnApplicationExit(object sender, EventArgs e)
        {
            if (tempFiles != null)
                foreach (string file in tempFiles)
                    try
                    {
                        File.Delete(file);
                    }
                    catch { }
        }
        /// <summary>
        /// Compiles script file into assembly with CSExecutor and loads it in current AppDomain
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="assemblyFile">The name of compiled assembly. If set to null a temporary file name will be used.</param>
        /// <param name="debugBuild">'true' if debug information should be included in assembly; otherwise, 'false'.</param>
        /// /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled/Loaded assembly.</returns>
        static public Assembly Load(string scriptFile, string assemblyFile, bool debugBuild, params string[] refAssemblies)
        {
            CSExecutor.ExecuteOptions oldOptions = CSExecutor.options;
            try
            {
                CSExecutor exec = new CSExecutor();
                exec.Rethrow = true;

                InitExecuteOptions(CSExecutor.options, CSScript.GlobalSettings, "", ref scriptFile);
                CSExecutor.options.DBG = debugBuild;
                if (refAssemblies != null && refAssemblies.Length != 0)
                {
                    string dir;
                    foreach (string file in refAssemblies)
                    {
                        dir = Path.GetDirectoryName(file);
                        CSExecutor.options.AddSearchDir(dir); //settings used by Compiler
                        CSScript.GlobalSettings.AddSearchDir(dir); //settings used by AsmHelper
                    }
                    CSExecutor.options.refAssemblies = refAssemblies;
                }

                Assembly retval = null;

                if (CacheEnabled)
                    retval = GetCachedScriptAssembly(scriptFile);


                if (retval == null)
                {
                    string outputFile = exec.Compile(scriptFile, assemblyFile, debugBuild);

                    if (!CSExecutor.ExecuteOptions.options.inMemoryAsm)
                    {
                        retval = Assembly.LoadFrom(outputFile);
                    }
                    else
                    {
                        //Load(byte[]) does not lock the assembly file as LoadFrom(filename) does
                        using (FileStream fs = new FileStream(outputFile, FileMode.Open))
                        {
                            byte[] data = new byte[fs.Length];
                            fs.Read(data, 0, data.Length);
                            string dbg = Path.ChangeExtension(outputFile, ".pdb");
                            if (File.Exists(dbg))
                            {
                                using (FileStream fsDbg = new FileStream(dbg, FileMode.Open))
                                {
                                    byte[] dbgData = new byte[fsDbg.Length];
                                    fsDbg.Read(dbgData, 0, dbgData.Length);
                                    retval = Assembly.Load(data, dbgData);
                                }
                            }
                            else
                                retval = Assembly.Load(data);
                        }
                    }

                    if (retval != null)
                        scriptCache.Add(new LoadedScript(scriptFile, retval));
                }
                return retval;
            }
            finally
            {
                CSExecutor.options = oldOptions;
            }
        }
        /// <summary>
        /// Compiles script file into assembly (temporary file) with CSExecutor and loads it in current AppDomain.
        /// This method is an equivalent of the CSScript.Load(scriptFile, null, false);
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <returns>Compiled/Loaded assembly.</returns>
        static public Assembly Load(string scriptFile)
        {
            return Load(scriptFile, null, false, null);
        }
        /// <summary>
        /// Compiles script file into assembly (temporary file) with CSExecutor and loads it in current AppDomain.
        /// This method is an equivalent of the CSScript.Load(scriptFile, null, false);
        /// </summary>
        /// <param name="scriptFile">The name of script file to be compiled.</param>
        /// <param name="refAssemblies">The string array containing fila nemes to the additional assemblies referenced by the script. Not used if set to null.</param>
        /// <returns>Compiled/Loaded assembly.</returns>
        static public Assembly Load(string scriptFile, params string[] refAssemblies)
        {
            return Load(scriptFile, null, false, refAssemblies);
        }
        /// <summary>
        /// Default implementation of displaying application messages.
        /// </summary>
        static void DefaultPrint(string msg)
        {
            //do nothing
        }
        static bool rethrow;

        /// <summary>
        /// LoadedScript is a class, which holds information about the script file location and it's compiled and loaded assmbly (current AppDomain). 
        /// </summary>
        public class LoadedScript
        {
            /// <summary>
            /// Creates instance of LoadedScript
            /// </summary>
            /// <param name="script">Script file location.</param>
            /// <param name="asm">Compiled script assembly loaded into current AppDomain.</param>
            public LoadedScript(string script, Assembly asm)
            {
                this.script = Path.GetFullPath(script);
                this.asm = asm;
            }

            /// <summary>
            /// Script file location.
            /// </summary>
            public string script;
            /// <summary>
            /// Compiled script assembly loaded into current AppDomain.
            /// </summary>
            public Assembly asm;
        }
        /// <summary>
        /// Controls if ScriptCache should be used when script file loading is requested (CSScript.Load(...)). If set to true and the script file was previously compiled and already loaded 
        /// the script engine will use that compiled script from the cache instead of compiling it again. 
        /// Note the script cache is always maintained by the script engine. The CacheEnabled property only indicates if the cached script should be used or not when CSScript.Load(...) method is called.
        /// </summary>
        public static bool CacheEnabled = true;
        /// <summary>
        /// Cache of all loaded script files for the current process.
        /// </summary>
        public static LoadedScript[] ScriptCache
        {
            get
            {
#if net1
                return (LoadedScript[])scriptCache.ToArray(typeof(LoadedScript));
#else
                return scriptCache.ToArray();
#endif
            }
        }
#if net1
        static ArrayList scriptCache = new ArrayList();
#else
        static List<LoadedScript> scriptCache = new List<LoadedScript>();
#endif
        /// <summary>
        /// Returns cached script assembly matching the scrpt file name. 
        /// </summary>
        /// <param name="file">Pull path of the script file.</param>
        /// <returns>Assembly loaded int the current AppDomain.
        /// Returns null if the loaded script cannot be found.
        /// </returns>
        public static Assembly GetCachedScriptAssembly(string file)
        {
            string path = Path.GetFullPath(file);
            foreach (LoadedScript item in ScriptCache)
                if (item.script == path && !ScriptAsmOutOfDate(path, item.asm.Location))
                {
                    return item.asm;
                }
            return null;
        }

        internal static bool ScriptAsmOutOfDate(string scriptFileName, string assemblyFileName)
        {
            if (File.GetLastWriteTimeUtc(scriptFileName) != File.GetLastWriteTimeUtc(assemblyFileName))
                return true;

            return MetaDataItems.IsOutOfDate(scriptFileName, assemblyFileName);
        }
        internal static string[] RemovePathDuplicates(string[] list)
        {
            lock (typeof(CSScript))
            {
                ArrayList retval = new ArrayList();
                foreach (string item in list)
                {
                    if (item.Trim() == "")
                    {
                        retval.Add(item);
                    }
                    else
                    {
                        string path = Path.GetFullPath(item).ToLower();
                        if (!retval.Contains(path))
                            retval.Add(path);
                    }
                }

                return (string[])retval.ToArray(typeof(string));
            }
        }
    }
}

namespace csscript
{
    /// <summary>
    /// This class implements access to the CS-Script global configuration settings.
    /// </summary>
    public class CSSEnvironment
    {
        /// <summary>
        /// The directory where CS-Script engine keeps autogenerated files (subject of HideAutoGeneratedFiles setting).
        /// </summary>
        public static string CacheDirectory
        {
            get
            {
                if (ScriptFile != null) //we are running the script engine
                {
                    //Console.WriteLine("<"+Path.GetFullPath(scriptFile).ToLower());
                    return Path.Combine(CSExecutor.GetScriptTempDir(), @"Cache\" + Path.GetFullPath(ScriptFile).ToLower().GetHashCode().ToString());
                }
                else
                    return null;
            }
        }
        /// <summary>
        /// Generates the name of the cache directory for the specified script file.
        /// </summary>
        /// <param name="file">Script file name.</param>
        /// <returns>Cache directory name.</returns>
        public static string GetCacheDirectory(string file)
        {
            return Path.Combine(CSExecutor.GetScriptTempDir(), @"Cache\" + Path.GetDirectoryName(Path.GetFullPath(file)).ToLower().GetHashCode().ToString());
        }
        /// <summary>
        /// The full name of the script file being executed.  
        /// </summary>
        public static string ScriptFile
        {
            get
            {
                scriptFile = FindExecuteOptionsField(Assembly.GetExecutingAssembly(), "scriptFileName");
                if (scriptFile == null)
                    scriptFile = FindExecuteOptionsField(Assembly.GetEntryAssembly(), "scriptFileName");
                return scriptFile;
            }
        }
        static private string scriptFile = null;

        /// <summary>
        /// The full name of the primary script file being executed. Usually it is the sam file as ScriptFile. 
        /// However these fields are different if analysed analised from the pre/post-script.
        /// </summary>
        public static string PrimaryScriptFile
        {
            get
            {
                if (scriptFileNamePrimary == null)
                {
                    scriptFileNamePrimary = FindExecuteOptionsField(Assembly.GetExecutingAssembly(), "scriptFileNamePrimary");
                    if (scriptFileNamePrimary == null || scriptFileNamePrimary == "")
                        scriptFileNamePrimary = FindExecuteOptionsField(Assembly.GetEntryAssembly(), "scriptFileNamePrimary");
                }
                return scriptFileNamePrimary;
            }
        }
        static private string scriptFileNamePrimary = null;

        static private string FindExecuteOptionsField(Assembly asm, string field)
        {
            Type t = asm.GetModules()[0].GetType("csscript.CSExecutor+ExecuteOptions");
            if (t != null)
            {
                foreach (FieldInfo fi in t.GetFields(BindingFlags.Static | BindingFlags.Public))
                {
                    if (fi.Name == "options")
                    {
                        //need to use reflection as we might be running either cscs.exe or the script host application
                        //thus there is no warranty which assembly contains correct "options" object
                        object otionsObject = fi.GetValue(null);
                        if (otionsObject != null)
                        {
                            object scriptFileObject = otionsObject.GetType().GetField(field).GetValue(otionsObject);
                            if (scriptFileObject != null)
                                return scriptFileObject.ToString();
                        }
                        break;
                    }
                }
            }
            return null;
        }
        private CSSEnvironment()
        {
        }
    }
    delegate void PrintDelegate(string msg);
    /// <summary>
    /// Repository for application specific data
    /// </summary>
    class AppInfo
    {
        public static string appName = "CSScriptLibrary";
        public static bool appConsole = false;
        public static string appLogo
        {
            get { return "C# Script execution engine. Version " + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + ".\nCopyright (C) 2004 Oleg Shilo.\n"; }
        }
        public static string appLogoShort
        {
            get { return "C# Script execution engine. Version " + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + ".\n"; }
        }
        //#pragma warning disable 414
        public static string appParams = "[/nl]:";
        //#pragma warning restore 414
        public static string appParamsHelp = "nl	-	No logo mode: No banner will be shown at execution time.\n";
    }
}
