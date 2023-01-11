using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace Pyxis.Core.Test
{
    /// <summary>
    /// UnmanagedTests is a bridge between pyxlib C++ unmanaged tests and the NUnit framework.
    /// 
    /// The goal if this class it to allow us to teat the c++ unmanaged as Nunit unit test.
    /// This provides the following benefits:
    ///   1) Resharper and team-city integration.
    ///   2) Easy to execute a single unmanaged unit test from Resharper menu.
    /// 
    /// This class uses the TestFrame c++ class to get list of all unmanaged unit test.
    /// Then we can create a NUnit unit test that will run that single unit test.
    /// 
    /// The function <see cref="DoTest"/> is encapsulating invoking the unmanaged unit test.
    /// It will also report that this specific unit test has been executed.
    /// 
    /// As a fail safe, the function <see cref="ZLast_RunUnmanagedTests"/> is checking that we
    /// Didn't miss any unmanaged unit test. This function should run last, therefore the ZLast prefix
    /// 
    /// Please note, running under Reshaper you should disable "Shadow copy assembly been tested" to allow
    /// plug-ins dlls to register unit tests.
    /// </summary>
    [TestFixture]    
    internal class UnmanagedTests
    {
        /// <summary>
        /// Keep track on all unit tests discovered by the TestFrame class
        /// </summary>
        private static SortedSet<string> s_allTests;

        /// <summary>
        /// Keep track on all unit test that hadn't been executed yet.
        /// </summary>
        private static SortedSet<string> s_testsLeft;

        private static void InitiateTestList()
        {
            if (s_allTests == null)
            {
                var tests = TestFrame.getInstance();

                var allTests = tests.getTests();
                s_allTests = new SortedSet<string>();
                s_testsLeft = new SortedSet<string>();

                foreach (var test in allTests)
                {
                    s_allTests.Add(test);
                    s_testsLeft.Add(test);
                }
            }
        }

        private static IEnumerable<string> TestsLeftToDo
        {
            get
            {
                InitiateTestList();
                return s_testsLeft.ToList();
            }
        }

        private void DoTest(string test)
        {
            InitiateTestList();

            if (s_testsLeft.Contains(test))
            {
                s_testsLeft.Remove(test);

                if (!TestFrame.getInstance().test(test))
                {
                    foreach (var line in TestFrame.getInstance().getTestLog(test))
                    {
                        System.Diagnostics.Trace.WriteLine(line);
                    }
                    Assert.Fail(test);
                }
            }
            else if (!s_allTests.Contains(test))
            {
                Assert.Fail("test '" + test + "' doesn't exist.");
            }
        }

        [Test]
        public void PYXGeometrySerializerTests()
        {
            DoTest("class PYXGeometrySerializer");
        }

        [Test]
        public void FIDStrTests()
        {
            DoTest("class FIDStr");
        }

        [Test]
        public void PYXFieldDefinitionTests()
        {
            DoTest("class PYXFieldDefinition");
        }

        [Test]
        public void PYXTableDefinitionTests()
        {
            DoTest("class PYXTableDefinition");
        }

        [Test]
        public void PYXValueTileTests()
        {
            DoTest("class PYXValueTile");
        }

        [Test]
        public void PYXChildIteratorTests()
        {
            DoTest("class PYXChildIterator");
        }

        [Test]
        public void PYXInnerChildIteratorTests()
        {
            DoTest("class PYXInnerChildIterator");
        }

        [Test]
        public void CompactIndexTests()
        {
            DoTest("class CompactIndex");
        }

        [Test]
        public void PYXCursorTests()
        {
            DoTest("class PYXCursor");
        }

        [Test]
        public void PYXDiamondIteratorTests()
        {
            DoTest("class PYXDiamondIterator");
        }

        [Test]
        public void PYXDirEdgeIteratorTests()
        {
            DoTest("class PYXDirEdgeIterator");
        }

        [Test]
        public void PYXEdgeIteratorTests()
        {
            DoTest("class PYXEdgeIterator");
        }

        [Test]
        public void PYXExhaustiveIteratorTests()
        {
            DoTest("class PYXExhaustiveIterator");
        }

        [Test]
        public void PYXFourierDiamondIteratorTests()
        {
            DoTest("class PYXFourierDiamondIterator");
        }

        [Test]
        public void PYXHexagonalIteratorTests()
        {
            DoTest("class PYXHexagonalIterator");
        }

        [Test]
        public void PYXHexDirectionIteratorTests()
        {
            DoTest("class PYXHexDirectionIterator");
        }

        [Test]
        public void HexagonTests()
        {
            DoTest("class Hexagon");
        }

        [Test]
        public void PYXIcosIndexTests()
        {
            DoTest("class PYXIcosIndex");
        }

        [Test]
        public void PYXIcosMathTests()
        {
            DoTest("class PYXIcosMath");
        }

        [Test]
        public void PYXIteratorLinqTesterTests()
        {
            DoTest("class PYXIteratorLinqTester");
        }

        [Test]
        public void PYXNeighbourIteratorTests()
        {
            DoTest("class PYXNeighbourIterator");
        }

        [Test]
        public void PYXProgressiveIteratorTests()
        {
            DoTest("class PYXProgressiveIterator");
        }

        [Test]
        public void PYXResolutionChangeIteratorTests()
        {
            DoTest("class PYXResolutionChangeIterator");
        }

        [Test]
        public void SnyderProjectionTests()
        {
            DoTest("class SnyderProjection");
        }

        [Test]
        public void PYXSpiralIteratorTests()
        {
            DoTest("class PYXSpiralIterator");
        }

        [Test]
        public void PYXIndexTests()
        {
            DoTest("class PYXIndex");
        }

        [Test]
        public void PYXMathTests()
        {
            DoTest("class PYXMath");
        }

        [Test]
        public void PYXValidDirectionIteratorTests()
        {
            DoTest("class PYXValidDirectionIterator");
        }

        [Test]
        public void WGS84CoordConverterTests()
        {
            DoTest("class WGS84CoordConverter");
        }

        [Test]
        public void PYXBoundingRectsCalculatorTests()
        {
            DoTest("class PYXBoundingRectsCalculator");
        }

        [Test]
        public void PYXCellTests()
        {
            DoTest("class PYXCell");
        }

        [Test]
        public void PYXCircleGeometryTests()
        {
            DoTest("class PYXCircleGeometry");
        }

        [Test]
        public void CombinedPyxisIndexTests()
        {
            DoTest("class CombinedPyxisIndex");
        }

        [Test]
        public void PYXConstantGeometryTests()
        {
            DoTest("class PYXConstantGeometry");
        }

        [Test]
        public void PYXCurveTests()
        {
            DoTest("class PYXCurve");
        }

        [Test]
        public void PYXEmptyGeometryTests()
        {
            DoTest("class PYXEmptyGeometry");
        }

        [Test]
        public void PYXGlobalGeometryTests()
        {
            DoTest("class PYXGlobalGeometry");
        }

        [Test]
        public void HitTestUtilsTests()
        {
            DoTest("class HitTestUtils");
        }

        [Test]
        public void PYXIcosTestTraverserTests()
        {
            DoTest("class PYXIcosTestTraverser");
        }

        [Test]
        public void PYXInnerTileTests()
        {
            DoTest("class PYXInnerTile");
        }

        [Test]
        public void PYXPrimeInnerTileIteratorTests()
        {
            DoTest("class PYXPrimeInnerTileIterator");
        }

        [Test]
        public void PYXMultiCellTests()
        {
            DoTest("class PYXMultiCell");
        }

        [Test]
        public void PYXMultiGeometry_PYXCell_Tests()
        {
            DoTest("class PYXMultiGeometry<class PYXCell>");
        }

        [Test]
        public void PYXPolygonTests()
        {
            DoTest("class PYXPolygon");
        }

        [Test]
        public void PYXTileTests()
        {
            DoTest("class PYXTile");
        }

        [Test]
        public void PYXTileCache_TestTile_Tests()
        {
            DoTest("class PYXTileCache<class TestTile>");
        }

        [Test]
        public void PYXTileCollectionTests()
        {
            DoTest("class PYXTileCollection");
        }

        [Test]
        public void TesteeTests()
        {
            DoTest("struct `anonymous namespace'::Testee");
        }

        [Test]
        public void ProcessIdentityTests()
        {
            DoTest("class ProcessIdentity");
        }

        [Test]
        public void ProcessIdentityCacheTests()
        {
            DoTest("class ProcessIdentityCache");
        }

        [Test]
        public void FeatureCollectionIndexProcessTests()
        {
            DoTest("class FeatureCollectionIndexProcess");
        }

        [Test]
        public void AppServicesTests()
        {
            DoTest("class AppServices");
        }

        [Test]
        public void BitUtilsTests()
        {
            DoTest("struct BitUtils");
        }

        [Test]
        public void CacheMapTesterTests()
        {
            DoTest("class CacheMapTester");
        }

        [Test]
        public void PYXColorPaletteTests()
        {
            DoTest("class PYXColorPalette");
        }

        [Test]
        public void PYXStringColorPaletteTests()
        {
            DoTest("class PYXStringColorPalette");
        }

        [Test]
        public void PYXValueColorPaletteTests()
        {
            DoTest("class PYXValueColorPalette");
        }

        [Test]
        public void CommandManagerTests()
        {
            DoTest("class CommandManager");
        }

        [Test]
        public void CoordLatLonTests()
        {
            DoTest("class CoordLatLon");
        }

        [Test]
        public void PYXCoordPolarTests()
        {
            DoTest("class PYXCoordPolar");
        }

        [Test]
        public void PYXCostTests()
        {
            DoTest("class PYXCost");
        }

        [Test]
        public void EllipsoidMathTests()
        {
            DoTest("class EllipsoidMath");
        }

        [Test]
        public void FileUtilsTests()
        {
            DoTest("class FileUtils");
        }

        [Test]
        public void GreatCircleArcTests()
        {
            DoTest("class GreatCircleArc");
        }

        [Test]
        public void HttpRequestTests()
        {
            DoTest("class HttpRequest");
        }

        [Test]
        public void IdentityCacheTests()
        {
            DoTest("class IdentityCache");
        }

        [Test]
        public void MemUtilsTests()
        {
            DoTest("class MemUtils");
        }

        [Test]
        public void MemoryManagerTests()
        {
            DoTest("class MemoryManager");
        }

        [Test]
        public void NotifierTests()
        {
            DoTest("class Notifier");
        }

        [Test]
        public void HistogramDoubleTests()
        {
            DoTest("class HistogramDoubleTester");
        }

        [Test]
        public void RangeTesterTests()
        {
            DoTest("class RangeTester");
        }
        
        [Test]
        public void SphereMathTests()
        {
            DoTest("class SphereMath");
        }

        [Test]
        public void SphereMathGreatCircleArcTests()
        {
            DoTest("class SphereMath::GreatCircleArc");
        }

        [Test]
        public void SSLUtilsChecksumTests()
        {
            DoTest("class SSLUtils::Checksum");
        }

        [Test]
        public void StdIntTests()
        {
            DoTest("class StdInt");
        }

        [Test]
        public void StringHistogramTests()
        {
            DoTest("class StringHistogram");
        }

        [Test]
        public void StringUtilsTests()
        {
            DoTest("class StringUtils");
        }

        [Test]
        public void SXSParserTests()
        {
            DoTest("class SXSParser");
        }
        [Test]
        public void PYXTaskTesterTests()
        {
            DoTest("class PYXTaskTester");
        }

        [Test]
        public void PYXValueTests()
        {
            DoTest("class PYXValue");
        }

        [Test]
        public void PYXValueColumnTests()
        {
            DoTest("class PYXValueColumn");
        }

        [Test]
        public void PYXValueMathTests()
        {
            DoTest("class PYXValueMath");
        }

        [Test]
        public void PYXValueTableTests()
        {
            DoTest("class PYXValueTable");
        }

        [Test]
        public void CSharpXMLDocTests()
        {
            DoTest("class CSharpXMLDoc");
        }

        [Test]
        public void XMLUtilsTests()
        {
            DoTest("class XMLUtils");
        }

        [Test]
        public void BadCoverageTests()
        {
            DoTest("class BadCoverage");
        }

        [Test]
        public void ConstCoverageTests()
        {
            DoTest("class ConstCoverage");
        }

        [Test]
        public void DefaultFeatureTests()
        {
            DoTest("class DefaultFeature");
        }

        [Test]
        public void NamedGeometryProcTests()
        {
            DoTest("class NamedGeometryProc");
        }

        [Test]
        public void PathProcessTests()
        {
            DoTest("class PathProcess");
        }

        [Test]
        public void ProcUtilsTests()
        {
            DoTest("class ProcUtils");
        }

        [Test]
        public void UrlProcessTests()
        {
            DoTest("class UrlProcess");
        }

        [Test]
        public void UserCredentialsProviderProcessTests()
        {
            DoTest("class UserCredentialsProviderProcess");
        }

        [Test]
        public void PYXXYBoundsGeometryTests()
        {
            DoTest("class PYXXYBoundsGeometry");
        }
        
        [Test]
        public void PYXCurveRegionTests()
        {
            DoTest("class PYXCurveRegion");
        }

        [Test]
        public void AreaStyleFeatureCollectionProcessTests()
        {
            DoTest("class AreaStyleFeatureCollectionProcess");
        }

        [Test]
        public void AttributeQueryTests()
        {
            DoTest("class AttributeQuery");
        }

        [Test]
        public void BehaviourFeatureCollectionProcessTests()
        {
            DoTest("class BehaviourFeatureCollectionProcess");
        }

        [Test]
        public void BitmapCropProcessTests()
        {
            DoTest("class BitmapCropProcess");
        }

        [Test]
        public void BitmapGridProcessTests()
        {
            DoTest("class BitmapGridProcess");
        }

        [Test]
        public void BitmapProcessTests()
        {
            DoTest("class BitmapProcess");
        }

        [Test]
        public void BlenderTests()
        {
            DoTest("class Blender");
        }

        [Test]
        public void BlurProcessTests()
        {
            DoTest("class BlurProcess");
        }

        [Test]
        public void CalculatorTests()
        {
            DoTest("class Calculator");
        }

        [Test]
        public void ChannelCombinerTests()
        {
            DoTest("class ChannelCombiner");
        }

        [Test]
        public void CheckerCoverageTests()
        {
            DoTest("class CheckerCoverage");
        }

        [Test]
        public void ColourizerTests()
        {
            DoTest("class Colourizer");
        }

        [Test]
        public void ConcatFeaturesTests()
        {
            DoTest("class ConcatFeatures");
        }

        [Test]
        public void CoordConverterImplTests()
        {
            DoTest("class CoordConverterImpl");
        }

        [Test]
        public void CoverageGeometryMaskProcessTests()
        {
            DoTest("class CoverageGeometryMaskProcess");
        }

        [Test]
        public void CoverageMaskProcessTests()
        {
            DoTest("class CoverageMaskProcess");
        }

        [Test]
        public void CsvRecordCollectionProcessTests()
        {
            DoTest("class CsvRecordCollectionProcess");
        }

        [Test]
        public void CsvRecordCollectionProcess_v1Tests()
        {
            DoTest("class CsvRecordCollectionProcess_v1");
        }

        [Test]
        public void DocumentProcessTests()
        {
            DoTest("class DocumentProcess");
        }

        [Test]
        public void ElevationToNormalProcessTests()
        {
            DoTest("class ElevationToNormalProcess");
        }

        [Test]
        public void EqualsTests()
        {
            DoTest("class Equals");
        }

        [Test]
        public void ExcelProcessTests()
        {
            DoTest("class ExcelProcess");
        }

        [Test]
        public void ExcelRecordCollectionProcessTests()
        {
            DoTest("class ExcelRecordCollectionProcess");
        }

        [Test]
        public void FeatureCollectionFilterTests()
        {
            DoTest("class FeatureCollectionFilter");
        }

        [Test]
        public void FeatureCollectionResolutionFilterTests()
        {
            DoTest("class FeatureCollectionResolutionFilter");
        }

        [Test]
        public void FeatureColourizerTests()
        {
            DoTest("class FeatureColourizer");
        }

        [Test]
        public void FeatureConditionCalculatorTests()
        {
            DoTest("class FeatureConditionCalculator");
        }

        [Test]
        public void FeatureFieldRasterizerTests()
        {
            DoTest("class FeatureFieldRasterizer");
        }

        [Test]
        public void FeatureRasterizerTests()
        {
            DoTest("class FeatureRasterizer");
        }

        [Test]
        public void FeaturesSummaryAttributeRangeFilterTests()
        {
            DoTest("class FeaturesSummaryAttributeRangeFilter");
        }

        [Test]
        public void FeaturesSummaryGeometryFilterTests()
        {
            DoTest("class FeaturesSummaryGeometryFilter");
        }

        [Test]
        public void FirstNonNullTests()
        {
            DoTest("class FirstNonNull");
        }

        [Test]
        public void GDALFileConverterProcessTests()
        {
            DoTest("class GDALFileConverterProcess");
        }

        [Test]
        public void GDALFileProcessTests()
        {
            DoTest("class GDALFileProcess");
        }

        [Test]
        public void GDALMultiProcessTests()
        {
            DoTest("class GDALMultiProcess");
        }

        [Test]
        public void GDALPipeBuilderTests()
        {
            DoTest("class GDALPipeBuilder");
        }

        [Test]
        public void GDALXYAsyncValueGetterTests()
        {
            DoTest("class GDALXYAsyncValueGetter");
        }

        [Test]
        public void GDALWCSProcessTests()
        {
            DoTest("class GDALWCSProcess");
        }

        [Test]
        public void GDALWCSProcessV2Tests()
        {
            DoTest("class GDALWCSProcessV2");
        }

        [Test]
        public void GDALWMSProcessTests()
        {
            DoTest("class GDALWMSProcess");
        }

        [Test]
        public void GeotagRecordCollectionTests()
        {
            DoTest("class GeotagRecordCollection");
        }

        [Test]
        public void GreyscaleToRGBProcessTests()
        {
            DoTest("class GreyscaleToRGBProcess");
        }

        [Test]
        public void GRIBProcessTests()
        {
            DoTest("class GRIBProcess");
        }

        [Test]
        public void HillShaderTests()
        {
            DoTest("class HillShader");
        }

        [Test]
        public void IconStyleFeatureCollectionProcessTests()
        {
            DoTest("class IconStyleFeatureCollectionProcess");
        }

        [Test]
        public void IcosTreeTestsTests()
        {
            DoTest("class IcosTreeTests");
        }

        [Test]
        public void IntersectionTests()
        {
            DoTest("class Intersection");
        }

        [Test]
        public void IntersectsTests()
        {
            DoTest("class Intersects");
        }

        [Test]
        public void LineStyleFeatureCollectionProcessTests()
        {
            DoTest("class LineStyleFeatureCollectionProcess");
        }

        [Test]
        public void LuaCoverageTests()
        {
            DoTest("class LuaCoverage");
        }

        [Test]
        public void MultiResSpatialAnalysisProcessTests()
        {
            DoTest("class MultiResSpatialAnalysisProcess");
        }

        [Test]
        public void NormalToRGBProcessTests()
        {
            DoTest("class NormalToRGBProcess");
        }

        [Test]
        public void NullCoverageTests()
        {
            DoTest("class NullCoverage");
        }

        [Test]
        public void OwsCoverageNetworkResourceProcessTests()
        {
            DoTest("class OwsCoverageNetworkResourceProcess");
        }

        [Test]
        public void OwsVectorNetworkResourceProcessTests()
        {
            DoTest("class OwsVectorNetworkResourceProcess");
        }

        [Test]
        public void PointAggregatorProcessTests()
        {
            DoTest("class PointAggregatorProcess");
        }

        [Test]
        public void PYXCoverageCacheTests()
        {
            DoTest("class PYXCoverageCache");
        }

        [Test]
        public void PYXDataCollectionTests()
        {
            DoTest("class PYXDataCollection");
        }

        [Test]
        public void PYXDefaultCoverageTests()
        {
            DoTest("class PYXDefaultCoverage");
        }

        [Test]
        public void PyxisPipeBuilderTests()
        {
            DoTest("class PyxisPipeBuilder");
        }

        [Test]
        public void PYXOGRDataSourceTests()
        {
            DoTest("class PYXOGRDataSource");
        }

        [Test]
        public void PYXrTreeTests()
        {
            DoTest("class PYXrTree");
        }

        [Test]
        public void PYXZoomInProcessTests()
        {
            DoTest("class PYXZoomInProcess");
        }

        [Test]
        public void PYXZoomOutProcessTests()
        {
            DoTest("class PYXZoomOutProcess");
        }

        [Test]
        public void ResolutionFilterTests()
        {
            DoTest("class ResolutionFilter");
        }

        [Test]
        public void StyledCoverageTests()
        {
            DoTest("class StyledCoverage");
        }

        [Test]
        public void StyledFeatureCollectionProcessTests()
        {
            DoTest("class StyledFeatureCollectionProcess");
        }

        [Test]
        public void StyledFeatureRasterizerTests()
        {
            DoTest("class StyledFeatureRasterizer");
        }

        [Test]
        public void StyledFeaturesSummaryProcessTests()
        {
            DoTest("class StyledFeaturesSummaryProcess");
        }

        [Test]
        public void UnionTests()
        {
            DoTest("class Union");
        }

        [Test]
        public void ViewPointProcessTests()
        {
            DoTest("class ViewPointProcess");
        }

        [Test]
        public void WPSProcessTests()
        {
            DoTest("class WPSProcess");
        }

        [Test]
        public void PyxisBlobProviderTests()
        {
            DoTest("class PyxisBlobProvider");
        }

        [Test]
        public void PYXCatalogTests()
        {
            DoTest("class PYXCatalog");
        }

        [Test]
        public void PYXDataSetTests()
        {
            DoTest("class PYXDataSet");
        }

        private string CreateManagedUnitTestCode(string test)
        {
            var safeTestName = test
                    .Replace("class ", "")
                    .Replace("struct ", "")
                    .Replace("`anonymous namespace'::", "")
                    .Replace('<', '_')
                    .Replace('>', '_')
                    .Replace("'", "")
                    .Replace("`", "")
                    .Replace(":", "");

            var unitTestCode = String.Format(@"
        [Test]
        public void {1}Tests()
        {{
            DoTest(""{0}"");
        }}", test, safeTestName);

            return unitTestCode;
        }

        [Test]
        //Please don't change function name. it start with ZLast_ to make sure it run last
        public void ZLast_RunUnmanagedTests()
        {
            if (TestsLeftToDo.Any())
            {
                var missingCode = String.Join("\n", TestsLeftToDo.Select(CreateManagedUnitTestCode));

                Trace.info("recommend adding the following unit-test function(s):\n" + missingCode);
            }

            //this function will run all unmanaged tests that didn't run yet.
            //this is a fail safe to make sure we run all unmanaged test - aka - someone add a unit test
            foreach (var test in TestsLeftToDo)
            {
                Trace.info("perform unmanaged unit test : " + test);                

                try
                {
                    DoTest(test);
                }
                catch (Exception ex)
                {
                    Assert.Fail("unmanaged unit test " + test + " throws an exception: " + ex.Message);
                }
            }            
        }
    }
}
