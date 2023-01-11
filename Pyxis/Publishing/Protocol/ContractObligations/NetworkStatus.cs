/******************************************************************************
NetworkStatus.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract.Services.GeoWebStreamService;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    internal class NetworkStatus : INetworkStatus
    {
        public static INetworkStatus CurrentStatus()
        {
            var status = new NetworkStatus();
            //TODO
            return status;
        }

        public float UploadBandWidthKbps { get; set; }

        public float DownLoadBandWidthKbps { get; set; }

        public float AverageUploadUtilizationKbps { get; set; }
    }
}
