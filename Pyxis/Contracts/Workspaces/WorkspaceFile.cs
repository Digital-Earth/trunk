namespace Pyxis.Contract.Workspaces
{
    public class WorkspaceFile
    {
        private readonly string m_path;

        public WorkspaceFile(string path)
        {
            m_path = path;
        }

        public string Path
        {
            get { return m_path;}
        }

        public bool Exists()
        {
            return System.IO.File.Exists(m_path);
        }

        public Workspace ReadWorkspace()
        {
            return WorkspaceParser.ReadFile(m_path);
        }

        public Workspace CreateEmpty()
        {
            return WorkspaceParser.CreateEmptyWorkspace(m_path);
            
        }

        public void UpdateOrInsertEndpoint(string name, Endpoint endpoint, string importFilename = null)
        {
            WorkspaceParser.UpdateOrInsertEndpoint(m_path,name,endpoint,importFilename);
        }

        public void UpdateOrInsertImport(string name, IImport import, string importFilename = null)
        {
            WorkspaceParser.UpdateOrInsertImport(m_path, name, import, importFilename);
        }

        public void UpdateOrInsertGlobe(string name, GlobeTemplate globeTemplate, string globeFilename = null)
        {
            WorkspaceParser.UpdateOrInsertGlobe(m_path, name, globeTemplate, globeFilename);
        }

        public void RemoveImport(string name)
        {
            WorkspaceParser.RemoveImport(m_path, name);
        }

        public void RemoveGlobe(string name)
        {
            WorkspaceParser.RemoveGlobe(m_path, name);
        }

        public void RemoveEndpoint(string name)
        {
            WorkspaceParser.RemoveEndpoint(m_path, name);
        }
    }
}