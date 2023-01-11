using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tao.OpenGl;

namespace Pyxis.WorldView.Studio.Layers.Html
{
    class OpenGLProgram
    {
        private int m_handle = -1;
        private int m_vertexShaderHandle = -1;
        private int m_fragmentShaderHandle = -1;

        public string VertexShaderCode { get; set; }
        public string FragmentShaderCode { get; set; }
       
        public bool Compile() {

            if (m_handle == -1)
            {
                m_handle = Gl.glCreateProgram();
            }

            if (!String.IsNullOrEmpty(VertexShaderCode))
            {
                m_vertexShaderHandle = CompileAndAttachShader(VertexShaderCode, Gl.GL_VERTEX_SHADER);
            }
            if (!String.IsNullOrEmpty(FragmentShaderCode))
            {
                m_fragmentShaderHandle = CompileAndAttachShader(FragmentShaderCode, Gl.GL_FRAGMENT_SHADER);
            }

            Gl.glLinkProgram(m_handle);

            int linked;
            Gl.glGetProgramiv(m_handle, Gl.GL_LINK_STATUS, out linked);

            return linked == Gl.GL_TRUE;
        }

        private int CompileAndAttachShader(string code, int shaderType)
        {
            int shaderHandle = Gl.glCreateShader(shaderType);

            var lines = code.Split(new char[] { '\n' });
            var linesLen = lines.Select(x => x.Length).ToArray();

            Gl.glShaderSource(shaderHandle, lines.Length, lines, linesLen);

            Gl.glCompileShader(shaderHandle);

            Gl.glAttachShader(m_handle, shaderHandle);

            return shaderHandle;
        }

        public void StartUsing()
        {
            Gl.glUseProgram(m_handle);
        }

        public void StopUsing()
        {
            Gl.glUseProgram(0);
        }

        public void Delete()
        {
            if (m_handle != -1)
            {
                Gl.glDeleteProgram(m_handle);
                m_handle = -1;
            }

            if (m_vertexShaderHandle != -1)
            {
                Gl.glDeleteShader(m_vertexShaderHandle);
                m_vertexShaderHandle = -1;
            }

            if (m_fragmentShaderHandle != -1)
            {
                Gl.glDeleteShader(m_fragmentShaderHandle);
                m_fragmentShaderHandle = -1;
            }            
        }
    }
}
