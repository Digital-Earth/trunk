using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations.Url
{
    class UrlUpdateOperation : IOperationMode
    {
        public string Command
        {
            get { return "url update"; }
        }

        public string Description
        {
            get { return "update url status"; }
        }

        public void Run(string[] args)
        {
            string status = "";
            var tagsToAdd = new List<string>();
            var tagsToRemove = new List<string>();

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("status", (value) => status = value),
                new ArgsParser.Option("remove-tag", (value) => tagsToRemove.Add(value)),
                new ArgsParser.Option("add-tag", (value) => tagsToAdd.Add(value)));

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} {{action}}", Command);
                return;
            }

            UpdateUrl(args.Skip(2).ToArray(), status, tagsToAdd, tagsToRemove);
        }

        private void UpdateUrl(string[] urls, string status, List<string> addTags, List<string> removeTags)
        {
            foreach (var url in urls)
            {
                bool foundUpdate = false;

                var root = MongoPersistance.GetRoot(url);
                if (root != null)
                {
                    switch (status.ToLower())
                    {
                        case "broken":
                            status = "Broken";
                            break;

                        case "offline":
                            status = "Offline";
                            break;

                        case "new":
                            status = "New";
                            break;

                        default:
                            status = "";
                            break;
                    }

                    if (addTags.Count > 0)
                    {
                        if (root.Metadata.Tags == null)
                        {
                            root.Metadata.Tags = new List<string>();
                        }

                        foreach (var tag in addTags)
                        {
                            if (!root.Metadata.Tags.Contains(tag))
                            {
                                root.Metadata.Tags.Add(tag);
                                foundUpdate = true;
                            }
                        }
                    }

                    if (removeTags.Count > 0)
                    {
                        if (root.Metadata.Tags == null)
                        {
                            root.Metadata.Tags = new List<string>();
                        }

                        foreach (var tag in removeTags)
                        {
                            if (root.Metadata.Tags.Contains(tag))
                            {
                                root.Metadata.Tags.Remove(tag);
                                foundUpdate = true;
                            }
                        }
                    }

                    if (status.HasContent())
                    {
                        root.Status = status;
                        foundUpdate = true;
                    }

                    if (foundUpdate)
                    {
                        MongoPersistance.UpdateRoot(root);
                        AutomationLog.UpdateInfo("root", root);
                    }
                }
            }
        }
    }
}