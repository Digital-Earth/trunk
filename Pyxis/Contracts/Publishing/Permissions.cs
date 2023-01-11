using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public static class PermissionFormats
    {
        public static string PyxNetV1 { get { return "application/pyxnet-v1"; } }
    }

    public class PyxNetNodeId
    {
        public Guid Id { get; set; }
        public PyxNetNodeId.PyxNetPublicKey PublicKey { get; set; }

        public class PyxNetPublicKey
        {
            public byte[] Key { get; set; }
        }
    }

    public class PyxNetPermissionRequest
    {
        public string Format { get; set; }
        public List<Guid> ResourceIds { get; set; }
        public PyxNetNodeId NodeId { get; set; }
    }

    public class CertificatePermissionGrant
    {
        public List<CertificatePermit> Permits;
        public List<DeniedPermit> NotGranted;
    }
    
    public class CertificatePermit
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public Pyxis.Contract.PermitType PermitType { get { return Contract.PermitType.Certificate; } }
        public DateTime Issued { get; set; }
        public DateTime Expires { get; set; }
        public string Format { get; set; }
        public Guid ResourceId { get; set; }
        public string Certificate { get; set; }
    }

    public class KeyPermissionGrant
    {
        public List<KeyPermit> Permits;
        public List<DeniedPermit> NotGranted;
    }

    public class KeyPermit
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public Pyxis.Contract.PermitType PermitType { get { return Contract.PermitType.ExternalApiKey; } }
        public DateTime Issued { get; set; }
        public DateTime Expires { get; set; }
        public Guid ResourceId { get; set; }
        public string Key { get; set; }
    }

    public class DeniedPermit
    {
        public Guid ResourceId { get; set; }
        public string Message { get; set; }
    }

    [Obsolete("Use PermissionGrant instead")]
    public class DeprecatedPermissionGrant
    {
        public List<DeprecatedPermit> Permits;
        public List<DeprecatedDeniedPermit> NotGranted;
    }

    [Obsolete("Use Permit instead")]
    public class DeprecatedPermit
    {
        public string Format { get; set; }
        public Guid ResourceId { get; set; }
        public string Certificate { get; set; }
    }

    [Obsolete("Use DeniedPermit instead")]
    public class DeprecatedDeniedPermit
    {
        public Guid ResourceId { get; set; }
        public string Message { get; set; }
    }

    public class TrustedNodes
    {
        public List<PyxNetNodeId> Nodes { get; set; }
    }

    public class LicensedAccess
    {
        public bool HasAccess { get; set; }
        public List<VersionedLicenseReference> UsingLicenses { get; set; }
    }
}
