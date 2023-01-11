using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.Description;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using LicenseServer.Extensions;
using LicenseServer.Models.Mongo;
using Microsoft.AspNet.Identity;
using Pyxis.Contract.Publishing;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using Map = LicenseServer.Models.Mongo.Map;
using Product = LicenseServer.Models.Mongo.Product;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Product")]
    public class ProductController : BaseResourceController<Product>
    {
        public ProductController() 
        { }

        // Inject for test
        public ProductController(TestMongoSetup setup) :
            base(setup)
        { }

        // Get api/v1/Product?ProductType={productType}
        public PageResult<dynamic> GetProduct(ProductType productType, ODataQueryOptions<Product> options)
        {
            return db.GetProducts(productType).ToPageResult(Request, options);
        }

        // Get api/v1/Product/Current
        [Route("Current")]
        public PageResult<dynamic> GetCurrent(ODataQueryOptions<Product> options)
        {
            return db.GetCurrentProducts().ToPageResult(Request, options);
        }

        // Get api/v1/Product/Current?ProductType={productType}
        [Route("Current")]
        [ResponseType(typeof(Product))]
        public HttpResponseMessage GetCurrent(ProductType productType, ODataQueryOptions<Product> options)
        {
            var product = db.GetCurrentProduct(productType);
            return CreateResponse(product);
        }

        // POST api/v1/Product
        [Authorize(Roles = PyxisIdentityRoleGroups.PyxisAdmins)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        public override HttpResponseMessage Post(Product product)
        {
            if (product.ProductType == null || product.ProductVersion == null || product.TransferType == null || product.Url == null || product.Key == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Product ProductType, ProductVersion, ProductTransferType, Url, and Key must be specified");
            }
            if (product.Metadata != null && product.Metadata.Visibility == null)
            {
                product.Metadata.Visibility = VisibilityType.Private;
            }
            return base.Post(product);
        }
    }
}
