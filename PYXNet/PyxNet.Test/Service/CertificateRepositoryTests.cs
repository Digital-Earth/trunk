using System.Collections.Generic;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    /// <summary>
    /// Unit tests for CertificateRepository
    /// </summary>
    [TestFixture]
    public class CertificateRepositoryTests
    {
        [Test]
        public void DatabaseCreation()
        {
            // Docs say this will throw if file not found, but doesn't seem to.
            System.IO.File.Delete("TestRepository.db3");

            try
            {
                using (CertificateRepository r = new CertificateRepository("TestRepository.db3"))
                {
                    // Create a certificate.
                    Certificate c = CertificateTests.CreateCertificate( 1000);

                    // Ensure that it isn't in the repository yet!
                    NUnit.Framework.Assert.IsNull(
                        r.GetServiceInstanceFact(c.ServiceInstance.ServiceInstanceId),
                        "Empty repository should not contain this service-instance-id.");
                    NUnit.Framework.Assert.IsNull(
                        r.GetServiceInstanceFact(c.ServiceInstance.ServiceId),
                        "Empty repository should not contain this service-id.");

                    // Add the repository, then make sure it's there.
                    r.Add(c);
                    NUnit.Framework.Assert.IsNotNull(
                        r.GetServiceInstanceFact(c.ServiceInstance.ServiceInstanceId),
                        "Repository should contain this service-instance-id.");
                    NUnit.Framework.Assert.IsNotNull(
                        r.GetServiceInstanceFact(c.ServiceInstance.ServiceId),
                        "Repository should contain this service-id.");

                    // Add a second certificate to the repository.
                    Certificate second = CertificateTests.CreateCertificate(
                        c.ServiceInstance.ServiceId, 1000);
                    NUnit.Framework.Assert.AreEqual(
                        c.ServiceInstance.ServiceId,
                        second.ServiceInstance.ServiceId,
                        "The two certificates must share the same service id.");
                    r.Add(second);
                    NUnit.Framework.Assert.IsNotNull(
                        r.GetServiceInstanceFact(second.ServiceInstance.ServiceInstanceId),
                        "Repository should contain this service-instance-id.");
                    NUnit.Framework.Assert.IsNotNull(
                        r.GetServiceInstanceFact(second.ServiceInstance.ServiceId),
                        "Repository should contain this second service-instance-id.");
                    NUnit.Framework.Assert.AreEqual(
                        2, Count(r[c.ServiceInstance.ServiceId]), 
                        "Repository should contain two copies of this service-id.");

                    // Remove the second certificate from the repository.
                    r.Remove(second);
                    NUnit.Framework.Assert.IsNull(
                        r.GetServiceInstanceFact(second.ServiceInstance.ServiceInstanceId),
                        "Repository should not contain this service-instance-id.");
                    NUnit.Framework.Assert.AreEqual(
                        1, Count(r[c.ServiceInstance.ServiceId]), 
                        "Repository should not contain this second service-instance-id.");
                }
            }
            finally
            {
                Assert.IsTrue(System.IO.File.Exists("TestRepository.db3"));
                System.IO.File.Delete("TestRepository.db3");
            }
            Assert.IsFalse(System.IO.File.Exists("TestRepository.db3"));
        }

        /// <summary>
        /// Returns a count of the elements in iEnumerable.
        /// </summary>
        /// <param name="iEnumerable">The collection.</param>
        /// <returns></returns>
        private int Count<ObjectType>(IEnumerable<ObjectType> iEnumerable)
        {
            int count = 0;
            foreach (ObjectType temp in iEnumerable)
            {
                ++count;
            }
            return count;
        }
    }
}