using Pyxis.Contract.Publishing;
using PyxNet.Service;
using System.Collections.Generic;
using System.Linq;

namespace PyxNet.Pyxis
{
    public class LSCertificateProvider : PyxNet.ICertificateProvider
    {
        private global::Pyxis.Publishing.User m_user;
        private Stack m_stack;

        public LSCertificateProvider(global::Pyxis.Publishing.User user, Stack stack)
        {
            m_user = user;
            m_stack = stack;
        }

        public IEnumerable<Certificate> GetCertificate(IEnumerable<ICertifiableFact> facts)
        {
            var geoSourcePermissionFacts = facts.Where(x => x is GeoSourcePermissionFact).Select(x => x as PyxNet.Service.GeoSourcePermissionFact);
            var resources = geoSourcePermissionFacts.Select(x => x.ResourceId.Guid).ToList();
            var pyxnetNodeId = new PyxNetNodeId() { Id = m_stack.NodeInfo.NodeId.Identity, PublicKey = new PyxNetNodeId.PyxNetPublicKey() { Key = m_stack.NodeInfo.NodeId.PublicKey.Key } };
            return m_user.GetPyxNetCertificate(resources, pyxnetNodeId).Permits.Select(x => DeserializeCertificate(x.Certificate));
        }

        private Certificate DeserializeCertificate(string certificateString)
        {
            var certificate = new Certificate();
            certificate.SerializationString = certificateString;
            return certificate;
        }
    }
}