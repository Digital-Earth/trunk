using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Configuration;
using System.Net.Mail;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using System.Web.Configuration;
using Microsoft.AspNet.Identity;

namespace LicenseServer.App_Utilities
{
    public class SendEmailService : IIdentityMessageService
    {
        public async Task SendAsync(IdentityMessage message)
        {
            var client = new SmtpClient();
            var email = new MailMessage("Support <support@pyxisinnovation.com>", message.Destination, message.Subject, message.Body);
            if (message.Body.Contains("<html>"))
            {
                email.BodyEncoding = Encoding.UTF8;
                email.IsBodyHtml = true;
            }
            await client.SendMailAsync(email);
            return;
        }
    }
}