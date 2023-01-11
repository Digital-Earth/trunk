using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Web;

namespace worldView.gallery.Services
{
    /// <summary>
    /// Generate a secret key (used for beta invites - and maybe things later as well) based on email and current date.
    /// The generated keys are valid for 2-3 month.
    /// The key would have the following strcuted: PROMOWORD-CODE
    /// PROMOWORD is something like "Earth", "My-Beta", etc..
    /// CODE is 6 uppercase letters or digits: 6KS23M, etc...
    /// </summary>
    public static class BetaKeyGenerator
    {
        private enum Format
        {
            Original,
            Short
        }

        private static List<string> GetCurrentDatePrefixes()
        {
            var today = DateTime.Today;
            var thisMonth = new DateTime(today.Year, today.Month, 1);
            var lastMonth = thisMonth.AddMonths(-1);
            var last2Month = thisMonth.AddMonths(-2);
            return new List<string>(){
                thisMonth.ToString("yyyy-MM-dd"),
                lastMonth.ToString("yyyy-MM-dd"),
                last2Month.ToString("yyyy-MM-dd")
            };
        }

        private static string[] s_generalUseKeys = new[] { 
            "DigitalEarth"             
        };

        private static string[] s_promoWords = new[] {
            "Earth", 
            "Gallery", 
            "Map",
            "Globe",
            "GeoSource",
            "Channel",
            "My-Beta",
            "Search",
            "Location",
            "Realtime",
            "Analyze",
            "Report",
            "Hexagon", 
            "Tessellation", 
            "Equal-Area" 
        };

        private static string GetPromoWord(byte p)
        {
            return s_promoWords[p % s_promoWords.Length];
        }

        private static string GenerateKeyFromString(string str, Format format)
        {
            using (MD5 md5Hash = MD5.Create())
            {
                byte[] data = md5Hash.ComputeHash(Encoding.UTF8.GetBytes(str));
                var code = String.Concat(Convert.ToBase64String(data).Where(c => Char.IsLetterOrDigit(c)).Take(6)).ToUpper();

                if (format == Format.Original)
                {
                    return GetPromoWord(data[0]) + "-" + code;
                }
                else
                {
                    return code;
                }
            }
        }

        static private List<string> GetValidKeysForEmail(string email)
        {
            return 
                GetCurrentDatePrefixes().Select(d => d + " " + email).Select(x => GenerateKeyFromString(x,Format.Short))
                .Concat(GetCurrentDatePrefixes().Select(d => d + " " + email).Select(x => GenerateKeyFromString(x,Format.Original)))
                .ToList();
        }

        static public bool IsKeyValidForEmail(string email,string key)
        {
            return s_generalUseKeys.Contains(key) || GetValidKeysForEmail(email).Contains(key);
        }

        static public string GetKeyForEmail(string email)
        {
            return GetValidKeysForEmail(email).First();
        }
    }
}