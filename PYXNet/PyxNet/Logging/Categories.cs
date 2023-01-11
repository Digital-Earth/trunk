using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Utilities.Logging;

namespace PyxNet.Logging
{
    public static class Categories
    {
        public static LogCategory Stack = new LogCategory("PyxNet");
        public static LogCategory Publishing = new LogCategory("PyxNet.Publishing");
        public static LogCategory Query = new LogCategory("PyxNet.Query");
        public static LogCategory Connection = new LogCategory("PyxNet.Connection");
        public static LogCategory Downloading = new LogCategory("PyxNet.Downloading");        
    }
}
