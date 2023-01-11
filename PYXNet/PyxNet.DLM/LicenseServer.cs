using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DLM
{
    class LicenseServer: PyxNet.Communications.INode 
    {
        #region INode Members

        public event PyxNet.Communications.MessageDelegate OnMessage;

        public void RaiseMessage(PyxNet.Communications.IMessage message)
        {
            this.OnMessage(this, message);
        }

        #endregion

        private void MessageHandler( 
            object sender, PyxNet.Communications.IMessage message)
        {
            // insert message testers here...
        }
    }
}
