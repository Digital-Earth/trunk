using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace PyxCrawler.Utilities
{
    internal class ProtocolComparer : IComparer<string>
    {
        private static readonly Dictionary<string, int> s_priority = new Dictionary<string, int>
        {
            {"WFS", 1},
            {"AGSF", 2},
            {"WCS", 3},
            {"WMS", 4},
            {"AGSM", 5}
        };

        public int Compare(string x, string y)
        {
            return s_priority[x] - s_priority[y];
        }
    }
}