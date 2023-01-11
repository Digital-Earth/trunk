using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.Publishing
{
    public enum DecisionType
    {
        Agree,
        Disagree
    }

    /// <summary>
    /// Represents a User's (dis)agreement with the terms of a License.
    /// Agreements expire based on license time limitations or until the 
    /// terms of the license agreement are in force (usually when the
    /// User's subscription runs out).
    /// </summary>
    public class Agreement : ResourceActivity
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public DecisionType Decision { get; set; }
        public DateTime Expiration { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public LicenseType LicenseType { get; set; }
        public Guid LicenseVersion { get; set; }
        public LicenseTrialLimitations Limitations { get; set; }

        // for deserializing from string
        public Agreement()
        {
        }

        public Agreement(VersionedLicenseReference licenseReference, UserInfo user, DecisionType decision, DateTime expiration)
            : base(licenseReference.Id, ActivityType.Agreement, user)
        {
            Decision = decision;
            LicenseVersion = licenseReference.Version;
            Expiration = expiration;
            LicenseType = licenseReference.LicenseType;
            Limitations = new LicenseTrialLimitations(licenseReference.Limitations);
        }

        public Agreement(Agreement agreement)
            : base(agreement)
        {
            Decision = agreement.Decision;
            LicenseVersion = agreement.LicenseVersion;
            Expiration = agreement.Expiration;
            LicenseType = agreement.LicenseType;
            Limitations = new LicenseTrialLimitations(agreement.Limitations);
        }
    }
}
