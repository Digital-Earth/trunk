using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using LicenseServer.Models.Mongo;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Extensions
{
    public static class MongoDBEntitiesExtensions
    {
        #region Resources

        public static IQueryable<Models.Mongo.Product> GetProducts(this IMongoDBEntities db, ProductType productType)
        {
            return db.GetResources<Models.Mongo.Product>().Where(p => p.ProductType == productType);
        }

        public static IQueryable<Models.Mongo.Product> GetCurrentProducts(this IMongoDBEntities db)
        {
            return db.GetResources<Models.Mongo.Product>()
                .Where(p => p.Metadata.SystemTags.Contains(ProductSystemTags.Production))
                .GroupBy(g => g.ProductType, (key, g) => g.OrderByDescending(p => p.ProductVersion).ThenByDescending(p => p.Metadata.Created).First());
        }

        public static Models.Mongo.Product GetCurrentProduct(this IMongoDBEntities db, ProductType productType)
        {
            return db.GetResources<Models.Mongo.Product>()
                .Where(p => p.ProductType == productType && p.Metadata.SystemTags.Contains(ProductSystemTags.Production))
                .OrderByDescending(p => p.ProductVersion)
                .ThenByDescending(p => p.Metadata.Created)
                .FirstOrDefault();
        }

        public static IQueryable<string> SuggestTerms(this IMongoDBEntities db, string search)
        {
            return db.SuggestTerms(search, null);
        }

        public static IQueryable<string> SuggestCompletions(this IMongoDBEntities db, string search)
        {
            return db.SuggestCompletions(search, null);
        }

        public static IQueryable<string> SuggestMatches(this IMongoDBEntities db, string search)
        {
            return db.SuggestMatches(search, null);
        }

        #endregion Resources

        #region Activities
        public static IQueryable<Agreement> GetAgreements(this IMongoDBEntities db)
        {
            return db.GetAgreements(DateTime.MinValue, DateTime.MaxValue);
        }
        public static IQueryable<Agreement> GetAgreementsById(this IMongoDBEntities db, Guid licenseId)
        {
            return db.GetAgreementsByLicenseId(licenseId, DateTime.MinValue, DateTime.MaxValue);
        }
        public static IQueryable<Agreement> GetAgreementsByIdAndVersion(this IMongoDBEntities db, Guid licenseId, Guid licenseVersion)
        {
            return db.GetAgreementsByLicenseVersionedId(licenseId, licenseVersion, DateTime.MinValue, DateTime.MaxValue);
        }
        public static IQueryable<Agreement> GetAgreementsByUser(this IMongoDBEntities db, Guid userId)
        {
            return db.GetAgreementsByUser(userId, DateTime.MinValue, DateTime.MaxValue);
        }
        public static IQueryable<Agreement> GetAgreementsByUserAndId(this IMongoDBEntities db, Guid userId, Guid licenseId)
        {
            return db.GetAgreementsByUserAndLicenseId(userId, licenseId, DateTime.MinValue, DateTime.MaxValue);
        }
        public static Agreement GetLatestAgreementByUserAndId(this IMongoDBEntities db, Guid userId, Guid licenseId)
        {
            return db.GetAgreementsByUserAndLicenseId(userId, licenseId, DateTime.MinValue, DateTime.MaxValue).OrderByDescending(a => a.Created).FirstOrDefault();
        }
        public static Agreement GetLatestAgreementByUserAndIdAndVersion(this IMongoDBEntities db, Guid userId, Guid licenseId, Guid licenseVersion)
        {
            return db.GetAgreementsByUserAndLicenseVersionedId(userId, licenseId, licenseVersion, DateTime.MinValue, DateTime.MaxValue).OrderByDescending(a => a.Created).FirstOrDefault();
        }
#endregion Activities
    }
}