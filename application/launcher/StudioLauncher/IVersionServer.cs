using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace StudioLauncher
{
    public interface IVersionServer
    {
        /// <summary>
        /// Gets the latest product version for a given product type.
        /// </summary>
        /// <param name="productType">The product type.</param>
        /// <returns>The product or null if no latest version is available.</returns>
        Product GetLatestVersion(ProductType productType);

        /// <summary>
        /// Sets the latest product version available for a given product.
        /// </summary>
        /// <param name="product">The product.</param>
        void SetLatestVersion(Product product);

        /// <summary>
        /// Gets all the product versions for a given product type.
        /// </summary>
        /// <param name="productType">The product type.</param>
        /// <returns>The list of products sorted by descending version number.</returns>
        List<Product> GetAllVersions(ProductType productType);
    }
}
