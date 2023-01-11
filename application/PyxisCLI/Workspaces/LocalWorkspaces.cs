using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http.Formatting;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core.Analysis;
using Pyxis.Utilities;
using File = System.IO.File;

namespace PyxisCLI.Workspaces
{
    class LocalWorkspaces : ILocalWorkspaces
    {
        private const string WorkspaceSuffix = ".ggs.json";
        private const string DeletedWorkspaceSuffix = ".ggs.deleted.{0}.json";
        public string RootFolder { get; set; }

        private readonly Dictionary<string, Workspace> m_workspaces = new Dictionary<string, Workspace>();
        private readonly LocalFileWatcher m_fileWatecher = new LocalFileWatcher();

        public IEnumerable<string> Names
        {
            get { return m_workspaces.Keys; }
        }

        public IEnumerable<KeyValuePair<string,Workspace>> Workspaces
        {
            get { return m_workspaces; }
        }

        public LocalWorkspaces(string root)
        {
            RootFolder = Path.GetFullPath(root);
        }

        public void LoadFolder(string directory = null)
        {
            if (String.IsNullOrEmpty(directory))
            {
                directory = RootFolder;
            }

            foreach (var file in Directory.EnumerateFiles(directory, "*" + WorkspaceSuffix))
            {
                LoadFileSafeAndTrackChanges(new WorkspaceFile(Path.Combine(directory, file)));
            }

            m_fileWatecher.WatchDirectory(directory, (path, eventType) =>
            {
                if (eventType == "created" && path.EndsWith(WorkspaceSuffix))
                {
                    Console.WriteLine("New workspace found at: {0}", path);
                    LoadFileSafeAndTrackChanges(new WorkspaceFile(Path.Combine(path)));
                }
            });
        }

        public WorkspaceFile GetWorkspaceFile(string name)
        {
            return new WorkspaceFile(GetWorkspacePath(name));
        }

        public void CreateWorkspace(string name)
        {
            var file = GetWorkspaceFile(name);

            if (!file.Exists())
            {
                file.CreateEmpty();
            }

            LoadFileSafeAndTrackChanges(file);
        }

        private void LoadFileSafe(WorkspaceFile file)
        {
            var name = Path.GetFileName(file.Path).Replace(WorkspaceSuffix, "");

            Workspace newWorkspace = null;
            try
            {
                newWorkspace = file.ReadWorkspace();
            }
            catch (Exception ex)
            {
                Program.LogExceptions(new Exception("Failed to reload workspace from " + file.Path, ex));
            }

            try
            {
                Add(name, newWorkspace);
            }
            catch (Exception ex)
            {
                Program.LogExceptions(new Exception("Failed to add workspace from " + file.Path, ex));
            }
        }

        private void LoadFileSafeAndTrackChanges(WorkspaceFile file)
        {
            LoadFileSafe(file);

            m_fileWatecher.Watch(file.Path, HandleFileChange);
        }

        private void HandleFileChange(string path)
        {
            var name = Path.GetFileName(path).Replace(WorkspaceSuffix, "");
            var file = new WorkspaceFile(path);
            if (file.Exists())
            {
                Console.WriteLine("Workspace updated: {0}", name);
                LoadFileSafe(file);
            }
            else
            {
                Console.WriteLine("Workspace removed: {0}", name);
                RemoveWorkspace(name);
            }
        }

        public void DeleteWorkspace(string name)
        {
            MarkWorkspaceAsDeleted(name);
        }

        public void MarkWorkspaceAsDeleted(string name)
        {
            var path = GetWorkspacePath(name);

            if (!File.Exists(path))
            {
                return;
            }

            var deletedTime = DateTime.Now.ToString("s").Replace(":", "").Replace("-", "");
            File.Move(path, path.Replace(WorkspaceSuffix, string.Format(DeletedWorkspaceSuffix, deletedTime)));

            RemoveWorkspace(name);
        }

        public void Add(string name, Workspace workspace)
        {
            m_workspaces[name] = workspace;
        }

        public string GetWorkspacePath(string name)
        {
            var filename = Path.Combine(RootFolder, name + WorkspaceSuffix);
            return filename;
        }

        public bool WorkspaceExists(string name)
        {
            return File.Exists(GetWorkspacePath(name));
        }

        public Workspace GetWorkspace(string name)
        {
            if (!m_workspaces.ContainsKey(name))
            {
                var file = GetWorkspaceFile(name);
                if (file.Exists())
                {
                    LoadFileSafeAndTrackChanges(file);
                }
                else
                {
                    throw new ItemNotFoundException("Failed to find workspace with name: " + name);
                }
            }

            return m_workspaces[name];
        }

        public void RemoveWorkspace(string name)
        {
            if (m_workspaces.ContainsKey(name))
            {
                m_workspaces.Remove(name);
            }
        }

        public Endpoint GetEndpoint(Reference reference)
        {
            var workspace = GetWorkspace(reference.Workspace);

            if (!workspace.Endpoints.ContainsKey(reference.Name))
            {
                throw new ItemNotFoundException(String.Format("Failed to find endpoint {0} at {1} workspace",
                    reference.Name, reference.Workspace));
            }
            return workspace.Endpoints[reference.Name];
        }

        public IImport GetImport(Reference reference)
        {
            var workspace = GetWorkspace(reference.Workspace);

            if (!workspace.Imports.ContainsKey(reference.Name))
            {
                var message = string.Format("Failed to find import {0} at {1} workspace.\nAvailable Imports:\n{2}",
                    reference.Name, reference.Workspace,
                    string.Join("\n",
                        workspace.Imports.Keys.Select(import => " * " + reference.Workspace + "/" + import)));
                throw new ItemNotFoundException(message);
            }
            return workspace.Imports[reference.Name];
        }

        public GlobeTemplate GetGlobe(Reference reference)
        {
            var workspace = GetWorkspace(reference.Workspace);

            if (!workspace.Globes.ContainsKey(reference.Name))
            {
                throw new ItemNotFoundException(String.Format("Failed to find globe {0} at {1} workspace",
                    reference.Name, reference.Workspace));
            }
            return workspace.Globes[reference.Name];
        }

        public GeoSource ResolveGeoSource(string refString, bool forceImport = false)
        {
            var reference = new Reference(refString);
            return ResolveGeoSource(reference, forceImport);
        }

        public GeoSource ResolveGeoSource(Reference reference, bool forceImport = false)
        {
            try
            {
                var import = GetImport(reference);

                var geoSource = ResolveGeoSource(import, reference.Domains, forceImport);

                if (Program.Verbose)
                {
                    Console.WriteLine("Resolve reference {0} -> {1}", reference, geoSource.Id);
                }
                return geoSource;
                
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to resolve reference {0}: {1}",reference,ex.Message);
                throw new Exception(string.Format("Failed to resolve reference {0}", reference), ex);
            }
            
        }

        public GeoSource ResolveGeoSource(ReferenceOrExpression referenceOrExpression, bool forceImport = false)
        {
            if (referenceOrExpression.Expression.HasContent())
            {
                return ResolveExpression(referenceOrExpression, forceImport);
            }
            else
            {
                return ResolveGeoSource(new Reference(referenceOrExpression.Reference), forceImport);
            }
        }

        public GeoSource ResolveGeoSource(IImport import, Dictionary<string,string> domains = null, bool forceImport = false)
        {
            var dataset = import.Resolve(domains);

            var data = domains == null ? 
                LocalPersistance.AttachData(import) :
                LocalPersistance.AttachData(new { import, domains });

            var geoSource = data.Get<GeoSource>("geosource");

            if (geoSource != null && !forceImport)
            {
                return geoSource;
            }

            geoSource = GeoSourceCreator.CreateFromDataSet(Program.Engine, dataset, import.Srs, import.GeoTag, import.Sampler);

            if (import.Style != null)
            {
                geoSource.Style = import.Style;
            }

            if (import.Specification != null)
            {
                geoSource.Specification.Merge(import.Specification);
            }

            if (geoSource != null)
            {
                data.Set("geosource", geoSource);
            }
            else
            {
                data.Delete("geosource");
            }

            return geoSource;
        }

        public GeoSource ResolveExpression(ReferenceOrExpression request, bool forceImport = false)
        {
            try
            {
                var data = LocalPersistance.AttachData(request);

                var geoSource = data.Get<GeoSource>("geosource");

                if (geoSource != null && !forceImport)
                {
                    return geoSource;
                }

                Func<string, GeoSource> resolver = (name) =>
                {
                    if (request.Symbols != null && request.Symbols.ContainsKey(name))
                    {
                        var local = request.Symbols[name];
                        return ResolveGeoSource(local);
                    }
                    //falback to default engine resolver
                    return ResolveGeoSource(name);
                };

                geoSource = Program.Engine.Calculate(resolver, request.Expression, typeof(double));
                data.Set("geosource", geoSource);

                if (Program.Verbose)
                {
                    Console.WriteLine("Resolved expression {0} -> {1}", request.Expression, geoSource.Id);
                }
                return geoSource;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to resolve expression {0}: {1}", request.Expression, ex.Message);
                throw new Exception(string.Format("Failed to resolve expression {0}",request.Expression), ex);
            }
        }

        public ILocalWorkspaces AuthorizedAs(IPrincipal user, Action<string> thorwFunction = null)
        {
            return new AuthorizedLocalWorkspaces(this, user, thorwFunction);
        }
    }
}
