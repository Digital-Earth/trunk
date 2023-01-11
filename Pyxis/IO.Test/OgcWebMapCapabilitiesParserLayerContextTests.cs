using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NUnit.Framework;
using Pyxis.IO.Sources.OGC;

namespace Pyxis.IO.Test
{
    [TestFixture]
    class OgcWebMapCapabilitiesParserLayerContextTests
    {
        [Test]
        public void LayerContextBorrowFormat()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.Format = "image/png";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual("image/png",child.Format);
        }

        [Test]
        public void LayerContextFormatGetsTrimmed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.Format = "  \t  image/png  \n ";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual("image/png", context.Format);
            Assert.AreEqual("image/png", child.Format);
        }

        [Test]
        public void ChildLayerContextOverwriteFormat()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.Format = "image/png";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();
            child.Format = "image/jpeg";

            Assert.AreEqual("image/jpeg", child.Format);
            Assert.AreEqual("image/png", context.Format);
        }

        [Test]
        public void SettingNullValueToFormatNotOverwriteParent()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.Format = "image/png";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            //empty string
            child.Format = "";
            Assert.AreEqual("image/png", child.Format);

            //nullvalue
            child.Format = null;
            Assert.AreEqual("image/png", child.Format);

            //long whitespace
            child.Format = "     ";
            Assert.AreEqual("image/png", child.Format);
        }

        [Test]
        public void LayerContextBorrowLatLonBBox()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.LatLonBBox = "<xml stuff here>";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual("<xml stuff here>", child.LatLonBBox);
        }

        [Test]
        public void LayerContextLatLonBBoxGetsTrimmed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.LatLonBBox = "\t    <xml stuff here>  \n   ";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual("<xml stuff here>", child.LatLonBBox);
        }

        [Test]
        public void ChildLayerContextOverwriteLatLonBBox()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.LatLonBBox = "<xml stuff here>";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();
            child.LatLonBBox = "<other xml stuff here>";

            Assert.AreEqual("<other xml stuff here>", child.LatLonBBox);
            Assert.AreEqual("<xml stuff here>", context.LatLonBBox);
        }

        [Test]
        public void SettingNullValueToLatLonBBoxNotOverwriteParent()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.LatLonBBox = "<xml stuff here>";

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            //empty string
            child.LatLonBBox = "";
            Assert.AreEqual("<xml stuff here>", child.LatLonBBox);

            //nullvalue
            child.LatLonBBox = null;
            Assert.AreEqual("<xml stuff here>", child.LatLonBBox);

            //long whitespace
            child.LatLonBBox = "     ";
            Assert.AreEqual("<xml stuff here>", child.LatLonBBox);
        }

        [Test]
        public void StylesGetBorrowed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddStyle("lines");
            context.AddStyle("dots");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual(new [] {"lines","dots"},child.Styles);
        }

        [Test]
        public void StylesGetConcatenatedInRightOrder()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddStyle("lines");
            context.AddStyle("dots");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            child.AddStyle("points");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "points", "lines", "dots" }, child.Styles);
        }

        [Test]
        public void BorrowedDiplicateStylesAreRemoved()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddStyle("lines");
            context.AddStyle("dots");

            //thils will be overwriten by child
            context.AddStyle("points");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            child.AddStyle("points");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "points", "lines", "dots" }, child.Styles);
        }

        [Test]
        public void DuplicateStylesAreNotInstertedTwice()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddStyle("lines");
            context.AddStyle("dots");

            //will be ignored
            context.AddStyle("lines");
            context.AddStyle("dots");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            child.AddStyle("points");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "points", "lines", "dots" }, child.Styles);
        }

        [Test]
        public void StylesGetTrimmed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddStyle("lines   \n  \t");
            context.AddStyle("\t \n   dots");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "lines", "dots" }, context.Styles);
        }

        [Test]
        public void EmptyStylesNotGetAdded()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddStyle("lines");
            context.AddStyle("");
            context.AddStyle(" ");
            context.AddStyle("\t");
            context.AddStyle("\n");
            context.AddStyle(null);
            context.AddStyle("dots");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "lines", "dots" }, context.Styles);
        }

        [Test]
        public void CrsGetBorrowed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddCrs("epsg:1234");
            context.AddCrs("epsg:4567");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "epsg:1234", "epsg:4567" }, child.Crs);
        }

        [Test]
        public void CrsGetTrimmed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddCrs("\t   \n epsg:1234   ");
            context.AddCrs("epsg:4567 \t   \n");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "epsg:1234", "epsg:4567" }, child.Crs);
        }

        [Test]
        public void EmptyCrsNotGetAdded()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddCrs("epsg:1234");
            context.AddCrs(null);
            context.AddCrs("  ");
            context.AddCrs("");
            context.AddCrs("\t");
            context.AddCrs("\n");
            context.AddCrs("epsg:4567");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "epsg:1234", "epsg:4567" }, child.Crs);
        }

        [Test]
        public void CrsGetReturnedInOrder()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddCrs("epsg:1234");
            context.AddCrs("epsg:4567");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            child.AddCrs("epsg:12");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "epsg:12", "epsg:1234", "epsg:4567" }, child.Crs);
        }

        [Test]
        public void DuplicatedBorrowedCrsGetRemoved()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddCrs("epsg:1234");
            context.AddCrs("epsg:4567");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            child.AddCrs("epsg:12");
            child.AddCrs("epsg:4567");

            //please notes, child style should appear first
            Assert.AreEqual(new[] { "epsg:12",  "epsg:4567", "epsg:1234" }, child.Crs);
        }

        [Test]
        public void DuplicatedCrsNotGetAdded()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();

            context.AddCrs("epsg:1234");
            context.AddCrs("epsg:4567");

            //gets ignored
            context.AddCrs("epsg:1234");
            context.AddCrs("epsg:4567");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            child.AddCrs("epsg:12");
            
            //please notes, child style should appear first
            Assert.AreEqual(new[] { "epsg:12", "epsg:1234", "epsg:4567" }, child.Crs);
        }

        [Test]
        public void BBoxGetBorrowed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.AddBBox("epsg:1","<xml1>");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual("<xml1>", child.GetBBox("epsg:1"));
        }

        [Test]
        public void BBoxGetTrimmed()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.AddBBox("epsg:1   ", "<xml1>");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();

            Assert.AreEqual("<xml1>", child.GetBBox("\n \t epsg:1"));
        }

        [Test]
        public void BBoxGetOverwritten()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.AddBBox("epsg:1", "<xml1>");
            context.AddBBox("epsg:2", "<xml2>");

            //create double child to ensure borrow works
            var child = context.CreateChildContext().CreateChildContext();
            child.AddBBox("epsg:1", "<child-xml1>");


            Assert.AreEqual("<child-xml1>", child.GetBBox("epsg:1"));
            Assert.AreEqual("<xml2>", child.GetBBox("epsg:2"));

            Assert.AreEqual("<xml1>", context.GetBBox("epsg:1"));
        }

        [Test]
        public void FindValidBBoxCrcReturnClosestCrsFirst()
        {
            var context = new OgcWebMapCapabilitiesParser.LayerContext();
            context.AddBBox("epsg:1", "<xml1>");
            context.AddBBox("epsg:2", "<xml2>");

            var child = context.CreateChildContext();
            var grandchild = child.CreateChildContext();
            
            grandchild.AddBBox("epsg:3", "<child-xml1>");


            Assert.AreEqual("epsg:3", grandchild.FindValidBBoxCrs());
            Assert.AreEqual("epsg:1", child.FindValidBBoxCrs());
            Assert.AreEqual("epsg:1", context.FindValidBBoxCrs());
        }
    }
}
