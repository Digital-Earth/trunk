using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DLM
{
    /// <summary>
    /// Generic message capabilities.  Consider refactoring...
    /// </summary>
    class Message: PyxNet.Communications.IMessage 
    {
        protected Message(MessageTypes messageType)
        {
            m_MessageType = (int) messageType;
        }

        #region IMessage Members

        protected enum MessageTypes
        {
            PublicationRequest = 42,
            PublicationRequestDenied,
            PublicationRequestAccepted
        }

        private int m_MessageType;
        public virtual int MessageType
        {
            get
            {
                return m_MessageType;
            }
            set
            {
                m_MessageType = value;
            }
        }

        public virtual byte[] Contents
        {
            get
            {
                throw new Exception("The method or operation is not implemented.");
            }
            set
            {
                throw new Exception("The method or operation is not implemented.");
            }
        }
        
        #endregion
    }
}
