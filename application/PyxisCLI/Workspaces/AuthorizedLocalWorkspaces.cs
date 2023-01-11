using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core.Analysis;
using PyxisCLI.Server.Models;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Workspaces
{
    class AuthorizedLocalWorkspaces : ILocalWorkspaces
    {
        private ILocalWorkspaces Source { get; set; }
        private IPrincipal User { get; set; }
        private Action<string> ThrowFunction { get; set; }

        public AuthorizedLocalWorkspaces(ILocalWorkspaces source, IPrincipal user, Action<string> throwFunction = null)
        {
            Source = source;
            User = user;
            ThrowFunction = throwFunction;
        }

        private void ThrowError(string message)
        {
            if (ThrowFunction != null)
            {
                ThrowFunction(message);
            }
            throw new ApiException(FailureResponse.ErrorCodes.Unauthorized, message);
        }


        public IEnumerable<string> Names { get { return Workspaces.Select(x => x.Key); } }

        public IEnumerable<KeyValuePair<string, Workspace>> Workspaces
        {
            get
            {
                return Source.Workspaces
                    //return only the workspaces we can access
                    .Where(ky => ky.Value.CanAccess(User))
                    //return only the details we are allow to see
                    .Select(ky => new KeyValuePair<string, Workspace>(ky.Key,
                                ky.Value.CreateCopyWithoutSensativeDetails(User)));
            }
        }

        public void CreateWorkspace(string name)
        {
            Source.CreateWorkspace(name);
        }

        public void DeleteWorkspace(string name)
        {
            var workspace = GetWorkspace(name);
            if (workspace.Owner != null && workspace.Owner.Id != User.Identity.Name)
            {
                ThrowError("Only workspaces owner can delete it.");
            }
            Source.DeleteWorkspace(name);
        }

        public bool WorkspaceExists(string name)
        {
            return Source.WorkspaceExists(name);
        }

        public void ThrowIfWorkspaceNotAuthorized(string name)
        {
            if (String.IsNullOrEmpty(name))
            {
                ThrowError("Can't validate access to workspace without a name.");
            }

            var workspace = Source.GetWorkspace(name);
            if (!workspace.CanAccess(User))
            {
                ThrowError(String.Format("Not authorized to access {0} workspace.", name));
            }
        }

        public void ThrowIfReferenceNotAuthorized(ReferenceOrExpression referenceOrExpression)
        {
            if (referenceOrExpression.Reference != null)
            {
                ThrowIfWorkspaceNotAuthorized(new Reference(referenceOrExpression.Reference).Workspace);
            }

            if (referenceOrExpression.Expression != null)
            {
                var refernces = Program.Engine.ParseExpression(referenceOrExpression.Expression).References;

                foreach (var refernce in refernces)
                {
                    var sanatizedRefe = Reference.SanatizeReference(refernce);

                    if (referenceOrExpression.Symbols != null && referenceOrExpression.Symbols.ContainsKey(sanatizedRefe))
                    {
                        ThrowIfReferenceNotAuthorized(referenceOrExpression.Symbols[sanatizedRefe]);
                    }
                    else
                    {
                        ThrowIfWorkspaceNotAuthorized(new Reference(refernce).Workspace);    
                    }
                }
            }
        }

        public Workspace GetWorkspace(string name)
        {
            ThrowIfWorkspaceNotAuthorized(name);
            return Source.GetWorkspace(name).CreateCopyWithoutSensativeDetails(User);
        }

        public WorkspaceFile GetWorkspaceFile(string name)
        {
            ThrowIfWorkspaceNotAuthorized(name);
            return Source.GetWorkspaceFile(name);
        }

        public Endpoint GetEndpoint(Reference reference)
        {
            ThrowIfWorkspaceNotAuthorized(reference.Workspace);
            return Source.GetEndpoint(reference);
        }

        public IImport GetImport(Reference reference)
        {
            ThrowIfWorkspaceNotAuthorized(reference.Workspace);
            return Source.GetImport(reference);
        }

        public GlobeTemplate GetGlobe(Reference reference)
        {
            ThrowIfWorkspaceNotAuthorized(reference.Workspace);
            return Source.GetGlobe(reference);
        }

        public GeoSource ResolveGeoSource(ReferenceOrExpression referenceOrExpression, bool forceImport = false)
        {
            ThrowIfReferenceNotAuthorized(referenceOrExpression);
            return Source.ResolveGeoSource(referenceOrExpression, forceImport);
        }
    }
}