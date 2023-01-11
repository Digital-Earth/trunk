using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.DataDiscovery
{
    public static class DataDiscoveryExtensions
    {
        /// <summary>
        /// Update the metadata description with a string that states the number of datasets and sub-catalogs.
        /// <param name="catalog">The catalog.</param>
        /// </summary>
        public static void UpdateDescription(this DataSetCatalog catalog)
        {
            if (catalog.Metadata == null)
            {
                catalog.Metadata = new SimpleMetadata();
            }
            // For local catalogs set a simple description
            catalog.Metadata.Description = string.Format("Found {0}{1}{2}{3}.",
                catalog.DataSets.Count > 0 ? catalog.DataSets.Count.ToString() + " data set" + (catalog.DataSets.Count != 1 ? "s" : "") : "",
                catalog.DataSets.Count > 0 && catalog.SubCatalogs.Count > 0 ? " and " : "",
                catalog.SubCatalogs.Count > 0 ? catalog.SubCatalogs.Count.ToString() + " data container" + (catalog.SubCatalogs.Count != 1 ? "s" : "") : "",
                catalog.DataSets.Count == 0 && catalog.SubCatalogs.Count == 0 ? "no data" : "");
        }

        /// <summary>
        /// Convert from a C++ catalog to a C# catalog.
        /// </summary>
        /// <param name="pyxCatalog">The C++ catalog.</param>
        /// <returns>The C# catalog or null if no catalog.</returns>
        public static DataSetCatalog FromPyxis(PYXCatalog_CSPtr pyxCatalog)
        {
            if (pyxCatalog != null && pyxCatalog.isNotNull())
            {
                var catalog = new DataSetCatalog();
                catalog.DataType = DataSetType.Local.ToString();

                // set uri and name
                catalog.Uri = pyxCatalog.getUri() ?? "";
                catalog.Metadata.Name = pyxCatalog.getName();

                // set the tag
                catalog.Metadata.Tags = new List<string> { "Local" };

                // copy the data sets
                if (pyxCatalog.DataSets != null)
                {
                    // Copy the data sets one by one
                    foreach (var dataSet in pyxCatalog.DataSets)
                    {
                        var newDataSet = FromPyxis(dataSet);
                        if (newDataSet != null)
                        {
                            catalog.DataSets.Add(newDataSet);
                        }                            
                    }
                }

                // copy the sub-catalogs
                if (pyxCatalog.SubCatalogs != null)
                {
                    // Copy the data sets one by one
                    foreach (var subCatalog in pyxCatalog.SubCatalogs)
                    {
                        var newSubCatalog = FromPyxis(subCatalog);
                        if (newSubCatalog != null)
                        {
                            catalog.SubCatalogs.Add(newSubCatalog);                            
                        }
                    }
                }

                return catalog;
            }

            return null;
        }

        /// <summary>
        /// Create a DataSet from a PYXDataSet.
        /// </summary>
        /// <param name="pyxDataSet">The pyxis data set</param>
        /// <returns>The C# data set or null if no data set.</returns>
        static private DataSet FromPyxis(PYXDataSet_CSPtr pyxDataSet)
        {
            if (pyxDataSet != null && pyxDataSet.isNotNull())
            {
                var dataSet = new DataSet();

                // set url and name
                dataSet.Uri = pyxDataSet.getUri();
                dataSet.Metadata.Name = pyxDataSet.getName();
                dataSet.Metadata.Description = "";
                dataSet.Layer = pyxDataSet.getLayer();

                // TODO replace Fields with a PipelineSpecification
                dataSet.Fields = pyxDataSet.getContentDefinition_const().getFieldNames().Cast<string>().ToList();

                // set missing files
                dataSet.MissingRequiredFilesAllOf = pyxDataSet.getMissingRequiredFilesAllOf().Cast<string>().ToList();
                dataSet.MissingRequiredFilesOneOf = pyxDataSet.getMissingRequiredFilesOneOf().Cast<string>().ToList();
                dataSet.MissingOptionalFiles = pyxDataSet.getMissingOptionalFiles().Cast<string>().ToList();

                PYXRect2DDouble bbox1 = new PYXRect2DDouble();
                PYXRect2DDouble bbox2 = new PYXRect2DDouble();

                pyxDataSet.getBoundingBox(bbox1, bbox2);

                if (!bbox1.degenerate() || !bbox2.degenerate())
                {
                    dataSet.BBox = new List<BoundingBox>();

                    if (!bbox1.degenerate())
                    {
                        dataSet.BBox.Add(new BoundingBox()
                        {
                            LowerLeft = new BoundingBoxCorner()
                            {
                                X = bbox1.xMin(),
                                Y = bbox1.yMin()
                            },
                            UpperRight = new BoundingBoxCorner()
                            {
                                 X = bbox1.xMax(),
                                Y = bbox1.yMax()
                            },
                            Srs = "4326"
                        });
                    }

                    if (!bbox2.degenerate())
                    {
                        dataSet.BBox.Add(new BoundingBox()
                        {
                            LowerLeft = new BoundingBoxCorner()
                            {
                                X = bbox2.xMin(),
                                Y = bbox2.yMin()
                            },
                            UpperRight = new BoundingBoxCorner()
                            {
                                X = bbox2.xMax(),
                                Y = bbox2.yMax()
                            },
                            Srs = "4326"
                        });
                    }
                }

                return dataSet;
            }

            return null;
        }

        public static List<BoundingBox> ConvertBBoxToWgs84(this List<BoundingBox> bboxs)
        {
            if (bboxs == null)
            {
                return null;
            }

            var fixedBBox = new List<BoundingBox>();

            foreach (var bbox in bboxs)
            {
                if (bbox.Srs == "4326")
                {
                    fixedBBox.Add(bbox);
                    continue;
                }

                var coordConverter = PYXCOMFactory.CreateCoordConvertorFromWKT(bbox.Srs ?? "4326");
                var rect = new PYXRect2DDouble(bbox.LowerLeft.X, bbox.LowerLeft.Y, bbox.UpperRight.X, bbox.UpperRight.Y);
                var geometry = PYXXYBoundsGeometry.create(rect, coordConverter.get(), 24);

                var rect1 = new PYXRect2DDouble();
                var rect2 = new PYXRect2DDouble();

                geometry.getBoundingRects(new WGS84CoordConverter(), rect1, rect2);

                if (!rect1.empty())
                {
                    fixedBBox.Add(new BoundingBox()
                    {
                        Srs = "4326",
                        LowerLeft = new BoundingBoxCorner() {X = rect1.xMin(), Y = rect1.yMin()},
                        UpperRight = new BoundingBoxCorner() {X = rect1.xMax(), Y = rect1.yMax()}
                    });
                }

                if (!rect2.empty())
                {
                    fixedBBox.Add(new BoundingBox()
                    {
                        Srs = "4326",
                        LowerLeft = new BoundingBoxCorner() {X = rect2.xMin(), Y = rect2.yMin()},
                        UpperRight = new BoundingBoxCorner() {X = rect2.xMax(), Y = rect2.yMax()}
                    });
                }
            }

            return fixedBBox;
        }
    }
}
