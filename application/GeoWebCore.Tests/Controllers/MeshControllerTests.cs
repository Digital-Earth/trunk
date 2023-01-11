using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Controllers;
using NUnit.Framework;

namespace GeoWebCore.Tests.Controllers
{
    [TestFixture]
    internal class MeshControllerTests
    {
        [Test]
        public void PrimeRootMesh()
        {
            var controller = new MeshController();

            var mesh = controller.Mesh("0", 10);
            Assert.AreEqual(mesh.Vertices.Length, 10*10);
        }

        [Test]
        public void SecondResolutionMesh()
        {
            var controller = new MeshController();

            var mesh = controller.Mesh("10", 10);
            Assert.AreEqual(mesh.Vertices.Length, 10 * 10);
        }
    }
}
