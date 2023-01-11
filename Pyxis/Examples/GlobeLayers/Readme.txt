GlobeLayers Example
------------------

This example would show you how to use Pyxis API.
This demo would show how to use the API in 5 steps:

1) Starting the Pyxis.Core.Engine using Pyxis.UI.PyxisEngineFactory
2) Showing your first globe using Pyxis.UI.PyxisView 
3) Using Pyxis.Publishing.Channel to find data over the GeoWeb
4) Streaming data from worldview.gallery and from OGC endpoints
5) Save the globe into an image.


Files to Look at
----------------
2) Form1.cs - How to initialize a PyxisEngine with PyxNet enabled
              Moreover, this would show you how to show and hide GeoSource

Creating the Engine
-------------------

The first thing to do if you want to use Pyxis API is to start Pyxis.Core.Engine.
An engine gets created using a Pyxis.Core.EngineConfig.

This example shows an easy way to do that when using simple WinForm application.

Drag and drop Pyxis.UI.PyxisEngineFactory component to the main form and enter
the email and API key in the settings.

Calling Pyxis.UI.PyxisEngineFactory.WhenReady(Action) or GetEngine() would initialize
the Pyxis.Core.Engine with the right settings for you.

Creating a Globe
----------------

It starts to be fun once you see the globe. Pyxis.UI.PyxisView allow you get the power of our 
C++ 3D OpenGL renderer of any geospatial data imported using the engine.

However, PyxisView is aimed to empower developers to customize how the globe would look like and add 
their own touch and features on top of the Pyxis globe.

PyxisView contains a list of Layers that render one on top the other. usually you would have 4 layers:
1) Pyxis.UI.Layers.BackgroundLayer - make sure you get the right background color you want.
2) Pyxis.UI.Layers.GlobeLayer - render a globe with elevation, coverages, polygons and icons.
3) Pyxis.UI.Layers.NavigationsControlsLayer - render navigation controls
4) Pyxis.UI.Layers.SelectionLayer - allow users to select an area on the globe

Use Pyxis.UI.PyxisViewExtensions.CreateGlobe helper function to create those 4 layers in one line.

You don't have to create those layers and you can add custom layers to create the user experience 
you need. Please note you can use any WinForms control on top of the PyxisView. 

Here is some information to decide between the 2 options:
1) Use WinForms controls on top PyxisView - use this option when you want to create simple pop-up
   and controls that do not require complex rendering. This is the easiest option to add controls
   on top of the globe.

2) Use CustomLayers - use this option when you want to use OpenGL capabilities on top of the globe.
   This is useful when you want to draw semitransparent elements on the globe or take advantage
   of 3d rendering. Adding a layer would also integrate with SaveToBitmap() function of PyxisView
   which can be used for printing. Custom layers can also allow you to change the logic of
   Mouse and Keyboard events but this topic is out of the scope of this demo.

This demo would show you how to use WinForms button on top PyxisView.

Using WorldView.Gallery
-----------------------

The Pyxis.Core.Engine starts to shine when many datasets are integrated together.

WorldView.Gallery allows users to easily find data from multiple publishers. 
Use Pyxis.Engine.GetChannel() to have access to all the GeoSources and Maps over the WorldView.Gallery.

Pyxis.Engine.GetChannel().GeoSources.GetById(Guid) - allows you to get a specific GeoSource
Pyxis.Engine.GetChannel().GeoSources.Search("Elevation") - allows you to search for GeoSources 
using a query string

Those GeoSource objects can be used with Pyxis.Egnine.GetAsCoverage/GetAsFeatures and with
Pyxis.UI.PyxisView.ViewState.Show.

Pyxis.Core.Engine uses our network protocols to allow secure streaming over the network.
This feature can be enabled by setting Pyxis.Core.EngineConfig.UsePyxNet=true.

SaveToBitmap
------------

PyxisView.SaveToBitmap allows you to save the current openGL state into a Bitmap object.
The Bitmap object can be used for printing or overlay it in different controls.

Please note that PyxisView.Layers.GlobeLayer streams data over the network. While the
data is been streamed over the network GlobeLayer would display the best available data
that has already been loaded. PyxisView.SaveToBitmap can be used at any moment. However,
GlobeLayer.WhenLoadingCompleted(Action) can be used if you want to wait until all data 
on screen has been streamed.