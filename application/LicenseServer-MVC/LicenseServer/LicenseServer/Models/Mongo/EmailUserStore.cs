using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Web;
using Microsoft.AspNet.Identity;
using MongoDB.AspNet.Identity;
using MongoDB.Bson;
using MongoDB.Driver;
using MongoDB.Driver.Builders;
using MongoDB.Driver.Linq;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    // Mongo Identity UserStore project doesn't expose the db.  Using reflection as a workaround.  
    // The project doesn't seem to be maintained and integrating the source is an option.
    // FindByNameAsync should be made case-insensitive in that case.
    // https://github.com/InspectorIT/MongoDB.AspNet.Identity
    public class EmailUserStore<TUser> : UserStore<TUser>, IUserEmailStore<TUser>, IQueryableUserStore<TUser> where TUser : global::MongoDB.AspNet.Identity.IdentityUser, IPyxisIdentityUser
    {
        private const string collectionName = "AspNetUsers";

        private MongoDatabase Db
        {
            get
            {
                var field = typeof(UserStore<TUser>).GetField("db", BindingFlags.NonPublic | BindingFlags.Instance);
                return (MongoDatabase) field.GetValue(this);
            }
        }

        public EmailUserStore(string connectionNameOrUrl)
            : base(connectionNameOrUrl)
        {
        }

        public Task<TUser> FindByEmailAsync(string email)
        {
            var query = Query<TUser>.Matches(p => p.Email, new BsonRegularExpression(new Regex("^" + email + "$", RegexOptions.IgnoreCase)));
            var user = Db.GetCollection<TUser>(collectionName).FindOne(query);
            return Task.FromResult(user);
        }

        public Task<bool> GetEmailConfirmedAsync(TUser user)
        {
            return Task.FromResult(user.EmailConfirmed.Value);
        }

        public Task SetEmailConfirmedAsync(TUser user, bool confirmed)
        {
            user.EmailConfirmed = confirmed;
            return Task.FromResult(0);
        }

        public Task<string> GetEmailAsync(TUser user)
        {
            return Task.FromResult(user.Email);
        }

        public Task SetEmailAsync(TUser user, string email)
        {
            user.Email = email;
            return Task.FromResult(0);
        }

        //TODO: we should consider remove this from the API
        public IQueryable<TUser> Users
        {
            get { return Db.GetCollection<TUser>(collectionName).AsQueryable(); }
        }
    }
}