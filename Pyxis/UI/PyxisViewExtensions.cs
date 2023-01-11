using System;
using System.Linq;
using Pyxis.Core;
using Pyxis.UI.Layers;

namespace Pyxis.UI
{
    /// <summary>
    /// Extension methods for Pyxis.UI.PyxisView.
    /// </summary>
    public static class PyxisViewExtensions
    {
        /// <summary>
        /// Get a Pyxis.UI.ILayer from the Pyxis.UI.PyxisView.
        /// </summary>
        /// <typeparam name="T">The specific type of Pyxis.UI.ILayer.</typeparam>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to find the Pyxis.UI.ILayer in.</param>
        /// <returns>The first instance of <typeparamref name="T"/> in the Pyxis.UI.PyxisView if one exists; otherwise, null.</returns>
        public static T GetLayer<T>(this PyxisView pyxisView) where T : class, ILayer
        {
            return pyxisView.GetLayers().FirstOrDefault(x => x is T) as T;
        }

        /// <summary>
        /// Get a Pyxis.UI.Layers.GlobeLayer from the Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to find the Pyxis.UI.Layers.GlobeLayer in.</param>
        /// <returns>The first instance of Pyxis.UI.Layers.GlobeLayer in the Pyxis.UI.PyxisView if one exists; otherwise, null.</returns>
        public static GlobeLayer GetGlobeLayer(this PyxisView pyxisView)
        {
            return pyxisView.GetLayer<GlobeLayer>();
        }

        /// <summary>
        /// Get a Pyxis.UI.Layers.NavigationControlsLayer from the Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to find the Pyxis.UI.Layers.NavigationControlsLayer in.</param>
        /// <returns>The first instance of Pyxis.UI.Layers.NavigationControlsLayer in the Pyxis.UI.PyxisView if one exists; otherwise, null.</returns>
        public static NavigationControlsLayer GetNavigationLayer(this PyxisView pyxisView)
        {
            return pyxisView.GetLayer<NavigationControlsLayer>();
        }

        /// <summary>
        /// Get a Pyxis.UI.Layers.SelectionLayer from the Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to find the Pyxis.UI.Layers.SelectionLayer in.</param>
        /// <returns>The first instance of Pyxis.UI.Layers.SelectionLayer in the Pyxis.UI.PyxisView if one exists; otherwise, null.</returns>
        public static SelectionLayer GetSelectionLayer(this PyxisView pyxisView)
        {
            return pyxisView.GetLayer<SelectionLayer>();
        }

        /// <summary>
        /// Create a globe, including navigation and selection layers, in the given Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to add the layers to.</param>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <returns>The created Pyxis.UI.Layers.GlobeLayer that is added to <paramref name="pyxisView"/>.</returns>
        public static GlobeLayer CreateGlobe(this PyxisView pyxisView, Engine engine)
        {
            return CreateGlobe(pyxisView, engine, true);
        }

        /// <summary>
        /// Create a globe, including a selection layer and optionally including a navigation layer, in the given Pyxis.UI.PyxisView.
        /// </summary>
        /// <param name="pyxisView">The Pyxis.UI.PyxisView to add the layers to.</param>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="navigationControls">A value indicating if a Pyxis.UI.Layers.NavigationControlsLayer is to be added.</param>
        /// <returns>The created Pyxis.UI.Layers.GlobeLayer that is added to <paramref name="pyxisView"/>.</returns>
        public static GlobeLayer CreateGlobe(this PyxisView pyxisView, Engine engine, bool navigationControls)
        {
            if (pyxisView.GetGlobeLayer() != null)
            {
                throw new Exception("Globe layer already existing on current PyxisView");
            }

            if (pyxisView.GetLayer<BackgroundLayer>() == null)
            {
                pyxisView.AddLayer(new BackgroundLayer());
            }

            var globeLayer = new GlobeLayer(pyxisView, engine);
            pyxisView.AddLayer(globeLayer);

            if (navigationControls)
            {
                var navigationLayer = new NavigationControlsLayer(globeLayer);
                pyxisView.AddLayer(navigationLayer);
            }

            var selectionLayer = new SelectionLayer(globeLayer);
            pyxisView.AddLayer(selectionLayer);

            return globeLayer;
        }
    }
}