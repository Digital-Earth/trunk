using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using System.Web;
using System.Web.Http.ModelBinding;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using Microsoft.AspNet.Identity;
using MongoDB.AspNet.Identity;
using LicenseServer.DTOs;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using Resource = Pyxis.Contract.Publishing.Resource;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/Metadata")]
    public class MetadataController : CORSMongoApiController
    {
        public MetadataController() 
        { }

        // Inject for test
        public MetadataController(TestMongoSetup setup) :
            base(setup)
        { }

        // GET api/v1/metadata  [types is null]
        // GET api/v1/metadata?types={ResourceType}
        public PageResult<dynamic> Get([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, ODataQueryOptions<Resource> options)
        {
            return Get(types, ResultFormat.Full, options);
        }

        // GET api/v1/metadata?format={Full|Basic|View}  [types is null]
        // GET api/v1/metadata?types={ResourceType}&format={Full|Basic|View}
        public PageResult<dynamic> Get([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, ResultFormat format, ODataQueryOptions<Resource> options)
        {
            var result = (types == null || types.Count() == 0) ? db.GetResources() : db.GetResources(types.ToList());
            return result.ToFormattedPageResult(format, Request, options);
        }

        // GET api/v1/metadata/5
        public HttpResponseMessage Get(Guid id)
        {
            var result = db.GetResourceById(id);
            if (result != null)
            {
                return Request.CreateResponse(HttpStatusCode.OK, result);
            }
            else
            {
                return Request.CreateResponse(HttpStatusCode.NotFound);
            }
        }

        // GET api/v1/metadata?search={search string}  [types is null]
        // GET api/v1/metadata?types={ResourceType}&search={search string}
        public PageResult<dynamic> Get([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, string search, ODataQueryOptions<Resource> options)
        {
            return Get(types, search, ResultFormat.Full, options);
        }

        // GET api/v1/metadata?search={search string}&format={Full|Basic|View}  [types is null]
        // GET api/v1/metadata?types={ResourceType}&search={search string}&format={Full|Basic|View}
        public PageResult<dynamic> Get([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, string search, ResultFormat format, ODataQueryOptions<Resource> options)
        {
            var result = (types == null || types.Count() == 0) ? db.SearchResources(search) : db.SearchResources(search, types.ToList());
            return result.ToFormattedPageResult(format, Request, options);
        }

        // GET api/v1/metadata/SuggestTerms?suggest={search string}  [types is null]
        // GET api/v1/metadata/SuggestTerms?types={ResourceType}&suggest={search string}
        [Route("SuggestTerms")]
        public PageResult<dynamic> GetTerms([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, string search, ODataQueryOptions<string> options)
        {
            IQueryable<string> result;
            if (search == null)
            {
                result = new List<string>().AsQueryable();
            }
            else
            {
                result = (types == null || types.Count() == 0) ? db.SuggestTerms(search) : db.SuggestTerms(search, types.ToList());
            }

            return result.ToPageResult(Request, options);
        }

        // GET api/v1/metadata/SuggestCompletions?completion={search string}  [types is null]
        // GET api/v1/metadata/SuggestCompletions?types={ResourceType}&completion={search string}
        [Route("SuggestCompletions")]
        public PageResult<dynamic> GetCompletions([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, string search, ODataQueryOptions<string> options)
        {
            IQueryable<string> result;
            if (search == null)
            {
                result = new List<string>().AsQueryable();
            }
            else
            {
                result = (types == null || types.Count() == 0) ? db.SuggestCompletions(search) : db.SuggestCompletions(search, types.ToList());
            }

            return result.ToPageResult(Request, options);
        }

        // GET api/v1/metadata/SuggestMatches?completion={search string}  [types is null]
        // GET api/v1/metadata/SuggestMatches?types={ResourceType}&completion={search string}
        [Route("SuggestMatches")]
        public PageResult<dynamic> GetMatches([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, string search, ODataQueryOptions<string> options)
        {
            return SuggestMatches(types, search, options);
        }

        // POST api/v1/metadata/SuggestMatches
        [HttpPost]
        [Route("SuggestMatches")]
        public PageResult<dynamic> PostMatches(SuggestionMatchQueryDTO query, ODataQueryOptions<string> options)
        {
            return SuggestMatches(query.Types, query.Text, options);
        }

        // GET api/v1/metadata?search={search string}&grouping={field}  [types is null]
        // GET api/v1/metadata?types={ResourceType}&search={search string}&grouping={field}
        public PageResult<dynamic> Get([ModelBinder(typeof(CommaSeparatedArrayModelBinder))] Pyxis.Contract.Publishing.ResourceType[] types, string search, string grouping, ODataQueryOptions<Pyxis.Contract.Publishing.ResourceGrouping> options)
        {
            var result = (types == null || types.Count() == 0) ? db.GetResourceGroupings(search, grouping) : db.GetResourceGroupings(search, types.ToList(), grouping);

            return result.ToPageResult(Request, options);
        }

        // POST api/v1/metadata/Query/Ids
        [HttpPost]
        [Route("Query/Ids")]
        public PageResult<dynamic> FindResourcesByIds(List<Guid> resources, ODataQueryOptions<Resource> options)
        {
            return FindResourcesByIds(resources, ResultFormat.Full, options);
        }

        // POST api/v1/metadata/Query/Ids?format={Full|Basic|View}
        [HttpPost]
        [Route("Query/Ids")]
        public PageResult<dynamic> FindResourcesByIds(List<Guid> resources, ResultFormat format, ODataQueryOptions<Resource> options)
        {
            return db.GetResourcesByIds(resources).ToFormattedPageResult(format, Request, options);
        }
        
        // POST api/v1/metadata
        [Authorize]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPost]
        public PageResult<dynamic> ResourceUpdates(List<Guid> resources, DateTime lastUpdated, ODataQueryOptions<Resource> options)
        {
            return db.GetUpdates(resources, lastUpdated).ToPageResult(Request, options);
        }

        // POST api/v1/metadata/Comment/5
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPost]
        [Route("Comment/{id}")]
        public HttpResponseMessage Comment(Guid id, Pyxis.Contract.Publishing.Comment comment)
        {
            HttpResponseMessage response;
            if (id == Guid.Empty)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Invalid resource");
            }
            if (comment == null || comment.Message == null)
            {
                return Request.CreateResponse(HttpStatusCode.NotModified, "Not Modified - Empty Comment");
            }

            try
            {
                var filledComment = new Pyxis.Contract.Publishing.Comment(id, comment.Message, CurrentUserIdentity.UserInfo, comment.ReplyTo); 
                db.InsertComment(filledComment);
                response = Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                response = exception.ToHttpResponse(Request);
            }
            return response;
        }

        // POST api/v1/metadata/Rating/5?Value={value}
        [Authorize(Roles = PyxisIdentityRoleGroups.ConfirmedAndNotApi)]
        [HostAuthentication(DefaultAuthenticationTypes.ExternalBearer)]
        [HttpPost]
        [Route("Rating/{id}")]
        public HttpResponseMessage Rating(Guid id, sbyte value)
        {
            HttpResponseMessage response;
            if (id == Guid.Empty)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Invalid resource");
            }
            if (value > 1 || value < -1)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Invalid rating value.  +1, 0, -1 are accepted");
            }

            try
            {
                var rating = new Pyxis.Contract.Publishing.Rating(id, CurrentUserIdentity.UserInfo, value);
                db.InsertRating(rating);
                response = Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (DataLayerException exception)
            {
                response = exception.ToHttpResponse(Request);
            }
            return response;
        }

        private PageResult<dynamic> SuggestMatches(Pyxis.Contract.Publishing.ResourceType[] types, string search, ODataQueryOptions<string> options)
        {
            IQueryable<string> result;
            if (search == null)
            {
                result = new List<string>().AsQueryable();
            }
            else
            {
                result = (types == null || types.Count() == 0) ? db.SuggestMatches(search) : db.SuggestMatches(search, types.ToList());
            }
            
            return result.ToPageResult(Request, options);
        }
    }
}
