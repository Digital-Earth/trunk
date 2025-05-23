﻿using Microsoft.Practices.Unity;
using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Http;
using System.Web.Http.Dependencies;
using System.Web.Routing;

namespace worldView.gallery
{
    public static class WebApiConfig
    {
        public static void Register(HttpConfiguration config)
        {
            // Web API routes
            config.MapHttpAttributeRoutes();
            config.DependencyResolver = CreateResolver();
            // Other Web API configuration not shown.
        }
        private static IDependencyResolver CreateResolver()
        {
            var unityContainer = new UnityContainer();
            unityContainer.RegisterInstance<IBlobProvider>(new AzureBlobProvider(storage.pyxis.Properties.Settings.Default.AzureConnectionString));
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
