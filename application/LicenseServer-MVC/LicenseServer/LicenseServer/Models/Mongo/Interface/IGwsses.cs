using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo.Interface
{
    public interface IGwsses
    {
        IQueryable<Gwss> GetGwsses();
        IQueryable<Gwss> GetGwssesStatuses();
        Gwss GetGwssById(Guid id);
        Gwss GetGwssStatusById(Guid id);
        void UpdateGwssStatus(Guid id, GwssStatus gwssStatus);
        void RemoveGwss(Guid id);
    }
}
