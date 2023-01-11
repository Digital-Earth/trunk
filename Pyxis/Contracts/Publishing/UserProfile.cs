using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Publishing
{
    public class UserProfile : User, IPyxisIdentityUser
    {
        public bool? AcceptTerms { get; set; }
        public bool? PromotionConsent { get; set; }
        public bool? AccountConsent { get; set; }

        public List<string> Roles { get; set; }
        public string ExternalLoginProvider { get; set; } 

        public string UserName { get; set; }
        public string FirstName { get; set; }
        public string LastName { get; set; }
        public string Email { get; set; }
        public bool? EmailConfirmed { get; set; }
        public string Country { get; set; }
        public string City { get; set; }

        public string BusinessName { get; set; }
        public bool? CollectTax { get; set; }
        public string PayPalEmail { get; set; }
    }
}
