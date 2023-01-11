using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Search;

namespace Pyxis.IO.Test.Search
{
    [TestFixture]
    class GazetteerExtensionsTests
    {
        #region Utility Functions 
        private static List<BoundingBox> CreateBBox(double minx,double miny,double maxx, double maxy)
        {
            return new List<BoundingBox>()
            {
                new BoundingBox()
                {
                    LowerLeft = new BoundingBoxCorner(minx, miny),
                    UpperRight = new BoundingBoxCorner(maxx, maxy)
                }
            };
        }

        private static DataSet AddSpecification(DataSet dataset, PipelineSpecification.PipelineOutputType outputType,
            params string[] fields)
        {
            dataset.Specification = new PipelineSpecification()
            {
                OutputType = outputType,
                Fields = fields.Select(field=>
                    new PipelineSpecification.FieldSpecification()
                    {
                        Name = field.StartsWith("n_") ? field.Substring(2) : field,
                        FieldType = field.StartsWith("n_") ? PipelineSpecification.FieldType.Number : PipelineSpecification.FieldType.String,
                        Metadata = new Metadata()
                        {
                            Name = field
                        }
                    }).ToList()                
            };
            return dataset;
        }

        #endregion

        [Test]
        public void RemoveDuplicateWorksFilterAllDatasetsWithoutBBox()
        {
            var datasets = new List<DataSet>();

            datasets.Add(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
            });

            Assert.IsEmpty(datasets.RemoveDuplicates());
        }


        [Test]
        public void RemoveDuplicateWorksFilterAllDatasetsWithoutDiscoveryReport()
        {
            var datasets = new List<DataSet>();

            datasets.Add(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
                BBox = CreateBBox(0,0,1,1)
            });

            Assert.IsEmpty(datasets.RemoveDuplicates());
        }

        [Test]
        public void RemoveDuplicateWorksWithoutSpecification()
        {
            var datasets = new List<DataSet>();

            datasets.Add(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 2,
                    GeometryType = "polygon",
                    DataSize = 1023
                }
            });

            Assert.AreEqual(datasets, datasets.RemoveDuplicates());
        }

        [Test]
        public void RemoveDuplicateMatchFeatureCountAndBBoxIfNoSpec()
        {
            var datasets = new List<DataSet>();

            datasets.Add(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 2,
                    GeometryType = "polygon",
                    DataSize = 1023
                }
            });

            datasets.Add(new DataSet()
            {
                Uri = "http://localhost/dataset/2",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 2,
                    GeometryType = "polygon",
                    DataSize = 1023
                }
            });

            Assert.AreEqual(datasets.Take(1).ToList(), datasets.RemoveDuplicates());
        }

        [Test]
        public void RemoveDuplicateMatchBBoxAndSpecForCoverages()
        {
            var datasets = new List<DataSet>();

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                }
            },PipelineSpecification.PipelineOutputType.Coverage,"n_RGB"));

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/2",
                BBox = CreateBBox(0, 0, 2, 2),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                }
            }, PipelineSpecification.PipelineOutputType.Coverage, "n_RGB"));

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/3",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                },
            }, PipelineSpecification.PipelineOutputType.Coverage, "n_RGB"));

            Assert.AreEqual(datasets.Take(2).ToList(), datasets.RemoveDuplicates());
        }

        [Test]
        public void RemoveDuplicateMatchFeatureCountAndBBoxAndSpecForVectors()
        {
            var datasets = new List<DataSet>();

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 12
                }
            }, PipelineSpecification.PipelineOutputType.Feature, "Name", "n_Age"));

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/2",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 13
                }
            }, PipelineSpecification.PipelineOutputType.Feature, "Name", "n_Age"));

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/3",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 12
                },
            }, PipelineSpecification.PipelineOutputType.Feature, "Name", "n_Age"));

            Assert.AreEqual(datasets.Take(2).ToList(), datasets.RemoveDuplicates());
        }

        [Test]
        public void RemoveDuplicateDisgwishBetweenEmptyVectorAndCoverage()
        {
            var datasets = new List<DataSet>();

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/1",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 0
                }
            }, PipelineSpecification.PipelineOutputType.Coverage, "Name", "n_Age"));

            datasets.Add(AddSpecification(new DataSet()
            {
                Uri = "http://localhost/dataset/2",
                BBox = CreateBBox(0, 0, 1, 1),
                DiscoveryReport = new DataSetDiscoveryReport()
                {
                    Status = DataSetDiscoveryStatus.Successful,
                    FeaturesCount = 0
                }
            }, PipelineSpecification.PipelineOutputType.Feature, "Name", "n_Age"));

            Assert.AreEqual(datasets.Take(2).ToList(), datasets.RemoveDuplicates());
        }
    }
}
