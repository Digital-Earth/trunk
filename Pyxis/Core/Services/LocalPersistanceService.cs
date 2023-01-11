using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Utilities;

namespace Pyxis.Core.Services
{
    internal class LocalPersistanceService : ServiceBase
    {
        protected override void StartService()
        {
            //enforce local persistance current directory is correct
            LocalPersistance.RootDirectory = Directory.GetCurrentDirectory();
        }

        protected override void StopService()
        {            
        }
    }
}
