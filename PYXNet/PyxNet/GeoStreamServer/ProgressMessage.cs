/******************************************************************************
StatusMessage.cs

begin      : February 7, 2011
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    /// <summary>
    /// This message is sent by the License Server to request the progress of pipelines published to the GWSS.  
    /// The GWSS, in return, responds with this message, informing of its progress in the form of XML.
    /// </summary>
    public class ProgressMessage : GeoStreamServerMessage
    {
        public const string MessageID = "GSpr";

        public string ProcRef
        {
            get
            {
                return m_procRef;
            }
            set
            {
                m_procRef = value;
            }
        }
        private string m_procRef;
        
        public string Progress
        {
            get
            {
                return m_progress;
            }
            set
            {
                m_progress = value;
            }
        }
        private string m_progress;

        public override string GetMsgID()
        {
            return MessageID;
        }

        public override string GetMsgName()
        {
            return "GeoStream-Progress";
        }

        public override void ToMessage(Message message)
        {
            base.ToMessage(message);

            message.Append(ProcRef);
            message.Append(Progress);
        }

        public override void FromMessageReader(MessageReader reader)
        {
            base.FromMessageReader(reader);

            ProcRef = reader.ExtractUTF8();
            Progress = reader.ExtractUTF8();
        }
    }
}
