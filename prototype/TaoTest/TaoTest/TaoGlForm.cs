using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Tao.OpenGl;
using Tao.Platform.Windows;

namespace TaoTest 
{
    public partial class TaoGlForm : Form 
    {
        public TaoGlForm()
        {
            InitializeComponent();
            m_glControl.InitializeContexts();

            Gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            Gl.glMatrixMode(Gl.GL_PROJECTION);
            Gl.glLoadIdentity();
            Gl.glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
        }

        private void m_glControl_Paint(object sender, PaintEventArgs e)
        {
            Gl.glClear(Gl.GL_COLOR_BUFFER_BIT);
            Gl.glColor3f(1.0f, 1.0f, 1.0f);
            Gl.glBegin(Gl.GL_TRIANGLES);
            Gl.glVertex3f(0.5f, 1.0f, 0.0f);
            Gl.glVertex3f(0.0f, 0.0f, 0.0f);
            Gl.glVertex3f(1.0f, 0.0f, 0.0f);
            Gl.glEnd();
            Gl.glFlush();
        }
    }

}