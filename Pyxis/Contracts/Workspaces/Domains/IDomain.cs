using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml;

namespace Pyxis.Contract.Workspaces.Domains
{
    public interface IDomain
    {
        IEnumerable<string> Values { get; }
        string Type { get; }
        bool Contains(string value);

        string FormatValue(string value, string format);
    }
}
