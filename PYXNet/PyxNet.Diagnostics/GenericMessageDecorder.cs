using System;
using System.Collections.Generic;
using System.Reflection;
using PyxNet.Pyxis;

namespace PyxNet.Diagnostics
{
    public static class GenericMessageDecorder
    {
        public delegate string DescribeDelegate(ITransmissible transmissible);
        private static Dictionary<Type, DescribeDelegate> MessageDescribers = new Dictionary<Type, DescribeDelegate>();

        static Dictionary<string,Type> KnownMessageTypes = new Dictionary<string, Type>();

        static GenericMessageDecorder()
        {
            InitMessagesIDs();
            InitMessageDescriptors();
        }

        private static void InitMessageDescriptors()
        {
            MessageDescribers[typeof (PyxNet.DataHandling.DataChunkRequest)] =
                delegate(ITransmissible transmissible)
                    {
                        var request = transmissible as PyxNet.DataHandling.DataChunkRequest;

                        return string.Format("Req[{0}...{1}]", request.Offset, (request.Offset + request.ChunkSize));
                    };

            MessageDescribers[typeof(PyxNet.DataHandling.DataChunk)] =
                delegate(ITransmissible transmissible)
                {
                    var chunk = transmissible as PyxNet.DataHandling.DataChunk;

                    return string.Format("Data[{0}...{1}]", chunk.Offset, (chunk.Offset + chunk.ChunkSize));
                };

            MessageDescribers[typeof(PyxNet.Pyxis.CoverageRequestMessage)] = 
                delegate(ITransmissible transmissible)
                    {
                        var request = transmissible as PyxNet.Pyxis.CoverageRequestMessage;

                        if (request.Mode == CoverageRequestMessage.TransferMode.Tile)
                        {
                            return request.TileIndex;
                        }

                        return request.Mode.ToString();
                    };
        }

        private static void InitMessagesIDs()
        {
            KnownMessageTypes[StackConnection.QueryMessageID] = typeof (PyxNet.Query);
            KnownMessageTypes[StackConnection.QueryHashTableMessageID] = typeof (PyxNet.QueryHashTable);
            KnownMessageTypes[StackConnection.KnownHubListMessageID] = typeof (PyxNet.KnownHubList);
            KnownMessageTypes[Stack.QueryResultMessageID] = typeof (PyxNet.QueryResult);
            KnownMessageTypes[StackConnection.LocalNodeInfoMessageID] = typeof (PyxNet.NodeInfo);

            foreach (Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (assembly.GetName().Name.StartsWith("System.") ||
                    assembly.GetName().Name.StartsWith("Microsoft."))
                {
                    continue;
                }

                foreach (var type in assembly.GetTypes())
                {
                    if (!type.IsAbstract && !type.IsInterface &&
                        typeof (ITransmissible).IsAssignableFrom(type))
                    {
                        string messageID = FindMessageID(type);

                        if (!String.IsNullOrEmpty(messageID))
                        {
                            if (KnownMessageTypes.ContainsKey(messageID))
                            {
                                System.Diagnostics.Trace.WriteLine(string.Format("Warning: {0} has more then one type associted with it: {1} and {2}", messageID, type, KnownMessageTypes[messageID]));
                            }
                            KnownMessageTypes[messageID] = type;
                        }
                    }
                }
            }
        }

        private static string FindMessageID(Type type)
        {
            try
            {
                foreach(var field in type.GetFields())
                {
                    if (field.Name.ToLower() == "messageid")
                    {
                        return field.GetValue(null).ToString();
                    }
                }

                System.Diagnostics.Trace.WriteLine("Failed to find messageID for type " + type.FullName);
                return null;
            }
            catch
            {
                System.Diagnostics.Trace.WriteLine("Failed to find messageID for type " + type.FullName);
                return null;
            }
        }

        public static string TryBuildDescription(ITransmissible messageObject)
        {
            var messageType = messageObject.GetType();

            if (MessageDescribers.ContainsKey(messageType))
            {
                var result = MessageDescribers[messageType](messageObject);

                if (result != null)
                    return result;
            }

            return "";
        }

        public static ITransmissible TryDecodeMessage(Message message)
        {
            if (KnownMessageTypes.ContainsKey(message.Identifier))
            {
                var messageType = KnownMessageTypes[message.Identifier];

                try
                {
                    //try to creat the object
                    var obj = Activator.CreateInstance(messageType);
                    var objTransmissibleWithReader = obj as ITransmissibleWithReader;

                    if (objTransmissibleWithReader != null)
                    {
                        objTransmissibleWithReader.FromMessage(message);
                        return objTransmissibleWithReader;
                    }
                }
                catch 
                {
                }

                try
                {
                    //try to create object from a message ctor
                    var obj = Activator.CreateInstance(messageType,new object[]{message});
                    return obj as ITransmissible;
                }
                catch
                {
                }

                try
                {
                    //try to create object from a message reader ctor
                    var obj = Activator.CreateInstance(messageType, new object[] { new MessageReader( message ) });
                    return obj as ITransmissible;
                }
                catch
                {
                }
            }

            //we failed to decode
            return null;
        }
    }
}