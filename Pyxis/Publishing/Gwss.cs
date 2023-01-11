/******************************************************************************
Gwss.cs

begin		: Oct. 21, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol;

namespace Pyxis.Publishing
{
    public class Gwss
    {
        private ILicenseServerClient LS { get; set; }

        public Gwss(string licenseServerUrl)
        {
            LS = new LicenseServerClient(licenseServerUrl);
        }

        public Gwss(ILicenseServerClient licenseServerClient)
        {
            LS = licenseServerClient;
        }

        public ILsStatus UpdateStatus(IGwssStatus status)
        {
            return LS.Servers.UpdateStatus(status);
        }
    }
}
