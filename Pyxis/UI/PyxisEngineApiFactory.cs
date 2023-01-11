using System;
using System.ComponentModel;
using System.Threading.Tasks;
using Pyxis.Core;
using Pyxis.Publishing;

namespace Pyxis.UI
{
    /// <summary>
    /// A System.ComponentModel.Component providing a Pyxis.Core.Engine.
    /// The configuration properties of the instance must be set before attempting to get a Pyxis.Core.Engine.
    /// </summary>
    public class PyxisEngineApiFactory : Component 
    {
        private readonly object m_engineLock = new object();
        private Engine m_engine;

        /// <summary>
        /// Initializes a new instance of Pyxis.UI.PyxisEngineApiFactory.
        /// </summary>
        public PyxisEngineApiFactory()
        {
        }

        /// <summary>
        /// Initializes a new instance of Pyxis.UI.PyxisEngineApiFactory and adds it to a System.ComponentModel.IContainer.
        /// This is intended for adding the Pyxis.UI.PyxisEngineApiFactory to the components of a System.Windows.Forms.Form.
        /// </summary>
        /// <param name="container">The components container of the System.Windows.Forms.Form.</param>
        public PyxisEngineApiFactory(IContainer container)
        {
            container.Add(this);
        }

        /// <summary>
        /// Gets or sets the ApplicationKey used to initialize a Pyxis.Core.Engine.
        /// </summary>
        [Description("Application key provided by the PYXIS innovation")]
        [Category("Authentication")]
        [DisplayName("Application Key")]
        public string ApplicationKey { get; set; }

        /// <summary>
        /// Gets or sets the user email address used to initialize a Pyxis.Core.Engine.
        /// </summary>
        [Description("User email attached to application key provided by the PYXIS innovation")]
        [Category("Authentication")]
        [DisplayName("User Email")]
        public string UserEmail { get; set; }

        /// <summary>
        /// Gets or sets whether the instantiated Pyxis.Core.Engine is allowed to stream data over the PYXIS GeoWeb - PyxNet.
        /// </summary>
        [Description("Allow Engine to stream data over the GeoWeb")]
        [Category("Pyxis Engine")]
        [DisplayName("Enable PyxNet")]
        public bool EnablePyxNet { get; set; }

        /// <summary>
        /// Get a started Pyxis.Core.Engine configured using the properties of the Pyxis.UI.PyxisEngineApiFactory instance.
        /// </summary>
        /// <returns>The started Pyxis.Core.Engine.</returns>
        /// <exception cref="System.ArgumentException">ApplicationKey and UserEmail must be set.</exception>
        public Engine GetEngine()
        {
            lock (m_engineLock)
            {
                if (m_engine == null)
                {
                    m_engine = Engine.Create(GetEngineConfig());
                    m_engine.Start();
                }
            }
            return m_engine;
        }

        private EngineConfig GetEngineConfig()
        {
            if (String.IsNullOrEmpty(ApplicationKey))
            {
                throw new ArgumentException("ApplicationKey must be provided", "ApplicationKey");
            }
            if (String.IsNullOrEmpty(UserEmail))
            {
                throw new ArgumentException("UserEmail must be provided", "UserEmail");
            }
            var config = EngineConfig.FromApiKey(new ApiKey(UserEmail, ApplicationKey));
            config.UsePyxnet = EnablePyxNet;
            return config;
        }

        /// <summary>
        /// Dispose of all the resources used by the Pyxis.UI.PyxisEngineApiFactory.
        /// </summary>
        /// <param name="disposing"></param>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (m_engine != null)
                {
                    m_engine.Stop();
                    m_engine = null;
                }
            }
            base.Dispose(disposing);
        }

        /// <summary>
        /// Add a System.Action to execute when the Pyxis.Core.Engine is running.
        /// </summary>
        /// <param name="action">The System.Action to execute.</param>
        public void WhenReady(Action action)
        {
            var scheduler = TaskScheduler.FromCurrentSynchronizationContext();

            Task.Factory.StartNew(() => {
                GetEngine().WhenReady(action, scheduler);
            });
        }
    }
}
