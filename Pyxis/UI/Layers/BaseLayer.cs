using System.Windows.Forms;

namespace Pyxis.UI.Layers
{
    /// <summary>
    /// A base implementation to inherit from for a layer in a Pyxis.UI.PyxisView.
    /// </summary>
    public abstract class BaseLayer : ILayer
    {
        /// <summary>
        /// Gets the name of layer used for tracing exceptions and profiling.
        /// </summary>
        public string Name { get; protected set; }

        /// <summary>
        /// Gets or sets if the layer is currently visible.
        /// </summary>
        public bool Visible { get; set; }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.BaseLayer class with the give name.
        /// </summary>
        /// <param name="name">The name of the layer.</param>
        protected BaseLayer(string name)
        {
            Visible = true;
            Name = name;
        }

        /// <summary>
        /// Draw the layer - called every frame.
        /// </summary>
        public virtual void Paint()
        {
        }

        /// <summary>
        /// Handle resize event.
        /// </summary>
        /// <param name="width">The new width.</param>
        /// <param name="height">The new height.</param>
        public virtual void HandleResize(int width, int height)
        {
        }

        /// <summary>
        /// Called when layer is about to be disposed.
        /// </summary>
        public virtual void HandleDispose()
        {
        }

        /// <summary>
        /// Represents the method that will handle the MouseMove event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleMouseMove(object sender, MouseEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleMouseDown(object sender, MouseEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleMouseUp(object sender, MouseEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleMouseClick(object sender, MouseEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseDoubleClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleMouseDoubleClick(object sender, MouseEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the MouseWheel event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleMouseWheel(object sender, MouseEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the KeyDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A KeyEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleKeyDown(object sender, KeyEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the KeyPress event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A KeyEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleKeyPress(object sender, KeyPressEventArgs e)
        {
            return false;
        }

        /// <summary>
        /// Represents the method that will handle the KeyUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A KeyEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false. Pyxis.UI.Layers.BaseLayer does not handle the event.</returns>
        public virtual bool HandleKeyUp(object sender, KeyEventArgs e)
        {
            return false;
        }
    }
}
