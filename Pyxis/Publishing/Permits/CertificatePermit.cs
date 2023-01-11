using Pyxis.Contract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Publishing.Permits
{
    class CertificatePermit : IPermit
    {
        public PermitType PermitType
        {
            get { return PermitType.Certificate; }
        }
        public DateTime Issued { get; private set; }
        public DateTime Expires { get; private set; }

        public string Certificate { get; private set; }

        public CertificatePermit(string certificate, DateTime expires, DateTime issued)
        {
            Certificate = certificate;
            Issued = issued;
            Expires = expires;
            if (Expires < Issued)
            {
                Expires = Issued.AddMinutes(5);
            }
        }

        public CertificatePermit(string certificate, DateTime expires)
            : this(certificate, expires, DateTime.UtcNow)
        { }

        public CertificatePermit(string certificate)
            : this(certificate, DateTime.UtcNow.AddDays(1))
        { }
    }
}
