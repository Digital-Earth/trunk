using System.Windows.Forms;

namespace Pyxis.UI
{
    /// <summary>
    /// Represents a graphical layer for user interaction within a Pyxis.UI.PyxisView.
    /// </summary>
    public interface ILayer
    {
        /// <summary>
        /// Gets the name of layer used for tracing exceptions and profiling.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Gets or sets if the layer is currently visible.
        /// </summary>
        bool Visible { get; set; }

        /// <summary>
        /// Draw the layer - called every frame.
        /// </summary>
        void Paint();

        /// <summary>
        /// Handle resize event.
        /// </summary>
        /// <param name="width">The new width.</param>
        /// <param name="height">The new height.</param>
        void HandleResize(int width, int height);

        /// <summary>
        /// Called when layer is about to be disposed.
        /// </summary>
        void HandleDispose();

        /// <summary>
        /// Represents the method that will handle the MouseMove event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleMouseMove(object sender, MouseEventArgs e);
        /// <summary>
        /// Represents the method that will handle the MouseDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleMouseDown(object sender, MouseEventArgs e);
        /// <summary>
        /// Represents the method that will handle the MouseUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleMouseUp(object sender, MouseEventArgs e);
        /// <summary>
        /// Represents the method that will handle the MouseClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleMouseClick(object sender, MouseEventArgs e);
        /// <summary>
        /// Represents the method that will handle the MouseDoubleClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleMouseDoubleClick(object sender, MouseEventArgs e);
        /// <summary>
        /// Represents the method that will handle the MouseWheel event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleMouseWheel(object sender, MouseEventArgs e);

        /// <summary>
        /// Represents the method that will handle the KeyDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A KeyEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleKeyDown(object sender, KeyEventArgs e);
        /// <summary>
        /// Represents the method that will handle the KeyPress event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A KeyEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleKeyPress(object sender, KeyPressEventArgs e);
        /// <summary>
        /// Represents the method that will handle the KeyUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">A KeyEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        bool HandleKeyUp(object sender, KeyEventArgs e);
    }

    /// <summary>
    /// ILayerWithTiming extends <see cref="Pyxis.UI.ILayer"/> to provide frame timing reports
    /// </summary>
    public interface ILayerWithTiming : ILayer
    {
        /// <summary>
        /// Get a rendering time report for the last frame rendered.
        /// </summary>
        /// <returns>The report.</returns>
        RenderingTimeReport GetLastFrameRenderingTimeReport();
    }
}
