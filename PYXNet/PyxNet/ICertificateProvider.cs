using PyxNet.Service;
using System.Collections.Generic;

namespace PyxNet
{
    public interface ICertificateProvider
    {
        IEnumerable<Certificate> GetCertificate(IEnumerable<ICertifiableFact> facts);
    }

    public class NullCertificateProvider : PyxNet.ICertificateProvider
    {
        public IEnumerable<Certificate> GetCertificate(IEnumerable<ICertifiableFact> facts)
        {
            yield break;
        }
    }
}