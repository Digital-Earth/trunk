using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using GeoWebCore.Models;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Utilities;

namespace GeoWebCore.Services
{
    static class ExpressionPublisher
    {
        private static object s_tasksLock = new object();
        private static Dictionary<string, Task<GeoSource>> s_tasks = new Dictionary<string, Task<GeoSource>>();

        public static Task<GeoSource> PublishExpressionAsync(PublishExpressionRequest request, PublishRequest publishRequest)
        {
            var key = GetRequestKey(request,publishRequest);

            Task<GeoSource> task = null;

            lock (s_tasksLock)
            {
                if (!s_tasks.TryGetValue(key, out task))
                {
                    task = new Task<GeoSource>(() => PublishExpression(request, publishRequest));

                    s_tasks[key] = task;

                    task.ContinueWith(async oldTask =>
                    {
                        await Task.Delay(TimeSpan.FromMinutes(5));
                        
                        lock (s_tasksLock)
                        {
                            if (s_tasks.ContainsKey(key) && s_tasks[key] == task)
                            {
                                s_tasks.Remove(key);
                            }
                        }
                    });

                    task.Start();
                }
            }

            return task;
        }

        public static void ForgetTask(Task<GeoSource> task)
        {
            lock (s_tasksLock)
            {
                foreach (var key in s_tasks.Keys)
                {
                    if (s_tasks[key] == task)
                    {
                        s_tasks.Remove(key);
                        return;
                    }
                }
            }
        }

        private static string GetRequestKey(PublishExpressionRequest request, PublishRequest publishRequest)
        {
            var data = JsonConvert.SerializeObject(request) + JsonConvert.SerializeObject(request);
            var bytes = Encoding.UTF8.GetBytes(data);
            var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
            var checksum = sha256ManagedChecksum.ComputeHash(bytes);
            var key = data.Length + Convert.ToBase64String(checksum).Replace("=", "").Replace("/", "").Replace("+", "").Replace("-", "");

            return key;
        }


        private static GeoSource PublishExpression(PublishExpressionRequest request, PublishRequest publishRequest)
        {
            Func<string, GeoSource> resolver = (reference) =>
            {
                foreach (var g in request.GeoSources)
                {
                    if (g.Metadata != null && g.Metadata.Name == reference)
                    {
                        return g;
                    }
                }
                return GeoSourceInitializer.Engine.ResolveReference(reference) as GeoSource;
            };

            var geoSource = GeoSourceInitializer.Engine.Calculate(resolver, request.Expression, typeof(double));

            //publish newly created GeoSource
            publishRequest.SetupGeoSource(geoSource);
            geoSource = PublishGeoSourceHelper.PublishGeoSourceForUser(publishRequest.Token, geoSource, null);

            //put this geo-source on the cache
            GeoSourceInitializer.Initialize(geoSource);

            return geoSource;
        }
    }
}
