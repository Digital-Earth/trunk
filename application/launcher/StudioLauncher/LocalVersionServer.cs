using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;
using StudioLauncher.Properties;

namespace StudioLauncher
{
    public class LocalVersionServer : IVersionServer
    {
        public Product GetLatestVersion(ProductType productType)
        {
            if (productType == ProductType.WorldViewStudio)
            {
                if (!string.IsNullOrEmpty(Settings.Default.LocalStudioVersion))
                {
                    return new Product()
                    {
                        ProductType = productType,
                        ProductVersion = new Version(Settings.Default.LocalStudioVersion)
                    };
                }
                return null;
            }
            throw new Exception("Can not get the latest version. Unsupported Product: " + productType);
        }

        public void SetLatestVersion(Product product)
        {
            if (product.ProductType == ProductType.WorldViewStudio)
            {
                Settings.Default.LocalStudioVersion = product.ProductVersion == null
                    ? ""
                    : product.ProductVersion.ToString();
                Settings.Default.Save();
                return;
            }
            throw new Exception("Can not set the Latest version setting. Unsupported product " + product.ProductType);
        }

        /// <summary>
        /// Gets all the product versions for a given product type.
        /// </summary>
        /// <param name="productType">The product type.</param>
        /// <returns>The the latest version.</returns>
        public List<Product> GetAllVersions(ProductType productType)
        {
            var products = new List<Product>();

            Product product = GetLatestVersion(productType);
            if (product != null)
            {
                products.Add(product);
            }

            return products;
        }
    }
}