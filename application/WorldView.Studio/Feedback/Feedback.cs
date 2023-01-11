using Newtonsoft.Json;
using Pyxis.WorldView.Studio.Properties;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Pyxis.WorldView.Studio.Feedback
{
    /// <summary>
    /// Feedback is a helper class that takes a screenshot and collects the trace.log from the running process.
    ///     
    /// Feedback contains addition information like:
    /// 1) current user
    /// 2) process CPU time
    /// 3) process memory 
    /// 4) OS
    /// 
    /// Usage:
    /// 
    /// var feedback = Feedback.Create(form); //the main application.
    /// 
    /// feedback.AttachScreenhot = true;
    /// feedback.AttachTrace = false;
    /// feddback.Message = "My application got frozen"
    /// 
    /// feedback.Send();
    /// </summary>
    class Feedback
    {
        private Pyxis.Contract.Diagnostics.Feedback Model { get; set; }

        private Bitmap Screenshot { get; set; }
        private string TraceFileContent { get; set; }

        /// <summary>
        /// Define if the feedback message should include a screen capture of current application
        /// </summary>
        public bool AttachScreenshot { get; set; }

        /// <summary>
        /// Define if the feedback message should include the trace.log
        /// </summary>
        public bool AttachTrace { get; set; }

        /// <summary>
        /// an attached string text to the feedback message.
        /// </summary>
        public string Message
        {
            get
            {
                return Model.Body;
            }
            set
            {
                Model.Body = value;
            }
        }

        private Feedback()
        {
            AttachScreenshot = true;
            AttachTrace = true;
        }

        public static Feedback Create(ApplicationForm form)
        {
            var feedback = new Feedback();

            feedback.Model = new Contract.Diagnostics.Feedback()
            {
                Subject = "WorldView.Studio (Version: " + Application.ProductVersion.ToString() + ") feedback",
                Body = "",
                Name = "",
                Url = form.HtmlLayer.Uri.ToString(),
                Attachments = new List<Contract.Diagnostics.FeedbackAttachment>()
            };

            feedback.Screenshot = form.PyxisView.SaveToBitmap();

            var process = System.Diagnostics.Process.GetCurrentProcess();

            const double bytesInMb = 1024.0 * 1024.0;

            Trace.info(String.Format(
@"Process:
    CPU time:
        Total: {0}
        User: {1}
        Kernel: {2}
    Memory:
        Current: {3:0.##} MB
        Peak: {4:0.##} MB
    Active Threads: {5}
OS: {6}",
                        process.TotalProcessorTime,
                        process.UserProcessorTime,
                        process.PrivilegedProcessorTime,
                        process.WorkingSet64 / bytesInMb,
                        process.PeakWorkingSet64 / bytesInMb,
                        process.Threads.Count,                        
                        Environment.OSVersion.VersionString
                        ));


            Trace.flush();
            var fileStream = File.Open(form.EngineConfig.WorkingDirectory + Path.DirectorySeparatorChar + "trace.log", FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            feedback.TraceFileContent = new StreamReader(fileStream).ReadToEnd();
            
            var user = form.Engine.GetUser();
            if (user != null)
            {
                feedback.Model.Name = user.UserName;

                try
                {
                    feedback.Model.Email = user.GetProfile().Email;
                }
                catch (Exception)
                {
                    feedback.Model.Email = "[failed]";
                }
            }

            return feedback;
        }

        public void Send() 
        {
            Model.Attachments = new List<Contract.Diagnostics.FeedbackAttachment>();

            if (AttachTrace)
            {
                Model.Attachments.Add(new Contract.Diagnostics.FeedbackAttachment
                {
                    Base64Content = Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(TraceFileContent)),
                    Name = "trace.log",
                    MimeType = "plain/text"
                });
            }

            if (AttachScreenshot)
            {
                using (var stream = new System.IO.MemoryStream())
                {
                    Screenshot.Save(stream, System.Drawing.Imaging.ImageFormat.Png);

                    Model.Attachments.Add(new Contract.Diagnostics.FeedbackAttachment
                    {
                        Base64Content = Convert.ToBase64String(stream.ToArray()),
                        Name = "screenshot.png",
                        MimeType = "image/png"
                    });
                }
            }

            var webClient = new WebClient();
            webClient.Headers.Add(HttpRequestHeader.ContentType, "application/json");
            webClient.UploadString(new Uri(Settings.Default.FeedbackUrl), "POST", JsonConvert.SerializeObject(Model));            
        }
    }
}
