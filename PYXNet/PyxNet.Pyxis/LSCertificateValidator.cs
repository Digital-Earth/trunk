using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using PyxNet.Service;

namespace PyxNet.Pyxis
{
    public class LSCertificateValidator : ICertificateValidator
    {
        private List<NodeId> m_trustedAuthorities = new List<NodeId>();
        private readonly Object m_trustedAuthoritiesLock = new Object();

        public bool IsCertificateValid(Certificate certificate)
        {
            lock (m_trustedAuthoritiesLock)
            {
                return m_trustedAuthorities.Contains(certificate.Authority.Server);
            }
        }

        public void SetTrustedAuthorities(global::Pyxis.Publishing.Channel channel)
        {
            SetTrustedAuthorities(channel.GetTrustedAuthorities().Nodes.Select(n => new NodeId { Identity = n.Id, PublicKey = new PyxNet.DLM.PublicKey(n.PublicKey.Key) }).ToList());
        }

        public void SetTrustedAuthorities(IEnumerable<NodeId> authorities)
        {
            lock (m_trustedAuthoritiesLock)
            {
                m_trustedAuthorities = new List<NodeId>(authorities);
            }
        }
    }
}
