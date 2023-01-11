using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using Pyxis.Core.IO.GeoJson;
using Pyxis.UI.Layers.Sprites;

namespace Pyxis.UI.Layers
{
    /// <summary>
    /// A layer representing a selection on the Pyxis.UI.Layers.GlobeLayer.
    /// This layer should be added on top of the globe layer in ordinary circumstances.
    /// </summary>
    public class SelectionLayer : BaseLayer
    {
        private readonly GlobeLayer m_globeLayer;
        private Sprite m_selection;  
        private readonly RootSprite m_renderer;

        private bool m_selecting;
        private Point m_mouseDownLocation;
        private Point m_mouseUpLocation;
        /// <summary>
        /// The action to perform on the selected Pyxis.Core.IO.GeoJson.Geometry after making the selection.
        /// </summary>
        private Action<Geometry> m_selectionAction;
        
        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.SelectionLayer class associated with a globe.
        /// </summary>
        /// <param name="globeLayer">The Pyxis.UI.Layers.GlobeLayer the selection will be made on.</param>
        public SelectionLayer(GlobeLayer globeLayer)
            : base("Selection")
        {
            m_globeLayer = globeLayer;
            m_renderer = new RootSprite();
        }

        /// <summary>
        /// Draw the selection - called every frame.
        /// </summary>
        public override void Paint()
        {
            if (m_selection == null)
            {
                m_selection = m_renderer.AddChildSprite(new SizeF(200, 100))
                    .SetLocation(100, 100)
                    .SetColor(Color.Red)
                    .SetOpacity(0.5);
                m_selection.Visible = false;
            }
            
            m_renderer.Draw();
        }

        /// <summary>
        /// Start making a box selection.
        /// </summary>
        /// <param name="onSelectionCompleted">The action to perform on the selected Pyxis.Core.IO.GeoJson.Geometry after making the selection.</param>
        /// <exception cref="System.Exception">A selection is already in progress.</exception>
        public void StartBoxSelection(Action<Geometry> onSelectionCompleted)
        {
            if (m_selecting)
            {
                throw new Exception("Selection in progress");
            }
            m_selecting = true;
            m_mouseDownLocation = new Point(-1, -1);
            m_selectionAction = onSelectionCompleted;
        }

        private void CompleteSelection()
        {
            m_selecting = false;
            m_selection.Visible = false;

            if (m_selectionAction != null)
            {
                var rect = new Rectangle(Math.Min(m_mouseDownLocation.X, m_mouseUpLocation.X), Math.Min(m_mouseDownLocation.Y, m_mouseUpLocation.Y), Math.Abs(m_mouseDownLocation.X - m_mouseUpLocation.X), Math.Abs(m_mouseDownLocation.Y - m_mouseUpLocation.Y));
                if (rect.IsEmpty)
                {
                    m_selectionAction(null);
                }
                else
                {
                    var geometry = CreateGeometry(rect);
                    m_selectionAction(geometry);
                }
                
                m_selectionAction = null;
            }
        }

        private Geometry CreateGeometry(Rectangle rect)
        {
            var points = new List<GeographicPosition>();

            if (SamplePoints(points, new Point(rect.Left, rect.Top), new Point(rect.Right, rect.Top), 5) &&
                SamplePoints(points, new Point(rect.Right, rect.Top), new Point(rect.Right, rect.Bottom), 5) &&
                SamplePoints(points, new Point(rect.Right, rect.Bottom), new Point(rect.Left, rect.Bottom), 5) &&
                SamplePoints(points, new Point(rect.Left, rect.Bottom), new Point(rect.Left, rect.Top), 5))
            {
                return new PolygonGeometry()
                {
                    Coordinates = new List<List<GeographicPosition>>() 
                    {
                        points 
                    }
                };
            }

            return null;
        }

        private bool SamplePoints(List<GeographicPosition> points, Point point1, Point point2,float sampleDistance)
        {
            var dx = point2.X-point1.X;
            var dy = point2.Y-point1.Y;
            var distance = (float)Math.Max(Math.Sqrt(dx*dx+dy*dy),1.0);

            var screenPoints = new List<int[]>();
            for (float i = 0; i < distance; i += sampleDistance)
            {
                screenPoints.Add(new[] { point1.X + (int)(dx * i / distance), point1.Y + (int)(dy * i / distance) });
            }
            var newPoints = m_globeLayer.ScreenToGeographicPosition(screenPoints);

            if (newPoints.All(x => x != null))
            {
                points.AddRange(newPoints);
                return true;
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Represents the method that will handle the MouseMove event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseMove(object sender, MouseEventArgs e)
        {
            m_renderer.HandleMouseMove(e);
            if (m_selecting && e.Button == MouseButtons.Left)
            {
                var rect = new Rectangle(Math.Min(m_mouseDownLocation.X, e.X), Math.Min(m_mouseDownLocation.Y, e.Y), Math.Abs(m_mouseDownLocation.X - e.X), Math.Abs(m_mouseDownLocation.Y - e.Y));

                if (m_mouseDownLocation != e.Location)
                {
                    m_selection.Visible = true;
                    m_selection.SetLocation(rect.X + rect.Width / 2f, rect.Y + rect.Height / 2f).SetSize(rect.Size);
                }
                else
                {
                    m_selection.Visible = false;
                }
                return true;
            }
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseDown(object sender, MouseEventArgs e)
        {
            m_renderer.HandleMouseDown(e);
            if (m_selecting && e.Button == MouseButtons.Left)
            {
                m_mouseDownLocation = e.Location;
                return true;
            }

            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public override bool HandleMouseUp(object sender, MouseEventArgs e)
        {
            m_renderer.HandleMouseUp(e);

            if (m_selecting && e.Button == MouseButtons.Left)
            {
                m_mouseUpLocation = e.Location;
                CompleteSelection();
                return true;
            }

            return false;            
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

