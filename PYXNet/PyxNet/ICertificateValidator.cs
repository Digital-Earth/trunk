using System.Collections.Generic;
using PyxNet.Service;

namespace PyxNet
{
    public interface ICertificateValidator
    {
        bool IsCertificateValid(Certificate certificate);
    }

    public class PermissiveCertificateValidator : ICertificateValidator
    {
        public bool IsCertificateValid(Certificate certificate)
        {
            return true;
        }
    }
}
