using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio
{
    /// <summary>
    /// CommandLineParser is a helper class to handle DDE and command line arguments
    /// 
    /// In order to allow processing a command only after the user logged in, this class implements 2 interfaces to handle commands:
    /// 1) RegisterHandler(argument,action) - this is the first tier of handling command lines. 
    ///      It allows the application to handle specific arguments (e.g. --close).
    ///      the handler action would be invoked with an argument if exists (e.g. --startup:http://www.globalgridsystems.com/Studio/theme).
    ///      the arg will be removed the from the command line - so later actions wont process it twice.
    /// 2) OnCommand event - this is the second tier of handling command lines.
    ///      this event would be trigger if the command is not empty (after the first tier).
    /// </summary>
    class CommandLineParser
    {
        public event EventHandler<CommandLineEventArgs> OnCommand;
        
        private Dictionary<string, Action<string>> Handlers { get; set; }

        public CommandLineParser()
        {
            Handlers = new Dictionary<string, Action<string>>();
        }

        /// <summary>
        /// Register a command line argument handler
        /// </summary>
        /// <param name="argument">argument to look for</param>
        /// <param name="handler">action to invoke when argument was found. the action would receive a value if exists</param>
        public void RegisterHandler(string argument, Action<string> handler)
        {
            Handlers[argument] = handler;
        }

        /// <summary>
        /// Parse a command line.
        /// </summary>
        /// <param name="commandLine">command line (command line will be parsed into arguments).</param>
        public void ParseCommand(string commandLine)
        {
            ParseCommand(CommandLineToArgs(commandLine));
        }

        /// <summary>
        /// Parse a command line.
        /// </summary>
        /// <param name="args">command line divided into arguments.</param>
        public void ParseCommand(string[] args) {
            var eventArgs = new CommandLineEventArgs() {
                Args = args
            };

            var filteredEventArgs = InvokeHandlers(eventArgs);

            if (filteredEventArgs.Args.Length > 0)
            {
                InvokeOnCommand(OnCommand, filteredEventArgs);
            }
        }

        private CommandLineEventArgs InvokeHandlers(CommandLineEventArgs eventArgs)
        {
            var extra = new List<string>();

            foreach (var arg in eventArgs.Args)
            {
                var parts = arg.Split(new[] { ':' }, 2);

                var name = parts[0].ToLower();
                var value = parts.Length > 1 ? parts[1] : "";

                if (Handlers.ContainsKey(name))
                {
                    try
                    {
                        Handlers[name](value);
                    }
                    catch (Exception e)
                    {
                        System.Diagnostics.Trace.WriteLine("failed to handle command line argument " + name + " with error: " + e.Message);
                    }
                }
                else
                {
                    extra.Add(arg);
                }
            }

            return new CommandLineEventArgs() { Args = extra.ToArray() };
        }

        private void InvokeOnCommand(EventHandler<CommandLineEventArgs> handler, CommandLineEventArgs eventArgs)
        {
            if (handler != null)
            {
                handler.Invoke(this, eventArgs);
            }
        }

        //from: http://stackoverflow.com/questions/298830/split-string-containing-command-line-parameters-into-string-in-c-sharp
        [DllImport("shell32.dll", SetLastError = true)]
        static extern IntPtr CommandLineToArgvW(
            [MarshalAs(UnmanagedType.LPWStr)] string lpCmdLine, out int pNumArgs);

        private static string[] CommandLineToArgs(string commandLine)
        {
            int argc;
            var argv = CommandLineToArgvW(commandLine, out argc);
            if (argv == IntPtr.Zero)
                throw new System.ComponentModel.Win32Exception();
            try
            {
                var args = new string[argc];
                for (var i = 0; i < args.Length; i++)
                {
                    var p = Marshal.ReadIntPtr(argv, i * IntPtr.Size);
                    args[i] = Marshal.PtrToStringUni(p);
                }

                return args;
            }
            finally
            {
                Marshal.FreeHGlobal(argv);
            }
        }
    }

    class CommandLineEventArgs : EventArgs
    {
        public string[] Args { get; set; }
    }
}
