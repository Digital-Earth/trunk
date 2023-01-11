using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for the StatusItemRegistry.
    /// </summary>
    [TestFixture]
    public class StatusItemRegistryTests
    {
        /// <summary>
        /// Tests simple Registry operations.
        /// </summary>
        [Test]            
        public void SimpleTest()
        {
            StatusItemRegistry registry = new StatusItemRegistry();
            Assert.AreEqual(0, registry.GetStatusItems().Count, 
                "Registry must be empty.");

            StatusItem statusItem1 = new StatusItem(registry, "1", "1");
            statusItem1.Priority = 1;
            registry.Register(statusItem1);
            Assert.AreEqual(1, registry.GetStatusItems().Count,
                "Registry must have 1 StatusItem.");

            StatusItem statusItem2 = new StatusItem(registry, "2", "2");
            statusItem2.Priority = 2;
            registry.Register(statusItem2);
            StatusItem statusItem3 = new StatusItem(registry, "3", "3");
            statusItem3.Priority = 3;
            registry.Register(statusItem3);
            Assert.AreEqual(3, registry.GetStatusItems().Count,
                "Registry must have 3 StatusItem.");

            Assert.AreEqual(statusItem2, registry.Find("2"), 
                "The Registry must contain statusItem2.");

            Assert.AreEqual(null, registry.Find("4"),
                "The Registry does not contain a StatusItem with name '4'."
                );

            registry.UnregisterAll();
            Assert.AreEqual(0, registry.GetStatusItems().Count,
                "Registry must be empty.");
        }

        /// <summary>
        /// Tests complex Registry operations.
        /// </summary>
        [Test]
        public void ComplexTest()
        {
            StatusItemRegistry registry = new StatusItemRegistry();
            StatusItem statusItem1 = new StatusItem(registry, "1", "1");
            statusItem1.Priority = 1;
            registry.Register(statusItem1);
            StatusItem statusItem2 = new StatusItem(registry, "2", "1");
            statusItem2.Priority = 1;
            try
            {
                registry.Register(statusItem2);
                Assert.Fail(
                    "Expected ArgumentException was not thrown!");
            }
            catch (ArgumentException)
            {
            }
            catch (Exception)
            {
                Assert.Fail(
                    "Expected ArgumentException was not thrown!");
            }

            statusItem2.Priority = 2;
            registry.Register(statusItem2);
            IList<StatusItem> statusItems = registry.GetStatusItems();
            Assert.AreEqual(1, statusItems.Count, "Since both the " + 
                                                  "StatusItems in the registry have the same category, " + 
                                                  "only the one with the higher priority should be returned."
                );

            Assert.AreEqual(statusItem2.Name, statusItems[0].Name,
                "statusItem2 should have been returned by the registry " +
                "since it has the higher priority.");
        }
    }
}