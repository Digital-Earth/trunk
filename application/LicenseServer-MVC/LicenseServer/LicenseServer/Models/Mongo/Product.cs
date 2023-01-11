using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class Product : Pyxis.Contract.Publishing.Product, IMongoResource
    {
        // for deserializing from string
        public Product()
        {
        }

        public Product(List<LicenseReference> licenses, Metadata metadata, Guid version, ProductType productType, Version productVersion, TransferType transferType, string url, string key)
            : base(licenses, metadata, version, productType, productVersion, transferType, url, key)
        {
        }

        public Product(Product basedOnProduct)
            : base(basedOnProduct)
        {
        }

        public Product(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, ProductType productType, Version productVersion, TransferType transferType, string url, string key)
            : base(id, licenses, metadata, version, productType, productVersion, transferType, url, key)
        {
        }
    }
}