using System;
using System.Collections.Generic;

namespace Pyxis.Utilities
{

    /// <summary>
    /// A Utility class to parse Main(args).
    /// 
    /// Usage:
    /// int count = 0;
    /// string option = "";
    /// var extraArgs = ArgumentParser.Parse(new string[]{"-c","-opt=something","extra1","-count","extra2"},
    ///                     new ArgumentParser.Option('c|count',x=>count++),
    ///                     new ArgumentParser.Option('o|opt|option',x=>option=x));
    ///                     
    /// The parse function invokes the lambda expression for each arg that is equal to the name or
    /// starts with name and "=" or name and ":". Any leading "-" are stripped from the arg before
    /// checking.
    /// All other args that don't match any argument are returned as extraArgs;
    /// </summary>
    public class ArgsParser
    {
        public class Option
        {
            public string[] Names { get; private set; }
            public Action<string> Action { get; private set; }


            public Option(string name, Action<string> action)
            {
                Names = name.Split('|');
                Action = action;
            }

            public bool IsMatch(string arg)
            {
                arg = arg.TrimStart('-');

                foreach (var name in Names)
                {
                    if (arg == name || arg.StartsWith(name + "=") || arg.StartsWith(name + ":"))
                    {
                        return true;
                    }
                }
                return false;
            }

            public void Parse(string arg)
            {
                arg = arg.TrimStart('-');

                foreach (var name in Names)
                {
                    if (arg == name)
                    {
                        Action("");
                        return;
                    }
                    else if (arg.StartsWith(name + "="))
                    {
                        Action(arg.Substring((name + "=").Length));
                        return;
                    }
                    else if (arg.StartsWith(name + ":"))
                    {
                        Action(arg.Substring((name + ":").Length));
                        return;   
                    }
                }
            }
        }

        public static string[] Parse(IEnumerable<string> args, params Option[] options)
        {
            var extraArgs = new List<string>();
            foreach (var arg in args)
            {
                bool match = false;

                // check for args that look like an option
                if (arg.StartsWith("-"))
                {
                    foreach (var option in options)
                    {
                        if (option.IsMatch(arg))
                        {
                            option.Parse(arg);
                            match = true;
                        }
                    }
                }

                if (!match)
                {
                    //we didn't find a matching option - add it to extraArgs
                    extraArgs.Add(arg);
                }
            }

            return extraArgs.ToArray();
        }
    }
}
