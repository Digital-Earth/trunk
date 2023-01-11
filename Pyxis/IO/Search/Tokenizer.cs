using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using ApplicationUtility;

namespace Pyxis.IO.Search
{
    public class Tokenizer
    {
        private static readonly HashSet<string> s_StopWords;

        static Tokenizer()
        {
            s_StopWords = new HashSet<string>("a about above after again against all am an and any are aren't as at be because been before being below between both but by can't cannot could couldn't did didn't do does doesn't doing don't down during each few for from further had hadn't has hasn't have haven't having he he'd he'll he's her here here's hers herself him himself his how how's i i'd i'll i'm i've if in into is isn't it it's its itself let's me more most mustn't my myself no nor not of off on once only or other ought our ours	ourselves out over own same shan't she she'd she'll she's should shouldn't so some such than that that's the their theirs them themselves then there there's these they they'd they'll they're they've this those through to too under until up very was wasn't we we'd we'll we're we've were weren't what what's when when's where where's which while who who's whom why why's with won't would wouldn't you you'd you'll you're you've your yours yourself yourselves".Split(' '));

            //add esri specific fields
            s_StopWords.Add("objectid");
            s_StopWords.Add("globalid");
            s_StopWords.Add("shape");

            //add socrata specific fields
            s_StopWords.Add("x");
            s_StopWords.Add("y");
            s_StopWords.Add("z");
            s_StopWords.Add("latitude");
            s_StopWords.Add("longitude");
            s_StopWords.Add("location");
            s_StopWords.Add("lat");
            s_StopWords.Add("lon");
            s_StopWords.Add("long");
        }

        public virtual IEnumerable<string> Tokenize(params string[] lines)
        {
            var words = lines
                .Where(line=>StringUtilities.HasContent(line))
                .Select(line => line.ToLower(CultureInfo.InvariantCulture))
                .SelectMany(
                    line => line.Split(new[] {' ', '\t', '\n', '\r', '_', '-', '(', ')', '/', '\\', '[', ']', '{', '}', '@', '<', '>','.', ',' }, StringSplitOptions.RemoveEmptyEntries))
                .Select(word=>word.ToLower().Trim())
                .Distinct()
                .Where(word => !s_StopWords.Contains(word));

            return words;
        }
    }
}