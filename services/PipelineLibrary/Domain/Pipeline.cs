/******************************************************************************
Pipeline.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Iesi.Collections.Generic;
using System;

namespace Pyxis.Services.PipelineLibrary.Domain
{
    /// <summary>
    /// A pipeline to be written to and read from the database.
    /// </summary>
    public class Pipeline
    {
        /// <summary>
        /// Auto-increment, unique identifier that acts as the primary key.
        /// Required by NHibernate.
        /// </summary>
        [System.ComponentModel.DefaultValue(0)]
        protected internal virtual int Id { get; private set; }

        public virtual string Name { get; set; }

        public virtual string Description { get; set; }

        /// <summary>
        /// The pipeline GUID (alongwith the version, makes up the ProcRef).
        /// </summary>
        public virtual Guid PipelineGuid { get; set; }

        /// <summary>
        /// The version (alongwith the PipelineGuid, makes up the ProcRef).
        /// </summary>
        public virtual int Version { get; set; }

        /// <summary>
        /// The pipeline definition in XML.
        /// </summary>
        public virtual string Definition { get; set; }

        public virtual bool IsImported { get; set; }

        public virtual bool IsDownloaded { get; set; }

        public virtual bool IsPublished { get; set; }

        public virtual bool IsProcessed { get; set; }

        public virtual int MaxProcessedResolution { get; set; }

        /// <summary>
        /// Whether the pipeline is visible in the Library UI.  IsHidden is used for:
        /// 1. Child processes of pipelines that shouldn't appear in the Library UI.
        /// 2. Pipelines "removed" (using the Remove command) from the Library UI.
        /// <remarks>
        /// Default value of true follows the convention of the old Library.
        /// Clients can only set this by calling IPipelineRepository.SetIsHidden.
        /// </remarks>
        /// </summary>

        public virtual bool IsHidden
        {
            get
            {
                return m_isHidden;
            }
            set
            {
                m_isHidden = value;
            }
        }

        private bool m_isHidden = true;

        /// <summary>
        /// Whether the pipeline should be deleted on application shutdown/startup.
        /// <remarks>
        /// Default value of true follows the convention of the old Library.
        /// Clients can only set this by calling IPipelineRepository.SetIsTemporary().
        /// </remarks>
        /// </summary>

        public virtual bool IsTemporary
        {
            get
            {
                return m_isTemporary;
            }
            set
            {
                m_isTemporary = value;
            }
        }

        private bool m_isTemporary = true;

        /// <summary>
        /// Gets the pipeline's ProcRef.
        /// </summary>
        public virtual ProcRef ProcRef
        {
            get
            {
                return new ProcRef(
                    pyxlib.strToGuid(PipelineGuid.ToString()), Version);
            }
        }

        /// <summary>
        /// The pipeline's identity.
        /// </summary>
        public virtual PipelineIdentity Identity { get; private set; }

        /// <summary>
        /// Any metadata associated with this pipeline.
        /// </summary>
        public virtual ISet<PipelineMetadata> Metadata
        { get; private set; }

        /// <summary>
        /// The types the pipeline outputs.
        /// </summary>
        public virtual ISet<PipelineOutputType> OutputTypes
        { get; private set; }

        /// <summary>
        /// The child/parent relationships the pipeline has with other pipelines.
        /// </summary>
        public virtual ISet<PipelineRelationship> Relationships
        { get; private set; }

        public Pipeline()
        {
            this.OutputTypes = new HashedSet<PipelineOutputType>();
            this.Metadata = new HashedSet<PipelineMetadata>();
            this.Relationships = new HashedSet<PipelineRelationship>();
        }

        /// <summary>
        /// Associates <see cref="PipelineOutputType"/> with this pipeline.
        /// </summary>
        /// <param name="vecOutputTypes">
        /// A vector of output types specified in an IProcess_SPtr's spec.
        /// </param>
        public virtual void AddOutputType(Vector_GUID vecOutputTypes)
        {
            foreach (GUID outputType in vecOutputTypes)
            {
                this.OutputTypes.Add(new PipelineOutputType { OutputType = outputType });
            }
        }

        public virtual void AddMetadata(string name, string value)
        {
            this.Metadata.Add(new PipelineMetadata()
            {
                Name = name,
                Value = value
            });
        }

        /// <summary>
        /// Assigns the provided pipeline as a child.
        /// </summary>
        /// <param name="childPipeline">The child pipeline.</param>
        public virtual void AssignChild(Pipeline childPipeline)
        {
            if (childPipeline.Id == 0)
            {
                throw new Exceptions.ChildPipelineNotInDatabaseException(
                    "Child pipeline must exist in the database.");
            }
            else
            {
                PipelineRelationship relationship = new PipelineRelationship();
                relationship.Parent = this;
                relationship.ChildPipelineId = childPipeline.Id;
                this.Relationships.Add(relationship);
            }
        }

        /// <summary>
        /// Assigns the provided process as a child.
        /// </summary>
        /// <param name="childPipeline">The child process.</param>
        public virtual void AssignChild(IProcess_SPtr childProcess)
        {
            Pipeline childPipeline =
                Repositories.PipelineRepository.Instance.GetByProcRef(
                new ProcRef(childProcess));
            if (childPipeline == null)
            {
                throw new Exceptions.ChildPipelineNotInDatabaseException(
                    "Child pipeline must exist in the database.");
            }
            else
            {
                this.AssignChild(childPipeline);
            }
        }

        // TODO[kabiraman]: this really belongs in the Identity property,
        // no need to have this method.
        /// <summary>
        /// Sets the pipeline's identity.
        /// </summary>
        /// <param name="identity">The identity.</param>
        protected internal virtual void SetIdentity(PipelineIdentity identity)
        {
            if (identity != null)
            {
                this.Identity = identity;
                identity.Pipelines.Add(this);
            }
        }

        /// <summary>
        /// Sets the pipeline's identity.
        /// </summary>
        /// <param name="process">The pipeline.</param>
        public virtual void SetIdentity(IProcess_SPtr process)
        {
            bool isStable = PipeUtils.isPipelineIdentityStable(process);

            var pipelineIdentity = new PipelineIdentity
            {
                IsStable = isStable,
                XmlIdentity = process.getIdentity()
            };

            this.Identity = pipelineIdentity;
            pipelineIdentity.Pipelines.Add(this);
        }

        /// <summary>
        /// Determines whether a pipeline outputs the provided output type.
        /// </summary>
        /// <param name="outputType">Type of the output.</param>
        /// <returns>
        /// true, if the pipeline outputs the provided output type; otherwise, false.
        /// </returns>
        public virtual bool ProvidesOutputType(GUID outputType)
        {
            foreach (PipelineOutputType o in this.OutputTypes)
            {
                if (o.OutputType == outputType)
                {
                    return true;
                }
            }

            return false;
        }

        public virtual string FindMetaData(string name)
        {
            foreach (var metadata in Metadata)
            {
                if (metadata.Name == name)
                    return metadata.Value;
            }
            return null;
        }
    }
}