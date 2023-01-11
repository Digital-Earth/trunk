using Microsoft.Practices.Unity;
using Owin;
using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using System;
using System.Collections.Generic;
using System.Web.Http;
using System.Web.Http.Dependencies;

namespace StorageServer
{
    public class FileServerOwinStartup
    {
        public void Configuration(IAppBuilder app)
        {
#if DEBUG
            app.UseErrorPage();
#endif

            HttpConfiguration config = new HttpConfiguration();
            config.DependencyResolver = CreateResolver();
            config.MapHttpAttributeRoutes();
            config.Formatters.Remove(config.Formatters.XmlFormatter);

            app.UseWebApi(config);
        }

        private IDependencyResolver CreateResolver()
        {
            var unityContainer = new UnityContainer();
            unityContainer.RegisterInstance<IBlobProvider>(new AzureBlobProvider(Properties.Settings.Default.AzureConnectionString));
            return new UnityResolver(unityContainer);
        }
    }

    public class UnityResolver : IDependencyResolver
    {
        protected IUnityContainer container;

        public UnityResolver(IUnityContainer container)
        {
            if (container == null)
            {
                throw new ArgumentNullException("container");
            }
            this.container = container;
        }

        public object GetService(Type serviceType)
        {
            try
            {
                return container.Resolve(serviceType);
            }
            catch (ResolutionFailedException)
            {
                return null;
            }
        }

        public IEnumerable<object> GetServices(Type serviceType)
        {
            try
            {
                return container.ResolveAll(serviceType);
            }
            catch (ResolutionFailedException)
            {
                return new List<object>();
            }
        }

        public IDependencyScope BeginScope()
        {
            var child = container.CreateChildContainer();
            return new UnityResolver(child);
        }

        public void Dispose()
        {
            container.Dispose();
        }
    }
}