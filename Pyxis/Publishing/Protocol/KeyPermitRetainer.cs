using Pyxis.Publishing.Permits;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Publishing.Protocol
{ 
    internal class KeyPermitRetainer : PermitRetainerBase<ExternalApiKeyPermit>
    {
        private UserClient m_userClient;
        private Guid m_keyGroup;

        public KeyPermitRetainer(UserClient userClient, Guid keyGroup)
        {
            m_userClient = userClient;
            m_keyGroup = keyGroup;
        }

        protected override ExternalApiKeyPermit DoGetPermit()
        {
            return m_userClient.RequestKeyPermit(m_keyGroup);
        }

        protected override void DoReleasePermit(ExternalApiKeyPermit permit)
        {
            m_userClient.ReleaseKeyPermit(m_keyGroup,permit);
        }
    }
}
