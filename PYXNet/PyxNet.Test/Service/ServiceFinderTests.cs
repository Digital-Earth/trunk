using System;
using NUnit.Framework;
using Pyxis.Utilities.Test;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    /// <summary>
    /// Unit tests for ServiceFinder
    /// </summary>
    [TestFixture]
    public class ServiceFinderTests
    {
        [Test]
        public void IntegrationTest()
        {
            using (PyxNet.Test.StackTestHelper stacks = new PyxNet.Test.StackTestHelper(
                PyxNet.Test.StackTestHelper.Topology.One,
                PyxNet.Test.StackTests.ShortInterval))
            {

                stacks.Stacks[0].Tracer.Enabled = true;
                stacks.Stacks[1].Tracer.Enabled = true;

                // Create a finder, hosted on stack-1
                ServiceFinder finder = new ServiceFinder(stacks.Stacks[1]);

                // Create a new certificate.
                Certificate publishedCertificate =
                    CertificateTests.CreateCertificate(600);
                ////NUnit.Framework.Assert.IsNull(finder.FindService(
                ////    publishedCertificate.ServiceInstance.ServiceId, queryTimeout),
                ////    "Should not be able to find service before it is published.");

                // Publish the certificate.
                stacks.Stacks[0].CertificateRepository.Add(publishedCertificate);

                // Wait for the qht to update itself...
                TimedTest.Verify(delegate() { return !stacks.Stacks[0].IsQueryHashTableDirty; },
                    TimeSpan.FromSeconds( 10),
                    "Stack was unable to update its Query Hash Table.");

                ////// Now that the service is published, find it.
                ////// First, check to make sure we don't already know about it.
                ////NUnit.Framework.Assert.IsNull(finder.FindService(
                ////    publishedCertificate.ServiceInstance.ServiceId),
                ////    "Should not be able to find service locally.");
                ////// Then check to make sure we can find it over the network.
                ////ServiceFinder finder2 = new ServiceFinder(stacks.Stacks[1]);
                ////ServiceInstance foundService = finder2.FindService(
                ////    publishedCertificate.ServiceInstance.ServiceId, queryTimeout);
                ////NUnit.Framework.Assert.IsNotNull(foundService,
                ////    "Unable to find a published service across the test network.");
                ////NUnit.Framework.Assert.IsNotNull(ServiceFinder.FindServer(stacks.Stacks[1],
                ////    publishedCertificate.ServiceInstance.ServiceId),
                ////    "Unable to find a published service locally.");
            }
        }
    }
}