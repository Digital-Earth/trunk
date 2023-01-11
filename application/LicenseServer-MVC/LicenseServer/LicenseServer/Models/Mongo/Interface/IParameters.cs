using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace LicenseServer.Models.Mongo.Interface
{
    public interface IParameters
    {
        IQueryable<KeyValuePair<string, object>> GetParameters();
        object GetParameter(string key);
        void UpdateParameter(KeyValuePair<string, object> pair);
    }
}