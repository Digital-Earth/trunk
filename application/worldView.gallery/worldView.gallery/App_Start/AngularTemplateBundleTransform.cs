using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Web;
using System.Web.Optimization;

namespace worldView.gallery
{
    /// <summary>
    /// Take collection of html files and import them into angular $templateCache manually.
    /// 
    /// The content of the bundle would look like:
    /// 
    /// AppName.run(function($templateCache) { 
    ///   templates = {id:template,id:template,....}; 
    ///   angular.forEach(templates,function(template,id) { 
    ///     $templateCache.put(id,template); 
    ///   });
    /// });
    /// </summary>
    public class AngularTemplateBundleTransform : IBundleTransform
    {
        /// <summary>
        /// the name of the app
        /// </summary>
        public string AppName { get; set; }
        public string Prefix { get; set; }

        public AngularTemplateBundleTransform()
        {
            AppName = "app";
        }

        public void Process(BundleContext context, BundleResponse response)
        {
            var templates = new Dictionary<string, string>();

            var contentBuilder = new StringBuilder();
            foreach (var file in response.Files)
            {
                var templateId = file.VirtualFile.VirtualPath;
                if (!String.IsNullOrEmpty(Prefix) && templateId.StartsWith(Prefix))
                {
                    templateId = templateId.Substring(Prefix.Length);
                }
                using (var templateFile = new StreamReader(file.VirtualFile.Open()))
                {
                    templates[templateId] = templateFile.ReadToEnd(); 
                }
                
            }

            contentBuilder.Append(AppName + ".run(function($templateCache) { templates = ");
            contentBuilder.Append(Newtonsoft.Json.JsonConvert.SerializeObject(templates));
            contentBuilder.Append("; angular.forEach(templates,function(template,id) { $templateCache.put(id,template); }); });");
            response.Content = contentBuilder.ToString();
            response.ContentType = "text/javascript";
        }
    }
}