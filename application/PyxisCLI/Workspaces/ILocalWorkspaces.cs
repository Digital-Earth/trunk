using System.Collections.Generic;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;

namespace PyxisCLI.Workspaces
{
    public interface ILocalWorkspaces
    {
        IEnumerable<string> Names { get; }

        IEnumerable<KeyValuePair<string, Workspace>> Workspaces { get; }

        void CreateWorkspace(string name);
        void DeleteWorkspace(string name);
        bool WorkspaceExists(string name);

        Workspace GetWorkspace(string name);
        WorkspaceFile GetWorkspaceFile(string name);

        Endpoint GetEndpoint(Reference reference);
        IImport GetImport(Reference reference);
        GlobeTemplate GetGlobe(Reference reference);

        GeoSource ResolveGeoSource(ReferenceOrExpression referenceOrExpression, bool forceImport = false);
    }
}