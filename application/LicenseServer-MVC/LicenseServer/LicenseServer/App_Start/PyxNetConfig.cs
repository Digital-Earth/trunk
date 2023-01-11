using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Web;
using LicenseServer.Extensions;
using LicenseServer.Properties;
using PyxNet;
using PyxNet.DLM;
using PyxNet.Service;

namespace LicenseServer
{
    internal static class PyxNetConfig
    {
        public static PyxNetLicenseService PrimaryLicenseService { get; private set; }
        public static PyxNetLicenseService SecondaryLicenseService { get; private set; }

        public class PyxNetLicenseService
        {
            private readonly NodeId m_nodeId;
            private readonly PrivateKey m_privateKey;
            private readonly ServiceInstance m_serviceInstance;
            public NodeId NodeId { get { return m_nodeId; } }
            public PrivateKey PrivateKey { get { return m_privateKey; } }
            public ServiceInstance ServiceInstance { get { return m_serviceInstance; } }

            public PyxNetLicenseService(string serializedKey, string nodeIdentity, string serializedServiceInstance)
            {
                m_privateKey = DeserializeKey(serializedKey);
                m_nodeId = GenerateNodeId(m_privateKey, nodeIdentity);
                m_serviceInstance = DeserializeServiceInstance(serializedServiceInstance);
            }
        }

        private static ServiceId s_serviceId = new ServiceId(new Guid("8fd82eaa-0dd8-45eb-b1d5-d8afa46f2a7f"));

        public static void ConfigurePyxNetParameters()
        {
            PrimaryLicenseService = new PyxNetLicenseService(Settings.Default.PrimarySerializedKey, Settings.Default.PrimaryNodeIdentity, Settings.Default.PrimarySerializedServiceInstance);
            SecondaryLicenseService = new PyxNetLicenseService(Settings.Default.SecondarySerializedKey, Settings.Default.SecondaryNodeIdentity, Settings.Default.SecondarySerializedServiceInstance);   
        }

        public static void CreateSerializedLicenseService()
        {
            var serializedNodeIdentity = Guid.NewGuid().ToString();
            var serializedKey = CreateSerializedKey();
            var privateKey = DeserializeKey(serializedKey);
            var nodeid = GenerateNodeId(privateKey, serializedNodeIdentity);
            var serializedServiceInstance = SerializeServiceInstance(nodeid);
        }

        private static string CreateSerializedKey()
        {
            var rsaCryptoProvider = new RSACryptoServiceProvider();
            return rsaCryptoProvider.ToXmlString(true).Base64Encode();
        }

        private static PrivateKey DeserializeKey(string serializedKey)
        {
            var rsaCryptoProvider = new RSACryptoServiceProvider();
            rsaCryptoProvider.FromXmlString(serializedKey.Base64Decode());
            return new PrivateKey(rsaCryptoProvider.ExportParameters(true));
        }

        private static NodeId GenerateNodeId(PrivateKey privateKey, string nodeIdentity)
        {
            var userId = new UserId(privateKey.PublicKey);
            var nodeId = new NodeId(new Guid(nodeIdentity));
            nodeId.UserId = userId;
            return nodeId;
        }

        private static string SerializeServiceInstance(NodeId nodeId)
        {
            var ser = ServiceInstance.Create(s_serviceId, nodeId);
            var m = ser.ToMessage();
            var sm = m.SerializationString;
            return sm.Base64Encode();
        }

        private static ServiceInstance DeserializeServiceInstance(string encodedServiceInstance)
        {
            var serviceInstanceMessage = new Message() { SerializationString = encodedServiceInstance.Base64Decode() };
            var serviceInstance = new ServiceInstance();
            serviceInstance.FromMessage(serviceInstanceMessage);
            return serviceInstance;
        }
    }
}