using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo.Interface
{
    public interface IActivities
    {
        void InsertComment(Comment comment);
        void InsertRating(Rating rating);

        // Agreement's track user's agreeing or disagreeing with the terms of a License
        IQueryable<Agreement> GetAgreements(DateTime from, DateTime until);
        IQueryable<Agreement> GetAgreementsByLicenseId(Guid licenseId, DateTime from, DateTime until);
        IQueryable<Agreement> GetAgreementsByLicenseVersionedId(Guid licenseId, Guid licenseVersion, DateTime from, DateTime until);
        IQueryable<Agreement> GetAgreementsByUser(Guid userId, DateTime from, DateTime until);
        IQueryable<Agreement> GetAgreementsByUserAndLicenseId(Guid userId, Guid licenseId, DateTime from, DateTime until);
        IQueryable<Agreement> GetAgreementsByUserAndLicenseVersionedId(Guid userId, Guid licenseId, Guid licenseVersion, DateTime from, DateTime until);
        IQueryable<Agreement> GetActiveAgreementsByUserAndLicenseReferences(Guid userId, List<LicenseReference> licenseReferences);
        LicensedAccess GetLicensedAccessToResource(Guid userId, Guid resourceId);
        void InsertAgreement(Agreement agreement);
    }
}
