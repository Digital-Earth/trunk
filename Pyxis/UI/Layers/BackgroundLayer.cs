using System.Drawing;
using Tao.OpenGl;

namespace Pyxis.UI.Layers
{
    /// <summary>
    /// A static, non-interactive layer.
    /// This layer should be the bottom layer in ordinary circumstances.
    /// </summary>
    public class BackgroundLayer : BaseLayer
    {
        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.BackgroundLayer class.
        /// </summary>
        public BackgroundLayer()
            : base("Background")
        {
            BackgroundColor = Color.FromArgb(249, 249, 249);
        }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.BackgroundLayer class using a System.Drawing.Color.
        /// </summary>
        /// <param name="color">The System.Drawing.Colore to use.</param>
        public BackgroundLayer(Color color) : this()
        {
            BackgroundColor = color;
        }

        /// <summary>
        /// The color of the layer.
        /// </summary>
        public Color BackgroundColor { get; set; }

        /// <summary>
        /// Draw the layer - called every frame.
        /// </summary>
        public override void Paint()
        {
            Gl.glClearColor(BackgroundColor.R / 255.0f, BackgroundColor.G / 255.0f, BackgroundColor.B / 255.0f, BackgroundColor.A / 255.0f);
            Gl.glClear(Gl.GL_COLOR_BUFFER_BIT | Gl.GL_DEPTH_BUFFER_BIT);
        }

        /// <summary>
        /// Handle resize event.
        /// </summary>
        /// <param name="width">The new width.</param>
        /// <param name="height">The new height.</param>
        public override void HandleResize(int width, int height)
        {
            Gl.glViewport(0, 0, width, height);
        }
    }
}
