using System;
using System.Drawing;
using System.Windows.Forms;
using ApplicationUtility;
using Pyxis.UI.Layers.Sprites;

namespace Pyxis.UI.Layers
{
    /// <summary>
    /// A layer that adds navigation controls to a Pyxis.UI.Layers.GlobeLayer.
    /// This layer should be added on top of the globe layer in ordinary circumstances.
    /// </summary>
    public class NavigationControlsLayer : BaseLayer
    {
        private readonly GlobeLayer m_globeLayer;
        private Sprite m_controls;
        private Sprite m_compass;        
        private readonly RootSprite m_renderer;

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.NavigationControlsLayer with an associated Pyxis.UI.Layers.GlobeLayer.
        /// </summary>
        /// <param name="globeLayer">The Pyxis.UI.Layers.GlobeLayer the navigation controls are associated with.</param>
        public NavigationControlsLayer(GlobeLayer globeLayer)
            : base("Navigation")
        {
            m_globeLayer = globeLayer;
            m_renderer = new RootSprite();
        }

        /// <summary>
        /// Draw the layer - called every frame.
        /// </summary>
        public override void Paint()
        {
            var camera = m_globeLayer.GetCamera();

            if (m_controls == null)
            {
                m_controls = m_renderer.AddChildSprite(ManagedBitmapServer.Instance.LoadBitmapFromApplicationResources("Textures.Camera.controls.png"))
                    .SetLocation(-80, -50)
                    .SetAnchor(ContentAlignment.BottomRight);

                m_controls.AddChildSprite(new SizeF(28, 28))
                    .SetLocation(-30, 0)
                    .SetOpacity(0.9)
                    .MouseClick += GlobeTilt;

                m_controls.AddChildSprite(new SizeF(28, 28))
                    .SetOpacity(0.9)
                    .MouseClick += GlobeZoomIn;

                m_controls.AddChildSprite(new SizeF(28, 28))
                    .SetLocation(30, 0)
                    .SetOpacity(0.9)
                    .MouseClick += GlobeZoomOut;
            }
            if (m_compass == null)
            {
                m_compass = m_renderer.AddChildSprite(ManagedBitmapServer.Instance.LoadBitmapFromApplicationResources("Textures.Compass.compass2.png"))
                    .SetLocation(50, -50)
                    .SetAnchor(ContentAlignment.BottomLeft)
                    .SetRotation(0);

                m_compass.MouseClick += GlobeNorthUp;
            }
           
            m_compass.RotationAngle = (float)camera.Heading;
            foreach (var child in m_controls.Children)
            {
                child.Opacity = (child.Opacity * 3 + (m_renderer.ActiveSprites.Contains(child) ? 0 : 0.9)) / 4;
            }
            m_renderer.Draw();            
        }

        private void GlobeNorthUp(object sender, MouseEventArgs e)
        {
            var camera = m_globeLayer.GetCamera();
            camera.Heading = 0;
            m_globeLayer.SetCamera(camera, TimeSpan.FromSeconds(0.5));
        }

        private void GlobeZoomOut(object sender, MouseEventArgs e)
        {
            var camera = m_globeLayer.GetCamera();
            camera.Range *= 2;
            camera.Range = Math.Min(camera.Range, 31000000);
            m_globeLayer.SetCamera(camera, TimeSpan.FromSeconds(0.5));
            m_controls.Children[2].Opacity = 0.9;
        }

        private void GlobeZoomIn(object sender, MouseEventArgs e)
        {
            var camera = m_globeLayer.GetCamera();
            if (camera.Range > 20)
            {
                camera.Range /= 2;
            }
            else
            {
                camera.Range = 10;
            }
            m_globeLayer.SetCamera(camera, TimeSpan.FromSeconds(0.5));
            m_controls.Children[1].Opacity = 0.9;
        }

        private int m_tilt = 1;

        private void GlobeTilt(object sender, MouseEventArgs e)
        {
            var camera = m_globeLayer.GetCamera();
            camera.Tilt = m_tilt * 20;
            m_globeLayer.SetCamera(camera, TimeSpan.FromSeconds(0.5));
            m_tilt = (m_tilt + 1) % 4;
            m_controls.Children[0].Opacity = 0.9;
        }

        /// <summary>
        /// Represents the method that will handle the MouseMove event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseMove(object sender, MouseEventArgs e)
        {
            return m_renderer.HandleMouseMove(e);
        }

        /// <summary>
        /// Represents the method that will handle the MouseDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseDown(object sender, MouseEventArgs e)
        {
            return m_renderer.HandleMouseDown(e);
        }

        /// <summary>
        /// Represents the method that will handle the MouseUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseUp(object sender, MouseEventArgs e)
        {
            return m_renderer.HandleMouseUp(e);
        }

        /// <summary>
        /// Represents the method that will handle the MouseClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseClick(object sender, MouseEventArgs e)
        {
            return m_renderer.HandleMouseClick(e);
        }

        /// <summary>
        /// Represents the method that will handle the MouseDoubleClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseDoubleClick(object sender, MouseEventArgs e)
        {
            return m_renderer.HandleMouseDoubleClick(e);
        }

        /// <summary>
        /// Dispose of the resources used by the layer.
        /// </summary>
        public override void HandleDispose()
        {
            m_renderer.HandleDispose();
        }
    }
}

