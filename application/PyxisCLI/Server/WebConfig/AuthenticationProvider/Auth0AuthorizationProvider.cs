using System;
using System.Collections.Generic;
using System.Net;
using System.Security.Cryptography;
using System.Threading.Tasks;
using ApplicationUtility;
using Jose;
using Newtonsoft.Json;

namespace PyxisCLI.Server.WebConfig.AuthenticationProvider
{
    /// <summary>
    /// Use Auth0 to authenticate JWT tokens
    /// </summary>
    internal class Auth0AuthenticationProvider : IAuthenticationProvider
    {
        protected Dictionary<string, RSACryptoServiceProvider> m_cachedJWKS;
        protected string m_cachedIssuer;
        protected DateTime m_cachedTimestamp = DateTime.MinValue;
        protected TimeSpan m_cachedMaxTime = TimeSpan.FromSeconds(30);

        protected async Task<Dictionary<string, RSACryptoServiceProvider>> FetchJWKS(string issuer)
        {
            if ((DateTime.Now - m_cachedTimestamp) < m_cachedMaxTime && issuer == m_cachedIssuer)
            {
                return m_cachedJWKS;
            }

            var result = new Dictionary<string, RSACryptoServiceProvider>();

            var jwksUrl = issuer + ".well-known/jwks.json";
            using (var client = new WebClient())
            {
                var obj = JsonConvert.DeserializeObject<JsonWebKeySet>(await client.DownloadStringTaskAsync(jwksUrl));
                if (obj.Keys != null)
                {
                    foreach (var key in obj.Keys)
                    {
                        if (key.Algorithm != "RS256" || key.Usage != "sig")
                        {
                            continue;
                        }

                        var keyId = key.KeyID;
                        RSACryptoServiceProvider rsaKey = new RSACryptoServiceProvider();
                        rsaKey.ImportParameters(new RSAParameters()
                        {
                            Exponent = Base64Url.Decode(key.Exponent),
                            Modulus = Base64Url.Decode(key.Modulus)
                        });
                        result[keyId] = rsaKey;
                    }
                }
            }

            m_cachedIssuer = issuer;
            m_cachedJWKS = result;
            m_cachedTimestamp = DateTime.Now;
            
            return result;
        }

        public async Task<TokenValidationResults> ValidateToken(string token)
        {
            if (!token.HasContent())
            {
                return TokenValidationResults.NotAuthorized;
            }

            try
            {
                var headers = JWT.Headers(token);
                var payload = JsonConvert.DeserializeObject<PayloadValidationFields>(JWT.Payload(token));
                
                var key = (await FetchJWKS(payload.Issuer))[headers["kid"].ToString()];
                var validPayload = JsonConvert.DeserializeObject<PayloadValidationFields>(JWT.Decode(token, key));

                return new TokenValidationResults() { Valid = true, UserId = validPayload.Subject };
            }
            catch (Exception)
            {
                return TokenValidationResults.NotAuthorized;
            }
        }

        public async Task<UserProfile> GetUserProfile(string token)
        {
            throw new NotImplementedException();
        }
    }

    public class JsonWebKey
    {
        [JsonProperty("alg")]
        public string Algorithm { get; set; }

        [JsonProperty("use")]
        public string Usage { get; set; }

        [JsonProperty("kid")]
        public string KeyID { get; set; }

        [JsonProperty("e")]
        public string Exponent { get; set; }

        [JsonProperty("n")]
        public string Modulus { get; set; }
    }

    public class JsonWebKeySet
    {
        [JsonProperty("keys")]
        public List<JsonWebKey> Keys { get; set; }
    }

    public class PayloadValidationFields
    {
        [JsonProperty("iss")]
        public string Issuer { get; set; }

        [JsonProperty("sub")]
        public string Subject { get; set; }

        [JsonProperty("aud")]
        public object Audiance { get; set; }

        [JsonProperty("exp")]
        public int Expiration { get; set; }
    }
}
