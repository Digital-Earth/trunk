using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.WebPages;

namespace PyxCrawler.App_Start
{
    public class DisplayModeConfig
    {
        public static void RegisterDisplayModes()
        {
            DisplayModeProvider.Instance.Modes.Insert(0,
               new DefaultDisplayMode("Phone")
               {
                   ContextCondition = (context => (
                     (context.GetOverriddenUserAgent() != null) &&
                     (
                       (context.GetOverriddenUserAgent().IndexOf("iPhone", StringComparison.OrdinalIgnoreCase) >= 0) ||
                       (context.GetOverriddenUserAgent().IndexOf("iPod", StringComparison.OrdinalIgnoreCase) >= 0) ||
                       (context.GetOverriddenUserAgent().IndexOf("Droid", StringComparison.OrdinalIgnoreCase) >= 0) ||
                       (context.GetOverriddenUserAgent().IndexOf("Blackberry", StringComparison.OrdinalIgnoreCase) >= 0) ||
                       (context.GetOverriddenUserAgent().StartsWith("Blackberry", StringComparison.OrdinalIgnoreCase))
                     )
                   ))
               });
            DisplayModeProvider.Instance.Modes.Insert(0,
              new DefaultDisplayMode("Tablet")
              {
                  ContextCondition = (context => (
                    (context.GetOverriddenUserAgent() != null) &&
                    (
                      (context.GetOverriddenUserAgent().IndexOf("iPad", StringComparison.OrdinalIgnoreCase) >= 0) ||
                      (context.GetOverriddenUserAgent().IndexOf("Playbook", StringComparison.OrdinalIgnoreCase) >= 0) ||
                      (context.GetOverriddenUserAgent().IndexOf("Transformer", StringComparison.OrdinalIgnoreCase) >= 0) ||
                      (context.GetOverriddenUserAgent().IndexOf("Kindle", StringComparison.OrdinalIgnoreCase) >= 0) ||
                      (context.GetOverriddenUserAgent().IndexOf("Xoom", StringComparison.OrdinalIgnoreCase) >= 0)
                    )
                  ))
              });
        }
    }
}