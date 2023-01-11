using Awesomium.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio.Layers.Html
{
    internal class JsProxyFactory
    {
        public WebView View { get; private set; }

        public string FactoryName { get; private set; }

        public JSObject JavascriptFactoryObject { get; set; }
        public JSObject JavascriptProxiesObject { get; set; }

        private Dictionary<string, JsProxy> Proxies { get; set; }

        public JsProxyFactory(WebView view, string name)
        {
            View = view;
            FactoryName = name;
            Proxies = new Dictionary<string, JsProxy>();

            JavascriptFactoryObject = (JSObject)view.CreateGlobalJavascriptObject(name);
            JavascriptProxiesObject = (JSObject)view.CreateGlobalJavascriptObject(name + "._");

            JavascriptFactoryObject.Bind("has", true, (s, e) =>
            {
                string proxyName = e.Arguments[0];
                e.Result = Proxies.ContainsKey(proxyName);
            });

            JavascriptFactoryObject.Bind("get", false, (s, e) =>
            {
                string proxyName = e.Arguments[0];
                if (!Proxies.ContainsKey(proxyName))
                {
                    return;
                }

                dynamic successFunc = ((JSObject)e.Arguments[1]).Clone();
                
                dynamic errorFunc = null;
                if (e.Arguments.Length > 2)
                {
                    errorFunc = ((JSObject)e.Arguments[2]).Clone();
                }

                var compiledProxy = Proxies[proxyName].CreateCompiledProxy();
                try
                {
                    //specific return type JSObject as ExecuteJavascriptWithResult return JSValue by default.
                    JSObject proxy = view.ExecuteJavascriptWithResult("(function() { return " + compiledProxy + "; } )()");

                    if (proxy == null)
                    {
                        throw new Exception("failed to compile proxy");
                    }

                    successFunc(proxy);
                }
                catch (Microsoft.CSharp.RuntimeBinder.RuntimeBinderException)
                {
                    Trace.info("Failed to invoke successFunc when proxy failed to compile. This error is hard to recover from. Refreshing view.");
                    view.Reload(false);
                }
                catch (Exception ex)
                {
                    Trace.info(String.Format("failed to register proxy {0} : {1}", proxyName, ex.Message));

                    try
                    {
                        if (errorFunc != null)
                        {
                            errorFunc(proxyName);
                        }
                    }
                    catch (Microsoft.CSharp.RuntimeBinder.RuntimeBinderException)
                    {
                        Trace.info("Failed to invoke errorFunc when proxy failed to compile. This error is hard to recover from. Refreshing view.");
                        view.Reload(false);
                    }
                }
            });
        }

        /// <summary>
        /// Create a JS Proxy
        /// </summary>
        /// <param name="name">name of the proxy</param>
        /// <param name="createProxyAction">A function that add C# bindings to the JS proxy</param>
        /// <returns>return created JsProxy</returns>
        public JsProxy CreateProxy(string name,Action<JsProxy> registerProxyFunctions = null)
        {
            var proxy = new JsProxy(View, FactoryName + "._." + name);

            if (registerProxyFunctions != null)
            {
                registerProxyFunctions(proxy);
            }

            Proxies[name] = proxy;

            return proxy;
        }
    }
}
