using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace GeoWebCoreRunner
{
    public class LogParser
    {
        public class LineParser
        {
            public Func<string, bool> ParseLine { get; set; }
            public Action Trigger { get; set; }
        }

        public List<LineParser> Parsers = new List<LineParser>();


        public void Parse(string line)
        {
            if (String.IsNullOrWhiteSpace(line))
            {
                return;
            }

            foreach (var parser in Parsers)
            {
                try
                {
                    if (parser.ParseLine(line))
                    {
                        parser.Trigger();
                    }
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Trace.WriteLine("failed to run a parser : " + ex.Message);
                }
            }
        }

        public void Contains(string str, Action trigger)
        {
            Parsers.Add(new LineParser()
            {
                ParseLine = (line) => line.Contains(str),
                Trigger = trigger
            });
        }

        public void Regex(string str, Action trigger)
        {
            var reg = new Regex(str);

            Parsers.Add(new LineParser()
            {
                ParseLine = (line) => reg.IsMatch(str),
                Trigger = trigger
            });
        }

        static class Triggers
        {
            public static Action Once(Action trigger)
            {
                bool called = false;

                return () =>
                {
                    if (!called)
                    {
                        try
                        {
                            trigger();
                        }
                        finally
                        {
                            called = true;
                        }
                    }
                };
            }

            public static Action OnceInTimeWindow(TimeSpan timeWindow, Action trigger)
            {
                bool called = false;
                DateTime lastCalled = DateTime.Now;

                return () =>
                {
                    //reset time window
                    if (called && (DateTime.Now - lastCalled) > timeWindow)
                    {
                        called = false;
                    }

                    //trigger once
                    if (!called)
                    {
                        try
                        {
                            trigger();
                        }
                        finally
                        {
                            called = true;
                            //start time window
                            lastCalled = DateTime.Now;
                        }
                    }
                };
            }

            public static Action MoreThan(int amount, Action trigger)
            {
                int count = 0;

                return () =>
                {
                    count++;
                    if (count > amount)
                    {
                        trigger();
                    }
                };
            }

            public static Action MoreThanInTimeWindow(int amount, TimeSpan timeWindow, Action trigger)
            {
                //ensure one trigger per timeWindow
                
                trigger = OnceInTimeWindow(timeWindow,trigger);

                var occurances = new List<DateTime>();
                return () =>
                {
                    var now = DateTime.Now;
                    //add new occurance.
                    occurances.Add(now);
                    //remove to old occurances
                    occurances.RemoveAll(time => (now - time) > timeWindow);

                    if (occurances.Count > amount)
                    {
                        trigger();
                    }
                };
            }
        }
    }

    
}
