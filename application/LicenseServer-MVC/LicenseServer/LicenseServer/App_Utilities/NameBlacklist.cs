using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace LicenseServer.App_Utilities
{
    static public class NameBlacklist
    {
        private static List<string> s_blacklist;

        static NameBlacklist()
        {
            s_blacklist = Properties.Settings.Default.NameBlacklist.Cast<string>().ToList();
            s_blacklist.Sort();
        }

        static public bool Contains(string name)
        {
            return s_blacklist.BinarySearch(name.Trim(), StringComparer.CurrentCultureIgnoreCase) >= 0;
        }
    }
}