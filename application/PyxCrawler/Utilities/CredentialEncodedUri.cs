using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Security;
using System.Web;
using AutoMapper.Internal;
using Pyxis.Contract;
using Pyxis.IO.Sources.Remote;

namespace PyxCrawler.Utilities
{
    internal class CredentialEncodedUri
    {
        public Uri Uri { get; private set; }
        public CredentialPermit Permit { get; private set; }

        public CredentialEncodedUri(Uri uri)
        {
            var userInfo = uri.UserInfo;
            Uri = new Uri(uri.GetComponents(UriComponents.AbsoluteUri & ~UriComponents.UserInfo, UriFormat.UriEscaped));
            if (string.IsNullOrWhiteSpace(userInfo))
            {
                return;
            }
            Permit = new CredentialPermit(userInfo);
        }
    }

    internal class CredentialPermit : INetworkPermit
    {
        public NetworkCredential Credentials { get; private set; } 

        public PermitType PermitType
        {
            get
            {
                return PermitType.AccessToken;
            }
        }

        public DateTime Issued
        {
            get
            {
                return DateTime.MinValue;
            }
        }

        public DateTime Expires
        {
            get
            {
                return DateTime.MaxValue;
            }
        }

        public CredentialPermit(string encodedCredential)
        {
            var credentialParts = encodedCredential.Split(':');
            var user = credentialParts[0];
            var password = credentialParts[1];
            Credentials = new NetworkCredential(user, password);
        }
    }
}