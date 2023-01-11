using Microsoft.WindowsAzure.Storage;
using Newtonsoft.Json;
using Pyxis.Contract.Diagnostics;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Net;
using System.Net.Mail;
using System.Text;
using System.Web;
using System.Web.Mvc;
using worldView.gallery.Models;
using worldView.gallery.Services;

namespace worldView.gallery.Controllers
{
    public class ApiController : Controller
    {
        [HttpPost]
        public void Feedback(Feedback feedback)
        {
            System.Diagnostics.Trace.WriteLine("we got feedback");

            SmtpClient smtpClient = new SmtpClient();
            var supportEmail = Configuration.SiteConfiguration.SupportEmail.Email;

            MailMessage mail = new MailMessage();
            mail.To.Add(supportEmail);
            mail.Subject = feedback.Subject ?? ("WorldView.Gallery feedback from " + feedback.Name);
            mail.Body = String.Format("User:{1} [{2}]\nEmail:{3}\n\nUrl:{0}\n\nMessage:\n{4}", feedback.Url, feedback.Name, feedback.UserId, feedback.Email, feedback.Body);

            if (feedback.Attachments != null)
            {
                foreach (var attachment in feedback.Attachments)
                {
                    var stream = new System.IO.MemoryStream(Convert.FromBase64String(attachment.Base64Content));
                    var mailAttachment = new Attachment(stream, attachment.Name, attachment.MimeType);
                    mail.Attachments.Add(mailAttachment);
                }
            }

            smtpClient.Send(mail);
        }       

        public string GetBetaKey(string email)
        {
            if (Request.IsLocal)
            {
                return BetaKeyGenerator.GetKeyForEmail(email);
            }
            throw new HttpException(404,"Not Found");
        }

        public bool ValidateBetaKey(string email, string key)
        {
            return BetaKeyGenerator.IsKeyValidForEmail(email,key);
        }

        //[HttpPost]
        [ValidateInput(false)]
        public ActionResult Create(string resourceType)
        {
            dynamic body;
            try
            {
                Request.InputStream.Position = 0;
                using (var stream = new System.IO.StreamReader(Request.InputStream))
                {
                    var bodyText = stream.ReadToEnd();
                    body = JsonConvert.DeserializeObject(bodyText);
                    body.Type = resourceType ?? "GeoSource";
                }
            }
            catch (Exception)
            {
                var metadata = new
                        {
                            Name = Request.Form["name"],
                            Description = Request.Form["description"],
                            Tags = (Request.Form["tags"] ?? "")
                                .Split(',')
                                .Select(x => x.Trim())
                                .Where(x => !String.IsNullOrEmpty(x))
                                .Select(x => FixTagName(x))
                                .ToList(),

                            SystemTags = (Request.Form["system-tags"] ?? "")
                                .Split(',')
                                .Select(x => x.Trim())
                                .Where(x => !String.IsNullOrEmpty(x))
                                .Select(x => FixTagName(x))
                                .ToList()
                        };

                var type = Request.Form["type"] ?? resourceType;

                if (type == "GeoSource")
                {
                    body = new
                    {
                        Type = type,
                        Id = Request.Form["id"] ?? Guid.NewGuid().ToString(),
                        Definition = Request.Form["definition"],
                        ProcRef = Request.Form["proc-ref"],
                        BasedOn = (Request.Form["based-on"] ?? "")
                                .Split(',')
                                .Select(x => x.Trim())
                                .Where(x => !String.IsNullOrEmpty(x))
                                .ToList(),
                        DataSize = int.Parse(Request.Form["data-size"] ?? "0"),
                        Metadata = metadata
                    };


                }
                else if (type == "Map")
                {
                    dynamic camera = null;

                    if (Request.Form["camera-latitude"] != null && Request.Form["camera-longitude"] != null)
                    {
                        camera = new {
                            Latitude = int.Parse(Request.Form["camera-latitude"]),
                            Longitude = int.Parse(Request.Form["camera-longitude"]),
                            Heading = int.Parse(Request.Form["camera-heading"]??"0"),
                            Tilt = int.Parse(Request.Form["camera-tilt"] ?? "0"),
                            Altitude = int.Parse(Request.Form["camera-altitude"] ?? "0"),
                            Range = int.Parse(Request.Form["camera-range"] ?? "10000"),
                        };
                    }

                    body = new
                    {
                        Type = type,
                        Id = Request.Form["id"] ?? Guid.NewGuid().ToString(),
                        Definition = Request.Form["definition"],
                        ProcRef = Request.Form["proc-ref"],
                        BasedOn = (Request.Form["based-on"] ?? "")
                                .Split(',')
                                .Select(x => x.Trim())
                                .Where(x => !String.IsNullOrEmpty(x))
                                .ToList(),
                        Camera = camera,
                        Metadata = metadata
                    };
                }
                else
                {
                    return Redirect("/");
                }
            }

            TempData["CreateBody"] = body;

            if (Request.QueryString["app"] != null && Request.QueryString["token"] != null)
            {
                return RedirectToRoute("create", new { type = body.Type, app = Request.QueryString["app"], token = Request.QueryString["token"] });
            }

            return RedirectToRoute("create",new { type = body.Type });
        }

        [HttpPost]
        public bool UploadCrash()
        {
            var files = Request.Files;
            if (files.Count > 0)
            {
                var file = Request.Files[0];
                
                var storageAccount = CloudStorageAccount.Parse(Configuration.SiteConfiguration.CrashDump.ConnectionString);
                var blobClient = storageAccount.CreateCloudBlobClient();

                var container = blobClient.GetContainerReference(Configuration.SiteConfiguration.CrashDump.ContainerName);
                container.CreateIfNotExists();

                var blockBlob = container.GetBlockBlobReference(file.FileName);
                blockBlob.UploadFromStream(file.InputStream);

                return true;
            }
            return false;
        }

        /// <summary>
        /// Sanitize tag name. basically make sure there is only 1 space between words and for each word, the first letter is capital 
        /// "oil" => "Oil",
        /// "OGC" => "OGC",
        /// "WebService" => "WebService"
        /// "alberta rocks!" => "Alberta Rocks!"
        /// </summary>
        private string FixTagName(string tag)
        {
            var result = new StringBuilder();
            var beginOfWord = true;
            foreach (var c in tag)
            {
                if (beginOfWord)
                {
                    if (Char.IsWhiteSpace(c)) { continue; }
                    if (Char.IsLower(c))
                    {
                        result.Append(Char.ToUpper(c));
                    }
                    else
                    {
                        result.Append(c);
                    }
                    beginOfWord = false;
                }
                else
                {
                    result.Append(c);
                    if (Char.IsWhiteSpace(c))
                    {
                        beginOfWord = true;
                    }
                }
            }
            return result.ToString();
        }
    }
}