using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace PyxisCLI.Server.Cluster
{
    public class RemoteTask
    {
        public class RemoteTaskStatus
        {
            public string Id { get; set; }
            public string Name { get; set; }
            public string Status { get; set; }
            public Dictionary<string, object> Details { get; set; }
            public Dictionary<string, object> State { get; set; }
            public Dictionary<string, object> Data { get; set; }
            public List<string> Log { get; set; }
        }

        public string Id { get; private set; }

        private RemoteTaskStatus m_status;


        public string Status
        {
            get { return m_status != null ? m_status.Status : "Unknown"; }
        }

        public List<string> Log
        {
            get { return m_status != null ? m_status.Log: new List<string>(); }
        }

        public Dictionary<string, object> Details
        {
            get { return m_status != null ? m_status.Details : new Dictionary<string, object>(); }
        }

        public Dictionary<string, object> State
        {
            get { return m_status != null ? m_status.State : new Dictionary<string, object>(); }
        }

        public RemoteTask(string taskId)
        {
            Id = taskId;
        }

        public RemoteTask(RemoteTaskStatus status)
        {
            m_status = status;
            Id = status.Id;
        }

        public void Refresh()
        {
            if (string.IsNullOrEmpty(Program.Cluster.Uri))
            {
                return;
            }

            using (var webClient = new WebClient())
            {
                var newStatus =
                    JsonConvert.DeserializeObject<RemoteTaskStatus>(webClient.DownloadString(Program.Cluster.Uri + "/_cluster/tasks/" + Id));

                if (newStatus != null && newStatus.Status != null)
                {
                    m_status = newStatus;
                }
            }
        }
    }
}
