using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Builder;
using System.Web.Http.OData.Query;
using LicenseServer.Controllers;
using LicenseServer.DTOs;
using LicenseServer.Models.Mongo;
using Newtonsoft.Json;

namespace LicenseServer.App_Utilities
{
    public static class CrmManager
    {
        private static readonly bool s_enabled = false;

        private static readonly string s_crmUrl = "https://secure.solve360.com";
        private static readonly NetworkCredential s_credentials;

        private static class CrmReferenceIds
        {
            public readonly static string CrmOwner = "112499556";
            public readonly static string UnconfirmedUserCategory = "148595190";
            public readonly static string RegisteredUserCategory = "139166035";
            public readonly static string NewsletterCategory = "123502088";
            public readonly static string WelcomeTemplate = "130561926";
        }
        
        static CrmManager()
        {
            s_credentials = new NetworkCredential(ConfigurationManager.AppSettings["Solve360UserName"], ConfigurationManager.AppSettings["Solve360Token"]);
            s_enabled = bool.Parse(ConfigurationManager.AppSettings["crm:enabled"]);
        }

        public static string AddContact(PyxisIdentityUser identity)
        {
            if (!s_enabled)
            {
                return "";
            }

            var categories = new List<string>();
            if (identity.IsInRole(PyxisIdentityRoles.Member))
            {
                categories.Add(CrmReferenceIds.RegisteredUserCategory);
            }
            else
            {
                categories.Add(CrmReferenceIds.UnconfirmedUserCategory);
            }
            if (identity.PromotionConsent.Value)
            {
                categories.Add(CrmReferenceIds.NewsletterCategory);
            }

            var solve360ContactModel = new
                {
                    firstname = identity.FirstName,
                    lastname = identity.LastName,
                    businessemail = identity.Email,
                    categories = new {add = categories},
                    ownership = CrmReferenceIds.CrmOwner,
                    custom15743704 = identity.Country,
                    custom15812839 = identity.City
                };

            var client = PrepareWebClient();

            dynamic result = JsonConvert.DeserializeObject(
                client.UploadString(
                    new Uri(s_crmUrl + "/contacts/"),
                    JsonConvert.SerializeObject(solve360ContactModel)));

            if(identity.IsInRole(PyxisIdentityRoles.Member))
            {
                ExecuteActivityTemplate(Convert.ToInt32(result.item.id), CrmReferenceIds.WelcomeTemplate);
            }

            return result.item.id;
        }

        public static void SetAccountConfirmation(PyxisIdentityUser identity)
        {
            if (!s_enabled)
            {
                return;
            }

            var command = identity.IsInRole(PyxisIdentityRoles.Member)
                ? (object)new
                {
                    add = new[] { CrmReferenceIds.RegisteredUserCategory },
                    remove = new[] { CrmReferenceIds.UnconfirmedUserCategory }
                }
                : new
                {
                    add = new[] { CrmReferenceIds.UnconfirmedUserCategory },
                    remove = new[] { CrmReferenceIds.RegisteredUserCategory }
                };
            var solve360UpdateModel = new
            {
                categories = command
            };

            UpdateModel(identity, solve360UpdateModel);

            if (identity.IsInRole(PyxisIdentityRoles.Member))
            {
                ExecuteActivityTemplate(Convert.ToInt32(identity.CrmId), CrmReferenceIds.WelcomeTemplate);
            }
        }

        public static void SetConsent(PyxisIdentityUser identity, bool promotionConsent)
        {
            if (!s_enabled)
            {
                return;
            }

            if (String.IsNullOrWhiteSpace(identity.CrmId))
            {
                return;
            }

            var command = promotionConsent 
                ? (object) new
                {
                    add = new[] {CrmReferenceIds.NewsletterCategory}
                }
                : new
                {
                    remove = new[] {CrmReferenceIds.NewsletterCategory}
                };
            var solve360UpdateModel = new
                {
                    categories = command
                };

            UpdateModel(identity, solve360UpdateModel);
        }

        public static void AddNote(PyxisIdentityUser identity, string note)
        {
            if (!s_enabled)
            {
                return;
            }

            if (String.IsNullOrWhiteSpace(identity.CrmId))
            {
                return;
            }
            var solve360NoteModel = new
            {
                parent = identity.CrmId,
                data = new
                {
                    details = note
                }
            };

            var client = PrepareWebClient();

            dynamic noteResult = JsonConvert.DeserializeObject(
               client.UploadString(
                   new Uri(s_crmUrl + "/contacts/note"),
                   JsonConvert.SerializeObject(solve360NoteModel)));
        }

        private static WebClient PrepareWebClient()
        {
            //client headers are been reset after every request.
            var client = new WebClient { Credentials = s_credentials };
            client.Headers[HttpRequestHeader.ContentType] = "application/json";
            client.Headers[HttpRequestHeader.Accept] = "application/json";
            return client;
        }

        private static void ExecuteActivityTemplate(int contactId, string templateId)
        {
            WebClient client;
            var solve360TemplateActivityModel = new
            {
                parent = contactId,
                data = new
                {
                    templateid = templateId
                }
            };

            client = PrepareWebClient();

            dynamic templateResult = JsonConvert.DeserializeObject(
                client.UploadString(
                    new Uri(s_crmUrl + "/contacts/template"),
                    JsonConvert.SerializeObject(solve360TemplateActivityModel)));
        }

        private static void UpdateModel(PyxisIdentityUser identity, object solve360UpdateModel)
        {
            var client = PrepareWebClient();

            dynamic noteResult = JsonConvert.DeserializeObject(
                client.UploadString(
                    new Uri(s_crmUrl + "/contacts/" + identity.CrmId),
                    "PUT",
                    JsonConvert.SerializeObject(solve360UpdateModel)));
        }
    }
}