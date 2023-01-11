using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace Pyxis.Contract.Publishing
{
    public interface IPyxisIdentityUser
    {
        bool? AcceptTerms { get; set; }
        bool? PromotionConsent { get; set; }
        bool? AccountConsent { get; set; }

        string Email { get; set; }
        bool? EmailConfirmed { get; set; }
        string ExternalLoginProvider { get; set; }
        string Country { get; set; }
        string City { get; set; }

        string BusinessName { get; set; }
        bool? CollectTax { get; set; }
        string PayPalEmail { get; set; }
    }
}