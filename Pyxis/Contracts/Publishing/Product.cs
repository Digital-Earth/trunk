using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;

namespace Pyxis.Contract.Publishing
{
    public static class ProductSystemTags
    {
        public static string Production { get { return "Production"; } }
        public static string Development { get { return "Development"; } }
    }

    public enum ProductType
    {
        WorldViewStudio,
        GeoWebStreamServer,
        TestProductType
    }

    public enum TransferType
    {
        BlobClientV1,
        Http,
        TestTransferType
    }

    public class Product : Resource
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public ProductType? ProductType { get; set; }

        [JsonConverter(typeof(VersionConverter))]
        public Version ProductVersion { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public TransferType? TransferType { get; set; }

        public string Url { get; set; }

        public string Key { get; set; }

        // for deserializing from string
        public Product()
        {
        }

        public Product(List<LicenseReference> licenses, Metadata metadata, Guid version, ProductType productType, Version productVersion, TransferType transferType, string url, string key)
            : base(ResourceType.Product, licenses, metadata, version)
        {
            ProductType = productType;
            ProductVersion = productVersion;
            TransferType = transferType;
            Url = url;
            Key = key;
        }

        public Product(Product basedOnProduct)
            : base(basedOnProduct)
        {
            ProductType = basedOnProduct.ProductType;
            TransferType = basedOnProduct.TransferType;
            ProductVersion = basedOnProduct.ProductVersion;
            Url = basedOnProduct.Url;
            Key = basedOnProduct.Key;
        }

        public Product(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, ProductType productType, Version productVersion, TransferType transferType, string url, string key)
            : this(licenses, metadata, version, productType, productVersion, transferType, url, key)
        {
            Id = id;
        }
    }
}