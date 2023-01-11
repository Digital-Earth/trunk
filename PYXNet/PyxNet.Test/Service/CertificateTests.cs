using System;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    //[TestFixture]
    public class CertificateTests
    {
        Stack authorityStack;
        ServiceInstance authority;
        ServiceInstance serviceInstance;

        private void InitializeTest(ServiceId service)
        {
            // Create an authority to authorize the certificate.
            authorityStack = PyxNet.Test.StackTestHelper.CreateStack("Authority", -1, false);

            /// Note that we don't care what node authorized us, or whether 
            /// or not they themselves are authorized.  
            ServiceId authorizingService = new ServiceId();
            authority = ServiceInstance.Create(authorizingService,
                authorityStack.NodeInfo.NodeId);

            // Create a "random" service provider.  We don't care who this is.
            NodeId serviceProvider = new NodeId();
            serviceProvider.PublicKey = new PyxNet.DLM.RSATool().PublicKey;

            // Create a service instance for that provider.
            serviceInstance = ServiceInstance.Create(service, serviceProvider);
        }

        private void InitializeTest()
        {
            InitializeTest(new ServiceId());
        }

        /// <summary>
        /// Creates a certificate that will expire in timeout seconds.
        /// </summary>
        /// <param name="timeout">Lifetime of certificate, in seconds.</param>
        /// <returns>The certificate.</returns>
        private Certificate CreateCertificateHelper(int timeout)
        {
            return ServiceInstanceCertificateHelper.Create(authorityStack.PrivateKey,
                authority, DateTime.Now.AddSeconds(timeout), serviceInstance);
        }

        /// <summary>
        /// Creates a certificate that will expire in timeout seconds.  This is
        /// used for testing purposes only!
        /// </summary>
        /// <param name="timeout">Lifetime of certificate, in seconds.</param>
        /// <returns>The certificate.</returns>
        public static Certificate CreateCertificate(int timeout)
        {
            CertificateTests helper = new CertificateTests();
            helper.InitializeTest();
            return helper.CreateCertificateHelper(timeout);
        }

        /// <summary>
        /// Creates a certificate that will expire in timeout seconds.  This is
        /// used for testing purposes only!
        /// </summary>
        /// <param name="timeout">Lifetime of certificate, in seconds.</param>
        /// <returns>The certificate.</returns>
        public static Certificate CreateCertificate(ServiceId serviceId, int timeout)
        {
            CertificateTests helper = new CertificateTests();
            helper.InitializeTest(serviceId);
            return helper.CreateCertificateHelper(timeout);
        }

        [Test]
        public void GeneralOperation()
        {
            InitializeTest();

            /// Certify the service instance.  Since signing and verifying 
            /// each take about .2 seconds, we will make a 2 second certificate.
            Certificate c = CreateCertificateHelper(2);

            // Check that the service is valid.
            if (!c.Valid)
            {
                // The certificate is invalid, but this may be because it 
                // has already timed out(!!!).  Create a much longer lived
                // certificate.  This absolutely must be valid, or else we
                // know that our certificates don't work.
                Certificate longTimer = CreateCertificateHelper(2000);
                Assert.Greater(longTimer.ExpireTime, DateTime.Now,
                    "Internal failure: it took more than 2000 seconds to create a signature!");
                Assert.IsTrue(longTimer.Valid,
                    "This timer lasts a long time, and should still be valid.");
            }

            // Test for timeout.
            TimeSpan timeRemaining = c.ExpireTime - DateTime.Now;
            if (timeRemaining.TotalMilliseconds > 0)
            {
                System.Threading.Thread.Sleep(c.ExpireTime - DateTime.Now);
            }
            System.Threading.Thread.Sleep(15);
            Assert.IsFalse(c.Valid,
                "This timer has expired, and should not be valid.");

            string certificateDescription = c.ToString();
            System.Diagnostics.Trace.WriteLine(certificateDescription);

            // TODO: Test for invalid signature.  Hard to do in the current system!
        }

        [Test]
        public void Serialization()
        {
            InitializeTest();

            // Create a certificate.
            Certificate c = CreateCertificateHelper(1000);

            // Serialize it
            Message m = c.ToMessage();

            // De-serialize
            Certificate extractedCertificate = new Certificate(
                new MessageReader(m));

            // Test that they are equal.
            Assert.AreEqual(c.ServiceInstance.ServiceInstanceId,
                extractedCertificate.ServiceInstance.ServiceInstanceId,
                "Extracted certificate must match original (service instance id mismatch).");
            Assert.AreEqual(c.ServiceInstance.ServiceId,
                extractedCertificate.ServiceInstance.ServiceId,
                "Extracted certificate must match original (service id mismatch).");
            Assert.AreEqual(c.ServiceInstance.Server,
                extractedCertificate.ServiceInstance.Server,
                "Extracted certificate must match original (server mismatch).");

            // Test that it is good.
            Assert.IsTrue(extractedCertificate.Valid,
                "Extracted message should be verifiable.");
        }

        [Test]
        public void DatabaseSerialization()
        {
            InitializeTest();

            // Create a certificate.
            Certificate c = CreateCertificateHelper(1000);

            // Serialize it
            System.Data.DataTable t = Certificate.CreateDataTable("DummyTable");
            System.Data.DataRow r = t.NewRow(); ;
            c.ToRow(r);

            // De-serialize
            Certificate extractedCertificate = new Certificate(r);

            Assert.AreEqual(c.ExpireTime, extractedCertificate.ExpireTime,
                "Extracted certificate must match original.");
            Assert.AreEqual(c.ServiceInstance.ServiceInstanceId,
                extractedCertificate.ServiceInstance.ServiceInstanceId,
                "Extracted certificate must match original.");

            // Test that it is good.
            Assert.IsTrue(extractedCertificate.Valid,
                "Extracted message should be verifiable.");
        }

        //[Test]
        //[ExpectedException( typeof(ArgumentException))]
        //public void DatabaseSerializationErrorHandling()
        //{
        //    InitializeTest();

        //    // Create a certificate.
        //    CertificateBase<ServiceInstance> c = CreateCertificateHelper(1000);

        //    // Serialize it
        //    System.Data.DataTable t = CertificateBase<ServiceInstance>.CreateDataTable("DummyTable");
        //    System.Data.DataRow r = t.NewRow(); ;
        //    c.ToRow(r);

        //    // Damage the row!
        //    r[CertificateBase<ServiceInstance>.IndexColumnName] = new ServiceId().Guid;

        //    // De-serialize.  This will throw.
        //    CertificateBase<ServiceInstance> extractedCertificate = new CertificateBase<ServiceInstance>(r);
        //}
    }
}