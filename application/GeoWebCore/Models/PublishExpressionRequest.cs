using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;

namespace GeoWebCore.Models
{
    public class PublishExpressionRequest
    {
        public string Expression { get; set; }
        public List<GeoSource> GeoSources { get; set; }
    }
}
