using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace StudioLauncher
{
    public partial class DownloadProgressUI : Form
    {
        public DownloadProgressUI()
        {
            InitializeComponent();
            CenterToScreen();
        }
        //protected override CreateParams CreateParams
        //{
        //    get
        //    {
        //        const int CS_DROPSHADOW = 0x20000;
        //        CreateParams cp = base.CreateParams;
        //        cp.ClassStyle |= CS_DROPSHADOW;
        //        return cp;
        //    }
        //}

        private object m_animationLock = new object();
        float m_progress;
        public int Progress
        {
            get
            {
                return (int)m_progress;
            }
            set
            {
                StartAnimation(value);
            }
        }

        private void StartAnimation(int animateTo)
        {

            Task.Factory.StartNew(() =>
                   {
                       lock (m_animationLock)
                       {
                           for (; m_progress < animateTo; m_progress += 0.5f)
                           {
                               if (InvokeRequired)
                               {
                                   this.Invoke(new Action(() =>
                                   {
                                       Invalidate();
                                   }));
                               }
                               else
                               {
                                   Invalidate();
                               }
                               Thread.Sleep(10);
                           }
                       }
                   });

        }
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            e.Graphics.SmoothingMode = SmoothingMode.HighQuality;

            var pen = new Pen(Color.FromArgb(75,255,164), 13);
            //e.Graphics.DrawArc(pen, new Rectangle(21, 20, 154, 155), -90, m_progress * 3.6f);
            e.Graphics.DrawLine(pen, 0, 174, m_progress * this.Width / 100, 174);
            
        }
    }
}
