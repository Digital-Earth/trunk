using System.Collections.Generic;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// IImport represent a named import infomration of geospatial data
    /// </summary>
    public interface IImport
    {
        string Type { get; }

        string Uri { get; }

        string Layer { get; }

        string Srs { get; set; }

        Style Style { get; }

        PipelineSpecification Specification { get; }

        GeoTagMethods GeoTag { get;  }

        string Sampler { get; set; }

        DataSet Resolve(Dictionary<string, string> domainsValues = null);
    }
}