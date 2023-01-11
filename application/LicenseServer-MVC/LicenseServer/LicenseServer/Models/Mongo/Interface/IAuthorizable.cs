using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LicenseServer.Models.Mongo.Interface
{
    public interface IAuthorizedUser
    {
        Guid Id { get; }
        bool IsAdmin { get; }
        bool IsPyxisAdmin { get; }
    }

    public interface IAuthorizedUserWithResources : IAuthorizedUser
    {
        List<Guid> Groups { get; }
        List<Guid> Galleries { get; }
    }

    /// <summary>
    /// Provides a mechanism for authorizing requests with respect to a User.
    /// </summary>
    public interface IAuthorizable
    {
        void SetAuthorizedUser(IAuthorizedUser authorizedUser);
    }

    /// <summary>
    /// Provides a mechanism for authorizing requests with respect to a User including their Resources.
    /// </summary>
    public interface IAuthorizableWithResources
    {
        void SetAuthorizedUser(IAuthorizedUserWithResources authorizedUser);
    }
}
