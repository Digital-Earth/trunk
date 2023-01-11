using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Runtime.Remoting.Messaging;
using System.Runtime.Serialization;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;
using NUnit.Framework;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing;
using Timer = System.Timers.Timer;

namespace ApplicationUtility
{
    public class PublishedResourceInfo
    {
        public string Id { get; set; }
        public string User { get; set; }
        public string Name { get; set; }
        public string Status { get; set; }
        public float? Progress { get; set; }
    }

    [Obsolete("this class was used at the downloable to ensure the user didn't quit the application before finish publishing, the new publish sequnce doesn't require it.")]
    static public class PublishProgressMonitor
    {
        static private readonly object s_pendingResourcesLock = new object();
        static private List<PublishedResourceInfo> s_pendingResources = new List<PublishedResourceInfo>();

        static private readonly object s_completionSourcesLock = new object();
        static private Dictionary<string, TaskCompletionSource<bool>> s_completionSources = new Dictionary<string, TaskCompletionSource<bool>>(); 

        static private TimeSpan s_updatePeriod = TimeSpan.FromMinutes(1);
        static private Timer s_timer = new Timer(s_updatePeriod.TotalMilliseconds);

        static private User s_user;

        static public bool AnyPending()
        {
            lock (s_pendingResourcesLock)
            {
                return s_pendingResources.Count > 0;
            }
        }

        static public void AddPublishedResource(PublishedResourceInfo info)
        {
            lock (s_pendingResourcesLock)
            {
                s_pendingResources.Add(new PublishedResourceInfo()
                {
                    User = info.User,
                    Id = info.Id,
                    Name = info.Name,
                    Status = "Contacting publishing servers"
                });
            }
            lock (s_completionSourcesLock)
            {
                if (!s_completionSources.ContainsKey(info.Id))
                {
                    s_completionSources[info.Id] = new TaskCompletionSource<bool>();
                }
            }
            Start();
        }

        /// <summary>
        /// Register to be signaled when sufficient progress has been made in the publishing 
        /// process to be able to close the publishing client.
        /// </summary>
        /// <param name="id">Identifier for the resource.</param>
        /// <returns>
        /// A task with result true if sufficient progress was made while monitoring, 
        /// otherwise false including when <paramref name="id"/> refers to a resource not being monitored.
        /// </returns>
        static public Task<bool> RegisterPublishEvent(string id)
        {
            lock (s_completionSourcesLock)
            {
                if (s_completionSources.ContainsKey(id))
                {
                    return s_completionSources[id].Task;
                }
                return Task<bool>.Factory.StartNew(() => { return false; });
            }
        }

        static public bool RemovePublishedResourceById(string id)
        {
            lock (s_completionSourcesLock)
            {
                if (s_completionSources.ContainsKey(id))
                {
                    s_completionSources[id].TrySetResult(false);
                }
            }
            lock (s_pendingResourcesLock)
            {
                var resource = s_pendingResources.FirstOrDefault(x => x.Id == id);
                if (resource == null)
                {
                    return false;
                }
                return s_pendingResources.Remove(resource);
            }
        }

        static public string GetPendingStatusesMessage()
        {
            List<PublishedResourceInfo> pendingResources;
            lock (s_pendingResourcesLock)
            {
                pendingResources = s_pendingResources.ToList();
            }

            if (pendingResources.Count == 0)
            {
                return "No published resources pending";
            }

            string message = "";
            foreach(var pendingResource in pendingResources)
            {
                message += pendingResource.Name + " is '" + pendingResource.Status;
                if (pendingResource.Progress != null)
                {
                    message += " (" + pendingResource.Progress.Value.ToString("0.00") + "%)";
                }
                message += "'\r\n";
            }

            return message;
        }

        static public void AssignUser(User user)
        {
            s_user = user;
        }

        static private void Start()
        {
            s_timer.Elapsed += CleanPendingResources;
            s_timer.Start();
        }

        static private void Stop()
        {
            s_timer.Stop();
            s_timer.Elapsed -= CleanPendingResources;
        }

        static private void CleanPendingResources(object sender, ElapsedEventArgs args)
        {
            Stop();
            lock (s_pendingResourcesLock)
            {
                foreach (var pendingResource in s_pendingResources.ToList())
                {
                    try
                    {
                        var pipelineServerStatuses = GetPendingResourceStatuses(pendingResource);
                        if (SufficientProgressMade(pipelineServerStatuses))
                        {
                            lock (s_completionSourcesLock)
                            {
                                if (s_completionSources.ContainsKey(pendingResource.Id))
                                {
                                    s_completionSources[pendingResource.Id].TrySetResult(true);
                                }
                            }
                            s_pendingResources.Remove(pendingResource);
                        }
                        else
                        {
                            UpdatePendingProgress(pendingResource, pipelineServerStatuses);
                        }
                    }
                    catch
                    {
                        System.Diagnostics.Trace.WriteLine("Failed to get status of pending published resource " + pendingResource.Name + ".  Continuing...");
                    }
                }
            }
            if (AnyPending())
            {
                Start();
            }
        }

        static private void UpdatePendingProgress(PublishedResourceInfo resourceInfo, List<IPipelineServerStatus> pipelineServerStatuses)
        {
            if (pipelineServerStatuses.Count == 0)
            {
                resourceInfo.Status = "Contacting publishing servers";
                return;
            }
            var mostAdvancedProgress = pipelineServerStatuses.FirstOrDefault(x => x.Status == PipelineStatusCode.Downloading.ToString());
            if (mostAdvancedProgress == null)
            {
                mostAdvancedProgress = pipelineServerStatuses.FirstOrDefault(x => x.Status == PipelineStatusCode.Initializing.ToString());
            }

            if (mostAdvancedProgress != null)
            {
                resourceInfo.Status = mostAdvancedProgress.Status;
                resourceInfo.Progress = mostAdvancedProgress.OperationStatus.Progress;
            }
            else
            {
                // this includes if statuses are set to removing
                resourceInfo.Status = "Contacting publishing servers";
            }
        }

        static private List<IPipelineServerStatus> GetPendingResourceStatuses(PublishedResourceInfo publishedResourceInfo)
        {
            if (s_user == null)
            {
                return null;
            }
            var resourceStatuses = s_user.GetPipelineDetails(publishedResourceInfo.Id);
            if (resourceStatuses == null)
            {
                return null;
            }
            else
            {
                return new List<IPipelineServerStatus>(resourceStatuses);
            }
        }

        static private bool SufficientProgressMade(List<IPipelineServerStatus> statuses)
        {
            return statuses != null 
                && null != statuses.FirstOrDefault(x => x.Status == PipelineStatusCode.Processing.ToString()
                || x.Status == PipelineStatusCode.Publishing.ToString()
                || x.Status == PipelineStatusCode.Published.ToString());
        }
    }
}