using System;
using System.Net;
using NUnit.Framework;
using Pyxis.Publishing.Permits;
using Pyxis.Publishing.Protocol;

namespace Pyxis.Publishing.Test
{
    /// <summary>
    /// Tests various combinations of valid and invalid Username, Email and Password for login
    /// </summary>
    [TestFixture]
    [Category("Integration")]   // allows tests marked "Integration" to be included and excluded from test run
    internal class AccessTokenGeneratorTests
    {
        private AccessTokenGenerator m_accessTokenGenerator;

        private string m_validUserName;
        private string m_invalidUserName;

        private string m_validEmail;
        private string m_invalidEmail;

        private string m_validPassword;
        private string m_invalidPassword;

        [SetUp]
        public void SetUp()
        {
            // use development server for testing
            m_accessTokenGenerator = new AccessTokenGenerator(Pyxis.Publishing.ApiUrl.DevelopmentLicenseServerRestAPI);

            m_validUserName = "Pyxis";
            m_invalidUserName = "invalid";

            m_validEmail = "lrakai@pyxisinnovation.com";
            m_invalidEmail = "invalid@invalid.com";

            m_validPassword = "Innovation1";
            m_invalidPassword = "invalid";
        }

        [TearDown]
        public void TearDown()
        {
        }

        [Test]
        public void TestLoginValidUserName()
        {
            var credential = new NetworkCredential(m_validUserName, m_validPassword);
            Assert.DoesNotThrow(delegate { m_accessTokenGenerator.GetAccessToken(credential); });
        }

        [Test]
        public void TestLoginInvalidUserName()
        {
            var credential = new NetworkCredential(m_invalidUserName, m_validPassword);
            Assert.Throws<Exception>(delegate { m_accessTokenGenerator.GetAccessToken(credential); });
        }

        [Test]
        public void TestLoginValidEmail()
        {
            var credential = new NetworkCredential(m_validEmail, m_validPassword);
            Assert.DoesNotThrow(delegate { m_accessTokenGenerator.GetAccessToken(credential); });
        }

        [Test]
        public void TestLoginInvalidEmail()
        {
            var credential = new NetworkCredential(m_invalidEmail, m_validPassword);
            Assert.Throws<Exception>(delegate { m_accessTokenGenerator.GetAccessToken(credential); });
        }

        [Test]
        public void TestLoginValidUserNameInvalidPassword()
        {
            var credential = new NetworkCredential(m_validUserName, m_invalidPassword);
            Assert.Throws<Exception>(delegate { m_accessTokenGenerator.GetAccessToken(credential); });
        }

        [Test]
        public void TestLoginValidEmailInvalidPassword()
        {
            var credential = new NetworkCredential(m_validEmail, m_invalidPassword);
            Assert.Throws<Exception>(delegate { m_accessTokenGenerator.GetAccessToken(credential); });
        }
    }
}
