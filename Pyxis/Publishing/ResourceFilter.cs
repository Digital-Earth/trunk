using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Publishing
{
    public class ResourceFilter
    {
        public int Skip { get; set; }
        public int Top { get; set; }
        public string Search { get; set; }
        public string Filter { get; set; }
        public string Select { get; set; }

        public ResourceFilter Clone()
        {
            return new ResourceFilter
            {
                Skip = this.Skip,
                Top = this.Top,
                Search = this.Search,
                Filter = this.Filter,
                Select = this.Select
            };
        }

        public ResourceFilter NextPage()
        {
            return NextPage(Top);
        }

        public ResourceFilter NextPage(int amount)
        {
            var result = Clone();
            result.Skip += amount;
            return result;
        }
    }

}
