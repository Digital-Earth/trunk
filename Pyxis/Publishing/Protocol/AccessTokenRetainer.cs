using Pyxis.Publishing.Permits;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Publishing.Protocol
{
    internal class AccessTokenRetainer : PermitRetainerBase<AccessToken>
    {
        private AccessTokenGenerator m_tokenGenerator;
        private NetworkCredential m_credential;

        public AccessTokenRetainer(AccessTokenGenerator tokenGenerator, NetworkCredential credential)
        {
            m_tokenGenerator = tokenGenerator;
            m_credential = credential;
        }

        protected override AccessToken DoGetPermit()
        {
            return m_tokenGenerator.GetAccessToken(m_credential);
        }

        protected override void DoReleasePermit(AccessToken permit)
        {
            //do nothing...
        }
    }

    internal class NonRenewableAccessTokenRetainer : PermitRetainerBase<AccessToken>
    {
        private AccessToken m_accessToken;

        public NonRenewableAccessTokenRetainer(AccessToken.TokenDetails tokenDetails)
        {
            m_accessToken = new AccessToken(tokenDetails);
        }

        protected override AccessToken DoGetPermit()
        {
            return m_accessToken;
        }

        protected override void DoReleasePermit(AccessToken permit)
        {
            //do nothing...
        }
    }
}
