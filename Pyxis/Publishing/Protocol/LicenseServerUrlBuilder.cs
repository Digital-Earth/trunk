/******************************************************************************
LicenseServerRestUrlBuilder.cs

begin		: Oct. 21, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Pyxis.Publishing
{
    internal class LicenseServerRestUrlBuilder
    {
        private string m_prefix;

        internal LicenseServerRestUrlBuilder(string prefix)
        {
            m_prefix = prefix;
        }

        internal string GwssNotification(string id)
        {
            return m_prefix + "GwssNotification/" + id;
        }

        internal string Pipeline(string procRef)
        {
            return m_prefix + "Pipeline/" + procRef;
        }

        internal string UserPipelineDetails(string procRef)
        {
            return m_prefix + "Pipeline/Details/" + procRef;
        }

        internal string AllPipelines()
        {
            return m_prefix + "Pipeline";
        }

        internal string GetPipelineResourceByProcRef(string procRef)
        {
            return m_prefix + "Pipeline/Resource/ProcRef/" + procRef;
        }

        internal string GetPipelineResourceByIdAndVersion(Guid id, Guid version)
        {
            return m_prefix + "Pipeline/Resource/" + id.ToString() + "?Version=" + version.ToString();
        }

        internal static Dictionary<Type, string> ResourceToNames = new Dictionary<Type, string>
        {
            { typeof(Resource), "Metadata" },
            { typeof(GeoSource), "GeoSource" },
            { typeof(License), "License" },
            { typeof(Gallery), "Gallery" },
            { typeof(User), "User" },
            { typeof(Map), "Map" },
            { typeof(Pipeline), "Pipeline" },
            { typeof(File), "File" },
            { typeof(Product), "Product" }
        };

        private static string ResourceName<T>() where T : Resource
        {
            string resourceType;
            if (ResourceToNames.TryGetValue(typeof(T), out resourceType))
            {
                return resourceType;
            }
            throw new Exception("Unsupported type " + typeof(T).Name);
        }

        internal string GetResources<T>(ResourceFilter filter) where T : Resource
        {
            return GetResources(ResourceName<T>(), filter);
        }

        internal string GetResources(string resourceType, ResourceFilter filter)
        {
            var queryParams = new Dictionary<string, string>();
            if (filter.Top != 0)
            {
                queryParams["$top"] = filter.Top.ToString();
            }
            if (filter.Skip != 0)
            {
                queryParams["$skip"] = filter.Skip.ToString();
            }
            if (!String.IsNullOrEmpty(filter.Filter))
            {
                queryParams["$filter"] = filter.Filter;
            }
            if (!String.IsNullOrEmpty(filter.Select))
            {
                queryParams["$select"] = filter.Select;
            }
            if (!String.IsNullOrEmpty(filter.Search))
            {
                queryParams["search"] = filter.Search;
            }

            return String.Format("{0}{1}?{2}",
                        m_prefix,
                        resourceType,
                        String.Join("&", queryParams.Select(x => x.Key + "=" + Uri.EscapeDataString(x.Value))));
        }

        internal string GetResourceById<T>(Guid id) where T : Resource
        {
            return String.Format("{0}{1}/{2}",
                    m_prefix,
                    ResourceName<T>(),
                    id);
        }

        internal string GetResourceByIdAndVersion<T>(Guid id, Guid version) where T : Resource
        {
            return String.Format("{0}{1}/{2}?Version={3}",
                    m_prefix,
                    ResourceName<T>(),
                    id,
                    version);
        }

        internal string GetResourceVersions<T>(Guid id) where T : Resource
        {
            return String.Format("{0}{1}/{2}/Versions",
                    m_prefix,
                    ResourceName<T>(),
                    id);
        }

        internal string PostResource<T>() where T : Resource
        {
            return String.Format("{0}{1}",
                    m_prefix,
                    ResourceName<T>()
                    );
        }

        internal string PutResource<T>(Guid id, Guid version) where T : Resource
        {
            return String.Format("{0}{1}/{2}?Version={3}",
                    m_prefix,
                    ResourceName<T>(),
                    id,
                    version);
        }

        internal string GetUserProfile()
        {
            return m_prefix + "User/Profile";
        }

        internal string GetAccessToken()
        {
            return m_prefix + "Account/TokenDetails";
        }

        internal string GetUserGalleries(Guid id)
        {
            return m_prefix + "User/" + id.ToString() + "/Galleries";
        }

        internal string TermsOfUse()
        {
            return m_prefix + "License/Terms";
        }

        internal string TrustedNodes()
        {
            return m_prefix + "Permit/Certificate/Trusted";
        }

        internal string RequestPermit<T>(params Guid[] resources) where T : IPermit
        {
            if (typeof(T) == typeof(Permits.AccessToken))
            {
                return m_prefix + "Account/Login";
            }
            if (typeof(T) == typeof(Permits.ExternalApiKeyPermit))
            {
                if (resources.Length == 0)
                {
                    throw new ArgumentException("Request key permit requires GroupKey Id");
                }
                return m_prefix + "Permit/Key/"+resources[0]+"/Request";
            }
            if (typeof(T) == typeof(Permits.CertificatePermit))
        {
            return m_prefix + "Permit/Certificate/Request";
        }
            throw new NotSupportedException("Unsupported permit type " + typeof(T).Name);
        }

        internal string ReleasePermit<T>(params Guid[] resources) where T : IPermit
        {
            if (typeof(T) == typeof(Permits.AccessToken))
            {
                throw new InvalidOperationException("AccessToken can't be released");
            }
            if (typeof(T) == typeof(Permits.ExternalApiKeyPermit))
            {
                if (resources.Length == 0)
                {
                    throw new ArgumentException("Release key permit requires GroupKey Id");
                }
                return m_prefix + "Permit/Key/" + resources[0] + "/Release";
            }
            if (typeof(T) == typeof(Permits.CertificatePermit))
        {
                throw new InvalidOperationException("Certificate can't be released");
            }
            throw new NotSupportedException("Unsupported permit type " + typeof(T).Name);
        }
    }
}