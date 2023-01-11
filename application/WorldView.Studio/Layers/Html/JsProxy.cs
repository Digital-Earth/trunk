using Awesomium.Core;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio.Layers.Html
{

    /// <summary>
    /// JsProxy allows easy creation of a JavaScript object that wraps C# function calls and callbacks.
    /// 
    /// The JsProxy object has the following API:
    ///    [bind-func-name] = function that is called to move into C#. 
    ///    getJSProxy - a light weight proxy object (for the caller)
    ///      
    ///    The light weight proxy object is responsible for:
    ///       1) Calling JSON.parse and JSON.stringify for complex objects.
    ///       2) Creating a callback wrapper and start.
    ///       
    /// On the C# side of things, you populate a JsProxy by using the following functions:
    ///    Bind(func-name, lambda) - The JsProxy creates a function that calls the given lambda.
    ///    BindAsync(func-name, lambda->Task T) - The JsProxy creates a function that calls the given task lambda and returns a promise with the task result.
    ///    Callback(func-name) - The JSProxy creates a function that accepts callbacks. Returns a C# action object to be invoked from C# side of things.
    ///    
    /// </summary>
    internal class JsProxy
    {
        public JSObject JavascriptObject { get; set; }

        private Dictionary<string, string> JavascriptProxyFunctions { get; set; }        
        public string ProxyName { get; set; }

        public JsProxy(WebView view, string name)
        {
            ProxyName = name;
            JavascriptObject = (JSObject)view.CreateGlobalJavascriptObject(name);
            JavascriptProxyFunctions = new Dictionary<string, string>();
            
            JavascriptObject.Bind("getJSProxy", false, (s, e) =>
            {
                dynamic func = ((JSObject)e.Arguments[0]).Clone();
                var compiledProxy = CreateCompiledProxy();

                func(view.ExecuteJavascriptWithResult("(function() { return " + compiledProxy + "; })()"));
            });
        }

        public string CreateCompiledProxy()
        {
            return "{ " + String.Join(",", JavascriptProxyFunctions.Select(f => String.Format("'{0}':{1}", f.Key, f.Value))) + " }";
        }



        private string CompileArgument<V1>(int argumentNumber)
        {
            if (typeof(V1).RequireJSON())
            {
                return "JSON.stringify(arguments[" + argumentNumber + "])";
            }
            else
            {
                return "arguments[" + argumentNumber + "]";
            }
        }

        private string CompileInvokeFunction(string funcName)
        {
            return ProxyName + "." + funcName + "()";
        }

        private string CompileInvokeFunction<V1>(string funcName)
        {
            return ProxyName + "." + funcName + "(" + CompileArgument<V1>(0) + ")";
        }

        private string CompileInvokeFunction<V1, V2>(string funcName)
        {
            return ProxyName + "." + funcName + "(" + CompileArgument<V1>(0) + "," + CompileArgument<V2>(1) + ")";
        }

        private string CompileInvokeFunction<V1, V2, V3>(string funcName)
        {
            return ProxyName + "." + funcName + "(" + CompileArgument<V1>(0) + "," + CompileArgument<V2>(1) + "," + CompileArgument<V3>(2) + ")";
        }

        private Func<string, string> CompileInvokeFunctionAsync(string funcName)
        {
            return (arg) => ProxyName + "." + funcName + "(" + arg + ")";
        }

        private Func<string, string> CompileInvokeFunctionAsync<V1>(string funcName)
        {
            return (arg) => ProxyName + "." + funcName + "(" + arg + "," + CompileArgument<V1>(0) + ")";
        }

        private Func<string, string> CompileInvokeFunctionAsync<V1, V2>(string funcName)
        {
            return (arg) => ProxyName + "." + funcName + "(" + arg + "," + CompileArgument<V1>(0) + "," + CompileArgument<V2>(1) + ")";
        }

        private Func<string, string> CompileInvokeFunctionAsync<V1, V2, V3>(string funcName)
        {
            return (arg) => ProxyName + "." + funcName + "(" + arg + "," + CompileArgument<V1>(0) + "," + CompileArgument<V2>(1) + "," + CompileArgument<V3>(2) + ")";
        }

        private Func<string, string> CompileInvokeFunctionAsync<V1, V2, V3, V4>(string funcName)
        {
            return (arg) => ProxyName + "." + funcName + "(" + arg + "," + CompileArgument<V1>(0) + "," + CompileArgument<V2>(1) + "," + CompileArgument<V3>(2) + "," + CompileArgument<V4>(3)+ ")";
        }


        private string CompileInvokeCallback(string funcName)
        {
            return "function() { " + ProxyName + "." + funcName + "(arguments[0]); }";
        }

        private string CompileReturnValueAsync<T>(Func<string, string> invokeCode)
        {
            Func<string, string> parseResult = x => x;
            if (typeof(T).RequireJSON())
            {
                parseResult = (string x) => "JSON.parse(" + x + ")";
            }

            var promise = @"{ _: { success: function (a) { var f = this.fs; if (f) { f(" + parseResult("a") + @"); } }, error: function (e) { var f = this.fe; if (f) { f(e); } } }, success: function (f) { this._.fs = f; return this; }, error: function (f) { this._.fe = f; return this; } }";

            return "function() { var p = " + promise + "; " + invokeCode("p._") + "; return p; }";
        }

        private string CompileReturnValue<T>(string invokeCode)
        {
            if (typeof(T).RequireJSON())
            {
                return "function() { return JSON.parse(" + invokeCode + "); }";
            }
            else
            {
                return "function() { return " + invokeCode + "; }";
            }
        }

        private void InvokeJavascriptCallback<T>(JSObject state, Func<T> func)
        {
            Task.Factory
                .StartNew(func)
                .ContinueWith((completedTask) =>
                {
                    if (completedTask.IsFaulted)
                    {
                        if (completedTask.Exception.InnerException != null)
                        {
                            state.InvokeAsync("error", completedTask.Exception.InnerException.Message);
                        }
                        else
                        {
                            // default AggregateException.Message "One or more exceptions has occurred"
                            state.InvokeAsync("error", completedTask.Exception.Message);
                        }
                    }
                    else
                    {
                        state.InvokeAsync("success", completedTask.Result.ToJSValue());
                    }
                }, TaskScheduler.FromCurrentSynchronizationContext());
        }

        public void Bind<T>(string name, Func<T> function)
        {
            JavascriptObject.Bind(name, true, (s, e) =>
            {
                e.Result = function().ToJSValue();
            });
            JavascriptProxyFunctions[name] = CompileReturnValue<T>(CompileInvokeFunction(name));
        }

        public void Bind<V1, T>(string name, Func<V1, T> function)
        {
            JavascriptObject.Bind(name, true, (s, e) =>
            {
                e.Result = function(e.Arguments[0].FromJSValue<V1>()).ToJSValue();
            });
            JavascriptProxyFunctions[name] = CompileReturnValue<T>(CompileInvokeFunction<V1>(name));
        }

        public void Bind<V1, V2, T>(string name, Func<V1, V2, T> function)
        {
            JavascriptObject.Bind(name, true, (s, e) =>
            {
                e.Result = function(e.Arguments[0].FromJSValue<V1>(), e.Arguments[1].FromJSValue<V2>()).ToJSValue();
            });
            JavascriptProxyFunctions[name] = CompileReturnValue<T>(CompileInvokeFunction<V1, V2>(name));
        }

        public void Bind<V1, V2, V3, T>(string name, Func<V1, V2, V3, T> function)
        {
            JavascriptObject.Bind(name, true, (s, e) =>
            {
                e.Result = function(e.Arguments[0].FromJSValue<V1>(), e.Arguments[1].FromJSValue<V2>(), e.Arguments[2].FromJSValue<V3>()).ToJSValue();
            });
            JavascriptProxyFunctions[name] = CompileReturnValue<T>(CompileInvokeFunction<V1, V2, V3>(name));
        }

        public void BindAsync<T>(string name, Func<T> function)
        {
            JavascriptObject.Bind(name, false, (s, e) =>
            {
                JSObject state = ((JSObject)e.Arguments[0]).Clone();
                InvokeJavascriptCallback(state, function);
            });
            JavascriptProxyFunctions[name] = CompileReturnValueAsync<T>(CompileInvokeFunctionAsync(name));
        }

        public void BindAsync<V1, T>(string name, Func<V1, T> function)
        {
            JavascriptObject.Bind(name, false, (s, e) =>
            {
                JSObject state = ((JSObject)e.Arguments[0]).Clone();
                var arg1 = e.Arguments[1].FromJSValue<V1>();
                InvokeJavascriptCallback(state, () => function(arg1));
            });
            JavascriptProxyFunctions[name] = CompileReturnValueAsync<T>(CompileInvokeFunctionAsync<V1>(name));
        }

        public void BindAsync<V1, V2, T>(string name, Func<V1, V2, T> function)
        {
            JavascriptObject.Bind(name, false, (s, e) =>
            {
                JSObject state = ((JSObject)e.Arguments[0]).Clone();
                var arg1 = e.Arguments[1].FromJSValue<V1>();
                var arg2 = e.Arguments[2].FromJSValue<V2>();
                InvokeJavascriptCallback(state, () => function(arg1,arg2));                    
            });
            JavascriptProxyFunctions[name] = CompileReturnValueAsync<T>(CompileInvokeFunctionAsync<V1, V2>(name));
        }

        public void BindAsync<V1, V2, V3, T>(string name, Func<V1, V2, V3, T> function)
        {
            JavascriptObject.Bind(name, false, (s, e) =>
            {
                JSObject state = ((JSObject)e.Arguments[0]).Clone();
                var arg1 = e.Arguments[1].FromJSValue<V1>();
                var arg2 = e.Arguments[2].FromJSValue<V2>();
                var arg3 = e.Arguments[3].FromJSValue<V3>();
                InvokeJavascriptCallback(state, () => function(arg1, arg2, arg3));                    
            });
            JavascriptProxyFunctions[name] = CompileReturnValueAsync<T>(CompileInvokeFunctionAsync<V1, V2, V3>(name));
        }

        public void BindAsync<V1, V2, V3, V4, T>(string name, Func<V1, V2, V3, V4, T> function)
        {
            JavascriptObject.Bind(name, false, (s, e) =>
            {
                JSObject state = ((JSObject)e.Arguments[0]).Clone();
                var arg1 = e.Arguments[1].FromJSValue<V1>();
                var arg2 = e.Arguments[2].FromJSValue<V2>();
                var arg3 = e.Arguments[3].FromJSValue<V3>();
                var arg4 = e.Arguments[4].FromJSValue<V4>();
                InvokeJavascriptCallback(state, () => function(arg1,arg2,arg3,arg4));                    
            });
            JavascriptProxyFunctions[name] = CompileReturnValueAsync<T>(CompileInvokeFunctionAsync<V1, V2, V3, V4>(name));
        }

        public Action<A1> Callback<A1>(string name)
        {
            dynamic callback = null;

            JavascriptObject.Bind(name, false, (s, e) =>
            {
                callback = ((JSObject)e.Arguments[0]).Clone();
            });

            JavascriptProxyFunctions[name] = CompileInvokeCallback(name);

            return (A1 arg) =>
            {
                if (callback != null)
                {
                    callback(arg.ToJSValue());
                }
            };
        }

        public Action<A1,A2> Callback<A1,A2>(string name)
        {
            dynamic callback = null;

            JavascriptObject.Bind(name, false, (s, e) =>
            {
                callback = ((JSObject)e.Arguments[0]).Clone();
            });

            JavascriptProxyFunctions[name] = CompileInvokeCallback(name);

            return (A1 arg1,A2 arg2) =>
            {
                if (callback != null)
                {
                    callback(arg1.ToJSValue(), arg2.ToJSValue());
                }
            };
        }

        public Action<A1, A2, A3> Callback<A1, A2, A3>(string name)
        {
            dynamic callback = null;

            JavascriptObject.Bind(name, false, (s, e) =>
            {
                callback = ((JSObject)e.Arguments[0]).Clone();
            });

            JavascriptProxyFunctions[name] = CompileInvokeCallback(name);

            return (A1 arg1, A2 arg2, A3 arg3) =>
            {
                if (callback != null)
                {
                    callback(arg1.ToJSValue(), arg2.ToJSValue(), arg3.ToJSValue());
                }
            };
        }
    }

    public static class JsUtility
    {
        public static JSValue ToJSValue<T>(this T obj)
        {
            if (obj == null)
            {
                return JSValue.Undefined;
            }
            if (obj is int)
            {
                return new JSValue(System.Convert.ToInt32(obj));
            }
            else if (obj is bool)
            {
                return new JSValue(System.Convert.ToBoolean(obj));
            }
            else if (obj is float || obj is double)
            {
                return new JSValue(System.Convert.ToDouble(obj));
            }
            else if (obj is string || obj is char || obj is JsonString)
            {
                return new JSValue(obj.ToString());
            }            
            else
            {
                return new JSValue(JsonConvert.SerializeObject(obj));
            }
        }

        public static bool RequireJSON(this Type type)
        {
            return type != typeof(int) && type != typeof(double) && type != typeof(char) && type != typeof(string);
        }

        public static T FromJSValue<T>(this JSValue value)
        {
            try
            {
                if (value.IsUndefined || value.IsNull)
                {
                    return default(T);
                }
                else if (typeof(T) == typeof(int))
                {
                    return (T)Convert.ChangeType((int)value, typeof(T));
                }
                else if (typeof(T) == typeof(double))
                {
                    return (T)Convert.ChangeType((double)value, typeof(T));
                }
                else if (typeof(T) == typeof(string))
                {
                    return (T)Convert.ChangeType((string)value, typeof(T));
                }
                else if (typeof(T) == typeof(JsonString))
                {
                    return (T)(object)new JsonString((string)value);
                }                    
                else
                {
                    return JsonConvert.DeserializeObject<T>((string)value);
                }
            }
            catch (Exception ex)
            {
                Trace.error(ex.Message);
                return default(T);
            }
        }
    }

    /// <summary>
    /// Utility class used for JSON.Parse on the JavaScript side but treat it as a string on C# side
    /// </summary>
    public class JsonString
    {
        private readonly string m_value;

        public JsonString(string value)
        {
            m_value = value;
        }

        public override string ToString()
        {
            return m_value;
        }

        public override bool Equals(object obj)
        {
            if (obj is JsonString)
            {
                return m_value.Equals((obj as JsonString).m_value);
            }
            return m_value.Equals(obj);
        }

        public override int GetHashCode()
        {
            return m_value.GetHashCode();
        }

        public static bool operator ==(JsonString a, JsonString b)
        {
            if (Object.ReferenceEquals(a, b)) return true;
            return a.Equals(b);
        }

        public static bool operator !=(JsonString a, JsonString b)
        {
            return !(a == b);
        }
    }
}
