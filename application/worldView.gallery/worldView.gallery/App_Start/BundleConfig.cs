using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Web.Optimization;
using System.Web.Routing;

namespace worldView.gallery
{
    public class BundleConfig
    {
        internal static void RegisterBundles(BundleCollection bundles)
        {
            

            bundles.Add(new ScriptBundle("~/bundles/jquery")
                .Include(
                    "~/scripts/jquery-2.1.1.js"
                ));

            bundles.Add(new ScriptBundle("~/bundles/tweenmax")
                .Include(
                    "~/scripts/TweenMax.min.js"
                ));
  
            bundles.Add(new ScriptBundle("~/bundles/angular")
                .Include(
                    "~/scripts/angular-1.2.20.js",
                    "~/scripts/angular-cookies-1.2.20.js",
                    "~/scripts/angular-route-1.2.20.js",
                    "~/scripts/angular-autocomplete.js"
                ));

            var pyxisBundle = new ScriptBundle("~/bundles/pyxis")
                .Include(
                    "~/scripts/pyxis/pyxis.js",
                    "~/scripts/pyxis/pyxis.user.js",
                    "~/scripts/pyxis/pyxis.gallery.js",
                    "~/scripts/pyxis/pyxis.area.js"
                );
            pyxisBundle.Transforms.Clear();

            bundles.Add(pyxisBundle);

            var siteBundle = new ScriptBundle("~/bundles/site")
              .Include(
                    "~/scripts/jquery-2.1.1.js",
                    "~/scripts/TweenMax.min.js",  
                    "~/scripts/angular-1.2.20.js",
                    "~/scripts/angular-cookies-1.2.20.js",
                    "~/scripts/angular-route-1.2.20.js",
                    "~/scripts/angular-autocomplete.js",
                    "~/scripts/pyxis/pyxis.js",
                    "~/scripts/pyxis/pyxis.user.js",
                    "~/scripts/pyxis/pyxis.gallery.js",
                    "~/scripts/pyxis/pyxis.area.js",
                    "~/contents/scripts/pyxis-ui.js",
                    "~/contents/scripts/analytics.js",
                    "~/contents/scripts/site.js",
                    "~/contents/scripts/studio/legacyservices/positionHelper.js"
              )
              .IncludeDirectory("~/contents/scripts/site", "*.js", true);
            siteBundle.Transforms.Clear();
            bundles.Add(siteBundle);

            bundles.Add(new ScriptBundle("~/bundles/studio-frameworks")
                .Include(
                    "~/scripts/jquery-2.1.1.js",
                    "~/scripts/TweenMax.min.js",
                    "~/scripts/angular-1.2.20.js",
                    "~/scripts/angular-cookies-1.2.20.js",
                    "~/scripts/angular-route-1.2.20.js",
                    "~/scripts/angular-autocomplete.js"
                ));

            var studioBundle = new ScriptBundle("~/bundles/studio")
                .Include(
                    "~/contents/scripts/flux/dispatcher.js",
                    "~/contents/scripts/analytics.js",
                    "~/contents/scripts/pyxis-ui.js",
                    "~/contents/scripts/studio.js",
                    "~/contents/scripts/studioConfig.js",
                    "~/contents/scripts/site/directives.js",
                    "~/contents/scripts/studio/directives.js"
                )
                .IncludeDirectory("~/contents/scripts/studio/legacyservices", "*.js", false)
                .IncludeDirectory("~/contents/scripts/studio/features", "*.js", false)
                .IncludeDirectory("~/contents/scripts/studio/services", "*.js", false)
                .IncludeDirectory("~/contents/scripts/studio/stores", "*.js", false);                

            studioBundle.Transforms.Clear();
            bundles.Add(studioBundle);


            bundles.Add(new ScriptBundle("~/bundles/studio-demo-frameworks")
                .Include(
                    "~/scripts/jquery-2.1.1.js",
                    //TODO: fix double reference of TweenMax in bundle.js and then uncomment this.
                    //"~/scripts/TweenMax.min.js",
                    "~/scripts/angular-1.2.20.js",
                    "~/scripts/angular-cookies-1.2.20.js",
                    "~/scripts/angular-route-1.2.20.js",
                    "~/scripts/angular-autocomplete.js",
                    "~/studio-demo/assets/scripts/vendor/underscore-min.js",
                    "~/studio-demo/assets/scripts/vendor/dat.gui.min.js",
                    "~/studio-demo/three.min.js",
                    "~/studio-demo/bundle.js"
                )); 


            var studioLocalizationBundle = new ScriptBundle("~/bundles/studio-localization")
                .Include(
                    "~/scripts/localization.js",
                    "~/i18n/studio.default.js"
                );
            studioLocalizationBundle.Transforms.Clear();
            bundles.Add(studioLocalizationBundle);

            var siteTemplatesBundle = new Bundle("~/bundles/site-templates")
                .Include(
                "~/contents/templates/Menus/*.html"
                );

            siteTemplatesBundle.Transforms.Add(new AngularTemplateBundleTransform()
            {
                Prefix = "/contents/templates"
            });
            bundles.Add(siteTemplatesBundle);

            var studioTemplatesBundle = new Bundle("~/bundles/studio-templates")
                .Include(
                "~/contents/templates/studio/*.html",
                "~/contents/templates/studio/popup-menu/*.html",
                "~/contents/templates/studio/template/*.html"
                );

            studioTemplatesBundle.Transforms.Add(new AngularTemplateBundleTransform()
            { 
                Prefix = "/contents/templates/studio"
            });
            bundles.Add(studioTemplatesBundle);

            var studioDefaultSkinBundle = new ScriptBundle("~/bundles/studio-default-skin")
                .Include("~/contents/scripts/studio/skin/bootstrap.js")
                .Include("~/contents/scripts/studio/skin/default.js");                
            
            studioDefaultSkinBundle.Transforms.Clear();
            bundles.Add(studioDefaultSkinBundle);

            var studioWebSkinBundle = new ScriptBundle("~/bundles/studio-web-skin")
                .Include("~/contents/scripts/studio/skin/bootstrap.js")
                .Include("~/contents/scripts/studio/skin/web.js");

            studioWebSkinBundle.Transforms.Clear();
            bundles.Add(studioWebSkinBundle);

            var studioDemoSkinBundle = new ScriptBundle("~/bundles/studio-demo-skin")
                .IncludeDirectory("~/contents/scripts/studio/demos", "*.js", false)
                .Include("~/contents/scripts/studio/skin/bootstrap.js")
                .Include("~/contents/scripts/studio/skin/demo.js");

            studioDemoSkinBundle.Transforms.Clear();
            bundles.Add(studioDemoSkinBundle);
        }
    }
}
