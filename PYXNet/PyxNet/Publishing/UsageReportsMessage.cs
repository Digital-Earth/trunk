using System;
using System.Collections.Generic;
using System.Text;

//using PyxNet.Pyxis;
//using Pyxis.Utilities;

namespace PyxNet.Publishing
{
    public class UsageReportsMessage : ITransmissible
    {
        public const string MessageID = "URep";

        /// <summary>
        /// UsageReportsMessages come in three flavours:
        ///   send   - send report contents (src -> dst)
        ///   ack    - acknowledge (dst -> src)
        ///   resend - ask resend of report (dst -> src)
        /// </summary>
        public enum TransferMode { ReportSend, ReceiptAck, ResendRequest };
        
        #region Properties

        /// <summary>
        /// Gets or sets the Transfer Mode, the message subtype.
        /// </summary>
        /// <value>TransferMode: send, ack, resend.</value>
        public TransferMode Mode
        { get; set; }

        /// <summary>
        /// Gets or sets the DST node.
        /// Node where report is being sent.
        /// </summary>
        /// <value>The DST node.</value>
        public Guid DstNode
        { get; set; }

        /// <summary>
        /// Gets or sets the SRC node.
        /// Node where report originated.
        /// </summary>
        /// <value>The SRC node.</value>
        public Guid SrcNode
        { get; set; }

        /// <summary>
        /// Gets or sets the name of the report file.
        /// </summary>
        /// <value>The name of the report file.</value>
        public string ReportFileName
        { get; set; }

        /// <summary>
        /// Gets or sets the report XML.
        /// </summary>
        /// <value>The report XML.</value>
        public string ReportXml
        { get; set; }

        #endregion

        /// <summary>
        /// Construct an empty UsageReportsMessage.
        /// </summary>
        public UsageReportsMessage()
        {
            Mode = TransferMode.ReportSend;
            ReportXml = null;
        }

        /// <summary>
        /// Construct a UsageReporstMessage from a message.   
        /// The message must be a PyxNet UsageReportsMessage message.
        /// </summary>
        /// <param name="message"></param>
        public UsageReportsMessage(Message message)
        {
            FromMessage(message);
        }

        #region Convert to/from message format

        /// <summary>
        /// Build a PyxNet message that contains the UsageReportMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the UsageReportMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.(will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append((int)Mode);
            message.Append(DstNode);
            message.Append(SrcNode);
            message.Append(ReportFileName);
            message.Append(ReportXml);
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(MessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a UsageReportsMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a UsageReportsMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a UsageReportMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Mode = (TransferMode)reader.ExtractInt();
            DstNode = reader.ExtractGuid();
            SrcNode = reader.ExtractGuid();
            ReportFileName = reader.ExtractUTF8();
            ReportXml = reader.ExtractUTF8();
        }

        #endregion

        /// <summary>
        /// Send this UsageReportsMessage.
        /// </summary>
        /// <param name="stack">Communication stack.</param>
        public void SendReport(PyxNet.Stack stack)
        {
            new SendMessage(stack, this, this.DstNode).Send();
        }

        /// <summary>
        /// Send acknowledgement for this UsageReportsMessage.
        /// </summary>
        /// <param name="stack">Communication stack.</param>
        public void SendAcknowledgement(PyxNet.Stack stack)
        {
            UsageReportsMessage ackMessage = new UsageReportsMessage(this.ToMessage());
            ackMessage.Mode = TransferMode.ReceiptAck;
            ackMessage.ReportXml = null;

            new SendMessage(stack, ackMessage, ackMessage.SrcNode).Send();
        }

        /// <summary>
        /// Send resend request for this message.
        /// </summary>
        /// <param name="stack">Communication stack.</param>
        public void ResendRequest(PyxNet.Stack stack)
        {
            UsageReportsMessage resendMessage = new UsageReportsMessage(this.ToMessage());
            resendMessage.Mode = TransferMode.ResendRequest;
            resendMessage.ReportXml = null;

            new SendMessage(stack, resendMessage, resendMessage.SrcNode).Send();
        }

        /// <summary>
        /// Send resend request message for a specific report.
        /// </summary>
        /// <param name="stack">Communication stack.</param>
        /// <param name="reportFileName">Name of the report file.</param>
        /// <param name="srcNode">The report's originating node.</param>
        public void ResendRequest(PyxNet.Stack stack, string reportFileName, Guid srcNode)
        {
            UsageReportsMessage resendMessage = new UsageReportsMessage();
            resendMessage.Mode = TransferMode.ResendRequest;
            resendMessage.ReportFileName = reportFileName;
            resendMessage.ReportXml = null;
            resendMessage.SrcNode = srcNode;                    // wrt original geostream server
            resendMessage.DstNode = stack.NodeInfo.NodeGUID;    // where report will be sent to

            new SendMessage(stack, resendMessage, resendMessage.SrcNode).Send();
        }

        /// <summary>
        /// Private class that encapsulates then entire message sending mechanism.
        /// </summary>
        private class SendMessage
        {
            #region Private Properties

            private UsageReportsMessage Msg
            { get; set; }

            private PyxNet.Stack Stack
            { get; set; }
            
            private Guid DstNode 
            { get; set; }

            #endregion

            /// <summary>
            /// Initializes a new instance of the <see cref="SendMessage"/> class.
            /// </summary>
            /// <param name="inStack">Communication stack.</param>
            /// <param name="inMsg">Usage report message.</param>
            /// <param name="inDstNode">Node Guid for destination.</param>
            public SendMessage( PyxNet.Stack inStack, UsageReportsMessage inMsg, Guid inDstNode )
            {
                Msg = inMsg;
                Stack = inStack;
                DstNode = inDstNode;
            }

            /// <summary>
            /// Mechanism to send message.
            /// </summary>
            public void Send()
            {
                System.Threading.ThreadPool.QueueUserWorkItem(delegate
                {
                    Pyxis.Utilities.UsageReports.Log.Info("UsageReportsMessage:SendMessage: report={0}", Msg.ReportFileName); 

                    try
                    {
                        PyxNet.NodeId nodeId = new PyxNet.NodeId(DstNode);

                        PyxNet.NodeInfo nodeInfo = PyxNet.NodeInfo.Find(Stack, nodeId, TimeSpan.FromSeconds(30));
                        PyxNet.StackConnection connection = Stack.GetConnection(nodeInfo, false, TimeSpan.FromSeconds(15));
                        if (connection != null)
                        {
                            bool result = connection.SendMessage(Msg.ToMessage());
                            Pyxis.Utilities.UsageReports.Log.Debug("UsageReportsMessage:SendMessage: result={0}", result);
                        }
                        else
                        {
                            Pyxis.Utilities.UsageReports.Log.Error("UsageReportsMessage:SendMessage: No Connection, message not sent.");
                        }
                    }
                    catch (Exception ex)
                    {
                        Pyxis.Utilities.UsageReports.Log.Error("UsageReportsMessage:SendMessage:Exception: {0}", ex.Message);
                    }
                });
            }
        }
    }
}
