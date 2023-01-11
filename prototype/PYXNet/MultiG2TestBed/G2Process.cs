using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;

namespace MultiG2TestBed
{
    class G2Process
    {
        //! name of a subdirectory to store filescope setup in.
        private String m_subdirectoryName;

        //! the directory that will hold the setting subdirectory
        private String m_rootDir;

        //! the full path to FileScope app (debug version)
        private String m_fileScopePath;

        //! the full path to FileScope app (debug version)
        private String m_fileScopeSettingsPath;

        private int m_listenPort = -1;

        private System.Diagnostics.Process m_fileScopeProcess;

        public G2Process(String subdirectoryName) : this(subdirectoryName, -1)
        {
        }

        public G2Process(String subdirectoryName, int listenPort)
        {
            m_listenPort = listenPort;
            m_subdirectoryName = subdirectoryName;
            m_rootDir = System.Windows.Forms.Application.StartupPath;
            m_fileScopePath = m_rootDir.Substring(0, m_rootDir.IndexOf("MultiG2TestBed"));
            m_fileScopePath += "FileScope\\bin\\Debug\\FileScope.exe";
            m_fileScopeSettingsPath = System.IO.Path.Combine(m_rootDir, m_subdirectoryName);
        }

        ~G2Process()
        {
            Stop();
        }

        public void Start()
        {
            if (m_fileScopeProcess == null)
            {
                String arguments = "\"path:" + m_fileScopeSettingsPath + "\"" ;
                if (m_listenPort != -1)
                {
                    arguments += " port:" + m_listenPort.ToString();
                }
                m_fileScopeProcess = System.Diagnostics.Process.Start(m_fileScopePath,
                    arguments);
            }
        }

        public void Stop()
        {
            try
            {
                if (m_fileScopeProcess.CloseMainWindow())
                {
                    m_fileScopeProcess.WaitForExit(5000);
                }
                if (m_fileScopeProcess.HasExited)
                {
                    m_fileScopeProcess.Close();
                }
                else
                {
                    Kill();
                }
            }
            catch
            {
            }
            finally
            {
                m_fileScopeProcess = null;
            }
        }

        public void Kill()
        {
            try
            {
                m_fileScopeProcess.Kill();
                m_fileScopeProcess.Close();
            }
            catch
            {
            }
            finally
            {
                m_fileScopeProcess = null;
            }
        }

        public override String ToString() 
        {
            return "Configuration stored in: " + m_fileScopeSettingsPath;
        }

    }
	
}
