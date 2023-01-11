using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

namespace Pyxis.Utilities.Shell
{
    public class ShellEngine
    {
        public TextReader In { get; set; }

        public TextWriter Out { get; set; }

        public TextWriter Error { get; set; }

        private bool ForegroundEnabled { get; set; }

        private ConsoleColor ForegroundColor
        {
            get
            {
                if (ForegroundEnabled)
                {
                    return Console.ForegroundColor;
                }
                return ConsoleColor.White;
            }
            set
            {
                if (ForegroundEnabled)
                {
                    Console.ForegroundColor = value;
                }
            }
        }

        private void ResetColor()
        {
            if (ForegroundEnabled)
            {
                Console.ResetColor();
            }
        }

        private Dictionary<string, ShellAction> Actions = new Dictionary<string, ShellAction>();

        public ShellEngine()
        {
            In = System.Console.In;
            Out = System.Console.Out;
            Error = System.Console.Error;
            ForegroundEnabled = true;

            AddDefaultActions();
        }

        public ShellEngine(TextReader inReader, TextWriter outWriter, TextWriter errorWriter)
        {
            In = inReader;
            Out = outWriter;
            Error = errorWriter;
            ForegroundEnabled = false;

            AddDefaultActions();
        }

        private void AddDefaultActions()
        {
            AddAction("h|help", "show help", () => ShowHelp());
            AddAction("$e", "show last exception", () => ShowLastException());
        }

        private void ShowLastException()
        {
            if (LastException == null)
            {
                Out.WriteLine("No Exception Found");
            }
            else
            {
                PrintException(LastException);
            }
        }

        private void PrintException(Exception exception)
        {
            var innerException = exception.InnerException;
            if (innerException != null)
            {
                PrintException(innerException);
            }
            Out.WriteLine("Exception Type:\n" + exception.GetType().Name);
            Out.WriteLine("Exception Message:\n" + exception.Message);
            Out.WriteLine("Stack Trace:\n" + exception.StackTrace);
        }

        private void ShowHelp()
        {
            foreach (var action in Actions)
            {
                ForegroundColor = ConsoleColor.Green;
                Out.Write(action.Key);
                ResetColor();
                if (action.Value.Arguments != null && action.Value.Arguments.Count > 0)
                {
                    ForegroundColor = ConsoleColor.Yellow;
                    Out.Write(" {0}", String.Join(" ", action.Value.Arguments));
                    ResetColor();
                }
                Out.WriteLine(" - {0}", action.Value.Description);
            }
        }

        public void ParseAndExecute()
        {
            Out.Write(">");
            var line = In.ReadLine();
            if (String.IsNullOrWhiteSpace(line))
            {
                return;
            }
            var words = line.Trim().Split(' ');
            var action = FindAction(words[0]);
            if (action != null)
            {
                try
                {
                    action.Action(words.Skip(1).ToArray());
                    LastException = null;
                }
                catch (Exception e)
                {
                    LastException = e;
                    Error.WriteLine("Error when exectuing '{0}' : {1})", line, e.Message);
                    PrintException(e);
                }
            }
        }

        public ShellAction FindAction(string name)
        {
            foreach (var action in Actions)
            {
                if (action.Key.Split('|').Any(x => x == name))
                {
                    return action.Value;
                }
            }
            return null;
        }

        public void AddAction(string name, string description, Action action)
        {
            Actions[name] = new ShellAction(description, new List<string>(), (args) => action());
        }

        public void AddAction(string name, string description, MethodInfo method)
        {
            Actions[name] = new ShellAction(description, method);
        }

        public void AddActionsFromType(Type type)
        {
            foreach (var method in type.GetMethods(System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public))
            {
                AddActionWithShellActionAttribute(null, method);
            }
        }

        public void AddActionsFromObject(object instance)
        {
            foreach (var method in instance.GetType().GetMethods(System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public))
            {
                AddActionWithShellActionAttribute(instance, method);
            }
        }

        private void AddActionWithShellActionAttribute(object instance, MethodInfo method)
        {
            var attributes = method.GetCustomAttributes(typeof(ShellActionAttribute), false);

            if (attributes.Length == 0)
            {
                return;
            }
            var attr = attributes[0] as ShellActionAttribute;
            var name = attr.Name;

            if (String.IsNullOrEmpty(name))
            {
                name = method.Name;
            }

            Actions.Add(name, new ShellAction(attr.Description, instance, method));
        }

        public Exception LastException { get; private set; }
    }
}