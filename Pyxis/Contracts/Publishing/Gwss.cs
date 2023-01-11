using System;

namespace Pyxis.Contract.Publishing
{
    public class Gwss
    {
        public Guid Id { get; set; }
        public DateTime LastHeard { get; set; }
        public GwssStatus Status { get; set; }
        public LsStatus Request { get; set; }

        // for deserializing from string
        public Gwss()
        {
        }
        
        public Gwss(Guid id, GwssStatus status, LsStatus request)
        {
            Id = id;
            LastHeard = DateTime.UtcNow;
            Status = status;
            Request = request;
        }
    }

    public class PipelineServerStatus
    {
        public string ProcRef { get; set; }
        public Guid ServerId { get; set; }
        public PipelineStatusImpl PublishedStatus { get; set; }
        public OperationStatus OperationStatus { get; set; }
    }
}