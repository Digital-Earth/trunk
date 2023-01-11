using System;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    /// <summary>
    /// Unit tests for PipelinePermissionFact
    /// </summary>
    [TestFixture]
    public class PipelinePermissionFactTests
    {
        [Test]
        public void Serialization()
        {
            PipelinePermissionFact myPipelinePermissionFact = new PipelinePermissionFact
            {
                AuthorizedNode = PyxNet.Test.NodeIdTests.GenerateRandomNodeId(),
                Id = Guid.NewGuid(),
                ProcessVersion = 42,
            };
            Message message = myPipelinePermissionFact.ToMessage();
            PipelinePermissionFact extractedPipelinePermissionFact = new PipelinePermissionFact(message);
            Assert.AreEqual(myPipelinePermissionFact.Id, extractedPipelinePermissionFact.Id);
            Assert.AreEqual(myPipelinePermissionFact.ProcessVersion, extractedPipelinePermissionFact.ProcessVersion);
            Assert.AreEqual(myPipelinePermissionFact.UniqueKeyword, extractedPipelinePermissionFact.UniqueKeyword);
            Assert.AreEqual(myPipelinePermissionFact.AuthorizedNode, extractedPipelinePermissionFact.AuthorizedNode);
        }
    }
}