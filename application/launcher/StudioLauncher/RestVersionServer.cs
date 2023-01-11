using System;
using System.Collections.Generic;
using System.Net;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using RestSharp;

namespace StudioLauncher
{
    /// <summary>
    /// Gets product information from a server.
    /// </summary>
    public class RestVersionServer : IVersionServer
    {
        private readonly RestClient m_client;
        private Product m_product;
        private DateTime m_lastCheck;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="baseUrl">The base URL for the license server.</param>
        public RestVersionServer(string baseURL)
        {
            m_client = new RestClient(baseURL);
        }

        /// <summary>
        /// Get the latest version of the product available from the server.
        /// </summary>
        /// <returns>The product or null if an error occurred.</returns>
        /// Note: Refactor to use Pyxis.Publishing.Protocol.ResourceClient<Product> in the future.
        public Product GetLatestVersion(ProductType productType)
        {
            if (m_product == null || DateTime.UtcNow.Subtract(m_lastCheck) > TimeSpan.FromMinutes(1))
            {
                var request = new RestRequest("Product/Current?ProductType=" + productType, Method.GET);
                var response = m_client.Execute(request);

                if (response.StatusCode != HttpStatusCode.OK || string.IsNullOrEmpty(response.Content))
                {
                    m_product = null;
                }
                else
                {
                    m_product = JsonConvert.DeserializeObject<Product>(response.Content);
                    m_lastCheck = DateTime.UtcNow;     
                }
            }
            return m_product;
        }

        /// <summary>
        /// Sets the latest version of the product on the server - not supported.
        /// </summary>
        /// <param name="product">The latest version of the product.</param>
        public void SetLatestVersion(Product product)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Used to concatenate paginated product lists from the server.
        /// </summary>
        internal class ProductsAndNextPage
        {
            public List<Product> Items = null;
            public string NextPageLink = "";
        };

        /// <summary>
        /// Gets all the product versions for a given product type. Handles paginated results
        /// from the server.
        /// </summary>
        /// <param name="productType">The product type.</param>
        /// <returns>The list of products sorted by descending version number or throws an exception on error.</returns>
        /// Note: Refactor to use Pyxis.Publishing.Protocol.ResourceClient<Product> in the future.
        public List<Product> GetAllVersions(ProductType productType)
        {
            var products = new List<Product>();
            var prefix = "Product?ProductType=" + productType;
            var skip = "";

            do
            {
                var request = new RestRequest(prefix + skip, Method.GET);
                var response = m_client.Execute(request);

                if (response.StatusCode != HttpStatusCode.OK || string.IsNullOrEmpty(response.Content))
                {
                    throw new Exception("Unable to get versions for: " + productType);
                }

                var result = JsonConvert.DeserializeObject<ProductsAndNextPage>(response.Content);
                products.AddRange(result.Items);

                // The results are paginated. If more pages exist, a NextPageLink is present at the
                // end of the JSON. It looks something like "&$skip=50". Extract it so we can make
                // our next request.
                skip = "";
                if (result.NextPageLink != null)
                {
                    var parts = result.NextPageLink.Split(new[] { prefix }, StringSplitOptions.None);
                    skip = (parts.Length) > 0 ? parts[parts.Length - 1] : "";
                }
            } while (skip.Length > 0);

            products.Sort((a, b) => b.ProductVersion.CompareTo(a.ProductVersion));

            return products;
        }
    }
}