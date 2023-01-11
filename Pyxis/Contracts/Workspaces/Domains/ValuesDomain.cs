using System.Collections.Generic;
using System.Linq;

namespace Pyxis.Contract.Workspaces.Domains
{
    public class ValuesDomain : IDomain
    {
        public ValuesDomain(IEnumerable<string> values)
        {
            ListOfValues = values.ToList();
        }

        public IEnumerable<string> Values
        {
            get { return ListOfValues; }
        }

        public List<string> ListOfValues { get; set; }

        public string Type
        {
            get { return "string"; }
        }

        public bool Contains(string value)
        {
            return ListOfValues.Contains(value);
        }

        public string FormatValue(string value, string format)
        {
            return value;
        }
    }
}