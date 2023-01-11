using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PyxNet.Pyxis
{
    public class CertificateRetainer
    {
        private Service.Certificate m_certificate;
        private readonly Object m_certificateLock = new Object();

        private static TimeSpan s_earlyCertificateExpirationOffset = TimeSpan.FromMinutes(10);
        private static TimeSpan s_noResourceRequestRetry = TimeSpan.FromMinutes(10);

        private Stack m_stack;
        private string m_procRefString;
        private DateTime m_lastRequested = DateTime.MinValue;

        public CertificateRetainer(Stack stack, string procRefString)
        {
            m_stack = stack;
            m_procRefString = procRefString;
        }

        public Service.Certificate GetCertificate()
        {
            lock (m_certificateLock)
            {
                if (m_certificate == null || (m_certificate.ExpireTime > DateTime.UtcNow - s_earlyCertificateExpirationOffset && !m_certificate.Valid))
                {
                    m_certificate = null;

                    if ((DateTime.Now - m_lastRequested) > s_noResourceRequestRetry)
                    {
                        try
                        {
                            var resource = m_stack.GetChannel().GetResourceByProcRef(m_procRefString);
                            m_lastRequested = DateTime.Now;
                            if (resource != null)
                            {
                                var fact = new List<Service.ICertifiableFact>() { new Service.GeoSourcePermissionFact(new Service.ResourceId(resource.Id), null, m_stack.NodeInfo.NodeId, m_procRefString) };
                                m_certificate = m_stack.CertificateProvider.GetCertificate(fact).FirstOrDefault();
                            }
                        }
                        catch (Exception e)
                        {
                            Trace.error("Failed to get certificate for " + m_procRefString + " due to error: " + e.Message);
                        }
                    }
                }
            }
            return m_certificate;
        }
    }
}
