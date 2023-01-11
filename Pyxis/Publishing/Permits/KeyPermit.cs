using Pyxis.Contract;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Publishing.Permits
{
    public class ExternalApiKeyPermit : IPermit
    {
        public PermitType PermitType
        {
            get { return PermitType.ExternalApiKey; }
        }
        public DateTime Issued { get ; private set; }
        public DateTime Expires { get ; private set; }

        public SecureString Key { get; private set; }

        public ExternalApiKeyPermit(SecureString key, DateTime expires, DateTime issued)
        {
            Key = key;
            Issued = issued;
            Expires = expires;
            if (Expires < Issued)
            {
                Expires = Issued.AddMinutes(5);
            }
        }

        public ExternalApiKeyPermit(SecureString key, DateTime expires)
            : this(key, expires, DateTime.UtcNow)
        { }

        public ExternalApiKeyPermit(SecureString key)
            : this(key, DateTime.UtcNow.AddDays(1))
        { }

        public ExternalApiKeyPermit(string key, DateTime expires, DateTime issued)
        {
            Key = new SecureString();
            foreach (var c in key)
            {
                Key.AppendChar(c);
            }
            Issued = issued;
            Expires = expires;
            if (Expires < Issued)
            {
                Expires = Issued.AddMinutes(5);
            }
        }

        public ExternalApiKeyPermit(string key, DateTime expires)
            : this(key, expires, DateTime.UtcNow)
        { }

        public ExternalApiKeyPermit(string key)
            : this(key, DateTime.UtcNow.AddDays(1))
        { }

    }
}
