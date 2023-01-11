using System.Collections.Generic;
using Pyxis.Contract.DataDiscovery;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// DataSetCatalog Method Extensions
    /// </summary>
    public static class DataSetCatalogExtensions
    {
        /// <summary>
        /// Enumerate all data sets in a catalog and its subcatalogs
        /// </summary>
        /// <param name="catalog">The root DataSetCatalog object.</param>
        /// <returns>Enumerated DataSet objects.</returns>
        public static IEnumerable<DataSet> AllDataSets(this DataSetCatalog catalog)
        {
            if (catalog.DataSets != null)
            {
                foreach (var dataset in catalog.DataSets)
                {
                    yield return dataset;
                }
            }
            if (catalog.SubCatalogs != null)
            {
                foreach (var subCatalog in catalog.SubCatalogs)
                {
                    foreach (var dataset in subCatalog.AllDataSets())
                    {
                        yield return dataset;
                    }
                }
            }
        }

        /// <summary>
        /// Enumerate all loadable data sets in a catalog and its subcatalogs
        /// </summary>
        /// <param name="catalog">The root DataSetCatalog object.</param>
        /// <returns>Enumerated DataSet objects.</returns>
        public static IEnumerable<DataSet> AllLoadableDataSets(this DataSetCatalog catalog)
        {
            if (catalog.DataSets != null)
            {
                foreach (var dataset in catalog.DataSets)
                {
                    if (dataset.IsLoadable())
                    {
                        yield return dataset;                        
                    }
                }
            }
            if (catalog.SubCatalogs != null)
            {
                foreach (var subCatalog in catalog.SubCatalogs)
                {
                    foreach (var dataset in subCatalog.AllDataSets())
                    {
                        if (dataset.IsLoadable())
                        {
                            yield return dataset;                            
                        }
                    }
                }
            }
        }
    }
}
