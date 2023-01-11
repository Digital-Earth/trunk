using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    public class PipelineEventArgs : System.EventArgs
    {
        public Pipeline Pipeline { get; set; }
    }
}
