using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Core.IO.GeoJson;

namespace PyxisCLI.Utilities
{
    /// <summary>
    /// FeatureFilterParser enable us to create a function that filter features.
    /// The syntax of expresion is : FIELD (=|!=|&lt;|&gt;|&lt;=|&gt;=) VALUE
    /// expresion can use OR and AND operations: expression AND expression OR expression
    /// 
    /// Idan: This class should have a major upgrade to use a full blown expression parsing.
    /// </summary>
    class FeatureFilterParser
    {
        public static Func<Feature, bool> Parse(string where)
        {      
            Func<Feature, bool> condition = null;
            foreach (var part in SplitByWord(where, "OR"))
            {
                Func<Feature, bool> andCondition = null;
                foreach (var cond in SplitByWord(part, "AND"))
                {
                    var simpleCondition = ExtractSimpleCondition(cond);

                    if (andCondition == null)
                    {
                        andCondition = simpleCondition;
                    }
                    else
                    {
                        andCondition = (feature) => andCondition(feature) && simpleCondition(feature);
                    }
                }

                if (condition == null)
                {
                    condition = andCondition;
                }
                else
                {
                    condition = (feature) => condition(feature) || andCondition(feature);
                }
            }
            return condition;
        }

        private static Func<Feature, bool> ExtractSimpleCondition(string cond)
        {
            var regex = new Regex(@"(?<field>[\w_\d]+)\s*(?<op>=|!=|<|>|>=|<=)\s*(?<value>[\d\.]+|"".*""|'.*')");
            var match = regex.Match(cond);

            if (match.Success)
            {
                var field = match.Groups["field"].Value;
                var op = match.Groups["op"].Value;
                var value = match.Groups["value"].Value;

                if (value.StartsWith("\"") || value.StartsWith("'"))
                {
                    var val = JsonConvert.DeserializeObject<string>(value);
                    if (op == "=")
                    {
                        return (feature) => feature.Properties[field].ToString() == val;
                    }
                    else if (op == "!=")
                    {
                        return (feature) => feature.Properties[field].ToString() != val;
                    }
                    else if (op == ">")
                    {
                        return (feature) => String.Compare(feature.Properties[field].ToString(), val) > 0;
                    }
                    else if (op == "<")
                    {
                        return (feature) => String.Compare(feature.Properties[field].ToString(), val) < 0;
                    }
                    else if (op == ">=")
                    {
                        return (feature) => String.Compare(feature.Properties[field].ToString(), val) >= 0;
                    }
                    else if (op == "<=")
                    {
                        return (feature) => String.Compare(feature.Properties[field].ToString(), val) <= 0;
                    }
                    throw new Exception("unknown operator : " + op);
                }
                else
                {
                    var val = JsonConvert.DeserializeObject<double>(value);
                    if (op == "=")
                    {
                        return (feature) => (double)feature.Properties[field] == val;
                    }
                    else if (op == "!=")
                    {
                        return (feature) => (double)feature.Properties[field] != val;
                    }
                    else if (op == ">")
                    {
                        return (feature) => (double)feature.Properties[field] > val;
                    }
                    else if (op == "<")
                    {
                        return (feature) => (double)feature.Properties[field] < val;
                    }
                    else if (op == ">=")
                    {
                        return (feature) => (double)feature.Properties[field] >= val;
                    }
                    else if (op == "<=")
                    {
                        return (feature) => (double)feature.Properties[field] <= val;
                    }
                    throw new Exception("unknown operator : " + op);
                }
            }
            else
            {
                throw new Exception("Failed to parse : " + cond);
            }
        }

        private static IEnumerable<string> SplitByWord(string where, string word)
        {
            var split = " " + word + " ";
            var index = where.IndexOf(split);
            while (index != -1)
            {
                var part = where.Substring(0, index);
                yield return part;
                where = where.Substring(index + split.Length);
                index = where.IndexOf(split);
            }
            yield return where;
        }
    }
}
