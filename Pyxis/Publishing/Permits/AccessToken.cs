using System;
using Newtonsoft.Json;
using Pyxis.Contract;
using RestSharp;

namespace Pyxis.Publishing.Permits
{
    public class AccessToken : IPermit
    {
        public class TokenDetails
        {
            [JsonProperty(propertyName: "access_token")]
            public string Token { get; set; }

            [JsonProperty(propertyName: "token_type")]
            public string TokenType { get; set; }

            [JsonProperty(propertyName: "expires_in")]
            public long ExpiresIn { get; set; }

            [JsonProperty(propertyName: "userName")]
            public string UserName { get; set; }

            [JsonProperty(propertyName: ".issued")]
            public DateTime Issued { get; set; }

            [JsonProperty(propertyName: ".expires")]
            public DateTime Expires { get; set; }
        }

        private TokenDetails Details { get; set; }

        public PermitType PermitType { get { return PermitType.AccessToken; } }
        public string UserName { get { return Details.UserName; } }
        public DateTime Issued { get { return Details.Issued; }  }
        public DateTime Expires { get { return Details.Expires; }  }
        public string Token { get { return Details.Token; } }

        public AccessToken(TokenDetails token)
        {
            Details = token;
        }

        public void Authenticate(RestRequest request)
        {
            request.AddHeader("Authorization", "Bearer " + Details.Token);
        }

        public string AsUrlComponent()
        {
            return Uri.EscapeDataString(JsonConvert.SerializeObject(Details));
        }
    }
}
