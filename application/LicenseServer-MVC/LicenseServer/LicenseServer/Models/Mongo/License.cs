using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class License : Pyxis.Contract.Publishing.License, IMongoResource
    {
        // for deserializing from string
        public License()
        {
        }

        public License(List<LicenseReference> licenses, Metadata metadata, Guid version, LicenseTerms terms, LicenseType licenseType, PublishingType publishingType, LicenseTrialLimitations limitations, LicenseExportOptions exportOptions)
            : base(licenses, metadata, version, terms, licenseType, publishingType, limitations, exportOptions)
        {
        }

        public License(License basedOnLicense)
            : base(basedOnLicense)
        {
        }

        public License(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, LicenseTerms terms, LicenseType licenseType, PublishingType publishingType, LicenseTrialLimitations limitations, LicenseExportOptions exportOptions)
            : base(id, licenses, metadata, version, terms, licenseType, publishingType, limitations, exportOptions)
        {
        }
    }
}