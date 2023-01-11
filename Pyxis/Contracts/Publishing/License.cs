using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public enum LicenseType
    {
        /// <summary>
        /// Permits licensed use without trial limitations.
        /// </summary>
        Full,
        /// <summary>
        /// Permits licensed use with trial limitations and can only be agreed to once.
        /// </summary>
        Trial
    }

    public enum PublishingType
    {
        /// <summary>
        /// Permits publishing of derivative works based on the Resource.
        /// </summary>
        Open,
        /// <summary>
        /// Publishing derivative works based on the Resource is not permitted.
        /// </summary>
        Closed
    }

    /// <summary>
    /// A LicenseTyped reference to a License.
    /// Implicitly refers to the latest version of a license.
    /// </summary>
    public class LicenseReference
    {
        public Guid Id { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public LicenseType LicenseType { get; set; }
        public LicenseTrialLimitations Limitations { get; set; }

        public LicenseReference()
        {
        }

        public LicenseReference(Guid id, LicenseType licenseType, LicenseTrialLimitations limitations)
        {
            Id = id;
            LicenseType = licenseType;
            Limitations = new LicenseTrialLimitations(limitations);
        }

        public static LicenseReference ReferenceFromLicense(License license)
        {
            return new LicenseReference(license.Id, license.LicenseType.Value, license.Limitations);
        }
    }

    /// <summary>
    /// A LicenseTyped reference to a License.
    /// Explicitly refers to a version of a license.
    /// </summary>
    public class VersionedLicenseReference : LicenseReference
    {
        public Guid Version { get; set; }

        public VersionedLicenseReference()
        {
        }

        public VersionedLicenseReference(Guid id, LicenseType licenseType, LicenseTrialLimitations limitations, Guid version)
            : base(id, licenseType, limitations)
        {
            Version = version;
        }

        public static VersionedLicenseReference VersionedReferenceFromLicense(License license)
        {
            return new VersionedLicenseReference(license.Id, license.LicenseType.Value, license.Limitations, license.Version);
        }
    }

    public class LicenseTerms
    {
        public string Text { get; set; }
    }

    public class LicenseTrialLimitations
    {
        public LicenseTrialLimitations()
        {
        }

        public LicenseTrialLimitations(LicenseTrialLimitations limitations)
        {
            Area = limitations.Area;
            Resolution = limitations.Resolution;
            Time = limitations.Time;
            Watermark = limitations.Watermark;
        }

        public string Area { get; set; }
        public uint? Resolution { get; set; }
        public TimeSpan? Time { get; set; }
        public string Watermark { get; set; }
    }

    public class LicenseExportFormats
    {
        public string CSV { get { return "CSV"; } }
        public string GeoTIFF { get { return "GeoTIFF"; } }
        public string Shapefile { get { return "Shapefile"; } }
        public string GeoJSON { get { return "GeoJSON"; } }
    }

    public class LicenseExportOptions
    {
        public LicenseExportOptions()
        {
        }

        public LicenseExportOptions(LicenseExportOptions exportOptions)
        {
            Formats = exportOptions.Formats == null ? null : new List<string>(exportOptions.Formats);
            AllowReport = exportOptions.AllowReport;
        }

        public List<string> Formats { get; set; }
        public bool? AllowReport { get; set; }
    }

    public class License : Resource
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public LicenseType? LicenseType { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public PublishingType? PublishingType { get; set; }
        public LicenseTerms Terms { get; set; }
        public LicenseTrialLimitations Limitations { get; set; }
        public LicenseExportOptions ExportOptions { get; set; }

        // for deserializing from string
        public License()
        {
        }

        public License(List<LicenseReference> licenses, Metadata metadata, Guid version, LicenseTerms terms, LicenseType licenseType, PublishingType publishingType, LicenseTrialLimitations limitations, LicenseExportOptions exportOptions)
            : base(ResourceType.License, licenses, metadata, version)
        {
            LicenseType = licenseType;
            PublishingType = publishingType;
            Terms = terms;
            Limitations = new LicenseTrialLimitations(limitations);
            ExportOptions = new LicenseExportOptions(exportOptions);
        }

        public License(License basedOnLicense)
            : base(basedOnLicense)
        {
            LicenseType = basedOnLicense.LicenseType;
            PublishingType = basedOnLicense.PublishingType;
            Terms = basedOnLicense.Terms;
            Limitations = new LicenseTrialLimitations(basedOnLicense.Limitations);
            ExportOptions = new LicenseExportOptions(basedOnLicense.ExportOptions);
        }

        public License(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, LicenseTerms terms, LicenseType licenseType, PublishingType publishingType, LicenseTrialLimitations limitations, LicenseExportOptions exportOptions)
            : this(licenses, metadata, version, terms, licenseType, publishingType, limitations, exportOptions)
        {
            Id = id;
        }
    }
}